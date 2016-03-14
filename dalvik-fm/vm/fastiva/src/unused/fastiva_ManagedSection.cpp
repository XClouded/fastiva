#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
#include <kernel/Runnable.h>
#include <fox/Task.h>
#include <fox/Sys.h>
#include <jni.h>
#include <fastiva_main.h>

/**
 * 코드 실행 영역을 세가지로 구분한다.
 *    1) ManagedSection : Fastiva 코드 영역
 *    2) UnmanagedSection : Native C/C++ 코드 영역 (JNI 포함)
 *    3) JavaSection : Java interpreter 코드 영역
 *
 * Exception Dispatch 는 구분된 Section 에 맞추어 처리된다.
 * 1. call [Fastiva Managed] from [Unmanaged Native C/C++(JNI)]
 *    Fastiva 영역 진입 직전에 TRY$를 사용 모든 unhandled-excpetion 을 catch 한다.
 *    Fastiva 영역 내에서 발생한 Exception에 대해서 Fastiva 영역 내에서 모두
 *    처리되지 못한 경우, 적절히 변경하여 전달한다.
 *    참고) JNI 호출인 경우, catched exception 을 task state로 저장 후, 정상 return.
 *
 * 2. call [Unmanaged Native C/C++(JNI)] from [Fastiva Managed]
 *    ManagedSection 을 close 하고, Exception 발생시 Application 을 종료한다.
 *  
 * 3. call [Fastiva Managed] from [Java]
 *    ManagedContext 생성시 JNIEnv* 를 설정하고, performance 향상을 위해 try-catch 문은 사용하지 않는다.
 *    ExeptionHandler 에서 ManagedSection의 끝을 파악하고, JniThrowException() 호출한다.
 *
 * 4. call [Java] from [Fastiva Managed]
 *    ManagedSection 을 close 하고, Exception 발생시 Application 을 종료한다.
 *    발생된 Java-Exception 을 C++ 형태로 변경하여 rethow 한다.
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
	if (!_setjmp(buf)) { // <- 단지 register 저장용으로 setjmp 를 호출.
		pStackContext->m_pStack = &buf;
	}
	else {
		// 혹시 몰라 일부러 넣은 stub code;
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
		/* 현재 시점에 task-switching 이 발생하여 
		   maybeInScanning = 0, pCurrTask->m_gcInterrupted = 2인 상태일 수 있다.
		   그 상황에서는 stack-scan 이 진행되는데, 현 상태에서 stack을 변경하면 안 된다.
		*/
		KASSERT("should not be here!!" == 0);
	}

	if (pCurrTask->m_gcInterrupted && fox_util_cmpxchg32(&pCurrTask->m_gcInterrupted, 2, 1)) {
#ifdef _DEBUG_TRACE
		fox_printf("pseudo gc interrrupt after non-managed call %x", pCurrTask);
#endif
		// doLocalGC_impl()을 호출할 수 없다. r4~r7 register 에 객체 ref가 저장되어 있을 수 있다.
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


	// @todo 2011.1201. Full-compile 피하기 위해 소스 일부만 수정. 나중에 m_pEnv 삭제할 것.
	pCurrTask->m_pJNIEnv0 = pFrame->m_pJNIEnv;
	bool maybeInScanning = true; // pCurrTask->m_gcInterrupted;
	if (maybeInScanning) {
		pCurrTask->enterCriticalSection();
	}
	else {
		/* 현재 시점에 task-switching 이 발생하여 
		   maybeInScanning = 0, pCurrTask->m_gcInterrupted = 2인 상태일 수 있다.
		   그 상황에서는 stack-scan 이 진행되는데, 현 상태에서 stack을 변경하면 안 된다.
		*/
		KASSERT("should not be here!!" == 0);
	}

	if (pCurrTask->m_pStackContext == ADDR_ZERO) {
		pFrame->m_pPrev = ADDR_ZERO;
		pFrame->m_depth = 0;
		pCurrTask->m_pStackContext = pFrame;
		if (kernelData.g_pMainThread != ADDR_ZERO) {
			// 초기화를 시작 중이다.
			// class-import가 발생하지 않도록 한다.
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


