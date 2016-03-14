#include <precompiled_libcore.h>

#include <kernel/fastiva_GC.h>
#include <java/lang/ref/Reference.h>
#include <java/lang/ref/WeakReference.h>
#include <java/lang/ref/ReferenceQueue.h>

#if 0
����) 
	marking �� bit-flag �� �ƴ�, gc-generation ���� ������, ���� GC�� ���� demarking �� �ʿ䰡 ��������.
	�� ��� ���ÿ� �ݵ�� marking ���� gc-generation�� ������� ���ϵ��� ��, marking-task�� suspend ����
	���ϵ��� �Ͽ��� �Ѵ�.
#endif

#define USE_RECURSIVE_STACK 0

FOX_BOOL gc_markStrongReachable_debug = false;
int gc_markStrongReachable_debug_cnt = 0;

struct fastiva_Scanner {
	fastiva_Instance_p m_pLast;

	fastiva_Scanner(fastiva_Instance_p pLast) {
		KASSERT(fastiva_getCurrentTask()->inCriticalSection() || fastiva_getCurrentTask() == kernelData.g_pSystemTask);
		KASSERT(pLast != NULL);
		this->m_pLast = pLast;
		pLast->m_pNextScan$ = NULL;
	}

	void add(fastiva_Instance_p pObj) {
		m_pLast->m_pNextScan$ = pObj;
		m_pLast = pObj;
		pObj->m_pNextScan$ = NULL;
	}
};

static void _markStrongReachable(fastiva_Instance_p pObj, fastiva_Scanner* pScanQ) {
	if (pObj == FASTIVA_NULL) {
		return;
	}
	GC_DEBUG_BREAK(pObj);
	if (!fox_GC::isReachable(pObj)) {
		// reachable-marking �� ���� ���� thread���� ���ÿ� �õ��� �� �ִ�.
		// marking ������ Ȯ���Ͽ��� �Ѵ�.
		fox_GC::markStrongReachable(pObj);
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_markStrongReachable, 0);
		}
		else {
			pScanQ->add(pObj);
		}
	}
	return;
}

void fm::gc_markStrongReachable(fastiva_Instance_p pObj) {
	//KASSERT(g_GC.isScanModeLocked() || fastiva_getCurrentTask() == kernelData.g_pSystemTask);
	KASSERT(pObj != FASTIVA_NULL);
	GC_DEBUG_BREAK(pObj);

#ifdef _DEBUG_TRACE
	if (g_GC.getScanMode() < TASK_SCANNING) {
		fox_printf("gc_markStrongReachable in non-scan-mode %d\n", g_GC.getScanMode());
		fox_exit(-1);
	}
#endif

	if (!fox_GC::isReachable(pObj)) {
		// reachable-marking �� ���� ���� thread���� ���ÿ� �õ��� �� �ִ�.
		// marking ������ Ȯ���Ͽ��� �Ѵ�.
		if (USE_RECURSIVE_STACK) {
			fox_GC::markStrongReachable(pObj);
			pObj->scanInstance$(_markStrongReachable, 0);
			return;
		}

		fox_semaphore_lock(g_GC.pScanNextLock);
		fox_GC::markStrongReachable(pObj);
		fastiva_Scanner scanQ(pObj);
		while (pObj != NULL) {
			pObj->scanInstance$(_markStrongReachable, &scanQ);
			pObj = pObj->m_pNextScan$;
		}
		fox_semaphore_release(g_GC.pScanNextLock);
	}

#ifdef _DEBUG_TRACE
	if (g_GC.getScanMode() < TASK_SCANNING) {
		fox_printf("gc_markStrongReachable in non-scan-mode %d\n", g_GC.getScanMode());
		fox_exit(-1);
	}
#endif
}


static void _markLocalReachable(fastiva_Instance_p pObj , fastiva_Scanner* pScanQ) {
	if (pObj == FASTIVA_NULL) {
		return;
	}
	GC_DEBUG_BREAK(pObj);
	if (!fox_GC::isLocalReachable(pObj)) {
		// reachable-marking �� ���� ���� thread���� ���ÿ� �õ��� �� �ִ�.
		// marking ������ Ȯ���Ͽ��� �Ѵ�.
		fox_GC::markLocalReachable(pObj);
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_markLocalReachable, 0);
		}
		else {
			pScanQ->add(pObj);
		}
	}
	return;
}

void fm::gc_markLocalReachable(fastiva_Instance_p pObj) {
	//KASSERT(g_GC.isScanModeLocked() || fastiva_getCurrentTask() == kernelData.g_pSystemTask);
	KASSERT(pObj != FASTIVA_NULL);
	GC_DEBUG_BREAK(pObj);
	if (!fox_GC::isLocalReachable(pObj)) {
		// reachable-marking �� ���� ���� thread���� ���ÿ� �õ��� �� �ִ�.
		// marking ������ Ȯ���Ͽ��� �Ѵ�.
		fox_GC::markLocalReachable(pObj);
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_markLocalReachable, 0);
			return;
		}

		fastiva_Scanner scanQ(pObj);
		while (pObj != NULL) {
			pObj->scanInstance$(_markLocalReachable, &scanQ);
			pObj = pObj->m_pNextScan$;
		}
	}
	return;
}

static void _derefLocalRef(fastiva_Instance_p pObj, fastiva_Scanner* pScanQ) {
	if (pObj == FASTIVA_NULL) {
		return;
	}
	GC_DEBUG_BREAK(pObj);
	KASSERT(GC_ENABLE_LOCAL_REF);

	if (!fox_GC::isPublished(pObj)
	&&  --pObj->m_localRef$ == 0
	&&  !fox_GC::isFinalizable(pObj)) {
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_derefLocalRef, 0);
		}
		else {
			pScanQ->add(pObj);
		}
	}
	return;
}

void fm::gc_derefLocalRef(fastiva_Instance_p pObj) {
	//KASSERT(g_GC.isScanModeLocked() || fastiva_getCurrentTask() == kernelData.g_pSystemTask);
	KASSERT(pObj != FASTIVA_NULL);
	GC_DEBUG_BREAK(pObj);
	KASSERT(GC_ENABLE_LOCAL_REF);

	if (!fox_GC::isPublished(pObj)
	&&  --pObj->m_localRef$ == 0
	&&  !fox_GC::isFinalizable(pObj)) {
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_derefLocalRef, 0);
			return;
		}

		fastiva_Scanner scanQ(pObj);
		while (pObj != NULL) {
			pObj->scanInstance$(_derefLocalRef, &scanQ);
			pObj = pObj->m_pNextScan$;
		}
	}
	return;
}


static void _markPublished(fastiva_Instance_p pObj, fastiva_Scanner* pScanQ) {
	if (pObj == FASTIVA_NULL) {
		return;
	}
	GC_DEBUG_BREAK(pObj);

	if (!fox_GC::isPublished(pObj)) {
		fox_GC::markPublished(pObj);
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_markPublished, 0);
		}
		else {
			pScanQ->add(pObj);
		}
	}
	return;
}

void fm::gc_markPublished(fastiva_Instance_p pObj) {
	//KASSERT(g_GC.isScanModeLocked() || fastiva_getCurrentTask() == kernelData.g_pSystemTask);
	//KASSERT(!fox_GC::isPublished(pObj));

	GC_DEBUG_BREAK(pObj);
	KASSERT (pObj != FASTIVA_NULL);

	if (!fox_GC::isPublished(pObj)) {
		fox_GC::markPublished(pObj);
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_markPublished, 0);
			return;
		}

		fastiva_Scanner scanQ(pObj);
		while (pObj != NULL) {
			pObj->scanInstance$(_markPublished, &scanQ);
			pObj = pObj->m_pNextScan$;
		}
	}
	return;
}


static void _markPublicReachable(fastiva_Instance_p pObj, fastiva_Scanner* pScanQ) {
	if (pObj == FASTIVA_NULL) {
		return;
	}
	GC_DEBUG_BREAK(pObj);

	if (!fox_GC::isPublishedReachable(pObj)) {
		fox_GC::markPublishedReachable(pObj);
		if (USE_RECURSIVE_STACK) {
			pObj->scanInstance$(_markPublicReachable, 0);
		}
		else {
			pScanQ->add(pObj);
		}
	}
	return;
}

void fm::gc_markPublicReachable(fastiva_Instance_p pObj) {
	KASSERT(pObj != FASTIVA_NULL);
	//KASSERT(!fox_GC::isPublishedReachable(pObj));
	GC_DEBUG_BREAK(pObj);

#ifdef _DEBUG_TRACE
	if (g_GC.getScanMode() < TASK_SCANNING) {
		fox_printf("gc_markPublicReachable in non-scan-mode %d\n", g_GC.getScanMode());
		fox_exit(-1);
	}
#endif

	if (!fox_GC::isPublishedReachable(pObj)) {
		if (USE_RECURSIVE_STACK) {
			fox_GC::markPublishedReachable(pObj);
			pObj->scanInstance$(_markPublicReachable, 0);
			return;
		}
		fox_semaphore_lock(g_GC.pScanNextLock);
		fox_GC::markPublishedReachable(pObj);
		fastiva_Scanner scanQ(pObj);
		while (pObj != NULL) {
			pObj->scanInstance$(_markPublicReachable, &scanQ);
			pObj = pObj->m_pNextScan$;
		}
		fox_semaphore_release(g_GC.pScanNextLock);
	}

#ifdef _DEBUG_TRACE
	if (g_GC.getScanMode() < TASK_SCANNING) {
		fox_printf("gc_markPublicReachable in non-scan-mode %d\n", g_GC.getScanMode());
		fox_exit(-1);
	}
#endif

}


void fm::gc_markFinalizerReachable(fastiva_Instance_p pObj) {
	KASSERT(pObj != FASTIVA_NULL);
	//KASSERT(!fox_GC::isPublishedReachable(pObj));
	GC_DEBUG_BREAK(pObj);

	int mark = pObj->m_mark$;
	fastiva_Instance_p pTopObj = pObj;
	// pObj �� ScanNext �� �� ��ϵǴ� ������ �����Ѵ�.
	fox_GC::markPublishedReachable(pObj);

	if (USE_RECURSIVE_STACK) {
		pObj->scanInstance$(_markPublicReachable, 0);
	}
	else {
		fox_semaphore_lock(g_GC.pScanNextLock);
		fastiva_Scanner scanQ(pObj);
		while (pObj != NULL) {
			pObj->scanInstance$(_markPublicReachable, &scanQ);
			pObj = pObj->m_pNextScan$;
		}
		fox_semaphore_release(g_GC.pScanNextLock);
	}
	pTopObj->m_mark$ = mark;

}


