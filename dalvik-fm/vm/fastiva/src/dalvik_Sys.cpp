#include <Dalvik.h>
#include <dalvik_Kernel.h>
#include <kernel/Module.h>
#include <java/lang/Throwable.inl>
#include <java/lang/NullPointerException.inl>

extern void fastiva_throwNullPointerException();
extern void fastiva_throwArithmeticException();
extern void fastiva_throwStackOverflowError();

#ifndef _WIN32
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <android/log.h>
#include <fcntl.h>


/** USE_ALTERNATE_SIG_STATCK
 * sig handler 용 별도 signal-stack을 할당한다.
 * SA_ONSTACK 을 설정하지 않을 경우, stack-overflow시 SIGSEGV 를 받을 수 없다.
 * 단, 별도 signal-stack 사용시, signal을 이용해 특정 thread의
 * stack 과 register의 snapshot을 얻는 것이 불가능할 수 있다.
 * sighandler의 3번째 인자로 sigcontext가 지원되는가를 확인하여야만 한다.
 */
#define USE_ALTERNATE_SIG_STATCK 0
#define SIG_FASTIVA_SUSPEND		SIGUSR2
#define SIG_FASTIVA_SCAN		0 // SIGRTMAX-2

#define fox_Task Thread
#define fox_task_currentTask()  dvmThreadSelf()

#if USE_ALTERNATE_SIG_STATCK
	static char sigstack[SIGSTKSZ];
#endif

#ifdef _ARM_
#include <asm/sigcontext.h>
struct ucontext {
	unsigned long	  uc_flags;
	struct ucontext  *uc_link;
	stack_t		  uc_stack;
	struct sigcontext uc_mcontext;
	sigset_t	  uc_sigmask;	/* mask last for extensibility */
};
#else
	struct ucontext;
#endif
typedef void (*SIG_HANDLER_FN)(int sig, siginfo_t *si, ucontext *uc);
typedef void (*ORG_SIG_HANDLER_FN)(int sig, siginfo_t *si, void *unused);

static SIG_HANDLER_FN org_segv_handler = NULL;
static SIG_HANDLER_FN org_ill_handler = NULL;
static SIG_HANDLER_FN org_bus_handler = NULL;
static SIG_HANDLER_FN org_abrt_handler = NULL;
static SIG_HANDLER_FN org_hup_handler = NULL;
static SIG_HANDLER_FN org_quit_handler = NULL;
static SIG_HANDLER_FN org_term_handler = NULL;
static SIG_HANDLER_FN org_usr2_handler = NULL;
static SIG_HANDLER_FN org_fpe_handler = NULL;
static JNIEnv* s_env;

void fox_pal_sys_initSignalHandler();


static int s_initialized = 0;

#ifndef _DEBUG
void fastiva_init_debug() {}
static const int debug_mode = 0; 
#else
static void test();

static char* read_int(char* buf, int* res) {

	for (;(u8)*buf <  '0'; buf++);
	int v = 0;
	for (;(u8)*buf >= '0'; buf ++) {
		v *= 10;
		v += (u8)*buf - '0';
	}
	*res = v;
	return buf;
}

extern volatile int dddd_thread_id;
extern volatile int sleep_on_break; 
//extern volatile int dddd;
extern volatile int debug_mode; 
extern volatile int dddd_break;
extern volatile int debugSuspendAll;
extern char debug_ex_type[1024];
extern char debug_class_not_found_type[1024];



void fastiva_init_debug() 
{
	int fd = open("/data/dalvik-cache/fastiva.dbg", O_RDONLY);
	char lineBuf[1024];
	if (fd < 0) {
		fd = open("/sdcard/fastiva/fastiva.dbg", O_RDONLY);
	}

	if (fd >= 0) {
		int cc = read(fd, lineBuf, sizeof(lineBuf)-1);
		close(fd);

		debug_mode = lineBuf[0] - '0';
		char* buf = &lineBuf[1];
		buf = read_int(buf, (int*)&dddd_thread_id);
		buf = read_int(buf, (int*)&dddd_break);
		if (dddd_thread_id < 0) {
			dddd_thread_id = 0;
		}
		buf = read_int(buf, (int*)&debugSuspendAll);
		//buf = read_int(buf, (int*)&dddd);

		char* ex_type = debug_ex_type;
		for (;(u8)*buf > ' '; buf++);
		for (;(u8)*buf <= ' '; buf++);
		while (*buf > ' ') {
			*ex_type++ = *buf++;
		}
		*ex_type = 0;

		ex_type = debug_class_not_found_type;
		for (;(u8)*buf > ' '; buf++);
		for (;(u8)*buf <= ' '; buf++);
		while (*buf > ' ') {
			*ex_type++ = *buf++;
		}
		*ex_type = 0;

		//ALOGE("dddd_break thread: %d at : %d sleep:%d", dddd_thread_id, dddd_break, sleep_on_break);
	}
	else {
		// debug_mode = 0;
		//ALOGE("no dddd_break");
	}
}
#endif


bool fastiva_sysInit() {
	void fox_pal_monitor_init();
	void fox_pal_semaphore_init();
	FOX_BOOL fox_pal_task_init();

	if (s_initialized) {
		return true;
	}

	s_initialized = true;


	//fox_pal_semaphore_init();
	//fox_pal_monitor_init();
	//if (!fox_pal_task_init()) {
	//	return false;
	//}
	fox_pal_sys_initSignalHandler();
#ifdef _DEBUG
	test();
#endif
	return true;
}

static void fatal_exit() {
	ALOGW("Fatal Error !!!");
	fox_printf("%s", (char*)strerror(errno));
	_exit(-1);
}


void fm__stopAppl() {
	ALOGW("appl stop!!");
	_exit(-1);
}

#if 0 // def _DEBUG
static void default_handler(int sig, siginfo_t *si, void *unused) {
	/**
	 * 현재의 signal 은 masking되어 있으므로 Loop 상태가 되는 것이 아니라,
	 * Default-Handler에게 전달된다.
	 */
	for (int i = 1; i < 20; i ++) {
	ALOGW("Fastiva execution aborted");
		usleep(1000*1000);
		FASTIVA_DBREAK();
	}
}
#endif

void fastiva_throwNullPointerException_unmanagedSection() {
	fox_printf("seg fault out of managed section\n");
	fox_exit(-1);
}

FOX_BOOL fastiva_task_inManagedSection(fox_Task* pTask);


#if USE_ALTERNATE_SIG_STATCK || (defined(FASTIVA_USE_CPP_EXCEPTION) && defined(_ARM_))
#ifdef __thumb__
	#define RETURN_TO_EXCEPTION_HANDLER(uc, handler)					\
			uc->uc_mcontext.arm_lr = (uc->uc_mcontext.arm_pc + 2) | 1; \
			uc->uc_mcontext.arm_pc = (long unsigned int)handler;
#else
	#define RETURN_TO_EXCEPTION_HANDLER(uc, handler)					\
			uc->uc_mcontext.arm_lr = uc->uc_mcontext.arm_pc + 4; \
			uc->uc_mcontext.arm_pc = (long unsigned int)handler;
#endif
#else
	#define RETURN_TO_EXCEPTION_HANDLER(uc, handler)					\
			clear_sig_masks(); handler();
#endif


//*
static void clear_sig_masks() {
	sigset_t ss;
	sigemptyset(&ss);
	pthread_sigmask(SIG_SETMASK, &ss, NULL);
}
//*/
#if 0 
SIGSEGV vs SIGBUS
--------------------------------------------------------------------------------
SIGSEGV     SEGV_MAPERR          address not mapped to object
            SEGV_ACCERR          invalid permissions for mapped object
--------------------------------------------------------------------------------
SIGBUS      BUS_ADRALN           invalid address alignment
            BUS_ADRERR           non-existent physical address
            BUS_OBJERR           object specific hardware error
--------------------------------------------------------------------------------
#endif

static int cnt_get_sp_bottom = 0;
int fastiva_sys_getCurrentStackBottom() {
	pthread_attr_t attr;
	void* stackaddr;
	size_t stacksize;
	pthread_getattr_np(pthread_self(), &attr);

	pthread_attr_getstack(&attr, &stackaddr, &stacksize);
#ifdef _DEBUG
	//ALOGD("stack-bottom: %x = %p + %x > local(%p)", (int)stackaddr + stacksize, stackaddr, stacksize, &attr);
	if (++cnt_get_sp_bottom == 3) {
		//fastiva_debug_break("", true);
	}
#endif
	
	return (int)stackaddr + stacksize;
}

static void checkMemoryException(int sig, siginfo_t *si, ucontext *uc, SIG_HANDLER_FN org_handler) {
	Thread* self = dvmThreadSelf();
	if (!USE_ALTERNATE_SIG_STATCK && (debug_mode >= 3 || self == NULL || self->status != THREAD_RUNNING)) {
		volatile int do_debug_break = debug_mode == 3;
		if (!do_debug_break) {
			org_handler(sig, si, uc);
		}
		fastiva_debug_break("seg fault", true);
		if (do_debug_break) {
			return;
		}
	}

	if (si->si_addr <= (void*)0x400) {
		/**
		 * posix signal handler 는 장시간 수행도 허용된다. 심지어
		 * handler 내에서 child-process를 실행하기도 한다.
		 * Linux sighandler 의 경우, Exception 이 발생한 thread (SEGV인 경우엔 항상 보장)
		 * 의 stack에 복구할 register와 원래 주소값을 넣은 후
		 * sighandler 가 호출된다. 즉, 내부적으로 setjmp, longjmp가 수행되는 형태다.
		 * (참고, sigreturn == longjmp?) throwException 내에서 longjmp가 수행되므로
		 * 현재 상태에서 throwException을 호출해도 무방하다.
		 */
		RETURN_TO_EXCEPTION_HANDLER(uc, fastiva_throwNullPointerException);
	}
	else {
		RETURN_TO_EXCEPTION_HANDLER(uc, fastiva_throwStackOverflowError);
	}
	return;
}

#ifdef _ARM_
#define LOG_SIGNAL(type, si, uc) \
	fox_printf("%s handler called %p at %x[th:%p]", type, (si == NULL) ? 0 : si->si_addr, uc == NULL ? 0 : (int)uc->uc_mcontext.arm_pc, fox_task_currentTask());
#else
#define LOG_SIGNAL(type, si, uc) \
	fox_printf("%s handler called %p [th:%p]", type, (si == NULL) ? 0 : si->si_addr, fox_task_currentTask());
#endif

static void sig_segv_handler(int sig, siginfo_t *si, ucontext *uc) {
	LOG_SIGNAL("segv", si, uc);
	checkMemoryException(sig, si, uc, org_segv_handler);
}

static void sig_bus_handler(int sig, siginfo_t *si, ucontext *uc) {
	LOG_SIGNAL("bus", si, uc);
	checkMemoryException(sig, si, uc, org_bus_handler);
}

static void sig_fpe_handler(int sig, siginfo_t *si, ucontext *uc) {
	clear_sig_masks();
	LOG_SIGNAL("fpe", si, uc);
	// ucontext 는 SEGV 인 경우에만 유효. 즉 RETURN_TO_EXCEPTION_HANDLER 를 사용할 수 없다.
	fastiva_throwArithmeticException();
}

static void sig_abrt_handler(int sig, siginfo_t *si, ucontext *uc) {
	clear_sig_masks();
	// called abort() function in libc
	LOG_SIGNAL("abrt", si, uc);
	fm__stopAppl();
	org_abrt_handler(sig, si, uc);
}

static void sig_hup_handler(int sig, siginfo_t *si, ucontext *uc) {
	clear_sig_masks();
	// controlling terminal closed (cf, ^D?)
	LOG_SIGNAL("hup", si, uc);
	fm__stopAppl();
	org_hup_handler(sig, si, uc);
}

static void sig_quit_handler(int sig, siginfo_t *si, ucontext *uc) {
	clear_sig_masks();
	// controlling terminal requests to finish application (cf, ^C)
	LOG_SIGNAL("quit", si, uc);
	fm__stopAppl();
	org_quit_handler(sig, si, uc);
}

static void sig_term_handler(int sig, siginfo_t *si, ucontext *uc) {
	clear_sig_masks();
	// process killed via kill or killall
	LOG_SIGNAL("term", si, uc);
	fm__stopAppl();
	org_term_handler(sig, si, uc);
}

static SIG_HANDLER_FN org_suspend_handler = NULL;
volatile int cancel_suspend=0;

static void sig_suspend_handler(int sig, siginfo_t *si, ucontext *uc) {
	void fastiva_doSuspend(void*);
    Thread* self = dvmThreadSelf();
	if (self->m_pNativeStackPointer == NULL) {
#if USE_ALTERNATE_SIG_STATCK
	    fastiva_doSuspend((void*)&uc->uc_mcontext);
#else
	    fastiva_doSuspend(alloca(1));
#endif
		assert(self->m_pNativeStackPointer == NULL);
    }
}

#ifdef FASTIVA_CONCURRENT_STACK_SCAN
static SIG_HANDLER_FN org_scan_handler = NULL;
static void sig_scan_handler(int sig, siginfo_t *si, ucontext *uc) {
	void fastiva_doScanThread(void*);
	fastiva_doScanThread(alloca(1));
}
#endif


static SIG_HANDLER_FN registerSigHandler(int sig, SIG_HANDLER_FN handler, bool clearMask) {
    struct sigaction orgAction;
    struct sigaction newAction;

    // 기존의 값을 복사;
    if (0 != sigaction(sig, NULL, &newAction)) {
    		fatal_exit();
    }
    if (true || clearMask) {
        /**
         * sa_mask:
         * signal_handler 수행 도중에 다른 signal 이 발생하지 않도록 blocking 하기 위한 signal 리스트이다.
         */
		sigemptyset(&newAction.sa_mask);
    }

    newAction.sa_sigaction = (ORG_SIG_HANDLER_FN)handler;
    /**
     * SA_RESTART:
     * IO-BLOCKING 된 thread가 있는 상태에서 signal 이 발생한 경우, 해당 thread 가 호출한 IO-API가
     * EINTR 을 반환하는 것을 방지(최소화)하기 위해 사용 된다. SA_RESTART가 지정된 경우에는 해당 IO-syscall 을 자동 재시작한다.
     */
	newAction.sa_flags = SA_RESTART | SA_SIGINFO;
	if (USE_ALTERNATE_SIG_STATCK && sig == SIGSEGV) {
		newAction.sa_flags |= SA_ONSTACK;
	}

	if (0 != sigaction(sig, &newAction, &orgAction)) {
    		fatal_exit();
    }

	return (SIG_HANDLER_FN)orgAction.sa_sigaction;
}

void fox_pal_sys_initSignalHandler() {

#ifdef _DEBUG
	if (debug_mode < 0) {
		void fastiva_init_debug();
		fastiva_init_debug();
	}
#endif


#if USE_ALTERNATE_SIG_STATCK
	stack_t ss, oss;
	ss.ss_sp = sigstack;//malloc(SIGSTKSZ);//sizeof(sigstack));//sigstack;
	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	if (sigaltstack(NULL, &oss) == -1) {
		ALOGW("get old stack fail");
    		fatal_exit();
	}

	ALOGW("check stack %p ->> %p (%d)", oss.ss_sp, ss.ss_sp, sizeof(sigstack));
	if (sigaltstack(&ss, NULL) == -1) {
		ALOGW("set new stack fail");
    		fatal_exit();
	}
#endif

    struct sigaction sa;
    //static struct sigaction segv_action;

	org_segv_handler = registerSigHandler(SIGSEGV, sig_segv_handler, false);
	ALOGW("segv handler registered %d", SIGSEGV);
    org_fpe_handler  = registerSigHandler(SIGFPE,  sig_fpe_handler, false);
	ALOGW("fpe handler registered %d", SIGFPE);

    org_suspend_handler  = registerSigHandler(SIG_FASTIVA_SUSPEND,  sig_suspend_handler, false);
	ALOGW("suspend handler registered %d", SIG_FASTIVA_SUSPEND);
#ifdef FASTIVA_CONCURRENT_STACK_SCAN
    org_scan_handler  = registerSigHandler(SIG_FASTIVA_SCAN,  sig_scan_handler, false);
	ALOGW("scan handler registered %d", SIG_FASTIVA_SCAN);
#endif

    // setup handlers of other signals
    static struct sigaction other_action;
    sigemptyset(&other_action.sa_mask);

    if (false) {
    		// read or write a specific physical memory address, mialigned data adrress
	    org_bus_handler  = registerSigHandler(SIGBUS,  sig_bus_handler, false);
		org_abrt_handler = registerSigHandler(SIGABRT, sig_abrt_handler, true);
		org_quit_handler = registerSigHandler(SIGQUIT, sig_quit_handler, true);
		org_term_handler = registerSigHandler(SIGTERM, sig_term_handler, true);
		org_hup_handler = registerSigHandler(SIGHUP,  sig_hup_handler, true);
    }
	ALOGW("sig handlers arg registered");
}


#ifdef _DEBUG
static int cntRecursive = 0;
static int test_stackOverflow() {
	char buf[8192*8];
    ALOGW("=== test_stackOverflow %d", cntRecursive ++);
    buf[test_stackOverflow()] = 3;
    return buf[8191];
}

static void test() {
	//fox_sys_initSignalHandler();
	int b = 0;
    ALOGW("=== fox_sys_initSignalHandler succeed %p", &b);

	//test_stackOverflow();

	//FASTIVA_BEGIN_MANAGED_SECTION(0);
#if 0
java_lang_Class_C$* pStatic;
java_lang_Class_p pClass3;
FASTIVA_TRY_EX$ {
		FASTIVA_EXCEPTION_CONTEXT$ {
			case 0: try_0: // 8 -- 15
				goto catch_18;
				FASTIVA_CATCH_EX$(java_lang_String, catch_18)
				FASTIVA_END_CATCH$();
			default:
				goto catch_18;
				FASTIVA_END_CATCH$();
		}
//A8:;
		//pStatic = java_lang_Class_C$::getRawStatic$();/**/
		fastiva_exContext.curr_try_id = 0;
		THROW_EX$((java_lang_Throwable_p)(void*)PSTR$(1, tttt));
		//pClass3 = pStatic->classForName_(NULL, false, NULL);
//A15:;
		fastiva_exContext.curr_try_id = -1;
		goto next_test;
catch_18:;
		fastiva_exContext.curr_try_id = -1;
		ALOGD("ZZZZZZZZZZZZZZZZZ %s", fastiva_exContext.catched_ex->clazz->descriptor);
		FASTIVA_END_EXCEPTION_CONTEXT$();
	}
#endif

next_test:;
	return;
	//FASTIVA_END_MANAGED_SECTION();
	//usleep(1000*1000);
	//_exit(0);
    //pthread_t p_thread;
    //pthread_create(&p_thread, NULL, func, (void *)NULL);
    //ALOGW("%x,%x\n", p_thread, &p_thread);
    //pthread_create(&p_thread, NULL, func, (void *)NULL);
    //ALOGW("%x,%x\n", p_thread, &p_thread);
}
#endif


void fox_debug_trap() {
	int err = errno;
	if (err != 0) {
		const char* error = strerror(errno);
		fox_printf("err %d, %s\n", err, error);
	}
}

static int g_debugPass = 0;

void fox_exit(int errcode) {
	FASTIVA_DBREAK();

	if (--g_debugPass >= 0) {
		return;
	}

	//dvmDumpThread(dvmThreadSelf(), false);

	//fox_Task* pCurrTask = fox_task_currentTask();
	//if (pCurrTask != NULL) {
	//	pCurrTask->dumpDebugStackTrace();
	//}
	fox_printf("Fastiva terminated: %d\n", errcode);
	exit(errcode);
}



#else

#include <windows.h> // for CRITICAL_SECTION
#include <stdio.h> // for SPRINTF
char debug_buff[DEBUG_BUFF_SIZE];

typedef union {
	unsigned char mode : 2;
	unsigned char reserved : 3;
	unsigned char rm : 3;
}X86_CODE;

static int getIDivJumpOffset(unsigned int baseAddr) {
	int ip = 0;
	X86_CODE code = {0};
	
	char *eip = (char *)(void *)baseAddr;
	if (eip[0] != 0xF7) {; //eip는 항상 0xF7(X86의 IDIV code)
		FASTIVA_DBREAK();
	}

	code = *(X86_CODE*)&eip[1];

	switch(code.mode) {
	case 0:  //mod 00
		if (code.rm == 0x05) { //disp32
			return 6;
		}
		else if (code.rm == 0x04) {            //[--][--] 한 byte더 읽어서 판별
			if ((eip[2] & 0x07) != 0x05) {
				return 0x03;        
			}
			else {
				return 0x07;
			}
		}

	case 1:
		if (code.rm == 0x04) { //[--][--]+disp8
			return 0x04;
		}
		else {
			return 0x03;                      
		}

	case 2:
		if (code.rm == 0x04) { //[--][--]+disp32
			return 0x07;
		}
		else {
			return 0x06;
		}

	case 3:
		return 0x02;
	}
	return 0;
}

static int g_pass_trap = 1; //  // 정해진 개수만큼 break 무시
static int g_in_trap = false;
void fastiva_init_debug() {}

int fastiva_dispatchNativeException(void* pExInfo) {
	//fastiva_Task* pTask = fastiva_getCurrentTask();

	//fox_unexpected_handler((_EXCEPTION_POINTERS*)pExInfo);
	_EXCEPTION_POINTERS *pExceptionInfo = (_EXCEPTION_POINTERS*)pExInfo;
	// The DemandLoadDll_ExceptionFilter function changes a 
	// thread's program counter. We can restrict the amount
	// of CPU-dependent code by defining the PROGCTR macro below.
	#if defined(_X86_)
	#define PROGCTR(Context)  ((Context)->Eip)
	#endif

	#if defined(_MIPS_)
	#define PROGCTR(Context)  ((Context)->Fir)
	#endif

	#if defined(_ALPHA_)
	#define PROGCTR(Context)  ((Context)->Fir)
	#endif

	#if defined(_PPC_)
	#define PROGCTR(Context)  ((Context)->Iar)
	#endif

	#if defined(_ARM_)
	#define PROGCTR(Context)  ((Context)->Pc)
	#endif
	uint addr;
	uint reg_ecx;


	switch (pExceptionInfo->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
#ifdef UNDER_CE
			addr = 0; //pExceptionInfo->ExceptionRecord->ExceptionInformation[0];
#else
			addr = pExceptionInfo->ExceptionRecord->ExceptionInformation[1];
#endif
			if (--g_pass_trap < 0) {
				if (g_in_trap) {
					g_in_trap = false;
				}
				else {
					g_in_trap = true;
					return EXCEPTION_CONTINUE_SEARCH;
				}
			}
#ifdef _DEBUG
			if ((int)addr <= 0x400) { //== (int)FASTIVA_NULL) {
				PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwNullPointerException;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
#else
			if (addr <= 0x400) { //3FF(int)FASTIVA_NULL) {
				PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwNullPointerException;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
#endif
			return EXCEPTION_CONTINUE_SEARCH;

		case EXCEPTION_INT_DIVIDE_BY_ZERO: 
			PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwArithmeticException;
			return EXCEPTION_CONTINUE_EXECUTION;


		case EXCEPTION_STACK_OVERFLOW: 
			PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwStackOverflowError;
			return EXCEPTION_CONTINUE_EXECUTION;

		case EXCEPTION_FLT_STACK_CHECK: 
			//PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwStackOverflowError;
			//return EXCEPTION_CONTINUE_EXECUTION;
		
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: 
			// The thread tried to access an array element that is out of bounds 
			// and the underlying hardware supports bounds checking.

		case EXCEPTION_INT_OVERFLOW: 
			// The result of an integer operation caused a carry out of the most 
			// significant bit of the result. ((signed int)0x80000000 / (int)-1)
		{
#ifdef UNDER_CE
			KASSERT(0);
#else
			_asm int 3;
			unsigned int XEIP = pExceptionInfo->ContextRecord->Eip;
			unsigned int jumpAddr = XEIP;
			jumpAddr += getIDivJumpOffset(XEIP);
			pExceptionInfo->ContextRecord->Eax = 0x80000000;
			pExceptionInfo->ContextRecord->Edx = 0x00000000;
			PROGCTR(pExceptionInfo->ContextRecord) = (int)jumpAddr;
#endif
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		case EXCEPTION_FLT_DENORMAL_OPERAND: 
			// One of the operands in a floating-point operation is denormal. 
			// A denormal value is one that is too small to represent as a standard 
			// floating-point value. 

		case EXCEPTION_FLT_DIVIDE_BY_ZERO: 
			// The thread tried to divide a floating-point value by a 
			// floating-point divisor of zero. 
		
		case EXCEPTION_FLT_INEXACT_RESULT: 
			// The result of a floating-point operation cannot be 
			// represented exactly as a decimal fraction. 
		
		case EXCEPTION_FLT_INVALID_OPERATION: 
			// This exception represents any floating-point exception not included in this list. 
		
		case EXCEPTION_FLT_OVERFLOW: 
			// The exponent of a floating-point operation is greater than the magnitude 
			// allowed by the corresponding type. 
		
		case EXCEPTION_FLT_UNDERFLOW: 
			// The exponent of a floating-point operation is less than the magnitude 
			// allowed by the corresponding type. 

		case EXCEPTION_ILLEGAL_INSTRUCTION: 
			// The thread tried to execute an invalid instruction. 

		case EXCEPTION_IN_PAGE_ERROR: 
			// The thread tried to access a page that was not present, 
			// and the system was unable to load the page. 
			// For example, this exception might occur if a network connection is lost 
			// while running a program over the network. 

		case EXCEPTION_INVALID_DISPOSITION: 
			// An exception handler returned an invalid disposition to the 
			// exception dispatcher. Programmers using a high-level language 
			// such as C should never encounter this exception. 

		case EXCEPTION_NONCONTINUABLE_EXCEPTION: 
			// The thread tried to continue execution after a noncontinuable exception occurred. 

		case EXCEPTION_PRIV_INSTRUCTION: 
			// The thread tried to execute an instruction whose operation is not 
			// allowed in the current machine mode. 

		case EXCEPTION_BREAKPOINT: 	
			// A breakpoint was encountered. 

		case EXCEPTION_SINGLE_STEP: 
			// A trace trap or other single-instruction mechanism signaled 


		case EXCEPTION_DATATYPE_MISALIGNMENT: 
			// The thread tried to read or write data that is misaligned on hardware 
			// that does not provide alignment. For example, 16-bit values must be 
			// aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on. 
			break;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

int fastiva_sys_getCurrentStackBottom() {
	CONTEXT win32_ctx;
	//memset(&win32_ctx,0,sizeof(CONTEXT));      
	win32_ctx.ContextFlags = CONTEXT_FULL; // | CONTEXT_DEBUG_REGISTERS;      
	if (!::GetThreadContext(GetCurrentThread(), &win32_ctx)) {
		int error = GetLastError();
		//return 0;
	}

	return (int)&win32_ctx | (4096-1);

	//return 0;//(int)stackaddr + stacksize;
}

pid_t fastiva_getCurrentThreadID() {
	return GetCurrentThreadId();
}

#endif


void fastiva_sendSuspendRequest(pthread_t thread) {
#ifndef _WIN32
	pthread_kill(thread, SIG_FASTIVA_SUSPEND);
#endif
}

void fastiva_sendScanRequest(pthread_t thread) {
#ifndef _WIN32
	pthread_kill(thread, SIG_FASTIVA_SCAN);
#endif
}

#ifdef _DEBUG
void ex_test(int a, int b, jlonglong c, jlonglong c1, jlonglong c2, jlonglong c3);
void (*ex_test_fn)(int a, int b, jlonglong c, jlonglong c1, jlonglong c2, jlonglong c3) = ex_test;


jmp_buf jb;

void doLongjmpTest2() {
	longjmp(jb, 1);
}

int ddjmp = 9;

struct TTKMP {
	~TTKMP() {
		ddjmp = -1;
		ALOGD("Unwind by longjmp");
	}
};

void doLongjmpTest() {
	if (!setjmp(jb)) {
		TTKMP t;
		doLongjmpTest2();
	}
	//printf("%d", ddjmp);
}


extern "C" void call_ex_test() {
	doLongjmpTest();
return;
	TRY$ {
		volatile int* null_p = 0;
		ex_test_fn(*null_p, 2, 3, 4, 5, 6);
	}
	CATCH_ANY$ {
		ALOGD("Exception catched");
	}
}
#endif