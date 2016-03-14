#ifndef __FOX_MONITOR_H__
#define __FOX_MONITOR_H__

#include <fox/Config.h>
#include <fox/errcode.h>

#ifdef __cplusplus
extern "C" {
#endif	

typedef struct _fox_Monitor {
	struct fox_MonitorSyncData*	m_pSyncData$;
#ifndef FOX_BIG_ENDIAN
	short m_cntBlocked$, m_cntRecursion$;
#else
	short m_cntRecursion$, m_cntBlocked$;
#endif
	struct fox_Task* m_pOwner$;
} fox_Monitor;


void FOX_FASTCALL(fox_monitor_init)(fox_Monitor*);

void FOX_FASTCALL(fox_monitor_lock)(fox_Monitor*);

#define fox_monitor_isLocked(pMonitor) ((pMonitor)->m_pOwner$ != ADDR_ZERO)

#define fox_monitor_isLockedBy(pMonitor, pTask) ((pMonitor)->m_pOwner$ == pTask)

FOX_BOOL FOX_FASTCALL(fox_monitor_tryLock)(fox_Monitor*);
	// 0 : The mutex is already owned by another task
    // else : The mutex is succesfully locked

FOX_BOOL FOX_FASTCALL(fox_monitor_release)(fox_Monitor*);
	// 0 : The mutex is not locked by cuurent task
    // else : The mutex is succesfully released

FOX_ERRCODE FOX_FASTCALL(fox_monitor_wait)(fox_Monitor*, unsigned int period);
	// FOX_E_INVAL	  : The mutex is not owned by the current task
	// FOX_E_TIMEDOUT : time expired.
	// FOX_E_INTR	  : operation inttrupted;
	
FOX_BOOL FOX_FASTCALL(fox_monitor_notify)(fox_Monitor*);
	// FOX_FALSE : The mutex is not owned by the current task

FOX_BOOL FOX_FASTCALL(fox_monitor_notifyAll)(fox_Monitor*);
	// FOX_FALSE : The mutex is not owned by the current task

#ifdef __cplusplus
}
#endif	

#endif // __FOX_MONITOR_H__
