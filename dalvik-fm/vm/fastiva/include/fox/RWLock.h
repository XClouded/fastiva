#ifndef __FOX_RWLOCK_H__
#define __FOX_RWLOCK_H__

#include <fox/Config.h>

#ifdef __cplusplus
extern "C" {
#endif	

struct fox_RWLock;

fox_RWLock* FOX_FASTCALL(fox_rwlock_create)();

// true : locked read or write.
FOX_BOOL fox_rwlock_isReleased(fox_RWLock*);

FOX_BOOL fox_rwlock_isReadLocked(fox_RWLock*);

FOX_BOOL fox_rwlock_isWriteLocked(fox_RWLock*);

void FOX_FASTCALL(fox_rwlock_lockRead)(fox_RWLock*);

void FOX_FASTCALL(fox_rwlock_lockWrite)(fox_RWLock*);

void FOX_FASTCALL(fox_rwlock_releaseRead)(fox_RWLock*);

void FOX_FASTCALL(fox_rwlock_releaseWrite)(fox_RWLock*);

void FOX_FASTCALL(fox_rwlock_destroy)(fox_RWLock*);


#ifdef __cplusplus
}
#endif	

#endif // __FOX_RWLOCK_H__
