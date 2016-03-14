#ifndef __FOX_SEMAPHORE_H__
#define __FOX_SEMAPHORE_H__

#include <fox/Config.h>

#ifdef __cplusplus
extern "C" {
#endif	

struct fox_Semaphore;

fox_Semaphore* FOX_FASTCALL(fox_semaphore_create)();

void FOX_FASTCALL(fox_semaphore_lock_GCI)(fox_Semaphore*);

// no-gc-interrupt
void FOX_FASTCALL(fox_semaphore_lock) (fox_Semaphore* pSemaphore);

FOX_BOOL FOX_FASTCALL(fox_semaphore_tryLock)(fox_Semaphore* pSemaphore);

FOX_BOOL FOX_FASTCALL(fox_semaphore_isLocked)(fox_Semaphore*);

void FOX_FASTCALL(fox_semaphore_release)(fox_Semaphore*);

void FOX_FASTCALL(fox_semaphore_destroy)(fox_Semaphore*);


#ifdef __cplusplus
}
#endif	

#endif // __FOX_SEMAPHORE_H__
