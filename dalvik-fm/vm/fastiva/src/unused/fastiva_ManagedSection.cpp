#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
#include <kernel/Runnable.h>
#include <fox/Task.h>
#include <fox/Sys.h>
#include <jni.h>
#include <fastiva_main.h>

/**
 * �ڵ� ���� ������ �������� �����Ѵ�.
 *    1) ManagedSection : Fastiva �ڵ� ����
 *    2) UnmanagedSection : Native C/C++ �ڵ� ���� (JNI ����)
 *    3) JavaSection : Java interpreter �ڵ� ����
 *
 * Exception Dispatch �� ���е� Section �� ���߾� ó���ȴ�.
 * 1. call [Fastiva Managed] from [Unmanaged Native C/C++(JNI)]
 *    Fastiva ���� ���� ������ TRY$�� ��� ��� unhandled-excpetion �� catch �Ѵ�.
 *    Fastiva ���� ������ �߻��� Exception�� ���ؼ� Fastiva ���� ������ ���
 *    ó������ ���� ���, ������ �����Ͽ� �����Ѵ�.
 *    ����) JNI ȣ���� ���, catched exception �� task state�� ���� ��, ���� return.
 *
 * 2. call [Unmanaged Native C/C++(JNI)] from [Fastiva Managed]
 *    ManagedSection �� close �ϰ�, Exception �߻��� Application �� �����Ѵ�.
 *  
 * 3. call [Fastiva Managed] from [Java]
 *    ManagedContext ������ JNIEnv* �� �����ϰ�, performance ����� ���� try-catch ���� ������� �ʴ´�.
 *    ExeptionHandler ���� ManagedSection�� ���� �ľ��ϰ�, JniThrowException() ȣ���Ѵ�.
 *
 * 4. call [Java] from [Fastiva Managed]
 *    ManagedSection �� close �ϰ�, Exception �߻��� Application �� �����Ѵ�.
 *    �߻��� Java-Exception �� C++ ���·� �����Ͽ� rethow �Ѵ�.
 */

#if 0 && !FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER

extern void	fastiva_component_initModuleInfo();
extern fastiva_ModuleInfo fastiva_component_ModuleInfo;

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;

    if (vm->GetEnv((void **) &fastiva_jniEnv, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }

	fastiva_component_initModuleInfo();
	fastiva_init(&fastiva_component_ModuleInfo);

    return JNI_VERSION_1_4;
}

#endif


FOX_BOOL fastiva_task_inManagedSection(fox_Task* pTask) {
	fastiva_StackContext* pStackContext;
	if (pTask == NULL) {
		fox_printf("Task not attched\n");
		return NULL;
	}
	pStackContext = ((fastiva_Task*)pTask)->m_pStackContext;
	return pStackContext != ADDR_ZERO;
}

static int g_idFrame = 2;

void fox_GC_doLocalGC(); //_impl(fastiva_Task* pCurrTask, bool isLocal);

void FOX_FASTCALL(fox_task_notifyCurrentTaskBlocked)(fox_Task* pTask0) {
}

void fastiva_Runtime::enterNativeSection_$$(
) {
#if 0
	_asm {
		mov [ecx+0], ebx
		mov [ecx+4], ebp;
		mov [ecx+8], esi;
		mov [ecx+12], edi;
	}
#endif
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	if (pCurrTask == ADDR_ZERO) {
		return;
	}
	fastiva_StackContext* pStackContext = pCurrTask->m_pStackContext;
	KASSERT(pStackContext != ADDR_ZERO);
	fox_jmp_buf buf;
	if (!_setjmp(buf)) { // <- ���� register ��������� setjmp �� ȣ��.
		pStackContext->m_pStack = &buf;
	}
	else {
		// Ȥ�� ���� �Ϻη� ���� stub code;
		fox_printf("somthing wrong with _setjmp%x", pCurrTask);
	}
}


void fastiva_Runtime::leaveNativeSection_$$(
) {
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	if (pCurrTask == ADDR_ZERO) {
		return;
	}
	fastiva_StackContext* pStackContext = pCurrTask->m_pStackContext;
	KASSERT(pStackContext != ADDR_ZERO);
	bool maybeInScanning = true; // pCurrTask->m_gcInterrupted;
	if (maybeInScanning) {
		pCurrTask->enterCriticalSection();
	}
	else {
		/* ���� ������ task-switching �� �߻��Ͽ� 
		   maybeInScanning = 0, pCurrTask->m_gcInterrupted = 2�� ������ �� �ִ�.
		   �� ��Ȳ������ stack-scan �� ����Ǵµ�, �� ���¿��� stack�� �����ϸ� �� �ȴ�.
		*/
		KASSERT("should not be here!!" == 0);
	}

	if (pCurrTask->m_gcInterrupted && fox_util_cmpxchg32(&pCurrTask->m_gcInterrupted, 2, 1)) {
#ifdef _DEBUG_TRACE
		fox_printf("pseudo gc interrrupt after non-managed call %x", pCurrTask);
#endif
		// doLocalGC_impl()�� ȣ���� �� ����. r4~r7 register �� ��ü ref�� ����Ǿ� ���� �� �ִ�.
		fox_GC_doLocalGC(); 
	}

	pStackContext->m_pStack = ADDR_ZERO;
	if (maybeInScanning) {
		pCurrTask->leaveCriticalSection();
	}
}

void fastiva_Runtime::beginManagedSection_$$(struct fastiva_ManagedSection* pCtx0) {
	fastiva_Task* pCurrTask = (fastiva_Task*)fox_task_currentTask();
	fastiva_StackContext* pFrame = (fastiva_StackContext*)pCtx0;
	pFrame->m_pStack = ADDR_ZERO;

	if (pCurrTask == ADDR_ZERO) {
		pCurrTask = (fastiva_Task*)fox_task_attachCurrentNativeTask();
		KASSERT(pCurrTask != ADDR_ZERO);
	}


	// @todo 2011.1201. Full-compile ���ϱ� ���� �ҽ� �Ϻθ� ����. ���߿� m_pEnv ������ ��.
	pCurrTask->m_pJNIEnv0 = pFrame->m_pJNIEnv;
	bool maybeInScanning = true; // pCurrTask->m_gcInterrupted;
	if (maybeInScanning) {
		pCurrTask->enterCriticalSection();
	}
	else {
		/* ���� ������ task-switching �� �߻��Ͽ� 
		   maybeInScanning = 0, pCurrTask->m_gcInterrupted = 2�� ������ �� �ִ�.
		   �� ��Ȳ������ stack-scan �� ����Ǵµ�, �� ���¿��� stack�� �����ϸ� �� �ȴ�.
		*/
		KASSERT("should not be here!!" == 0);
	}

	if (pCurrTask->m_pStackContext == ADDR_ZERO) {
		pFrame->m_pPrev = ADDR_ZERO;
		pFrame->m_depth = 0;
		pCurrTask->m_pStackContext = pFrame;
		if (kernelData.g_pMainThread != ADDR_ZERO) {
			// �ʱ�ȭ�� ���� ���̴�.
			// class-import�� �߻����� �ʵ��� �Ѵ�.
			fm::attachNativeTask(pCurrTask);
		}
	}
	else {
		if (!pCurrTask->m_pStackContext->isClosed()) {
			fox_debug_printf("ManagedStack corrupted");
		}
		KASSERT(pCurrTask->m_pStackContext->isClosed());
		pFrame->m_pPrev = pCurrTask->m_pStackContext;
		pFrame->m_depth = pCurrTask->m_pStackContext->m_depth + 1;
		if (pCurrTask->m_gcInterrupted && fox_util_cmpxchg32(&pCurrTask->m_gcInterrupted, 2, 1)) {
#ifdef _DEBUG_TRACE
			fox_printf("pseudo gc interrrupt before entering managed section %x", pCurrTask);
#endif
			fox_GC_doLocalGC(); //_impl(pCurrTask, false);
		}
	}

	pCurrTask->m_pStackContext = pFrame;
	if (maybeInScanning) {
		pCurrTask->leaveCriticalSection();
	}
}

void fastiva_Runtime::endManagedSection_$$(struct fastiva_ManagedSection* pCtx0) {
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	fastiva_StackContext* pFrame = (fastiva_StackContext*)pCtx0;
	KASSERT(pCurrTask->m_pStackContext == pFrame);
	pCurrTask->m_pStackContext = (fastiva_StackContext*)pFrame->m_pPrev;
}


