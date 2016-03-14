#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
#include <kernel/Runnable.h>
#include <fox/Task.h>
#include <fox/kernel/TaskContext.h>

/*
void FOX_FASTCALL(register_synchonize$)(fastiva_Instance_p pObj, fastiva_Synchronize* pSync) {
	fm::monitorEnter(*(fastiva_Instance_p*)pSync = pObj);

	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	((int*)pSync)[1] = (int)pCurrTask->m_pTopRewinder;
	pCurrTask->m_pTopRewinder = pSync;
}
*/





void fastiva_Runtime::monitorEnter(fastiva_Instance_p pObj) {
	fox_Monitor* pMonitor = &pObj->m_monitor$;

#if 0 // ndef FASTIVA_NO_MULTI_CPU_SUPPORT
	if (!fm::isPublished(pObj)) {
		if (pMonitor->m_pOwner$ != ADDR_ZERO) {
			KASSERT(pMonitor->m_cntBlocked$ == -1);
			KASSERT(fox_task_currentTask() == pMonitor->m_pOwner$);
			pMonitor->m_cntRecursion$ --;
		}
		else {
			KASSERT(pMonitor->m_cntBlocked$ == 0);
			KASSERT(pMonitor->m_cntRecursion$ == 0);
			pMonitor->m_pOwner$ = fox_task_currentTask();
			*((int*)&pMonitor->m_cntBlocked$) = -1;
		}
	}
	else
#endif
	{
		fox_monitor_lock(pMonitor);
	}
	KASSERT(fox_task_currentTask() == pMonitor->m_pOwner$);
}

void fastiva_Runtime::monitorExit(fastiva_Instance_p pObj) {
	fox_Monitor* pMonitor = &pObj->m_monitor$;
#ifdef _DEBUG
	fox_Task* pCurrTask = fox_task_currentTask();
	KASSERT(pCurrTask == pMonitor->m_pOwner$);
#endif

#ifndef FASTIVA_NO_MULTI_CPU_SUPPORT
	if (!fm::isPublished(pObj)) {
		if (*((int*)&pMonitor->m_cntBlocked$) == -1) {
			pMonitor->m_pOwner$ = ADDR_ZERO;
			*((int*)&pMonitor->m_cntBlocked$) = 0;
		}
		else {
			pMonitor->m_cntRecursion$ ++;
		}
	}
	else
#endif
	{
		fox_monitor_release(pMonitor);
	}
}




void* FOX_FASTCALL(fox_monitor_lock_ex) (fox_Monitor* pMonitor);

void fastiva_Runtime::linkSynchronized(fastiva_SynchronizedLink* pLink) {
	fastiva_Synchronize* pSync = (fastiva_Synchronize*)(void*)pLink;
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	pSync->pushRewinder(pCurrTask);
	//pSync->m_pPrev = (fastiva_Synchronize*)pCurrTask->m_pTopRewinder;
	//pCurrTask->m_pTopRewinder = pSync;
}


void fastiva_Runtime::unlinkSynchronized(fastiva_SynchronizedLink* pLink) {
	fastiva_Synchronize* pSync = (fastiva_Synchronize*)(void*)pLink;
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	pCurrTask->m_pTopRewinder = pSync->m_pPrev;
}

void fastiva_Rewinder::pushRewinder(fastiva_Task* pCurrTask) {
	KASSERT(this->m_pTask == ADDR_ZERO);
	this->m_pTask = pCurrTask;
	this->m_pPrev = pCurrTask->m_pTopRewinder;
	pCurrTask->m_pTopRewinder = this;
}


void fastiva_Runtime::beginSynchronized(fastiva_Instance_p pObj, fastiva_Synchronize* pSync) {

	fox_Monitor* pMonitor = &pObj->m_monitor$;
	fastiva_Task* pCurrTask;

#if 0 //ndef FASTIVA_NO_MULTI_CPU_SUPPORT
	if (!fm::isPublished(pObj)) {
		if (pMonitor->m_pOwner$ != ADDR_ZERO) {
			// 이미 LOCKING된 상태;
			KASSERT(pMonitor->m_cntBlocked$ == -1);
			KASSERT(fox_task_currentTask() == pMonitor->m_pOwner$);
			pSync->m_pObj = ADDR_ZERO;
			return;
		}

		KASSERT(pMonitor->m_cntBlocked$ == 0);
		KASSERT(pMonitor->m_cntRecursion$ == 0);
		*((int*)&pMonitor->m_cntBlocked$) = -1;
		pMonitor->m_pOwner$ = fox_task_currentTask();
		pCurrTask = (fastiva_Task*)pMonitor->m_pOwner$;
		goto push_synchonize_stack;
	}
	else
#endif
	{
		pCurrTask = (fastiva_Task*)fox_monitor_lock_ex(pMonitor);
	}

	if (pCurrTask == ADDR_ZERO) {
		// 이미 LOCKING된 상태라면 Synchronize를 등록할 필요가 없다.
		// endSynchronized 도 호출되지 않는다.
		KASSERT(pSync->m_pTask == ADDR_ZERO);
	}
	else {
push_synchonize_stack:
		pSync->m_pObj = pObj;
		pSync->pushRewinder(pCurrTask);
		//pSync->m_pPrev = (fastiva_Synchronize*)pCurrTask->m_pTopRewinder;
		//pCurrTask->m_pTopRewinder = pSync;
	}
}

fastiva_Rewinder::~fastiva_Rewinder() {
	fastiva_Task* pCurrTask = this->m_pTask;
	if (pCurrTask == ADDR_ZERO) {
		return;
	}
	pCurrTask->m_pTopRewinder = this->m_pPrev;
}

void fastiva_Runtime::endSynchronized(fastiva_Instance_p pObj, fastiva_Synchronize* pSync) {
	KASSERT (pObj == pSync->m_pObj);
	KASSERT (pObj != ADDR_ZERO);

	fastiva.monitorExit(pObj);
}

