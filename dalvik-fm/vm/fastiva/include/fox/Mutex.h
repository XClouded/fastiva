#ifndef __FOX_MUTEX_H__
#define __FOX_MUTEX_H__

#include <fox/Config.h>
#include <fox/errcode.h>
#include <fox/Semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif	


struct fox_Mutex {
	void* m_pReserved; // virtual-table
	fox_Semaphore* m_hSemaphore0;
	void* m_pOwner0;
	int   m_cntRecursion0;
};

typedef struct fox_Mutex* FOX_HMUTEX;

void FOX_FASTCALL(fox_mutex_init)(FOX_HMUTEX);

void FOX_FASTCALL(fox_mutex_destroy)(FOX_HMUTEX);

void FOX_FASTCALL(fox_mutex_lock_GCI)(FOX_HMUTEX);

// gc-interrupt 를 무시한다.
void FOX_FASTCALL(fox_mutex_lock)(FOX_HMUTEX);

int FOX_FASTCALL(fox_mutex_isLocked)(FOX_HMUTEX);

int FOX_FASTCALL(fox_mutex_isLockedBySelf)(FOX_HMUTEX);

int FOX_FASTCALL(fox_mutex_tryLock)(FOX_HMUTEX);
	// 0 : The mutex is already owned by another task
    // else : The mutex is succesfully locked

FOX_BOOL FOX_FASTCALL(fox_mutex_release)(FOX_HMUTEX);
	// FOX_FALSE : The mutex is not owned by the current task

#ifdef __cplusplus
}
#endif	

#endif // __FOX_MUTEX_H__
