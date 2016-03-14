#include <Dalvik.h>
#include <dalvik_Kernel.h>
#include <java/io/IOException.inl>
#include <java/lang/ClassNotFoundException.inl>
#include <java/lang/ArithmeticException.inl>
#include <java/lang/InterruptedException.inl>
#include <java/lang/IllegalAccessException.inl>
#include <java/lang/ArrayStoreException.inl>
#include <java/lang/ClassCastException.inl>
#include <java/lang/InterruptedException.inl>
#include <java/lang/InternalError.inl>
#include <java/lang/ClassNotFoundException.inl>
#include <java/lang/OutOfMemoryError.inl>
#include <java/lang/StackOverflowError.inl>
#include <java/io/IOException.inl>
#include <java/lang/VirtualMachineError.h>
#include <java/lang/NullPointerException.inl> 
#include <java/lang/UnsatisfiedLinkError.inl> 
#include <java/lang/IllegalArgumentException.inl> 
#include <java/lang/StringIndexOutOfBoundsException.inl> 
#include <java/lang/ArrayIndexOutOfBoundsException.inl> 
#include <java/lang/IllegalMonitorStateException.inl> 
#include <java/lang/String.inl> 

#include <java/util/NoSuchElementException.inl>
#include <java/io/InterruptedIOException.inl>
#include <java/lang/NoClassDefFoundError.inl>

#include <fcntl.h>

#include <string.h>
#ifdef _WIN32
    //#include <fastiva_malloc.h>
#else
    #include <alloca.h>
	#include <pthread.h>
#endif
#ifdef _ARM_
	#include <setjmp.h>
#endif
typedef int REG_BUF[7];
FOX_BOOL g_enableDebugTrace = false;
FOX_BOOL g_enableExceptionTrace = false;

extern void fastiva_debug_out(java_lang_String_p pStr);

#if ANDROID
#define fastiva_Task Thread
#define fastiva_getCurrentTask dvmThreadSelf
#else
#define fastiva_getCurrentTask()  ((fastiva_Task*)fox_task_currentTask())
#endif

#if 1 //def _DEBUG
void fastiva_init_debug();
extern fastiva_Instance_p DEBUG_OBJ;
extern fastiva_Task* DEBUG_TASK;
volatile int debug_mode = -1; 
//volatile int dddd = -1; 
volatile int dddd_thread_id = -1;
volatile int sleep_on_break = -1; 
volatile int dddd_break = -1; 
volatile int debugSuspendAll = 1;
volatile int disable_debug_break = 0;
char debug_ex_type[1024];
char debug_class_not_found_type[1024];
bool g_fastiva_enable_call_log = false;
bool g_fastiva_enable_jni_call_log = false;
int g_showJniTrace = 0;
#endif


bool fastiva_dvmRequestSuspendAllThread();


void fastiva_debug_break(const char* msg, bool forceSleep) {
	dddd_break ++;
	if (!forceSleep) {
		ALOGE("##### debug_message %i %s", dvmGetSysThreadId(), msg);
		//dvmDumpThread(dvmThreadSelf(), false);
		return;
	}

	if (disable_debug_break) {
		return;
	}

	sleep_on_break |= forceSleep;

	ALOGE("ZZZZZZZZZZZZZ debug_break %i %s", dvmGetSysThreadId(), msg);
	bool needResume = false;
	if (debugSuspendAll) {
		needResume = fastiva_dvmRequestSuspendAllThread();
	}
    dvmLockMutex(&gDvm.threadSuspendCountLock);

	dvmDumpThread(dvmThreadSelf(), false);
	while (sleep_on_break != 0) {
		g_showJniTrace = true;
		ALOGE("ZZZZZZ--ZZZZZZZ debug_break %i %s", dvmGetSysThreadId(), msg);
		usleep(1000 * 1000 * 1);
		fastiva_init_debug();
		if (debug_mode == 0) {
			break;
		}
	}
    dvmUnlockMutex(&gDvm.threadSuspendCountLock);

	if (needResume) {
		dvmResumeAllThreads(SUSPEND_FOR_DEBUG);
	}
}

/*
extern "C" int fastiva_fastiva_debug_break(int forceSleep) {
	fastiva_debug_break(forceSleep != 0);
	return forceSleep;
}
*/
void d2f_getBytes(char* buf, int max_len, java_lang_String_p str) {
	max_len --;
	int len = str->length_();
	if (len > max_len) {
		len = max_len;
	}
	for (int i = 0; i < len; i ++) {
		buf[i] = (char)str->charAt_(i);
	}
	buf[len] = 0;
}

void check_break_for_exception(Thread* self, java_lang_Throwable_p ex) {
	if (ex->getClass$() == java_lang_ClassNotFoundException_C$::getRawStatic$()) {
		if (debug_class_not_found_type[0] != 'L' || 0 != strcmp(ex->clazz->descriptor, debug_class_not_found_type)) {
		return;
	}
	}

	int dd = self->m_exceptionCount ++;

    ALOGE("dddd: %d-%d %s", self->threadId, dd, ex->clazz->descriptor);
	if (self->exception != NULL) {
		//return;
	}

	//assert(0 != strcmp(ex->clazz->descriptor, "Ljava/lang/ClassCastException;"));
	if (0) {
	java_lang_String_p str = ex->getMessage_();
	if (str != NULL) {
		char buf[1204];
		d2f_getBytes(buf, 1204, str);
		ALOGE("%s", buf);
		//assert(0 != strcmp(buf, "mmap failed: ENODEV (No such device)"));
	}
	}

	fastiva_init_debug();
	if (debug_mode <= 0) {
		return;
	}

	static volatile int sleep_mode = true;

	if (debug_ex_type[0] == 'L' && !strcmp(ex->clazz->descriptor, debug_ex_type)) {
	    fastiva_debug_break("exception break", sleep_mode);
	}
	else if (((int)self->threadId == dddd_thread_id) && dd == dddd_break) {
        fastiva_debug_break("exception break", sleep_mode);
    }
	else if (ex->clazz == java_lang_UnsatisfiedLinkError_C$::getRawStatic$()) {
        fastiva_debug_break("java_lang_ClassCastException", sleep_mode);
	}
}

void fastiva_popDalvikException() {
	Thread* thread = dvmThreadSelf();
	fastiva_popDalvikException(thread);
}

jlonglong fastiva_popDalvikException_Ex(jlonglong v) {
	Thread* thread = dvmThreadSelf();
	fastiva_popDalvikException(thread);
	return v;
}


void fastiva_popDalvikException(Thread* thread) {
	Object* ex = thread->exception;
	if (ex != NULL) {
		dvmClearException(thread);
		THROW_EX$((java_lang_Throwable_p)ex);
	}
}

#ifndef FASTIVA_USE_CPP_EXCEPTION
static void fastiva_rewind(fastiva_Task* pCurrTask, void* sp) {
	fastiva_Rewinder* pMonitor = pCurrTask->m_pTopRewinder;
	if (sp == 0) {
		sp = (void*)-1;
	}
	while (pMonitor != NULL && (void*)pMonitor < sp) {
		//Exception Handler ï¿½ï¿½ï¿½Ä¿ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿?Synchronizeï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½Ñ´ï¿½.
		assert((void *)pMonitor != (void *)-1);
		pMonitor->~fastiva_Rewinder();
		FASTIVA_ASSERT(pMonitor < pMonitor->m_pPrev || pMonitor->m_pPrev == NULL);
		// It is possible that pMonitor > (void*)newTopHandlder
		// see pushExceptionContext
		// FASTIVA_ASSERT(pMonitor < (void*)newTopHandlder);
		pMonitor = pMonitor->m_pPrev;
	}
	// ÃÖÁ¾ rewinder ´Â newTopHandlder º¸´Ù Å« ÁÖ¼Ò°ªÀ» °¡Áú ¼ö ÀÖ´Ù.
	// ÇØ´ç rewinder °¡ ÇØ´ç ÇÚµé·¯º¸´Ù ¸ÕÀú pushµÈ °æ¿ìÀÌ´Ù.
	pCurrTask->m_pTopRewinder = pMonitor;
}
#endif

void fastiva_Runtime::throwException(java_lang_Throwable_p pThrowable) {

	fastiva_Task* pCurrTask = fastiva_getCurrentTask();


#ifdef _DEBUG 
	check_break_for_exception(pCurrTask, pThrowable);
#endif

#ifdef FASTIVA_USE_CPP_EXCEPTION
	// to protect GC.
	pCurrTask->m_pTopHandler = (fastiva_ExceptionContext*)(void*)pThrowable;
	throw pThrowable;
#else
    FASTIVA_ASSERT(pCurrTask->status == THREAD_RUNNING);
	dvmSetException(pCurrTask, pThrowable);
	dvmCheckSuspendPending(pCurrTask);

	// all the fastiva_Thread_T$ start has a exception_context;
	fastiva_ExceptionContext* pTrap = (fastiva_ExceptionContext*)pCurrTask->m_pTopHandler;
	InterpSaveState* interpState = &pCurrTask->interpSave;
	while (true) {
        FASTIVA_ASSERT(pTrap != NULL);
		while (interpState != NULL && (uint)pTrap - 1 > (uint)interpState->bailPtr - 1) {

			fastiva_rewind(pCurrTask, interpState->bailPtr);
			fastiva_dvmCatchFastivaException(pCurrTask);
		}
        if (pTrap->curr_try_id >= 0) {
            break;
        }
		pCurrTask->m_pTopHandler = pTrap = pTrap->m_pParent;
	}

	dvmClearException(pCurrTask);

#if 0 // interpreter ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½???
	// top stack frame ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½.
	fastiva_StackFrame* pTopFrame = pCurrTask->m_pTopStackFrame;
	fastiva_StackFrame* pTrapFrame = pTrap->m_pStackFrame;
	if (pTopFrame != pTrapFrame) {
		fastiva_StackFrame* pFrame = pTopFrame;
		void**pFrameSlot = (void**)pTopFrame - 1;
		while (pFrame != pTrapFrame) {
			*pFrameSlot ++ = (void*)pFrame->m_pMethod;
			pFrame = pFrame->m_pPrevFrame;
		}
		*pFrameSlot = 0;

#if !defined _ARM_ && !defined ANDROID
		for (pFrameSlot =  (void**)pTopFrame - 1; *pFrameSlot != 0; pFrameSlot ++) {
			fm::dumpStackTrace((fastiva_Method*)*pFrameSlot);
		}
#endif

		pCurrTask->m_pTopStackFrame = pTrapFrame;
	}

	// FASTIVA_TRY_EX$ï¿½ï¿½ ï¿½Þ¸ï¿½ TRY$ï¿½ï¿½ CATCHï¿½ï¿½ ï¿½ï¿½ï¿½Ã¿ï¿½ 
	// ExceptionContext-Handlerï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½È´ï¿½. (pTrapï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½Ê´Â´ï¿½.)
	if (pTrap->curr_try_id >= 0xFFFF) {
		pCurrTask->m_pTopHandler = pTrap->m_pParent;
	}
#endif

	assert(pTrap != ADDR_ZERO);

	fastiva_rewind(pCurrTask, pTrap->m_pTopRewinder);
	assert(pCurrTask->m_pTopRewinder == pTrap->m_pTopRewinder);

#if !defined(_WIN32) || defined(_ARM_)
	// ï¿½ï¿½ï¿½ï¿½) longjmp ï¿½ï¿½ _longjmp ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½Ù¸ï¿½ï¿½ï¿½.
	FASTIVA_LONGJMP(pTrap->m_buf, (int)pThrowable);
	//FASTIVA_DBREAK();

#elif 1//def _DEBUG
	int buf_addr = (int)pTrap->m_buf;
	int ret_res  = (int)pThrowable;
	__asm {
		mov edx, buf_addr;//dword ptr [esp + 4];
		mov eax, ret_res;
		mov ebp, dword ptr [edx + 0];
		mov ebx, dword ptr [edx + 4];
		mov edi, dword ptr [edx + 8];
		mov esi, dword ptr [edx + 12];
		mov esp, dword ptr [edx + 16]; 
		cmp eax, 1; // maybe for setting state flag
		adc eax, 0;
		//mov esp, dword ptr [edx + 0];
		add esp, 4;
		jmp dword ptr[edx + 20];
	};
	//longjmp(pTrap->m_buf, (int)pThrowable);
#else
	int buf_addr = (int)pTrap->m_buf;
	int ret_res  = (int)pThrowable;
#ifdef __GNUC__
	__asm__ __volatile__ (
        "movl	%0, %%edx \n\t"
        "movl	4(%%edx), %%ebx \n\t"
        "movl	8(%%edx), %%edi \n\t"
        "movl	12(%%edx), %%esi \n\t"
        "movl	%1, %%eax \n\t"
        "movl	16(%%edx), %%ebp \n\t"
        "cmpl	$1, %%eax \n\t"
        "adc	$0, %%eax \n\t"
        "movl	0(%%edx), %%esp \n\t"
        "add	$4, %%esp \n\t"
        "jmpl	*20(%%edx) \n\t"
        :
        : "m"(buf_addr), "m"(ret_res)
        );
#else
	__asm {
		mov edx, buf_addr;//dword ptr [esp + 4];
		mov ebx, dword ptr [edx + 4];
		mov edi, dword ptr [edx + 8];
		mov esi, dword ptr [edx + 12];
		mov eax, ret_res;//dword ptr [esp + 8]; // res
		mov ebp, dword ptr [edx + 16];
		cmp eax, 1; // maybe for setting state flag
		adc eax, 0;
		mov esp, dword ptr [edx + 0];
		add esp, 4;
		jmp dword ptr[edx + 20];
	};
#endif
#endif
#endif // FASTIVA_USE_CPP_EXCEPTION
} 


void fastiva_throwStackOverflowError() {
#if !FASTIVA_SCAN_INTO_JNI_STACK
	// halt gc-scanning.
	dvmChangeStatus(NULL, THREAD_RUNNING);
#endif
#if 0 // def _DEBUG
		ALOGE("##### fastiva_throwStackOverflowError %i", dvmGetSysThreadId());
		dvmDumpThread(dvmThreadSelf(), false);
#endif
	volatile int no_throw = 0;
	if (!no_throw) {
		THROW_EX$(FASTIVA_NEW(java_lang_StackOverflowError)());
	}
}

void fastiva_throwNullPointerException() {
#if !FASTIVA_SCAN_INTO_JNI_STACK
	// halt gc-scanning.
	dvmChangeStatus(NULL, THREAD_RUNNING);
#endif
	THROW_EX$(FASTIVA_NEW(java_lang_NullPointerException)());
}

void fastiva_throwArithmeticException() {
#if !FASTIVA_SCAN_INTO_JNI_STACK
	// halt gc-scanning.
	dvmChangeStatus(NULL, THREAD_RUNNING);
#endif
	volatile int no_throw = 0;
	if (!no_throw) {
		THROW_EX$(FASTIVA_NEW(java_lang_ArithmeticException)());
	}
}

void fastiva_Runtime::throwArrayIndexOutOfBoundsException() {
	THROW_EX$(FASTIVA_NEW(java_lang_ArrayIndexOutOfBoundsException)());
}

extern "C" void fastiva_Runtime::throwClassCastException(fastiva_Instance_p pObj, java_lang_Class_p pClass) {
	THROW_EX$(FASTIVA_NEW(java_lang_ClassCastException)());
}

#ifndef FASTIVA_USE_CPP_EXCEPTION
int	fastiva_Runtime::pushExceptionHandler(fastiva_ExceptionContext* pNewTrap) {

	int reserved_for_stack_overflow_sig_handler[32];
	reserved_for_stack_overflow_sig_handler[0] = 0;

	fastiva_Task* pCurrTask = fastiva_getCurrentTask();

	fastiva_ExceptionContext* pTrap = (fastiva_ExceptionContext*)pCurrTask->m_pTopHandler;
	assert(pTrap == ADDR_ZERO || (int)pTrap > (int)pNewTrap);
	pNewTrap->m_pParent = pTrap;
	pNewTrap->m_pTopRewinder = pCurrTask->m_pTopRewinder;
	// Cf) It is possible pCurrTask->m_pTopRewinder > pNewTrap
	//     In single function the stack variant order is unpredictable.
	//assert((int)pNewTrap->m_pTopRewinder > (int)pCurrTask->m_pTopHandler);
	assert((u4)pNewTrap < (u4)pCurrTask->m_pTopHandler - 1);
	pCurrTask->m_pTopHandler = pNewTrap;
	return 0;
}




void fastiva_Runtime::removeExceptionHandler(fastiva_ExceptionContext* pTrap) {
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	fastiva_ExceptionContext* pTop = (fastiva_ExceptionContext*)pCurrTask->m_pTopHandler;
	assert((pTop == pTrap));
	pCurrTask->m_pTopHandler = pTop->m_pParent;
#if 0 //def _DEBUG
	if (pCurrTask->m_pTopHandler == 0) {
		int a = 3;
	}
	if ((pTop != pTrap)) {
		fastiva_ExceptionContext* pParent = pTrap->m_pParent;
		if (pParent != pTop) {
			while (pParent != ADDR_ZERO) {
				if (pParent == pTop) {
					break;
				}
				pParent = pParent->m_pParent;
			}
		}
	}
#endif
}

void fastiva_Runtime::rethrow(fastiva_ExceptionContext* pTrap, void* pThrowable) {
	//fm::removeExceptionHandler(pTrap);
	pTrap->curr_try_id = -1;
	THROW_EX$((java_lang_Throwable_p)pThrowable);
}

void fastiva_Runtime::pushRewinder(
	fastiva_Task* pCurrTask,
	fastiva_Rewinder* pRewinder
) {
	FASTIVA_ASSERT(pCurrTask == dvmThreadSelf());
	FASTIVA_ASSERT((void*)pRewinder < pCurrTask->m_pTopHandler);
	pRewinder->m_pTask = pCurrTask;
	pRewinder->m_pPrev = pCurrTask->m_pTopRewinder;
	pCurrTask->m_pTopRewinder = pRewinder;
}

void fastiva_Runtime::rewind(
	fastiva_Rewinder* pRewinder
) {
	fastiva_Task* pCurrTask = pRewinder->m_pTask;
	if (pCurrTask == ADDR_ZERO) {
		return;
	}
	FASTIVA_ASSERT((void*)pCurrTask->m_pTopRewinder < pCurrTask->m_pTopHandler);
	pCurrTask->m_pTopRewinder = pRewinder->m_pPrev;
}
#endif


#if 0
void fm::throwIOException(int errnum) {
	THROW_EX_NEW$(java_io_IOException, (fm::getErrorString(errnum)));
}

java_lang_String_p fm::getErrorString(int errnum)
{
    // note: glibc has a nonstandard strerror_r that returns char* rather
    // than POSIX's int.
    // char *strerror_r(int errnum, char *buf, size_t n);
#ifdef _WIN32
    wchar_t* ret = _wcserror(errnum);
	return fastiva.createStringW(ret, wcslen(ret));
#else
    char* ret = strerror(errnum);
	return fastiva.createStringA(ret, strlen(ret));
#endif

}

void fox_debug_trace(const char* func, const char* file, int line) {
	fox_printf("%s (%d:%d)\n", func, file, line);
}
static int s_traceDepth = 0;
static int s_cntTrace = 0;
FOX_DEBUG_TRACE_FRAME::FOX_DEBUG_TRACE_FRAME(const char* func, const char* file, int line) {
	this->pTask = fastiva_getCurrentTask();
	pTask->enterDebugStackTrace(func);
	if (g_enableDebugTrace) {
		//fox_printf("%d:(%x:%x) %s %s:%d\n", pTask->m_invokeJNI_retAddr ++, pTask, this, func, file, line);
		fox_printf("%d:[%d] %s\n", ++s_cntTrace, pTask->m_invokeJNI_retAddr ++, func);
	}
}

FOX_DEBUG_TRACE_FRAME::~FOX_DEBUG_TRACE_FRAME() {
	pTask->leaveDebugStackTrace();
	if (g_enableDebugTrace) {
		//fox_printf("%d:(%x:%x) :%d\n", --pTask->m_invokeJNI_retAddr, pTask, this, ++s_cntTrace);
	}
};




#endif