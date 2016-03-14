#ifndef __FASTIVA_RECURSIVE_MUTEX__H__
#define __FASTIVA_RECURSIVE_MUTEX__H__

struct fastiva_recursive_mutex {
	volatile int lockOwner;
	volatile int cntLock;
	volatile int signal1;
	volatile int signal2;
};
int  dvmTryLockMutex(fastiva_recursive_mutex*);
void dvmInitMutex(fastiva_recursive_mutex*);
void dvmLockMutex(fastiva_recursive_mutex*);
void dvmUnlockMutex(fastiva_recursive_mutex*);
void dvmWaitCond(pthread_cond_t* pCond, fastiva_recursive_mutex* mutex);
void dvmNotiCond(pthread_cond_t* pCond, fastiva_recursive_mutex* mutex);

#endif // __FASTIVA_RECURSIVE_MUTEX__H__
