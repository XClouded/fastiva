#ifndef __FOX_KERENL_CONTEXT_H__
#define __FOX_KERENL_CONTEXT_H__

#include <fox/Task.h>
#include <fox/Monitor.h>
#include <fox/Mutex.h>
#include <fox/Condition.h>
#include <fox/Atomic.h>

#ifdef _WIN32
	#include <windows.h>
#endif

#ifdef UNDER_CE
	#ifndef FASTIVA_NO_MULTI_CPU_SUPPORT
		#define FASTIVA_NO_MULTI_CPU_SUPPORT
	#endif
#endif
// #define FASTIVA_NO_MULTI_CPU_SUPPORT

#ifdef FASTIVA_NO_MULTI_CPU_SUPPORT
	#define FOX_ASM_LOCK	// 
#else
	#define FOX_ASM_LOCK    lock
#endif


struct fox_MonitorSyncData;

#ifdef _WIN32
#define NO_USE_POSIX_SIGNAL_LIKE_GC_INTERRUPT 0
#else 
#define NO_USE_POSIX_SIGNAL_LIKE_GC_INTERRUPT 0
#endif

#define TASK_RUNNABLE	0x00
#define TASK_BLOCKED	0x01
#define TASK_WAITING	0x02
#define TASK_SUSPENDED	0x80
#define TASK_TERMINATED -1

struct fox_Condition {
	int m_signal;
	// @todo m_cntLocked�� m_handle�� ���� �ٸ� page�� �Ҵ��Ͽ� cache-hit ������ �ؼ��Ѵ�.
	int m_cntLocked;

	int m_signal2;
	int m_readLock;
#ifdef _WIN32
	HANDLE m_handle;
#endif

	bool acceptSignal() {
		while (true) {
			int sig = this->m_signal;
			if (sig == 0) {
				return false;
			}
			KASSERT(sig > 0);
			if (fox_util_cmpxchg32(&this->m_signal, sig - 1, sig)) {
				return true;
			}
		}
	}
};

struct fox_Semaphore : fox_Condition {
};

struct fox_RWLock : fox_Condition {
};


struct fox_WaitingTaskChain {
	union {
		void* fastiva_env_vtable$;
		fox_MonitorSyncData* m_pNext;
	};

	// waiting ���� task�� ���� timer �Ǵ� interrupt�� ���� wake�ȴ�.
	// �ش� ó���� ������ �ϱ� ���� Double-Link�� ����Ѵ�.
	fox_Task*		m_pNextWaiting;
	fox_Task*		m_pPrevWaiting;

	void setNextWaiting(fox_Task* pTask);
	void isolate();
	bool inWaitingQ() { return m_pNextWaiting != ADDR_ZERO; }
};

#define FOX_STACK_SIZE			(128 * 1024)

struct fox_Task : fox_WaitingTaskChain {

private:
	int				m_suspendLock;
	fox_TaskState	m_state;
	fox_Mutex		m_stackLock;
public:
	fox_Condition*	m_pInterruptCondVar;
	fox_Condition*	m_pLockedCondition;

	int				m_isInterrupted;
	char			m_trap;	//	just for debugging purpose;

	void*			m_hNativeTask;
	//void*			m_hInterrupt;
	int				m_taskID;
	int				m_priority;
	void (FOX_FASTCALL(*m_pfnRun_interlocked))(void*);

	void*			m_pUserData;
	void*			m_pTIB;
	void*			m_spBottom;   // sleep_ex�� ���Խ��� stack-pointer�̴�.
	fox_Monitor*	m_pBlockedBy; // debuggin info : ���� blocked�� monitor�� �����ش�.
	fox_MonitorSyncData* m_pLocalSyncData;

//#if !NO_USE_POSIX_SIGNAL_LIKE_GC_INTERRUPT
#ifdef _WIN32
	CONTEXT         m_gcInterruptContext;
#endif
	FOX_BOOL        m_gcInterrupted;
//#endif

	int m_stackTraceDepth;
	const char* m_functionTrace[128];

	//int				m_resumeAddr;
	//ABNORMAL_TERMINATION_HANDLER m_pfnAbnormalTerminationHandler;
	//signed char		m_state;

	//void*			m_pStackBottom; // stack-pointer�� ���� ������ �� ���ȴ�.

	//void  setInterrupted() {
	//	if (fox_util_xchg32(&m_isInterrupted, 1) == 0) {
	//		fox_task_wake(this);
	//	}
	//}


	void init() {
		fox_mutex_init(&m_stackLock);
		m_pInterruptCondVar = fox_condition_create(0);
	}

	void enterDebugStackTrace(const char* func_sig) {
		if (m_stackTraceDepth < sizeof(m_functionTrace) / sizeof(m_functionTrace[0])) {
			m_functionTrace[m_stackTraceDepth++] = func_sig;
		}
	}

	void leaveDebugStackTrace() {
		m_stackTraceDepth--;
	}

	void dumpDebugStackTrace() {
		for (int i = m_stackTraceDepth; --i >= 0; ) {
			fox_printf("[BT:%d-%x] %s/n", i, this, m_functionTrace[i]);
		}
	}

	/**
	 true ��: Stack ���� �� StackContext ���� ���� : Stack scanning ��.
	          Suspend �� Signal-Interrupt(GC) ���� : Instance Tree Marking ��. 
	*/
	FOX_BOOL inCriticalSection() {
		return fox_mutex_isLocked(&m_stackLock);
	}

	void enterCriticalSection_GCI() {
		fox_mutex_lock_GCI(&m_stackLock);
	}

	void enterCriticalSection() {
		fox_mutex_lock(&m_stackLock);
	}

	void leaveCriticalSection() {
		fox_mutex_release(&m_stackLock);
	}

	bool isTerminated() {
		return m_hNativeTask == ADDR_ZERO;
	}

	void setTerminated() {
		m_hNativeTask = ADDR_ZERO;
	}

	int getState() {
		return m_state;
	}

	void setState(fox_TaskState state) {
		m_state = state;
	}


	FOX_BOOL isInterrupted() {
		return m_isInterrupted;
	}

};

class AutoLocalHeapLock {
	fox_Task* m_pTask;
public:
	AutoLocalHeapLock(fox_Task* pTask) {
		m_pTask = pTask;
		pTask->enterCriticalSection();
	}

	~AutoLocalHeapLock() {
		m_pTask->leaveCriticalSection();
	}
};


fox_Task* fox_kernel_createTaskContext(int contextSize = -1);

void FOX_FASTCALL(fox_task_create_ex)(void(FOX_FASTCALL(*pfnRun))(void*), void* userData, int priority, fox_Task* pNewTask, int stackSize);

void FOX_FASTCALL(fox_task_notifyCurrentTaskBlocked)(fox_Task* pCurrentTask);

void FOX_FASTCALL(fox_task_notifyCurrentTaskReleased)(fox_Task* pCurrentTask);

void FOX_FASTCALL(sys_task_suspend)(fox_Task* pTask);

void FOX_FASTCALL(sys_task_resume)(fox_Task* pTask);

void FOX_FASTCALL(sys_task_spin)();

int  FOX_FASTCALL(sys_task_getStackSize)();

FOX_BOOL FOX_FASTCALL(sys_task_sleep_ex)(fox_Task* pCurrTask, unsigned int period);

void FOX_FASTCALL(sys_task_setSystemIdleTask)(fox_Task* pTask);

void FOX_FASTCALL(sys_task_initGCTask)(fox_Task* pTask);

void FOX_FASTCALL(sys_task_spinLock)(volatile int* pLock);

void FOX_FASTCALL(sys_task_setCurrentTask)(fox_Task* pTask);

static inline void sys_task_release_spinLock(volatile int* pLock) {
	*pLock = 0;
}

void sys_task_interruptTask(fox_Task* pTask);

void sys_task_exitInterrupt(fox_Task* pCurrTask);


#endif // __FOX_KERENL_CONTEXT_H__