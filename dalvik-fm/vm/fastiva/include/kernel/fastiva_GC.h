#include <kernel/Kernel.h>
#include <kernel/HeapMark.h>
#include <fox/Task.h>
#include <fox/Atomic.h>
#include <fox/Heap.h>
#include <fox/Mutex.h>
#include <fox/RWLock.h>
#include <fox/Monitor.h>
#include <kernel/Runnable.h>

#include <java/util/Vector.inl>
#include <java/lang/Throwable.inl>
#ifndef FASTIVA_CLDC
	#include <java/io/FileDescriptor.inl>
#endif


#ifdef FASTIVA_NO_MULTI_CPU_SUPPORT
	#define GC_IDLE_PRIORITY	0
#else
	#define GC_IDLE_PRIORITY	0
#endif

#define GC_ENABLE_LOCAL_REF		0
#ifdef _DEBUG
	#define DEBUG_LOCAL_REF_AS_GC_GENERATION	(!GC_ENABLE_LOCAL_REF)
#else
	#define DEBUG_LOCAL_REF_AS_GC_GENERATION 1
#endif

#define FASTIVA_ENABLE_FINALIZE
//#undef KASSERT
//#define KASSERT(t)	if (!(t)) { fox_puts(#t); FOX_DEBUG_BREAK(); }

static inline void fox_non_printf(...) {}
#define k_debug_puts(t)		fox_debug_puts(t)
#define k_debug_printf		fox_debug_printf

extern fastiva_Instance_p DEBUG_INSTANCE;
#ifdef _DEBUG
	#define GC_DEBUG_BREAK(ptr)	if ((void*)ptr == DEBUG_INSTANCE) {FOX_DEBUG_BREAK(); }
	#define GC_UNLINK_BREAK(ptr)	if ((void*)ptr == DEBUG_INSTANCE) { FOX_DEBUG_BREAK(); }
#else
	#define GC_DEBUG_BREAK(ptr)		// ignore
	#define GC_UNLINK_BREAK(ptr)	// if ((void*)ptr == DEBUG_INSTANCE) { FOX_DEBUG_BREAK(); }
#endif

#if 0
void FOX_FASTCALL(fox_scheduler_boostPriority)(fastiva_Task* pTask);
void FOX_FASTCALL(fox_scheduler_normalize)(fastiva_Task* pTask);
#else //v 3
#define fox_scheduler_boostPriority(T)	FASTIVA_DBREAK()
#define fox_scheduler_normalize(T)		FASTIVA_DBREAK()
#endif

#define FASTIVA_SUPPORT_WEAK_REF

#define DEC_LOCAL_REF			SCAN_TYPE_A	 
#define MARK_PUBLISHED			SCAN_TYPE_B	 
#define MARK_STRONG_REACHABLE	SCAN_TYPE_C	 
#define MARK_PUBLIC_REACHABLE	SCAN_TYPE_D	 


// #define ENABLE_FINALIZER_TASK_CACHE
// finalize() ȣ��� task�� �̸� ����� �ΰ�, wait ���·� ���� ��,
// finalizable-Q�� �غ�Ǹ� ������ �����ϴ� ����̴�. 
// monitor-locking ������ ������ �̼��ϳ��� �ӵ��� ���Ұ� ������,
// resource ���࿡�� ������ ���� �ʴ´�.
// �� option�� ������� �ʴ´�. (2006.02/22)

struct QueueHeader : public fastiva_Instance_T$ {
	fastiva_Instance_p m_pLastLocal$;
	fastiva_Instance_p m_pFirstLost;
	fastiva_Instance_p m_pLastLost;
};

struct LocalQueueHeader : public QueueHeader {
	fastiva_Instance_p m_pFirstPublished;
	fastiva_Instance_p m_pLastPublished;
};

struct FinalizeInfo {
	fastiva_Instance_p m_pFirstLost;
	fastiva_Instance_p m_pLastLost;
	fastiva_Instance_p m_pFinalizableQ;
};

enum ScanMode {
	PUBLIC_SCANNING= 1,
	TASK_SCANNING = 0,
	SOFT_REF_SCANNING= 2,
	SCAN_FINISHED = -1,
	GC_IDLE = -2,
};

class fox_GC { //: public Fastiva {
private:
	
	java_lang_Class_p pClassFileDesc;
	fastiva_Instance_p pPublishedNFQ;
//	friend class fastiva_Prototype;
//	friend class fm::ArrayClass;
//	friend class fm::PropertyArrayClass;
#if 0
	friend class fm::PropertiesClass;
#endif

	int cntRookie;
	int cntPublic;

	int scanPriority;
	int totalScanPri;
	int cntPri;

	//fastiva_Task* pScanningTask;
	//int minRefCount;

public:
	static void FOX_FASTCALL(unlinkHeapInstance)(fastiva_Instance_p pObj);
//	static void FOX_FASTCALL(unlinkHeapInstanceEx)(fastiva_Instance_p pObj);
private:
	static void initHeapSlot();

	static void FOX_FASTCALL(realignSlotTable)(jbool removeEmptyTable);

	static fastiva_Instance_p getLastInstance(fastiva_Instance_p pObj);

	//typedef void (FOX_FASTCALL(*FIELD_VISITOR))(fastiva_Instance_p pObj);
	//typedef void (FOX_FASTCALL(*FIELD_VISITOR_EX))(fastiva_Instance_p pObj, int param);
	//typedef void (FOX_FASTCALL(*FIELD_VISITOR_EX3))(fastiva_Instance_p pObj, int param1, int param2);


public:
	static jbool FOX_FASTCALL(scanLocalStack)(fastiva_Task* hTask, bool isLocal);

	static void FOX_FASTCALL(call_finalize)(
		fastiva_Task* pScanTask
	);


private:
	static void FOX_FASTCALL(adjustLocalScanContext)(fastiva_ExceptionContext* pTrap);
	static jbool FOX_FASTCALL(clearLocalQ)(
		fastiva_Task* pTask, fastiva_Instance_p pLocalQ, fastiva_Instance_p FinalizableQ, bool isLocal);

	static int clearLocalQ(LocalQueueHeader* pHeader, bool unlinkNow);
	static void clearPublicQ(QueueHeader* pHeader, bool unlinkNow);
	static void FOX_FASTCALL(clearPhantomQ)();


	static fastiva_Instance_p FOX_FASTCALL(scanPublicInstance)(fastiva_Instance_p pObj);

	static int  FOX_FASTCALL(demarkQueue)(fastiva_Instance_p pFirst);
	static void FOX_FASTCALL(removeGarbagePackage)();

	static void FOX_FASTCALL(finalize_InstanceQ)(fastiva_Instance_p pObjQ);
	static void FOX_FASTCALL(shiftGeneration)(fastiva_Instance_p pObj, int target_G, int CURR_G);
	
	static void markPackage(fastiva_Package* pPackage);

	/*
	static void FOX_FASTCALL(visitFieldsEx)(
		fastiva_Instance_p pObj, 
		int param, 
		FIELD_VISITOR_EX pfnVisitor
	);

	static void FOX_FASTCALL(visitFieldsEx3)(
		fastiva_Instance_p pObj, 
		int ref_delta, 
		int param, 
		FIELD_VISITOR_EX3 pfnVisitor
	);
*/


	static void FOX_FASTCALL(addPublished)(fastiva_Instance_p pObj);
	static void FOX_FASTCALL(addPublishedNF)(fastiva_Instance_p pObj);
/*
	static void FOX_FASTCALL(markReachableInstanceTree)(fastiva_Instance_p pObj);
	static void FOX_FASTCALL(markPublishedBabyTree)(fastiva_Instance_p pObj);
	static void FOX_FASTCALL(markPublishedReachableBabyTree)(fastiva_Instance_p pObj);
	static void FOX_FASTCALL(markPublishedTree)(
		fastiva_Instance_p pObj, 
		int ref_delta,
		int mark_flag
	);
	static void FOX_FASTCALL(markPublishedReachableTree)(
		fastiva_Instance_p pObj,
		int ref_delta,
		int mark_flag
	);
	static void FOX_FASTCALL(derefLocalTree)(
		fastiva_Instance_p pObj
	);
	static void FOX_FASTCALL(markReachableTree)(fastiva_Instance_p pObj);
	static void FOX_FASTCALL(markReferentTouchedTree)(fastiva_Instance_p pObj);

*/

	// finalizing
	
	static void FOX_FASTCALL(addPublicLost)(
		fastiva_Instance_p pFirstObj
	);
	
	static int FOX_FASTCALL(getFinalizableList)(
		finalize_Task* pCurrTask,
		FinalizeInfo* pInfo
	);
	
	static void FOX_FASTCALL(markFinalizing)(
		fastiva_Task* pTask
	);

	static void FOX_FASTCALL(runFinalize)(
		void* param
	);

	static void* FOX_FASTCALL(validateReferent)(
		class java_lang_ref_Reference* pObj,
		bool markNow
	);

	static void FOX_FASTCALL(markSoftReferent)(
		fastiva_Instance_p pObj
	);

public:

	fastiva_Instance_p* aGlobalSlot;
	enum { sizGlobalSlot = 512 * 1024};
	int freeGlobalSlotOffset;
	int globalListLock;
	fm::SafeInstanceQ phantomQ;
	fastiva_Instance_p pPublishedQ;

	uint nextGC_T;
	int cntAllocated;
	int cntUnlink;
	static int cntGC;
	static ScanMode m_scanMode;
	fox_Semaphore* pScanNextLock;
	int pfnPsuedoFinalize;

	class java_util_Vector* pJniLockedV;

	static void FOX_FASTCALL(lockGlobalList)();
	static void FOX_FASTCALL(releaseGlobalList)();

	static void FOX_FASTCALL(lockGlobalObj)(fastiva_Instance_p pObj);
	static void FOX_FASTCALL(releaseGlobalObj)(fastiva_Instance_p pObj);

	fox_Monitor pGCFinished[1];
	fox_Monitor pTrigger[1];
	fox_Monitor pFinalizeTaskMonitor[1];
	fox_RWLock* pScanModeLock;
	fm::SafeInstanceQ referenceQ;
	fm::SafeInstanceQ softRefQ;
	fastiva_Task* pCurrentScanningTask;
	static int cntPublishedRookie;
	int cntLocalTrigger;
	int isGCTriggered;
	//int stackBuffer[FOX_STACK_SIZE/sizeof(int) + 64];

	void lockScanMode(FOX_BOOL forWrite) {
		if (forWrite) {
			fox_rwlock_lockWrite(pScanModeLock);
		}
		else {
			fox_rwlock_lockRead(pScanModeLock);
		}
	}

	void releaseScanMode(FOX_BOOL forWrite) {
		if (forWrite) {
			fox_rwlock_releaseWrite(pScanModeLock);
		}
		else {
			fox_rwlock_releaseRead(pScanModeLock);
		}
	}

	FOX_BOOL isScanModeLocked() {
		return !fox_rwlock_isReleased(pScanModeLock);
	}

	bool isCurrentTaskInScanning() {
		return fastiva_getCurrentTask() == pCurrentScanningTask;
	}

	static void registerLocalInstance(fastiva_Task* pTask, fastiva_Instance_p pRookie, int size);

	static void clearReference(
		fastiva_Instance_p pFirstSoftReference, 
		fastiva_Instance_p pFirstReference,
		bool removeSoftRef
	);

	ScanMode getScanMode() { 
		KASSERT(isScanModeLocked());
		return m_scanMode; 
	}

	void setScanMode(ScanMode mode) { 
		KASSERT(isScanModeLocked());
		m_scanMode = mode; 
	}

	void init();

	static bool FOX_FASTCALL(addFinalizing)(
		fastiva_Task* pScanTask, 
		fastiva_Instance_p pObj,
		fastiva_Instance_p pLastObj
	);

	static bool FOX_FASTCALL(addLost)(
		fastiva_Task* pScanTask, 
		fastiva_Instance_p pFirstLost,
		fastiva_Instance_p pLastLost
	);

	int getScanPriority() {
		return scanPriority;
	}

	int calcScanPriority() {
		int pri = this->totalScanPri / this->cntPri;
		if (pri < 9) {
			return pri + 1;
		}
		else {
			return 9;
		}
	}

	void adjustScanPriority(int pri) {
		/* �ܼ��� ��� task�� ��� priority�� ���ϴ� ������δ� 
		scan-priority�� ���� �� ����. main-task���� ��κ��� 
		task�� IDLE ������ ��찡 ���� �����̴�.
		*/
		cntPri ++;
		totalScanPri += pri;
		/*
		if ((cntPri & 0xFFF) == 0) {
			int scan_pri = getScanPriority();
			if (scan_pri > Kernel::g_pSystemTask->m_priority) {
				fox_monitor_lock(this->pTrigger);
				boostScanPriority(scan_pri);
				fox_monitor_release(this->pTrigger);
			}
		}
		*/
	}

	void resetScanPriority() {
		cntPri = 1; // divide by zero ����.
		totalScanPri = GC_IDLE_PRIORITY;
		scanPriority = GC_IDLE_PRIORITY;
	}


	void boostScanPriority(int pri) {
		// assert g_GC.pTrigger is locked.
		if (this->scanPriority < pri) {
			this->scanPriority = pri;
		}
		if (Kernel::g_pSystemTask->m_priority < pri) {
			fox_task_setPriority(Kernel::g_pSystemTask, pri);
		}
	}


	/*
	void resumeScanning(fastiva_Task* pCurrTask) {
		int task_pri = pCurrTask->m_priority;
		if (task_pri < 5) {
			task_pri = 5;
		}
		if (this->scanPriority < task_pri) {
			fox_monitor_lock(pTrigger);
			if (this->scanPriority < task_pri) {
				this->scanPriority = task_pri;
				if (Kernel::g_pSystemTask->m_priority < task_pri) {
					// priority-10 �� GC task�� priority�� ������ 
					// ���� ���ɼ��� ������, Ư���� �������� �����Ƿ� �����Ѵ�.
					fox_task_setPriority(Kernel::g_pSystemTask, task_pri);
				}
				fox_monitor_notify(pTrigger);
			}
			fox_monitor_release(pTrigger);
		}

		*
		if (this->requested_G < 0) {
			fox_monitor_lock(pTrigger);
			// GC_task�� priority�� �������� �ʴ´�.
			int dirtyG  = 0;
			if (requested_G < dirtyG) {
				//fox_monitor_lock(g_GC.pTrigger);
				if (requested_G < 0) {
					requested_G = dirtyG;
					fox_monitor_notify(pTrigger);
				}
				else {
					requested_G = dirtyG;
				}
				//fox_monitor_release(g_GC.pTrigger);
			}
			fox_monitor_release(pTrigger);
		}
		*
	}
	*/
	
	static void executeGC();

	// ���߿� private���� �ű��.
	static fastiva_Instance_p getHeapInstance(fastiva_Instance_p pUnknown);

	/*
	static HeapMark* getHeapMark(fastiva_Instance_p pObj) {
		return ((HeapMark*)(void*)&pObj->m_marked);
	}

	static uint getHeapSlotID(fastiva_Instance_p pObj) {
		return (pObj->m_marked & HEAP_SLOT_ID_MASK);
	}


	static fox_HeapSlot* getHeapSlot(fastiva_Instance_p pObj) {
		fox_HeapSlot * pSlot = getHeapSlotEx(getHeapSlotID(pObj));
		KASSERT(pSlot->m_pObject == pObj);
		return pSlot;
	}
	*/

	/*
	static void FOX_FASTCALL(visitFields)(
		fastiva_Instance_p pObj, 
		FIELD_VISITOR pfnVisitor
	);
	*/
	


	/**************************************************************************************/
	// heap-marking 
	/**************************************************************************************/

	static void FOX_FASTCALL(publishInstance)(
		fastiva_Instance_p pNewValue
	);


	static void markIfScanning(fastiva_Instance_p pObj);


	static bool isPublished(fastiva_Instance_p pObj) {
		return ((short)pObj->m_mark$) < 0;
	}

	static void markPublished(fastiva_Instance_p pObj) {
		KASSERT(!isPublished(pObj));
		// local-insance �̹Ƿ� thread-safe �ϴ�.
		pObj->m_mark$ |= HM_PUBLISHED;
		cntPublishedRookie ++;
	}

	static void markPublishedReachable(fastiva_Instance_p pObj) {
		if (DEBUG_LOCAL_REF_AS_GC_GENERATION && !isReachable(pObj)) {
			pObj->m_localRef$ = cntGC;
		}
        if (!isPublished(pObj)) {
		    cntPublishedRookie ++;
        }
		pObj->m_mark$ |= HM_PUBLISHED | HM_STRONG_REACHABLE;
	}

	static FOX_BOOL isPublishedReachable(fastiva_Instance_p pObj) {
		int hm = HM_PUBLISHED | HM_STRONG_REACHABLE;
		return (pObj->m_mark$ & hm) == hm;
	}


	void demarkPublished(fastiva_Instance_p pObj) {
		// addPublicLost���� ȣ��ȴ�. Unreachable�̹Ƿ� thread-safe.
		// ��, �� ���� markFinalizing�� ���� ȣ��� �����̰ų�,
		// public-finalizer task�� suspend�� �����̾�� �Ѵ�.
		// re-assign�� ���� publishing�� ����ǰ� �ְų� �� �� �Ŀ�
		// published-mark�� �����ؼ��� �ȵȴ�.
		pObj->m_mark$ &= ~HM_PUBLISHED;
	}

	void markReferentTouched(fastiva_Instance_p pObj) {
		markStrongReachable(pObj);
	}

	bool isStackTouched(fastiva_Instance_p pObj) {
		return *(unsigned char*)&pObj->m_mark$ & 1;
	}

	static void markStackTouched(fastiva_Instance_p pObj) {
		//GC_DEBUG_BREAK(pObj);
		*(unsigned char*)&pObj->m_mark$ |= 1;
	}

	static int isReachable(fastiva_Instance_p pObj) {
		int reachable = pObj->m_mark$ & HM_STRONG_REACHABLE;//== (unsigned char)currentMark;
		if (DEBUG_LOCAL_REF_AS_GC_GENERATION) {
			//KASSERT(!reachable || pObj->m_localRef$ == cntGC || pObj->getClass$() == java_lang_Class_C$::getRawStatic$()) ;
		}
		return reachable;
	}

	static int isSoftReachable(fastiva_Instance_p pObj) {
		int reachable = pObj->m_mark$ & HM_SOFT_REACHABLE;//== (unsigned char)currentMark;
		return reachable;
	}

	static void markSoftReachable(fastiva_Instance_p pObj) {
		//GC_DEBUG_BREAK(pObj);
		// marking �ϴ� ���� �����ϹǷ� thread �浹�� �־ �����ϴ�.
		//KASSERT(isPublished(pObj) ? m_scanMode >= TASK_SCANNING : m_scanMode == TASK_SCANNING);
		*(unsigned char*)&pObj->m_mark$ |= HM_SOFT_REACHABLE;//(unsigned char)currentMark;
		//if (DEBUG_LOCAL_REF_AS_GC_GENERATION) {
		//	pObj->m_localRef$ = cntGC;
		//}
	}

	static FOX_BOOL isLocalReachable(fastiva_Instance_p pObj) {
		return pObj->m_mark$ & (HM_PUBLISHED | HM_STRONG_REACHABLE);
	}

	static void markLocalReachable(fastiva_Instance_p pObj) {
		KASSERT(!isPublished(pObj) && m_scanMode != TASK_SCANNING);
		*(unsigned char*)&pObj->m_mark$ |= HM_STRONG_REACHABLE;//(unsigned char)currentMark;
	}

	static void markStrongReachable(fastiva_Instance_p pObj) {
		//GC_DEBUG_BREAK(pObj);
		// marking �ϴ� ���� �����ϹǷ� thread �浹�� �־ �����ϴ�.
		KASSERT(isPublished(pObj) ? (1 || m_scanMode >= TASK_SCANNING) : m_scanMode == TASK_SCANNING);
		*(unsigned char*)&pObj->m_mark$ |= HM_STRONG_REACHABLE;//(unsigned char)currentMark;
		if (DEBUG_LOCAL_REF_AS_GC_GENERATION) {
			pObj->m_localRef$ = cntGC;
		}
	}

	static void demarkReachable(fastiva_Instance_p pObj) {
#ifdef _DEBUG
		*(unsigned char*)&pObj->m_mark$ &= HM_SOFT_REACHABLE;
#else
		*(unsigned char*)&pObj->m_mark$ = 0;
#endif
		if (DEBUG_LOCAL_REF_AS_GC_GENERATION) {
			pObj->m_localRef$ = cntGC;
		}
	}

	static int demarkReachableQ(fastiva_Instance_p pObj) {
		int cnt = 0;
		while (pObj != ADDR_ZERO) {
			demarkReachable(pObj);
			pObj = pObj->m_pNext$;
			cnt ++;
		}
		return cnt;
	}

	static int isPhantomRefReachable(fastiva_Instance_p pObj) {
		return pObj->m_mark$ & HM_PHANTOM_REF_REACHABLE;
	}

	static void markPhantomRefReachable(fastiva_Instance_p pObj) {
		KASSERT(!isReachable(pObj));
		// public-lost instance�̹Ƿ� thread-safe �ϴ�.
		pObj->m_mark$ |= HM_PHANTOM_REF_REACHABLE;
	}

	int demarkPhantomRefReachable(fastiva_Instance_p pObj) {
		// finalize() ȣ�� �� �Ŀ� �˻� �ȴ�. scanLocalStack�� 
		// runFinalize() ������ ���� task->disableSuspend() ��
		// ȣ���� �����̹Ƿ�, local-scanning�� ���� ���� �����̴�.
		// thread-safe �ϴ�. 
		if (isPhantomRefReachable(pObj)) {
			pObj->m_mark$ &= ~HM_PHANTOM_REF_REACHABLE;
			return true;
		}
		return false;
	}

	static int isFinalizable(fastiva_Instance_p pObj) {
		return (pObj->m_mark$ & HM_FINALIZABLE);
	}

	void demarkFinalizable(fastiva_Instance_p pObj) {
		// finalize-task���� finalize() ȣ�� ������ ȣ��ȴ�.
		pObj->m_mark$ &= ~(HM_FINALIZABLE | HM_PUBLISHED);
	}

	void markFinalizable(fastiva_Instance_p pObj) {
		// allocInstance ���ο����� ȣ��Ǹ�, Local-Q�� 
		// �ش� instance �� ��ϵǱ� ���� ȣ��ǹǷ� thread-safe�ϴ�.
		pObj->m_mark$ |= HM_FINALIZABLE;
	}

	int getGlobalRef(fastiva_Instance_p pObj) {
		return pObj->m_globalRef$;
	}


	fastiva_Instance_p* getFreeGlobalSlot() {
		return (fastiva_Instance_p*)((char*)aGlobalSlot + freeGlobalSlotOffset);
	}

	fastiva_Instance_p* popFreeGlobalSlot() {
		fastiva_Instance_p* pSlot = (fastiva_Instance_p*)
			((char*)aGlobalSlot + freeGlobalSlotOffset);
		this->freeGlobalSlotOffset += sizeof(void*);
		return pSlot;
	}

	
	static void registerGlobal(fastiva_Instance_p pObj);

	static void unregisterGlobal(fastiva_Instance_p* pGlobalSlot);

	static int getMonitorOffset() {
		return (int)&((fastiva_Instance_p)1000)->m_monitor$ - 1000;
	}

	static void doLocalGC(fastiva_Task* pCurrTask);
	static fastiva_Task* tryLocalGC();

};

extern fox_GC g_GC;
#define ROOT_G 7

