#ifndef __FOX_TASK_H__
#define __FOX_TASK_H__

#include <fox/Config.h>
#include <fox/errcode.h>

#ifdef __cplusplus
extern "C" {
#endif	

enum fox_TaskState {
	// 참고 Android.VMThread.StateMap index와 동일하게 맞추었다.
	STARTING = 6,
	NEW = 5,
	RUNNABLE = 1,
	BLOCKED = 3,
	WAITING = 4,
	TIMED_WAITING = 2,
	TERMINATED = 0,
};

typedef struct fox_Task* FOX_HTASK;

typedef void (FOX_FASTCALL(*FOX_TASK_START_ROUTINE))(void*);

typedef void (FOX_FASTCALL(*ABNORMAL_TERMINATION_HANDLER))();


FOX_HTASK FOX_FASTCALL(fox_task_create)(FOX_TASK_START_ROUTINE, void* userData, int priority);

FOX_HTASK FOX_FASTCALL(fox_task_currentTask)();

void* FOX_FASTCALL(fox_task_getUserData)(FOX_HTASK);

void* FOX_FASTCALL(fox_task_getCurrentTaskUserData)();

void FOX_FASTCALL(fox_task_yield)();

FOX_BOOL FOX_FASTCALL(fox_task_suspend)(FOX_HTASK);

FOX_BOOL FOX_FASTCALL(fox_task_resume)(FOX_HTASK);

// false if waked via interrupted;
FOX_BOOL FOX_FASTCALL(fox_task_sleep)(unsigned int period);

void FOX_FASTCALL(fox_task_wake)(FOX_HTASK);

void FOX_FASTCALL(fox_task_stop)(FOX_HTASK, ABNORMAL_TERMINATION_HANDLER);

int FOX_FASTCALL(fox_task_getPriority)(FOX_HTASK); // MIN = 1, MAX = 15

void FOX_FASTCALL(fox_task_setPriority)(FOX_HTASK, int priority);

FOX_BOOL FOX_FASTCALL(fox_task_isActive)(FOX_HTASK);

//======================================================//

//void FOX_FASTCALL(fox_task_startScheduler)();

//void FOX_FASTCALL(fox_task_stopScheduler)();

int FOX_FASTCALL(fox_task_getRegisters)(fox_Task* pTask, int* aReg);

//======================================================//
// util
//======================================================//

FOX_HTASK FOX_FASTCALL(fox_task_attachCurrentNativeTask)();

void FOX_FASTCALL(fox_task_interrupt)(FOX_HTASK);

FOX_BOOL FOX_FASTCALL(fox_task_isInterrupted)(FOX_HTASK);

FOX_BOOL FOX_FASTCALL(fox_task_clearInterrupted)(FOX_HTASK);

void FOX_FASTCALL(fox_task_join)(FOX_HTASK);

// 현재 task의 priority를 최상급으로 높여 다른 fox_Task가 실행되지
// 못하는 상태를 만든다.
void FOX_FASTCALL(fox_task_lockScheduler)();

// 현재 task의 priority를 원래대로 복구한다.
void FOX_FASTCALL(fox_task_unlockScheduler)();


//======================================================//

//void FOX_FASTCALL(fox_scheduler_lock)();

//void FOX_FASTCALL(fox_scheduler_unlock)();

//FOX_BOOL FOX_FASTCALL(fox_scheduler_isLocked)();

//void FOX_FASTCALL(fox_task_disableInterrupt)();

//void FOX_FASTCALL(fox_task_enableInterrupt)();


/*======================================================//



void* FOX_FASTCALL(fox_task_getStackPointer)(FOX_HTASK);

/* wait() 구현을 위해 사용됨.
void* FOX_FASTCALL(fox_task_getStackBottom)(FOX_HTASK);

void FOX_FASTCALL(fox_task_boostPriority)(FOX_HTASK);

FOX_BOOL FOX_FASTCALL(fox_task_checkBoost)(FOX_HTASK hThread);

void FOX_FASTCALL(fox_task_restorePriority)(FOX_HTASK);

//void FOX_FASTCALL(fox_task_enterCriticalSection)(int idxLock);

//void FOX_FASTCALL(fox_task_leaveCriticalSection)(int idxLock);
void FOX_FASTCALL(fox_task_removeRunnable)(FOX_HTASK);
*/
#ifdef __cplusplus
}
#endif	

#endif // __FOX_TYPES_H__
