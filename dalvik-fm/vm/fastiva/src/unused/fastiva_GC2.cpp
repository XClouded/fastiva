#include <precompiled_libcore.h>

// ================================================================================================
// 2011.0913 웹이란 클자를 콕 넣어 주시압? 
// 한글 주석이 있는 경우, Visual Studio 가 원본 소스가 변경되었다고 디버깅시 소스를 찾지 못하는 문제있슴.
// '웹'이란 글자를 넣은 주석을 소스 맨 앞에 넣어주면 UTF8이 아니란 것을 자동(?) 인식한다.
// 근본적인 해결은 소스 자체를 UTF8 로 인코딩을 바꿔버리기...
// ================================================================================================

#include <kernel/fastiva_GC.h>
#include <java/lang/ref/Reference.h>
#include <java/lang/ref/WeakReference.h>
#include <java/lang/ref/ReferenceQueue.h>
#include <fox/kernel/sys_heap.h>

#if FASTIVA_SUPPORTS_JAVASCRIPT
#include "../interpreter/JsScriptEx.h"
#include <com/wise/jscript/JsActivation.h>
#endif

#ifdef _DEBUG
	#define IF_DEBUG	if (true) 
#else
	#define IF_DEBUG	if (false) 
#endif

#define SUSPEND_TASK_WHILE_STACK_SCAN  1 // pthread 는 thread-suspend 를 제공하지 않는다.
#define TRIGGERED_BY_SELF  10

#define MARK_FINALIZING 

jlonglong gc_start_t;
extern "C" FOX_LONGLONG FOX_FASTCALL(currentTimeMillis_MD)();

fox_GC g_GC;
static fox_Semaphore* s_pPublicQ_lock;
#ifdef UNLINK_GARBAGE_AFTER_SCAN
	static fastiva_Task* g_triggerTask = 0;
#else
	static const fastiva_Task* g_triggerTask = 0;
#endif


int fox_GC::cntGC = 1;
ScanMode fox_GC::m_scanMode;
#ifdef _DEBUG
	#define GC_DEBUG
#elif defined(_DEBUG_TRACE)
	#define GC_DEBUG
#else 
	//#define GC_DEBUG
#endif

#ifdef GC_DEBUG
	#define gc_debug_printf fox_printf
	#define IF_GC_DEBUG	if (true) 
#else
	#define gc_debug_printf fox_debug_printf
	#define IF_GC_DEBUG	if (false) 
#endif



//#define gc_verbose_printf  fox_debug_printf
static __inline void gc_verbose_printf(...) {};//

#ifdef GC_DEBUG
	//extern int g_cntMemoryReadErr;
	//extern int g_cntSpin;
	//extern int g_cacheTypeCheckHit;
	//extern int g_cntHeapBlock;
	fastiva_Instance_p DEBUG_OBJ = (fastiva_Instance_p)(0x28b1db79);
	fastiva_Task* DEBUG_TASK;
	int g_cntWeakReachable = 0;
#endif
	//int g_cntMarked = 0;
static int s_cntRefUnlink = 0;
int g_cntFinalizeTask = 0;
int g_debugRookieMark = 0x000;
int fox_GC::cntPublishedRookie = 0;

#ifdef USE_SPIN_LOCK
	#ifdef ANDROID
		#error no support spin lock in linux
	#endif
#else
	fox_Semaphore* g_globalListLock;
	static fox_Semaphore* g_globalStaticLock;
#endif


#define FASTIVA_SUPPORT_FINALIZE
#define FASTIVA_NO_REACHABLE_MARKING_AFTER_LOCAL_SCAN
#define GC_INTERVAL		100*1000 // 10초

static FOX_BOOL sys_stack_lock(fastiva_Task* pTask)	{
	/**
	  시스템 태스크의 프라이오리티를 10(TIMECRITICAL)으로 설정하지 말것.
	  디버깅 시스템을 죽이는 문제 발생.
	*/

	KASSERT(pTask != fox_task_currentTask());
	int cntLoop = 100;
	while (pTask->m_scanState == IN_NEED_SCAN) {
		pTask->enterCriticalSection();
		if (pTask->isTerminated()) {
			return true;
		}
		FOX_BOOL res = true;
#ifdef _WIN32
		if (!pTask->m_gcInterrupted) {
			res = fox_task_suspend(pTask);
			return true;
		}
#endif
		if (pTask->m_pStackContext != 0 && pTask->m_pStackContext->m_pStack == 0) {
			pTask->leaveCriticalSection();
			// interrupt 처리가 되지 않았을 수 있다.
			// sys_task_interruptTask(pTask);
#ifdef _WIN32
			::Sleep(10);
#else
			usleep(10*1000);
#endif
			if (++cntLoop > 10) {
				fox_printf("local-thread running");
				cntLoop = 0;
			}
		}
		else {
			return true;
		}
	}
	return false;
}

static inline void sys_stack_release(fastiva_Task* pTask) {
	if (!pTask->m_gcInterrupted) {
		fox_task_resume(pTask);
	}
	else {
		//pTask->releaseGcStack();
	}
	pTask->leaveCriticalSection();
}


#ifdef __SCANNING_MODEL
1. Public-Scan을 Local-Scan 보다 먼저 한다.
	Public-Scan이 끝난 또는 그 중간에 ref-tree가 변경되는 경우,
	해당 ref-tree도 Public-Reachable 로 marking한다.
	즉, Public-Scan 이 끝난 상태에서는, 모든 Public-Reachable은
	항상 valid한 HeapMark를 가지고 있다.
	Public-Scan 이 끝난 상태에서 Local-Stack에 남아있는 Public-Instance는 
	반드시 다른 Public-Instance-RefTree에 assign되어야만 다른 Local-Stack으로
	이전될 수 있으므로, Rechable-Marking없이 Ref-Stack이 변경될 수 없다.

2. Local-Scan 시에는 해당 Thread를 Suspend하여야만 한다.
    Scanning 하는 Task를 Suspend하지 않을 경우,
	임의의 함수 내에서 stack에 값을 저장할 때, 
	해당 위치에 대해 이미 scnaning 끝난 상태일 수 있다.
	(참조, 아래의 설명은 Non-Suspend Local-Scan에 관한 것으로, 그 유효성을 검증한다.)
		Local-Scan 시에 de-assign되는 Object의 ref-tree도 Rechable-Marking한다.
		Local-Scan시에 stack 내용의 변경이 수시로 발생할 수 있으므로, 
		deassign된 instance도 stack내에 보관중인 것으로 간주한다.

3. Local-Scan 이 끝나고 나면,
	GC 중에 새로이 생성된 instance를 제외한 모든 instance는 정확한
	HeapMark를 유지하고 있다. 즉, Local-Scan 이 후에 Stack내용이
	수시로 변한다고 가정하여도 Stack 내에 있는 모든 Public-Instance는
	Reachable로 marking된 상태임이 보장된다.

4. 한 Task에 대한 Local-GC가 종료된 시점 당시에는 현재 stack 내에서
참조 가능한 모든 ref는 marking된 상태이다. (Unreachable 도 포함해서)

5. Local-Scan 까지 종료된 시점 이 후에는 모든 Public-Reachable은 
reachable marking되어 있음이 보장된다. (단, Local-Scan 시
FinalizeTask의 Local-stack을 scan 하지 않는다면, Finalize-stack 내에 있는 
public-instance 는 marking되지 않을 수 있다.)

#endif



void fox_GC::init() {
#ifndef USE_SPIN_LOCK
	//g_globalListLock = 0;
	g_globalListLock = fox_semaphore_create();
#endif
	memset(this, 0, sizeof(g_GC));
	g_globalStaticLock = fox_semaphore_create();
	this->m_scanMode = GC_IDLE;
	this->pScanModeLock = fox_rwlock_create();
	this->softRefQ.init(pScanModeLock);
	this->referenceQ.init(pScanModeLock);
	this->phantomQ.init(pScanModeLock);
	this->cntGC = 0;
	this->cntAllocated = 0;
	this->cntUnlink = 0;
	cntPublishedRookie = 0;
	this->cntLocalTrigger = 0;
	//this->currentMark = HM_PUBLISHED;
	this->aGlobalSlot = (fastiva_Instance_p*)sys_heap_virtualAlloc(0, sizGlobalSlot);
	this->freeGlobalSlotOffset = 0;
	this->cntPublic = 0;
//	this->scanPriority = GC_IDLE_PRIORITY;
	this->resetScanPriority();

	struct java_lang_Object_$$ : java_lang_Object {}; 
	java_lang_Object_$$ dummy;
	int finalize_slot_offset = FASTIVA_VIRTUAL_SLOT_OFFSET(java_lang_Object, void, finalize, 0, ());
	this->pfnPsuedoFinalize = ((int**)(void*)&dummy)[0][finalize_slot_offset];
	fox_printf("pfnPsuedoFinalize %x\n", pfnPsuedoFinalize);

	this->pScanNextLock = fox_semaphore_create();

	fox_monitor_init(this->pTrigger);
	fox_monitor_init(this->pGCFinished);

	this->pPublishedQ = ADDR_ZERO;
}


//extern char* g_pVirtualHeap;
fastiva_Instance_p DEBUG_INSTANCE = (fastiva_Instance_p)0x1dbc9d;
jbool FOX_FASTCALL(fastiva_GC_init)(void* pHeap, int heapSize) {
	if (!fox_heap_init(heapSize)) {
		return false;
	}
#ifdef _DEBUG
	//DEBUG_INSTANCE = (fastiva_Instance_p)((int)g_pVirtualHeap + 0x1dbc9d);
#endif
	s_pPublicQ_lock = fox_semaphore_create();
	g_GC.init();
	return true;
}


inline void fox_GC::lockGlobalList() {
#ifndef USE_SPIN_LOCK
	fox_semaphore_lock(g_globalListLock);
#else
	sys_task_spinLock(g_GC.globalListLock);
#endif
}

inline void fox_GC::releaseGlobalList() {
#ifndef USE_SPIN_LOCK
	fox_semaphore_release(g_globalListLock);
#else
	sys_task_release_spinLock(g_GC.globalListLock);
#endif
}

FOX_BOOL FOX_FASTCALL(fox_heap_checkObject)(void* ptr);

#if FASTIVA_SUPPORT_JNI_STUB

extern bool fastiva_jni_DeleteWeakGlobalRef(void *env0, fastiva_Instance_p pObj);

void fastiva_GC_jni_clearWeakGlobalRef(void* env0) {
	g_GC.lockGlobalList();
	int cntGlobalSlot = g_GC.freeGlobalSlotOffset / sizeof(void*);
	fastiva_Instance_p* pGlobalSlot = g_GC.aGlobalSlot + cntGlobalSlot;
	int cntDelete = 0;
	for (; cntGlobalSlot-- > 0; ) {
		pGlobalSlot--;
		fastiva_Instance_p pObj = *pGlobalSlot;
		if (pObj->m_javaRef$ != 0) {
			if ((pObj->m_globalRef$ & ~0x8000) >= 0x1
			&&  fastiva_jni_DeleteWeakGlobalRef(env0, pObj)) {
				pObj->m_globalRef$ -= 0x1;
				cntDelete ++;
			}
		}
	}
	g_GC.releaseGlobalList();
	if (cntDelete <= 1024) {
		fastiva_GC_doGC(true);
	}
}
#endif

void fox_GC::registerGlobal(fastiva_Instance_p pObj) {
#ifndef USE_SPIN_LOCK
	KASSERT(fox_semaphore_isLocked(g_globalListLock));
#else
	KASSERT(g_GC.globalListLock != 0);
#endif
	KASSERT(fox_heap_checkObject(pObj) || pObj->getClass$() == java_lang_Class_C$::getRawStatic$());
	int gref = pObj->m_globalRef$;
	if (gref != 0) {
		// already registered.
		pObj->m_globalRef$ = ++gref;
		return;
	}
		
	/*
	if (g_GC.freeGlobalSlotOffset >= g_GC.sizGlobalSlot) {
		void** pNewSlot = (void**)fox_heap_malloc(g_GC.sizGlobalSlot += 64*1024);
		int* pSrc = (int*)g_GC.aGlobalSlot;
		int* pDst = (int*)pNewSlot;
		for (int i = g_GC.freeGlobalSlotOffset / sizeof(void*); i -- > 0; ) {
			*pDst ++ = *pSrc ++;
		}
		g_GC.aGlobalSlot = (fastiva_Instance_p*)pNewSlot;
	}
	*/
	fastiva_Instance_p* pFreeSlot = g_GC.popFreeGlobalSlot();
	*pFreeSlot = pObj;
	pObj->m_globalRef$ = 0x8001;

	publishInstance(pObj);
}


void fox_GC::unregisterGlobal(fastiva_Instance_p* pGlobalSlot) {
#ifndef USE_SPIN_LOCK
	KASSERT(fox_semaphore_isLocked(g_globalListLock));
#else
	KASSERT(g_GC.globalListLock != 0);
#endif
	KASSERT((*pGlobalSlot)->m_globalRef$ == 0);
	g_GC.freeGlobalSlotOffset -= sizeof(void*);
	// 마지막 Slot을 삭제된 Slot으로 옮긴다.
	void** pFreeSlot = (void**)((char*)g_GC.aGlobalSlot + g_GC.freeGlobalSlotOffset);
	*pGlobalSlot = (fastiva_Instance_p)*pFreeSlot;
	
}




void fm::detachNativeTask(fastiva_Task* pTask) {
	/*
	fastiva_Instance header[1];
	header->m_pNext$ = pTask->m_pLocalQ.removeAll();

	pTask->m_cntRookie = 0;
	if (header->m_pNext$ != ADDR_ZERO) {
		fastiva_Instance_p pObj = header->m_pNext$;
		fastiva_Instance_p pPrev = header;
		fastiva_Instance_p pLost = ADDR_ZERO;
		fastiva_Instance_p pNext;
		
		// GC 1 Cycle 이상 회전하지 못하도록 한다.
		fox_monitor_lock(pTask->m_pSuspendMonitor);

		for (; pObj != ADDR_ZERO; pObj = pNext) {
			HeapMark* pMark = fox_GC::getHeapMark(pObj);
			pNext = pObj->m_pNext$;
			if (!pMark->isPublished()) {
				pPrev->m_pNext$ = pNext;
				pObj->m_pNext$ = pLost;
				pLost = pObj;
			}
			else {
				pPrev = pObj;
				KASSERT(g_GC.getScanMode() >= TASK_SCANNING
					|| !g_GC.isReachable());
				// 현재 GC 수행 중이 아니라면, 현 Task 내에는 
				// ReachableMarking된 인스탄스가 존재할 수 없다.
				// 즉, 현재 marking된 public-instance는 현 GC 수행
				// 중에 marking된 것이다.
			}

		}

		if (header->m_pNext$ != ADDR_ZERO) {
			g_phantomQ.insert(header->m_pNext$, pPrev);
		}

		if (pLost != ADDR_ZERO) {
			fox_GC::addFinalizing(pTask, pLost, ADDR_ZERO);
		}

		fox_monitor_release(pTask->m_pSuspendMonitor);
	}
	//*/
}

int FASTIVA_DLL_API fastiva_waitExit();

static void FOX_NAKED FOX_FASTCALL(fastiva_setCurrentTask)(void* pTask) {
#ifdef _ARM_
	FASTIVA_DBREAK();
#else
	_asm mov eax, fs:[18h];
	_asm mov [eax + 14h], ecx;
	_asm ret;
#endif
};




#define ENABLE_WATCH_DOG
#ifdef ENABLE_WATCH_DOG
static int g_cntWatchDog = 1000;
static int g_maxIdleCount = 20;
static int g_enableWatchDogBreak = 2;
extern int g_ScreenRefreshed;

void FOX_FASTCALL(watch_dog_task)(void* param) {
	return;

	uint last_GC_T = 0;
	int WAIT_T = GC_INTERVAL * 3 / 2;

	//fox_task_sleep(30000);
	fox_monitor_lock(g_GC.pTrigger);

	while (kernelData.g_userThreadCount_interlocked > 0) {
		fox_monitor_wait(g_GC.pTrigger, WAIT_T);
		/*
		if (!g_ScreenRefreshed) {
			FASTIVA_DBREAK();
		}
		g_ScreenRefreshed = 0;
		/*/
		if (last_GC_T == g_GC.nextGC_T) {
			int aLocalRookie[11];
			for (int i = 11; i -- > 0; ) {
				aLocalRookie[i] = 0;
			}
			fastiva_Task* pTask = kernelData.g_pTaskQ_interlocked;
			for (; pTask != ADDR_ZERO; pTask = pTask->m_pNext0) {
				aLocalRookie[pTask->m_priority] += pTask->m_cntLocalRookie;
				pTask->m_cntLocalRookie = 0;
			}

			int scan_pri = 0;
			for (int i = 1; i < 10; i ++) {
				if (aLocalRookie[i] > aLocalRookie[scan_pri]) {
					scan_pri = i;
				}
			}

			g_GC.boostScanPriority(scan_pri);
			WAIT_T = GC_INTERVAL * 2 / 3;
		}
		else {
			last_GC_T = g_GC.nextGC_T;
			WAIT_T = GC_INTERVAL * 3 / 2;
		}
		//*/
	}
	fox_monitor_release(g_GC.pTrigger);
}
#endif

void FOX_FASTCALL(gc_task)(void* param) {
	//*
	TRY$ { // fm::kernel_initFastiva()가 성공해야만 try/catch를 사용할 수 있다.

		//Java와 C는 argument의 개수가 틀리다. 즉 cntArg-1이 java의 argument이다.
		extern void fastiva_kernel_exitFastiva(int status, bool shutdown);

		((fastiva_Task*)Kernel::g_pSystemTask)->m_trap = true;

#ifdef ENABLE_WATCH_DOG
		fastiva_Task* pWatchDogTask = (fastiva_Task*)fox_task_create(watch_dog_task, 0, 10);
		//* GC용 watch_dog_task는 반드시 GC-task와 동일한 processor에 배당되어야 한다.
		sys_task_initGCTask(pWatchDogTask);
		fox_task_resume(pWatchDogTask);
#endif
		/* v3.2006 임시로 막음
		#ifndef FASTIVA_CLDC
			g_GC.pClassFileDesc = java_io_FileDescriptor_C$::importClass$();
		#endif
		*/

		//sys_task_setCurrentTask(fox_task_currentTask());

		KASSERT(fox_task_currentTask() == Kernel::g_pSystemTask);

		fox_task_setPriority(Kernel::g_pSystemTask, GC_IDLE_PRIORITY);

		g_GC.nextGC_T = fastiva_getTickCount0() + GC_INTERVAL;

		/*
		fastiva_waitExit();
		/*/
		while (true || kernelData.g_userThreadCount_interlocked > 0) {
			// 2011.0913 TFO를 위해 임시로 종료 불가하도록 수정.
			// 아래 내용 참조.
			// WIN32 에서는 반드시 SystemThread내에서 GC가 수행되어야 한다.
			// isBadReadPtr()함수가 이에 영향을 받기 때문이다.
			fox_GC::executeGC();
		}
		//*/
		fastiva_kernel_exitFastiva(0, false);
	}
	CATCH_ANY$ {
		fox_printf("!!! Uncaught exception catched in gc_task: \n");
		fm::printError(catched_ex$);
	}
	//*/
}





void fox_GC::publishInstance(fastiva_Instance_p pObj) {
	/* 주의) publishing 이 종료된 instance를 특정 field에 assign하기
	   직전에 GC-cycle이 변경될 수 있다. 반드시,
	   Publishing 이 후에 owner와 field의 assign 관계를 확인하여야 한다.
	*/
	/* Publishing 도중 suspend 되어서는 안된다.
	   Publishing 도중 suspend 되면, 해당 ref-tree는 반드시 stack-marking이 
	   이루어지나, stack-marking 직후 해당 task 수행이 제개된 경우,
	   다음의 assembly 상에서 이전 G로 marking될 수 있다.
	   mov eax, g_GC.currentMark;
	   // 이 상태에서 suspend되고 stack amrking되면,
	   // eax 는 변경되기 전 currentMark를 가지고 있다.
	   mov pObj->m_mark, eax; -> unlink 상태가 되어버린다.

	/* 1) marking 도중엔 task가 절대 suspend되어서는 안된다.
		일부만 marking 된 instance 의 ref-tree가 unreachable 상태로 남아
		unlink 되어버리기 때문이다.
	   2) marking 도중에 task가 suspend 되어서는 아니된다.
	    suspend된 상태에서 GC-Cycle이 종료된 경우, 
	    suspend 도중에 GC-scan이 끝났음에도 계속해서
	    reachable-marking이 이루어지기 때문이다.
	   3) marking 도중에 task가 suspend 되어버리면,
	    일부 ref만 marking된 상태에서 나머지 Instance가
		unlink 처리되어 버릴 수 있다.
    */
	g_GC.lockScanMode(false);
	if (g_GC.getScanMode() < TASK_SCANNING) {// || pCurrTask->m_scanState != IN_NEED_SCAN) {
		/** 2011.0926
		task-scan 이 종료된 이후에는 marking을 하지 말아야 한다.
		해당 객체를 published-q 로 옮기지 않아 demarking 되지 않기 때문이다.
		SoftRef 는 get() 호출시에 자동 marking 되므로 현재 newValue의
		형식에는 염려할 필요가 없다.
		*/
		if (!isPublished(pObj)) {
			fastiva_Task* pCurrTask = fastiva_getCurrentTask();
			AutoLocalHeapLock __critical_section(pCurrTask);
			KASSERT(pCurrTask->m_scanState != IN_SCANNING);
			fm::gc_markPublished(pObj);
		}
	}
	else if (!isPublishedReachable(pObj)) {
		/** 2011.0926
		Scanning 중에서 CriticalSection 에 진입해 있으므로
		scanState 가 IN_SCANNING 인 상황은 발생할 수 없다.
		*/
		fastiva_Task* pCurrTask = fastiva_getCurrentTask();
		AutoLocalHeapLock __critical_section(pCurrTask);
		KASSERT(pCurrTask->m_scanState != IN_SCANNING);
		fm::gc_markPublicReachable(pObj);
#if 0
		// 2011.0111 주의. CriticalSection 내에서 scan_state 를 검사할 것!
		if (pCurrTask->m_scanState == IN_NEED_SCAN || g_GC.getScanMode() == PUBLIC_SCANNING) {
			/*
			 마킹된 객체는 현재 Task를 scan한 후 publicQ 로 옮겨진다.
			*/
			fm::gc_markPublicReachable(pObj);
		}
		else if (!isPublished(pObj)) {
			/*
			 현재 객체는 다음 GC까지 localQ 에 소속되어 있으므로,
			 마킹할 필요가 없다.
			*/
			fm::gc_markPublished(pObj);
		}
		else {
			/**
			 Scan 종료 후, 객체를 생성하였고, 이를 public 객체 field 에 assgin 한 경우,
			 해당 객체는 published 상태로 머물러 있다.
			*/
			//const fastiva_InstanceContext * pCtx = fm::getInstanceContext(pObj);
			//fox_printf("Lost public found: %s/%s(%x)\n", pCtx->getPackageName(), pCtx->m_pBaseName, *(int*)pObj);
			//fox_exit(-1);
		}
#endif
	}
	g_GC.releaseScanMode(false);
}



#ifdef SCANNING_STRATEGY
	Publishing 없이 local GC 를 하기 위한 새로운 방법.
	*) Assign 시에 cntLocalRef를 증가시키되, de-assign시에는
	cntLocalRef를 감소시키지 않고 destructor가 호출될 때에만,
	cntLocalRef를 감소시킨다. 이 방식을 사용하면, local-destruct에 의해
	cntLocalRef가 0이된 Instance는 garbage임을 확신할 수 있다.
	참고) 어떤 instance에도 assign되지 않은 rookie-object로부터
	local-destructor가 시작된다.


	Public-Scan vs. Local-Scan 
	1) stack-scan을 먼저 수행하고, public-instance에 대한 scan을 나중에 하는 경우,
	 public-instance들을 marking하는 도중에 그 ref-tree의 내용이 변경되기 때문에
	 stack 내에 참조되는 instance가 unlink될 수 있다. 이를 방지하기 위해서는
	 deref된 public-instance도 함께 marking해 주어야만 한다. 이로 인해 효율이 
	 매우 안 좋으므로 public-scan을 먼저 한다. 필요한 경우, Local-scan만을 수행한 후,
	 Published 되지 않은 Local-Lost-instance만을 제거하는 방식도 유용하다.

	 
	2) Task-scan 이 후에 생성된 Local-instance는 GC 되지 않는다.
	3) GC 수행 여부와 관계없이, Local-Instance가 Published-Instance에
	 assign될 때, 반드시 그 sub-tree를 Publishing 해주어야 한다. 
	 Publishing과 동시에 다른 Task에서도 해당 instance를 참조하는 것이 
	 가능하기 때문이다. Published Instance는 두 개 이상의 task에서
	 동시에 참조하는 것이 가능하므로, Local-GC 될 수 없다.

	<Finalizing-instance의 assign.>
	 Finalizing-instance의 assign은 GC 수행 여부와 관계없이 발생한다.
	 모든 Finalizing-Instance는 unreachable 상태이므로, Finalizer-Task의
	 Local-Instance나 Published-Instance에만 assign될 수 있다.
	 Finalizer-Task의 Local-Instance 는 unlink 가 가능하므로,
	 Published-Instance에 대한 assign만 예외처리하면 된다.
	 Finalizable-Instance를 Local-Instance로 변경하고, finalize() 이후,
	 published 되지 않은 instance를 삭제하는 방식을 이용하여
	 이 문제를 쉽게 해소할 수 있다.
#endif 


void fox_GC::markIfScanning(fastiva_Instance_p pObj) {
	KASSERT(pObj != ADDR_ZERO);
	g_GC.lockScanMode(false);
	if (g_GC.getScanMode() >= TASK_SCANNING && !g_GC.isReachable(pObj)) {

		fastiva_Task* pCurrTask = fastiva_getCurrentTask();
		/* 1) isReachable(pSelf) 검사 직후, task가 
		suspend되어 demarking되는 경우에 대비하여 suspend를 금지한다.
		*/
		AutoLocalHeapLock __critical_section(pCurrTask);
//		if (pCurrTask->m_scanState == IN_NEED_SCAN) {

			// suspend에 의해 scan 이 종료되었을 수 있으므로 scanning 여부 한 번 더 검사.
			// stack scan 시에만 pOldValue를 취급한다.
			fm::gc_markStrongReachable(pObj);
//		}
//		else {
//			/* stack scan이 종료된 이후에는 어떤 것도 marking 하지 않는다.*/
//		}
	}
	g_GC.releaseScanMode(false);
}



struct RawInterface {
	java_lang_Object_p m_pObj;
	const void** m_pIVTable;
};






void fastiva_Runtime::setInstanceField(
	fastiva_Instance_p pSelf, 
	fastiva_Instance_p pNewValue, 
	void* pField
) {
	int isInterface = false;
	fastiva_Instance_p pNewObj;

	if (*(void**)pField == pNewValue) {
		return;
	}

	if (((int)pNewValue & 0xFFFF) == 0x0410) {
		int a = 3;
	}

	GC_DEBUG_BREAK(pNewValue);
	GC_DEBUG_BREAK(pSelf);
	KASSERT(pNewValue == ADDR_ZERO || fox_heap_checkObject(pNewValue) || pNewValue->getClass$() == java_lang_Class_C$::getRawStatic$());

	if (!g_GC.isPublished(pSelf)) {
		//KASSERT(pNewValue == ADDR_ZERO || !g_GC.isReachable(pSelf) || g_GC.isReachable(pNewValue));

		/* owner가 marking되지 않은 상태이거나, 둘 다 marking된 상태라면?
		   Nothing to do.
		*/
		/* owner가 marking된 상태이고, new-value가 demarked 상태이면?
		   - Local-Scan 중이거나 이미 Local-Scan이 종료된 상태.
		   - Public-Scan은 이미 종료된 상태.
		   1) Local-scan 시작 당시 capture 한 stack내에서 참조가능한
		   모든 instance는 reachable-marking된다. new-value 가
		   capture 당시 stack 또는 그 ref-tree 상에 존재하고 있었다고 가정하면,
		   그 ref-tree 상에서 제거될 때에도 marking되고 (dessign 처리 참조)
		   stack 내에 있으면 당연히 marking된다.
		   2) newValue가 stack-capture 이 후 새로이 생성된 instance라면,
		   해당 instance는 reachable marking해 주지 않아도 무방하다.
		   결론) nothing to do.
		*/
		fastiva_Instance_p pOldValue = *(fastiva_Instance_p*)pField;
		*(const void**)pField = pNewValue;
		if (pOldValue != ADDR_ZERO) {
			if (!g_GC.isPublished(pOldValue)) {
				if (GC_ENABLE_LOCAL_REF) {
				    pOldValue->m_localRef$ --;
			    }
			}
			if (SUSPEND_TASK_WHILE_STACK_SCAN) {
				// 모든 local-instance 에 대한 marking 이 끝난 이후이다.
			}
			else if (g_GC.getScanMode() == TASK_SCANNING) {
				fastiva_Task* pCurrTask = fastiva_getCurrentTask();
				if (pCurrTask == g_GC.pCurrentScanningTask) {
					AutoLocalHeapLock __critical_section(pCurrTask);
					if (pCurrTask == g_GC.pCurrentScanningTask) { // 한 번 더 검사.
						// local-instance의 ref-tree를 변경시에는 현 task-stack을
						// scan할 때만 oldValue를 marking해 준다. old-value는
						// capure 후 변경된 stack에 저장되어 있을 수 있다.
						// (참고. public-lost와 달리 다른 task stack에 대해선
						// 염려하지 않아도 된다. pSelf 자체가 local-instance이므로)
						//fastiva_Instance_p pOldObj = !isInterface
						//	? (fastiva_Instance_p)pOldValue
						//	: ((fm::Interface_p)pOldValue)->getInstance$();
#if PROTOTYPE_GC
						fox_GC::markReachableTree(pOldValue);
#else
						fm::gc_markStrongReachable(pOldValue);
#endif
					}
				}
			}
		}
		if (pNewValue != FASTIVA_NULL && !g_GC.isPublished(pNewValue)) {
			if (GC_ENABLE_LOCAL_REF) {
				pNewValue->m_localRef$++;
			}
			KASSERT(!g_GC.isPublished(pNewValue));
			// 2008.0205. refCo8nt 변화 과정 중에 published 상태로 변경될 수 있다.
			// 그대로 남겨 두면, 실제로 publishing 될 때, 그 하위 field가
			// publishing되지 못하므로 미리 publishing하여야 한다.
			//if (g_GC.isPublished(pNewValue)) {
			//	fox_GC::publishInstance(pNewValue);
			//}
		}
		return;
	}

	pNewObj = (fastiva_Instance_p)pNewValue;
	
	if (pNewObj != ADDR_ZERO) {
		// publishing(LOCAL-G -> PUBLIC-G)을 field-assign전에 해야 하는 이유.
		// field-assign 이 후에 publishing을 하는 경우, 
		// asssign된 pNewObj의 local-reference가 다른 thread의 stack으로
		// 옮겨져 publishing 되지 못하고, 해당 field가 다른 값으로 변경되어, 
		// 현 task상에서 unreachable상태에 도달할 수 있다.
		fox_GC::publishInstance(pNewObj);
	}

	fastiva_Instance_p pOldValue = (fastiva_Instance_p)fox_util_xchg32(pField, (int)pNewValue);
	GC_DEBUG_BREAK(pOldValue);

	if (SUSPEND_TASK_WHILE_STACK_SCAN || g_GC.getScanMode() != TASK_SCANNING) {
		// 대체된 oldValue 는 현재 쓰래드의 스택에서 참조되고 있을 수 있다.
		// 스택 스캔 중에 해당 thread는 항상 멈추므로, 현재 Thread 는 이미 scan 되었거나,
		// 스캔되지 않은 상태이다.
		// *(void**)pField = (void*)pNewValue;
	}

	if (pOldValue != ADDR_ZERO) {

		// lost-public-instance(Global scan시에는 marking되지 않은 instance)는 
		// 두 개 이상의 local-stack에 의해 참조될 수 있다.
		// Task-A 에 대한 stack-capture 이 후에 stack 내에 참조된 lost-public-instance의
		// ref-tree를 모두 marking하기 전에, (Suspend되지 않은)Task-A가 lost-public-instance의
		// field의 내용을 새로이 변경된 stack에 push 하였고, 그 직 후
		// Task-B가 해당 Field 값을 변경하였다면, Task-A를 local-scan하는 중에
		// 변경 전의 field (Task-A가 변경된 stack 에 보관한	instance)를 
		// marking할 방법이 없다.
		// 따라서, Stack-Scan이 진행되는 도중에 Public-Lost-Instance의 field를
		// 변경할 시에는 해당 task에 대한 scan 여부와 관계없이 old-value를
		// reachable marking해 주어야만 한다.

		// 참고) 현재 field의 onwer가 reachable 상태라 하더라도, pOldValue는 
		// unreachble 상태일 수 있다.
		// owner가 stack 내에만 존재하는 Public-Lost이고, stack-marking되고
		// 있는 상태일 수 있다. 만약 현재, this 를 marking 하고,
		// 다른 field 들을 marking하는 도중에, 현 field의 값이 바뀌었다면,
		// pOldValue는 marking 되지 않을 수 있다. pOldValue는 현재 scan 중인
		// task에서 참조 중 일 수 있으므로, (현 task 는 scan 중이 아닐 수 있다.)
		// mark_public_lost를 호출해 주어야만 한다.
		g_GC.markIfScanning(pOldValue);
	}

}

inline static bool isInstancePointer(void* ptr) {
	// return ptr < (void*)JsNULL && ptr > (void*)JsTRUE;
	return ptr < (void*)0xFFFF0000 && ptr > (void*)0x00010001;
}

/*
void fm::setJsPropertyItem(
	Variant_ap pSelf, 
	JsValueEx newValue, 
	int index
) {
	Variant_A::Buffer buf(pSelf);
	void* pField = &buf[index];
	fm::setJsPropertyItem((java_lang_Object_p)pSelf, *(JsVariant*)&newValue, (JsVariant*)pField);
}
*/

#if FASTIVA_SUPPORTS_JAVASCRIPT
void fm::setGenericField(
	fastiva_Instance_p pSelf, 
	fastiva_lang_Generic_p pNewValue, 
	void* pFieldPos
) {
	JsVariant* pField = (JsVariant*)pFieldPos;
	JsVariant oldValue = *pField;
	JsVariant newValue;
	*(JsGeneric_p*)&newValue = pNewValue;
	if (*(JsGeneric_p*)&oldValue == pNewValue) {
		return;
	}

	fastiva_Instance_p pNewObj = newValue.asObject();

	GC_DEBUG_BREAK(pNewObj);
	GC_DEBUG_BREAK(pSelf);

	if (!g_GC.isPublished(pSelf)) {
		/* owner가 marking되지 않은 상태이거나, 둘 다 marking된 상태라면?
		   Nothing to do.
		*/
		/* owner가 marking된 상태이고, new-value가 demarked 상태이면?
		   - Local-Scan 중이거나 이미 Local-Scan이 종료된 상태.
		   - Public-Scan은 이미 종료된 상태.
		   1) Local-scan 시작 당시 capture 한 stack내에서 참조가능한
		   모든 instance는 reachable-marking된다. new-value 가
		   capture 당시 stack 또는 그 ref-tree 상에 존재하고 있었다고 가정하면,
		   그 ref-tree 상에서 제거될 때에도 marking되고 (dessign 처리 참조)
		   stack 내에 있으면 당연히 marking된다.
		   2) newValue가 stack-capture 이 후 새로이 생성된 instance라면,
		   해당 instance는 reachable marking해 주지 않아도 무방하다.
		   결론) nothing to do.
		*/
		*pField = newValue;
		if (oldValue.isHeapInstance()) {
			fastiva_Instance_p pOldValue = oldValue.ensureObject();
			if (!g_GC.isPublished(pOldValue)) {
				if (GC_ENABLE_LOCAL_REF) {
				    pOldValue->m_localRef$ --;
			    }
			}
			if (g_GC.getScanMode() == TASK_SCANNING) {
				fastiva_Task* pCurrTask = fastiva_getCurrentTask();
				if (pCurrTask == g_GC.pCurrentScanningTask) {
					AutoLocalHeapLock __critical_section(pCurrTask);
					if (pCurrTask == g_GC.pCurrentScanningTask) { // 한 번 더 검사.
						// local-instance의 ref-tree를 변경시에는 현 task-stack을
						// scan할 때만 oldValue를 marking해 준다. old-value는
						// capure 후 변경된 stack에 저장되어 있을 수 있다.
						// (참고. public-lost와 달리 다른 task stack에 대해선
						// 염려하지 않아도 된다. pSelf 자체가 local-instance이므로)
						//fastiva_Instance_p pOldObj = !isInterface
						//	? (fastiva_Instance_p)pOldValue
						//	: ((fm::Interface_p)pOldValue)->getInstance$();
#if PROTOTYPE_GC
						fox_GC::markReachableTree(pOldValue);
#else
						fastiva_Instance::markStrongReachable(pOldValue);
#endif
					}
				}
			}
		}
		if (pNewObj != FASTIVA_NULL && !g_GC.isPublished(pNewObj)) {
			if (GC_ENABLE_LOCAL_REF) {
			pNewObj->m_localRef$++;
			}
			// 2008.0205. refCount 변화 과정 중에 published 상태로 변경될 수 있다.
			// 그대로 남겨 두면, 실제로 publishing 될 때, 그 하위 field가
			// publishing되지 못하므로 미리 publishing하여야 한다.
			if (g_GC.isPublished(pNewObj)) {
				fox_GC::publishInstance(pNewObj);
			}
		}
		return;
	}

	//pNewObj = (fastiva_Instance_p)newValue;

	if (pNewObj != FASTIVA_NULL) {
		// publishing(LOCAL-G -> PUBLIC-G)을 field-assign전에 해야 하는 이유.
		// field-assign 이 후에 publishing을 하는 경우, 
		// asssign된 pNewObj의 local-reference가 다른 thread의 stack으로
		// 옮겨져 publishing 되지 못하고, 해당 field가 다른 값으로 변경되어, 
		// 현 task상에서 unreachable상태에 도달할 수 있다.
		fox_GC::publishInstance(pNewObj);
	}

	if (SUSPEND_TASK_WHILE_STACK_SCAN || g_GC.getScanMode() != TASK_SCANNING) {
		*pField = newValue;
	}
	else {
		*(JsGeneric_p*)&oldValue = (JsGeneric_p)fox_util_xchg32(pField, *(jint*)&newValue);
		fastiva_Instance_p pOldValue = oldValue.asObject();
		GC_DEBUG_BREAK(pOldValue);

		if (pOldValue != FASTIVA_NULL && !g_GC.isReachable(pOldValue)) {
			// lost-public-instance(Global scan시에는 marking되지 않은 instance)는 
			// 두 개 이상의 local-stack에서 참조될 수 있다.
			// Task-A 에 대한 stack-capture 이 후에 stack 내에 참조된 lost-public-instance의
			// ref-tree를 모두 marking하기 전에, (Suspend되지 않은)Task-A가 lost-public-instance의
			// field의 내용을 새로이 변경된 stack에 push 하였고, 그 직 후
			// Task-B가 해당 Field 값을 변경하였다면, Task-A를 local-scan하는 중에
			// 변경 전의 field (Task-A가 변경된 stack 에 보관한	instance)를 
			// marking할 방법이 없다.
			// 따라서, Stack-Scan이 진행되는 도중에 Public-Lost-Instance의 field를
			// 변경할 시에는 해당 task에 대한 scan 여부와 관계없이 old-value를
			// reachable marking해 주어야만 한다.

			// 참고) 현재 field의 onwer가 reachable 상태라 하더라도, pOldValue는 
			// unreachble 상태일 수 있다.
			// owner가 stack 내에만 존재하는 Public-Lost이고, stack-marking되고
			// 있는 상태일 수 있다. 만약 현재, this 를 marking 하고,
			// 다른 field 들을 marking하는 도중에, 현 field의 값이 바뀌었다면,
			// pOldValue는 marking 되지 않을 수 있다. pOldValue는 현재 scan 중인
			// task에서 참조 중 일 수 있으므로, (현 task 는 scan 중이 아닐 수 있다.)
			// mark_public_lost를 호출해 주어야만 한다.

			fastiva_Task* pCurrTask = fastiva_getCurrentTask();
			AutoLocalHeapLock __critical_section(pCurrTask);

			// suspend에 의해 scan 이 종료되었을 수 있으므로 scanning 여부 한 번 더 검사.
			// stack scan 시에만 pOldValue를 취급한다.
			if (g_GC.getScanMode() == TASK_SCANNING) { // 한 번 더 검사.
				//fastiva_Instance_p pOldObj = !fm::isInterface(pOldValue)
				//	? (fastiva_Instance_p)pOldValue
				//	: ((fm::Interface_p)pOldValue)->getInstance$();
#if PROTOTYPE_GC
				fox_GC::markReachableTree(pOldValue);
#else
				fastiva_Instance::markStrongReachable(pOldValue);
#endif
			}
		}
	}

	if (pNewObj == FASTIVA_NULL) {
		return;
	}

	if (g_GC.getScanMode() < TASK_SCANNING) {
		// field-assign 직전에 현 task가 suspend되거나, GC-cycle이 변경될 수 있다.
		return;
	}

	// 현재 public-scan이 종료되고, local-stack-scan이 진행 중인 경우, 
	// reachable-mark 되지 않은 public-instance가 아직 stack 내에 존재할 수 있다. 
	// 해당 stack을 scan하기 전에 해당 instance를 field-assign하고 stack 내에서 
	// 없에 버릴 수 있다. 따라서, 이미 reachable-marking된 published-instance에
	// 새로운 instance를 assign 하는 경우, 해당 instance의 ref-tree를 reachable 상태로 
	// 변경해 주어야만 한다. 

	// isReachable() 검사 전에 반드시 field의 값을 먼저 변경하여 주어야만
	// 한다. isReachable(pSelf) 검사 직 후 해당 instance가 
	// 다른 task에 의해서 marking 되어 버릴 수 있기 때문이다.
	// 그 후에 marking되지 않은 채로 newObject를 field-assign하여서 아니 된다.
	if (!g_GC.isReachable(pSelf) || g_GC.isReachable(pNewObj)) {
		return;
	}

	/* 현재 시점에서 task가 SUSPEND된 경우, pNewObj가 stack에
	남아 있으므로, pOwnerMark와 관계없이 reachable-marking된다.
	만일 curr-local-stack이 이미 scan이 끝났다면,
	pNewObj는	반드시 reachable-marking된 상태이다.
	*/
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();

	/* 1) isReachable(pSelf) 검사 직후, task가 
	suspend되어 demarking되는 경우에 대비하여 suspend를 금지한다.
	*/
	AutoLocalHeapLock __critical_section(pCurrTask);

	if (g_GC.isReachable(pSelf)) { // 한 번 더 검사.
		// 아직 scan되지 않은 public-instance 또는 scan되지 않은
		// stack에 남겨진 unreachable-instance의 assign이다.
		// assign 후, stack내의 참조가 제거될 수 있으므로,
		// Reachable 상태로 marking해 주어야 한다.
		// value가 여러 task에서 참조될 수 있으므로 
		// marking 도중 ref-tree 가 변경될 수 있으나,
		// marking 시 항시 owner를 먼저 marking함으로써 
		// thread-safety 문제를 해결할 수 있다.
#if PROTOTYPE_GC
		fox_GC::markReachableTree(pNewObj);
#else
		fastiva_Instance::markStrongReachable(pNewObj);
#endif
	}
}
#endif

jbool fm::isPublished(fastiva_Instance_p pObj) {
	return ((short)pObj->m_mark$ < 0);
}


void fastiva_Runtime::setStaticField(
	fastiva_MetaClass_p pSelf, 
	fastiva_Instance_p pNewValue, 
	void* pField
) {
	fastiva_Instance_p pNewObj;
	fastiva_Instance_p pOldObj;
	void* pOldValue;

	if (((int)pNewValue & 0xFFFF) == 0x0410) {
		int a = 3;
	}


	if (*(void**)pField == pNewValue) {
		return;
	}

	GC_DEBUG_BREAK(pNewValue);

	fox_GC::lockGlobalList();

	if (pNewValue != ADDR_ZERO) {
		pNewObj = (fastiva_Instance_p)pNewValue;

		// scanningG 검사 전에 등록되어야 한다.
		fox_GC::registerGlobal(pNewObj);

	}

	// publishing(LOCAL-G -> PUBLIC-G)을 field-assign전에 해야 하는 이유.
	// field-assign 이 후에 publishing을 하는 경우, 
	// asssign된 pNewObj의 local-reference가 다른 thread의 stack으로
	// 옮겨져 publishing 되지 못하고, 해당 field가 다른 값으로 변경되어, 
	// 현 task상에서 unreachable상태에 도달할 수 있다.
	// static field의 변경을 동기화한다.
	pOldValue = *(void**)pField;
	*(void**)pField = (void*)pNewValue;

	if (pOldValue != ADDR_ZERO) {
		pOldObj = (fastiva_Instance_p)pOldValue;
		KASSERT((pOldObj->m_globalRef$ & ~0x8000) >= 0x1);
		GC_DEBUG_BREAK(pOldObj);
		//if (g_GC.getScanMode() == TASK_SCANNING) {
			// static-field는 public-scan 시에 marking되므오,
			// stack scan 시에는 모두 marking된 상태이다.
			// 단, 2개의 task가 하나의 field에 동시에 값을 assign한 경우,
			// 첫번째 assign된 new-value가 미쳐 reachable-marking되지 않은
			// 상태일 가능성은 있다.
			// KASSERT(g_GC.isReachable(pOldObj));
		//}
		pOldObj->m_globalRef$ --;
		fox_GC::markIfScanning(pOldObj);
	}

	fox_GC::releaseGlobalList();

}

#if FASTIVA_SUPPORTS_JAVASCRIPT

void fm::setStaticField(
	fastiva_Class_p pSelf, 
	fastiva_lang_Generic_p pNewValue, 
	void* pField
) {
	const int isInterface = false;
	fastiva_Instance_p pNewObj;
	fastiva_Instance_p pOldObj;
	void* pOldValue;
	JsVariant newValue = pNewValue;

	if (*(void**)pField == pNewValue) {
		return;
	}

	GC_DEBUG_BREAK(pNewValue);

	if (!newValue.isHeapInstance()) {
		fox_GC::lockGlobalList();
		// static field의 변경을 동기화한다.
		pOldValue = *(void**)pField;
		*(void**)pField = (void*)pNewValue;

		if (pOldValue != ADDR_ZERO) {
			pOldObj = (fastiva_Instance_p)pOldValue;
			KASSERT((pOldObj->m_globalRef$ & ~0x8000) >= 0x1);
			pOldObj->m_globalRef$ --;
		}
		fox_GC::releaseGlobalList();
		return;
	}

	pNewObj = (fastiva_Instance_p)pNewValue;

	fox_GC::lockGlobalList();
	// static field의 변경을 동기화한다.

	// scanningG 검사 전에 등록되어야 한다.
	fox_GC::registerGlobal(pNewObj);
	
	pOldValue = *(void**)pField;
	*(void**)pField = (void*)pNewValue;
	JsVariant oldValue = (JsGeneric_p)pOldValue;

	if (oldValue.isHeapInstance()) {
		pOldObj = (fastiva_Instance_p)pOldValue;
		KASSERT((pOldObj->m_globalRef$ & ~0x8000) >= 0x1);
		GC_DEBUG_BREAK(pOldObj);
		if (g_GC.getScanMode() == TASK_SCANNING) {
			// static-field는 public-scan 시에 marking되므오,
			// stack scan 시에는 모두 marking된 상태이다.
			// 단, 2개의 task가 하나의 field에 동시에 값을 assign한 경우,
			// 첫번째 assign된 new-value가 미쳐 reachable-marking되지 않은
			// 상태일 가능성은 있다.
			// KASSERT(g_GC.isReachable(pOldObj));
		}
		pOldObj->m_globalRef$ --;
	}
	fox_GC::releaseGlobalList();
}
#endif

void fastiva_lockGlobalRef(
	fastiva_Instance_p pObj
) {
	GC_DEBUG_BREAK(pObj);

	// @todo conservative-scan을 사용하지 않으면, Local-Reference 문제가 발생한다.
	// JNI 연동시 별도로 Local-Reference를 사용하여야만 한다. 
	// 참고) JNI spec상으로는 conservative-scan만으로는 안전한 GC가 가능하지 않은
	// 구조로 되어 있다. 
	// 1안) JNI call 중에는 아예 GC를 멈추는 방법을 적극 고려한다.
	//      file, inet 등 IO를 제외한 JNI dll들은 함수 내부에서 blocking되는 
	//      일이 거의 없거나, 있어도 매우 순간적인 동기화 과정에 불과하다(?)
	// 2안) JNI-call 직전에 남은 stack을 둘로 나누어, Local_Ref buffer로 사용한다.
	// 3안) LocalRef를 생성하여 linked-list로 관리한다
	// 4안) 2안을 기본으로 하되, Buffer가 부족한 경우, 3안을 사용한다.

	//  JNI-Lock은 그 field에도 영향을 미친다.
	//  한 번 publishing된 instance는 clearHeapMark 이후에도 Thread의 rookieQ에서
	//  제거됨므로, finalize에서 JNI-FOX_ASM_LOCK을 다시 publishing 한다.
	//
	KASSERT(pObj != ADDR_ZERO);
	/*
	if (g_GC.getScanMode() >= TASK_SCANNING) {
		fox_util_inc16(&pObj->m_globalRef$);
		// GC 수행 중에 새로이 LOCKING되는 Object 들은
		// 그 ref-tree를 reachable marking 해준다.
		// 참고) aGlobalObj 에 대한 scanning을 시작한 이후에
		// 새로이 Locking되는 instance 는 모두 marking 해 주어야 한다.
		fastiva_Task* pCurrTask = fastiva_getCurrentTask();
		AutoLocalHeapLock __critical_section(pCurrTask);
		if (g_GC.getScanMode() >= TASK_SCANNING) { // 한 번 더 검사.
			fox_GC::markReachableTree(pObj);
		}
		return;
	}
	*/

	g_GC.lockGlobalList();
	g_GC.registerGlobal(pObj);
	g_GC.releaseGlobalList();

}

	
void fastiva_releaseGlobalRef(
	fastiva_Instance_p pObj
) {
	GC_DEBUG_BREAK(pObj);
	//if (g_GC.getScanMode() >= TASK_SCANNING) {
	//	fox_util_dec16(&pObj->m_globalRef$);
	//}
	//else {
		g_GC.lockGlobalList();
		KASSERT(pObj != ADDR_ZERO && (pObj->m_globalRef$ & ~0x8000) >= 0x1);
		pObj->m_globalRef$ -= 0x1;
		g_GC.releaseGlobalList();
	//}
}


void fox_GC::addPublishedNF(fastiva_Instance_p pObj) {
	GC_DEBUG_BREAK(pObj);
	KASSERT(fastiva_getCurrentTask() == Kernel::g_pSystemTask);
	KASSERT(g_GC.isFinalizable(pObj));
	//KASSERT(pObj->m_localRef$ == cntGC);
	pObj->m_pNext$ = g_GC.pPublishedNFQ;
	g_GC.pPublishedNFQ = pObj;
	g_GC.cntPublic ++;
}



void fox_GC::addPublished(fastiva_Instance_p pObj) {
	GC_DEBUG_BREAK(pObj);
	KASSERT(fastiva_getCurrentTask() == Kernel::g_pSystemTask);
	//KASSERT(pObj->m_localRef$ == cntGC);
	pObj->m_pNext$ = g_GC.pPublishedQ;
	g_GC.pPublishedQ = pObj;
	g_GC.cntPublic ++;
}


int test_v;
int test_func() {
	FASTIVA_DBREAK();
	return test_v;
}

static int g_cntTT = 0;

#if PROTOTYPE_GC
void fox_GC::markReferentTouchedTree(fastiva_Instance_p pObj) {
	KASSERT(pObj != ADDR_ZERO);
	KASSERT(fastiva_getCurrentTask()->inCriticalSection());
	fox_GC::markReachableTree(pObj);
}
#endif



jbool fox_GC::scanLocalStack(fastiva_Task* pTask, bool isLocal) {

	// scanning을 하는 동안 해당 thread를 멈출 경우, 
	// system-task의 priority가 낮아 해당 thread의 suspend 시간이
	// 매우 커지는 문제가 발생한다. Single-CPU 환경에서는 system-thread의
	// priority를 최고로 높임으로써, suspend 대용으로 사용 가능하나
	// Multi-CPU 환경에서는 불가능하다. - StackLock을 별도로 사용한다.
	// 이에 scan할 Task를 suspend 하고, GC-task priority를 해당 task와
	// 같거나 높게 설정한 뒤, stack을 scan한다.
	// ***************************************************************
	// 중요) stack scan시 해당 task를 suspend 시키는 방법은 특정 task의
	// local-stack 참조는 항상 thread-safe하다는 것을 보증할 수 있다.
	// 그렇지 않은 경우, stack-scan 중에, 이미 scan된 영역에 임의의 pointer를
	// assign 하는 경우, 이를 scan할 수 있는 방법이 없다.
	// 참고) suspend되는 시간을 줄이기 위해 stack 내용을 별도의 buffer로 복사한 뒤
	// 해당 buffer를 scan하는 방법을 사용할 수도 있다.
	// *****************************************************************
	// local-garbage의 unlink 시점.
	// local-garbage를 각 thread-scan 직 후에 unlink하면, 좀 더 메모리의
	// 재사용성을 높일 수 있다. 특히 다수의 thread가 사용되고, 메모리를
	// 한계 용량에 가깝게 많이 쓰는 경우, memory 해제 시기를 좀 더 앞당겨 
	// 메모리 부족으로 인해 특정 Thread가 block되는 현상을 감소시킬 수 있다.
	// 그러나, 해당 시간의 지연만큼, GC 효율성이 감소하고 (scan 이후 생성된
	// 모든 Instance는 미리 marking되므로), 일반 Mobile 환경에서는 다수의 
	// Thead를 가동하는 App가 매우 드물어, 실용적인 효과는 미지수이다.


	if (pTask == fox_task_currentTask()) {
		pTask->enterCriticalSection();
	}
	else if (!sys_stack_lock(pTask)) {
		return !(pTask->isTerminated());
	}

#if FASTIVA_SUPPORT_JNI_STUB
	pTask->clearGlobalArrayCache();
#endif

	jbool isTaskTerminated = pTask->isTerminated();
	if (pTask->m_scanState != IN_NEED_SCAN) {
		sys_stack_release(pTask);
		return !isTaskTerminated;
	}
	pTask->m_scanState = IN_SCANNING;
	
	typedef void (FOX_FASTCALL(*SCAN_METHOD))(fastiva_Instance_p);

	SCAN_METHOD scanMethod = isLocal
		? fm::gc_markLocalReachable : fm::gc_markStrongReachable;
	pTask->m_gcInterrupted = 0;

	fastiva_StackContext* pStackContext;
	pStackContext = pTask->m_pStackContext;

	/** 2011.0909 항상 LocalQ clear 하도록 변경.
	fastiva_Instance_p pScannedLocal = pTask->m_pLocalQ;
	fastiva_Instance_p pScannedNF = pTask->m_pLocalNFQ;
	/*/
	fastiva_Instance_p pScannedLocal = pTask->resetLocalQ();
	fastiva_Instance_p pScannedNF = pTask->resetLocalNFQ();
	//*/

	gc_verbose_printf("rookie: %d, total:%d, first:%x\n", pTask->m_cntLocalRookie, pTask->m_cntLocalInstance, pScannedLocal);
	pTask->m_cntLocalRookie = 0;

	int g_pStackBuffer[SUSPEND_TASK_WHILE_STACK_SCAN ? 256 : FOX_STACK_SIZE/sizeof(int) + 64];

#ifdef UNLINK_GARBAGE_AFTER_SCAN
	//if (pTask->m_pScanQ == ADDR_ZERO) {
		pTask->m_pScanQ = pScannedLocal;
	//}
	//else {
	//	pTask->m_pLastScan->m_pNext$ = pScannedLocal;
	//}

	//if (pTask->m_pScanNFQ == ADDR_ZERO) {
		pTask->m_pScanNFQ = pScannedNF;
	//}
	//else {
	//	pTask->m_pLastScanNF->m_pNext$ = pScannedNF;
	//}
#endif

	int* pStackBottom = 0;
	if (pStackContext == ADDR_ZERO) {
		// managed-section을 벗어났다. 모든 Local-Instance를 해제한다.
#ifdef UNLINK_GARBAGE_AFTER_SCAN
		pTask->m_scanState = SCANNED;
		sys_stack_release(pTask);
		return false;
#endif
	}
	else {
		KASSERT(!isTaskTerminated);

		// 2011.0911 LocalGC 모드 구현
		// pTask->m_scanState = IN_SCANNING;

		int cntRegister = fox_task_getRegisters(pTask, g_pStackBuffer);
		if (cntRegister != 0) {
			pStackContext->m_pStack = (void*)g_pStackBuffer[0];
		}

		pStackBottom = g_pStackBuffer + cntRegister;
		g_GC.pCurrentScanningTask = pTask;
		int sizStack = 0;

		while (pStackContext != ADDR_ZERO) {
			KASSERT(pStackContext->isClosed());
			int* stack_bottom = (int*)pStackContext->getBottom();
			int* stack_top    = (int*)pStackContext->getTop();
#ifdef _DEBUG
			sizStack += (int)stack_bottom - (int)stack_top; 
			gc_debug_printf("task: %x(%x) sp:%x, size:%d\n", pTask, pTask->m_taskID, pStackContext->m_pStack, (int)stack_bottom - (int)stack_top);
#endif
			if (!SUSPEND_TASK_WHILE_STACK_SCAN) {
				KASSERT((pStackBottom - g_pStackBuffer) + (stack_bottom - stack_top) < FOX_STACK_SIZE / sizeof(int));
			}
			while (stack_top < stack_bottom) {
				if (SUSPEND_TASK_WHILE_STACK_SCAN) {
					fastiva_Instance_p pObj = (fastiva_Instance_p)*stack_top ++;
					pObj = fox_GC::getHeapInstance(pObj);
					if (pObj != ADDR_ZERO) {
						scanMethod(pObj);
					}
				}
				else {
					*pStackBottom ++ = *stack_top ++;
				}
			}
			pStackContext = (fastiva_StackContext*)pStackContext->m_pPrev;
		}
	
	}
scan_finished:

	if (SUSPEND_TASK_WHILE_STACK_SCAN) {
		g_GC.pCurrentScanningTask = ADDR_ZERO;
		// scanState를 SCANNING -> SCANNED 로 바꾸는 것은 THREAD-SAFE하다.
		pTask->m_scanState = SCANNED;
#ifndef UNLINK_GARBAGE_AFTER_SCAN
		clearLocalQ(pTask, pScannedLocal, pScannedNF, isLocal);
		if (isLocal) {
			demarkReachableQ(pTask->m_pLocalQ);
			demarkReachableQ(pTask->m_pLocalNFQ);
		}
#endif
		sys_stack_release(pTask);
	}
	else {
		sys_stack_release(pTask);

		java_lang_Object_p* pSlot = (java_lang_Object_p*)g_pStackBuffer;
		while (pSlot < (void*)pStackBottom) {

			fastiva_Instance_p pObj = *pSlot++;
			fastiva_Instance_p pObj2 = pObj;
			pObj = fox_GC::getHeapInstance(pObj);
			if (pObj != ADDR_ZERO) {
				if (pTask->isTerminated()) {
					break;
				}
				scanMethod(pObj);
			}
		}
		g_GC.pCurrentScanningTask = ADDR_ZERO;

		// scanState를 SCANNING -> SCANNED 로 바꾸는 것은 THREAD-SAFE하다.
		pTask->m_scanState = SCANNED;
#ifndef UNLINK_GARBAGE_AFTER_SCAN
		clearLocalQ(pTask, pScannedLocal, pScannedNF, isLocal);
#endif
	}

	IF_GC_DEBUG {
		if (isTaskTerminated) {
			if (pTask->m_pLocalQ != NULL || pTask->m_pLocalNFQ != NULL) {
				fox_printf("termiated local Q is not empty %x, %x", pTask->m_pLocalQ, pTask->m_pLocalNFQ);
				fox_exit(-1);
			}
		}
	}

	// 주의) task가 다시 resume 된 이 후에 pTask->isTerminated() 를 다시 검사하여,
	// Task를 TaskQ에서 제거하면 안된다. 반드시 내부 LocalQ를 모두 비운 상태에서만
	// taskQ에서 task를 제거할 수 있다.
	return !isTaskTerminated;

	// stack scan 도중, 새로이 생성된 instance 는, 현재 copy된 stack에
	// 존재하지 않는 local-instance를 참조하는 것은 불가능하다.
}

void fox_GC::adjustLocalScanContext(fastiva_ExceptionContext* pTrap) {
#if 0
	fastiva_Instance_p pEnd = pTrap->m_pEndOfRookie; 
	if (pEnd == ADDR_ZERO) {
		return;
	}
	fastiva_ExceptionContext* pParentTrap = pTrap->m_pParent;
	fastiva_Instance_p pParentEnd = pParentTrap->m_pEndOfRookie;
	adjustLocalScanContext(pParentTrap);

	if (pEnd == pParentEnd) {
		// 참고. pParentTrap->m_pEndOfRookie와 pParentEnd는 같은 값이 아니다.
		pTrap->m_pEndOfRookie = pParentTrap->m_pEndOfRookie;
	}
	else if (!g_GC.isReachable(pEnd) || g_GC.isPublished(pEnd)) {
		pEnd = pEnd->m_pNext$;
		while (pEnd != pParentEnd) {
			if (g_GC.isReachable(pEnd) && !g_GC.isPublished(pEnd)) {
				pTrap->m_pEndOfRookie = pEnd;
				return;
			}
			pEnd = pEnd->m_pNext$;
		}
		// 참고. pParentTrap->m_pEndOfRookie와 pParentEnd는 같은 값이 아니다.
		pTrap->m_pEndOfRookie = pParentTrap->m_pEndOfRookie;
	}
#endif
}

jbool fox_GC::clearLocalQ(fastiva_Task* pTask, fastiva_Instance_p pScannedLocal, fastiva_Instance_p pScannedNF, bool isLocal) {

	LocalQueueHeader pLocalQ[1];
	pLocalQ->m_pNext$ = pScannedLocal;
	LocalQueueHeader pFinalizableQ[1];
	pFinalizableQ->m_pNext$ = pScannedNF;


	// Finalizing을 먼저 호출해 주어야 한다.
	// 먼저 추가된 finalizing-instance는 현재의 Finalizable-Instance들을
	// 참조하고 있을 수 있으며, 해당 instance의 reassign이 발생하는 순간에 
	// reachable이나, published 상태를 검사해서는 안된다.

	if (pTask->m_pTopHandler != ADDR_ZERO) {
		adjustLocalScanContext((fastiva_ExceptionContext*)pTask->m_pTopHandler);
	}
	else {
		// managed-section을 벗어났다.
		// @todo 2008.0211 - managed-section을 벗어난 task를 매번 scan할 필요가 있을까?
	}

	clearLocalQ(pFinalizableQ, false);
	/*
	참고) addFinalizing과 addLost는 반드시 pTask가 suspend되지 않은 상태에서 호출한다.
	(해당 task가 fox_heap_alloc() 도중에 suspend된 상태이면, heap_unlink가 불가하다.)
	*/
#ifdef MARK_FINALIZING
	if (!isLocal) {
		markFinalizing(pTask);
	}
	else {
		/**
		2012.02.01 addFinalizing() 시 NF객체의 subTree 가 모두 publishing 된다.
		해당 객체들은 unlink 되지 않으므로 통과!
		*/
	}
#endif

	if (pFinalizableQ->m_pFirstLost != ADDR_ZERO) {
		addFinalizing(pTask, pFinalizableQ->m_pFirstLost, pFinalizableQ->m_pLastLost);
		if (isLocal) {
			KASSERT(!isReachable(pFinalizableQ->m_pFirstLost));
		}
	}


	clearLocalQ(pLocalQ, true);

	/** 2011.0909 코드 대체
	pTask->m_pLocalQ = pLocalQ->m_pNext$;
	pTask->m_pLocalNFQ = pFinalizableQ->m_pNext$;
	/*/
	pTask->insertLocalQ(pLocalQ->m_pNext$, pLocalQ->m_pLastLocal$, pFinalizableQ->m_pNext$, pFinalizableQ->m_pLastLocal$);
	//*/

	//* LocalGC 는 multi-thread로 실행된다.
	//  충돌이 발생하지 않도록 동기화한다.
	fox_semaphore_lock(s_pPublicQ_lock);
	if (pLocalQ->m_pFirstPublished != ADDR_ZERO) {
		if (isLocal) {
			fastiva_Instance_p pObj = pLocalQ->m_pFirstPublished;
			for (; pObj != NULL; pObj = pObj->m_pNext$) {
				demarkReachable(pObj);
			}
		}
		pLocalQ->m_pLastPublished->m_pNext$ = g_GC.pPublishedQ;
		g_GC.pPublishedQ = pLocalQ->m_pFirstPublished;
	}


	if (pFinalizableQ->m_pFirstPublished != ADDR_ZERO) {
		IF_GC_DEBUG { 
			if (isLocal) {
				fastiva_Instance_p pObj = pFinalizableQ->m_pFirstPublished;
				for (; pObj != NULL; pObj = pObj->m_pNext$) {
					KASSERT(!isReachable(pObj));
				}
			}
		}
		pFinalizableQ->m_pLastPublished->m_pNext$ = g_GC.pPublishedNFQ;
		g_GC.pPublishedNFQ = pFinalizableQ->m_pFirstPublished;
	}
	fox_semaphore_release(s_pPublicQ_lock);

	/*
	참고) addFinalizing과 addLost는 반드시 pTask가 suspend되지 않은 상태에서 호출한다.
	(해당 task가 fox_heap_alloc() 도중에 suspend된 상태이면, heap_unlink가 불가하다.)
	*/
	if (pFinalizableQ->m_pLastLocal$ == ADDR_ZERO) {
		KASSERT(pFinalizableQ->m_pNext$ == ADDR_ZERO);
	}

	if (pLocalQ->m_pLastLocal$ == ADDR_ZERO) {
		KASSERT(pLocalQ->m_pNext$ == ADDR_ZERO);
	}

	return !pTask->isTerminated();
}

int fox_GC::clearLocalQ( LocalQueueHeader* pHeader, bool unlinkNow) {

	fastiva_Instance_p pPrev = pHeader;
	fastiva_Instance_p pLocal = pHeader->m_pNext$;

	fastiva_Instance_T$  lostHeader;
	fastiva_Instance_p pLastLost = &lostHeader;

	fastiva_Instance_T$  publishedHeader;
	fastiva_Instance_p pLastPublished = &publishedHeader;

	int cntLocal = 0;
	int cntLost = 0;
	int cntPublished = 0;

	if (unlinkNow) {
		sys_huge_heap_lock();
	}

	while (pLocal != ADDR_ZERO) {
		GC_DEBUG_BREAK(pLocal);
		fastiva_Instance_p pNext = pLocal->m_pNext$;

		if (g_GC.isPublished(pLocal)) {
			pPrev->m_pNext$ = pNext;
			pLastPublished->m_pNext$ = pLocal;
			pLastPublished = pLocal;
			IF_GC_DEBUG { cntPublished ++; }
		}
		else if (g_GC.isReachable(pLocal)) {
			pPrev = pLocal;
			IF_GC_DEBUG { cntLocal ++; }
		}
		else if (unlinkNow) {
			pPrev->m_pNext$ = pNext;
			fox_GC::unlinkHeapInstance(pLocal);
			IF_GC_DEBUG { cntLost ++; }
		}
		else {
			pPrev->m_pNext$ = pNext;
			pLastLost->m_pNext$ = pLocal;
			pLastLost = pLocal;
			IF_GC_DEBUG { cntLost ++; }
		}
		pLocal = pNext;
	}

	if (unlinkNow) {
		sys_huge_heap_unlock();
	}

	pLastLost->m_pNext$ = ADDR_ZERO;
	pLastPublished->m_pNext$ = ADDR_ZERO;
	pHeader->m_pLastLocal$ = pPrev;
	pHeader->m_pFirstLost = lostHeader.m_pNext$;
	pHeader->m_pLastLost = pLastLost;
	pHeader->m_pFirstPublished = publishedHeader.m_pNext$;
	pHeader->m_pLastPublished = pLastPublished;

	IF_GC_DEBUG { fox_printf("cntLocal: %d, cntPublished: %d, cntUnlink: %d\n", cntLocal, cntPublished, cntLost);}

	KASSERT(pPrev->m_pNext$ == ADDR_ZERO);

	return 0;
}

void fox_GC::clearPublicQ(QueueHeader* pHeader, bool unlinkNow) {

	fastiva_Instance_p pPrev = pHeader;
	fastiva_Instance_p pLocal = pHeader->m_pNext$;

	fastiva_Instance_T$  lostHeader;
	fastiva_Instance_p pLastLost = &lostHeader;
	if (unlinkNow) {
		sys_huge_heap_lock();
	}

	while (pLocal != ADDR_ZERO) {
		GC_DEBUG_BREAK(pLocal);
		fastiva_Instance_p pNext = pLocal->m_pNext$;

		if (g_GC.isReachable(pLocal)) {
			pPrev = pLocal;
		}
		else {
			pPrev->m_pNext$ = pNext;
			if (unlinkNow) {
				fox_GC::unlinkHeapInstance(pLocal);
			}
			else {
				pLastLost = pLastLost->m_pNext$ = pLocal;
			}
		}
		pLocal = pNext;
	}

	if (unlinkNow) {
		sys_huge_heap_unlock();
	}
	pLastLost->m_pNext$ = ADDR_ZERO;
	pHeader->m_pLastLocal$ = pPrev;
	pHeader->m_pFirstLost = lostHeader.m_pNext$;
	pHeader->m_pLastLost = pLastLost;
}



void fox_GC::markSoftReferent(fastiva_Instance_p pObj) {
	fastiva_Instance_p pNext;
	for (; pObj != ADDR_ZERO; pObj = pObj->m_pNext$) {

		GC_DEBUG_BREAK(pObj);

		java_lang_ref_Reference* pRef = (java_lang_ref_Reference*)pObj;
		fastiva_Instance_p pReferent = (fastiva_Instance_p)pRef->m_referent;
		if (pReferent != ADDR_ZERO) {
#if PROTOTYPE_GC
			markReachableTree(pReferent);
#else
			fm::gc_markStrongReachable(pReferent);
#endif		
		}
	}
}



void fox_GC::clearPhantomQ() {
#ifndef UNLINK_IN_FINALIZER
	if (g_GC.phantomQ.getFirst() != ADDR_ZERO) {
		// reassign 되거나, Phantom-Rechahble 인 finalized instance 들을
		// public-Q에 다시 추가한다.
		fastiva_Instance_p pObj = g_GC.phantomQ.removeAll();

		// reassign된 instance가 다시 unrechable 상태로 바뀌어,
		// 오랫동안, Finalizer-task에 머문 후, 추가되었을 수 있다.
		// 현재의 g_GC.currentMark와 충돌되지 않도록, demarking한다.
		sys_huge_heap_lock();

		while (pObj != ADDR_ZERO) {
			fastiva_Instance_p pNext = pObj->m_pNext$;
			g_GC.demarkReachable(pObj);

			if (g_GC.demarkPhantomRefReachable(pObj)) {
				// phantom-ref 처리 후 삭제한다.
				addPublished(pObj);
			}
			else if (g_GC.isPublished(pObj)) {
				// re-assign된 instance 또는 그 ref-tree에 속한 instance이다.
				addPublished(pObj);
			}
			else {
				unlinkHeapInstance(pObj);
			}
			pObj = pNext;
		}
		sys_huge_heap_unlock();

	}
#else
	!!! ERROR !!!
#endif
}


void fox_GC_doLocalGC_impl(fastiva_Task* pCurrTask, bool isLocal) {
	g_GC.lockScanMode(false);
	pCurrTask->enterCriticalSection();
	if (pCurrTask->m_gcInterrupted) {
        KASSERT(g_GC.getScanMode() == GC_IDLE || pCurrTask->m_gcInterrupted != TRIGGERED_BY_SELF);
        KASSERT(isLocal || pCurrTask->m_scanState == IN_NEED_SCAN);
		gc_verbose_printf("doLocalGC started %x(%x)\n", pCurrTask, pCurrTask->m_taskID);
		gc_start_t = currentTimeMillis_MD();
		fox_GC::scanLocalStack(pCurrTask, isLocal);
		jlonglong end = currentTimeMillis_MD();
	}
	pCurrTask->leaveCriticalSection();
	g_GC.releaseScanMode(false);
	sys_task_exitInterrupt(pCurrTask);
}

void fox_GC_doLocalGC_0(bool isLocal) {
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	fox_jmp_buf buf;
	if (!_setjmp(buf)) { // <- 단지 register 저장용으로 setjmp 를 호출.
		void* old_sp = pCurrTask->m_pStackContext->m_pStack;
		pCurrTask->m_pStackContext->m_pStack = &buf;
		fox_GC_doLocalGC_impl(pCurrTask, isLocal);
		pCurrTask->m_pStackContext->m_pStack = old_sp;
	}
	else {
		// 혹시 몰라 일부러 넣은 stub code;
		gc_verbose_printf("somthing wrong with _setjmp%x", pCurrTask);
	}
}

void fox_GC_doLocalGC() {
	fox_GC_doLocalGC_0(false);
}


int task_interrupted_for_gc = 0;
#ifdef GC_DEBUG
extern int	g_cntHeapAlloc;
extern int	g_totalHeapAllocSize;
extern int	g_usableBlockCount;
#endif

void fox_GC__markScanFinished() {

	g_GC.lockScanMode(true);
	g_GC.setScanMode(SCAN_FINISHED);
	task_interrupted_for_gc = false;
	fastiva_Task* pTask = kernelData.g_pTaskQ_interlocked;
	while (pTask != ADDR_ZERO) {
		fastiva_Task* pNext = pTask->m_pNext0;
		pTask->m_scanState = NOT_SCANNED;
		pTask = pNext;
	}
	g_GC.releaseScanMode(true);
}


void fox_GC::executeGC() {
	CHECK_STACK_FAST();
	KASSERT(fox_task_currentTask() == Kernel::g_pSystemTask);

	/** Stack-Scan 이 먼저냐? Heap-Scan이 먼저냐?
	  Stack-Scan을 먼저 하면, Local-GC를 먼저 수행함으로 Memory 반환 효율을
	  높일 수 있다. Stack-Scan 이후 또는 도중에 발생한 deref 는 stack-scan을
	  다시 수행한 후에, 처리된다.  그러나, Stack-Scan을 먼저 하게 되면,
	  HeapScan 종료 후 오래 전의 Stack 내용을 기준으로 GC를 하게 되므로
	  GC 효율이 감소하는 문제가 있을 수 있다. 

	  Heap-Scan을 먼저 수행하면, Heap-Scan 도중에 발생한 deref 를 stack-scan
	  후에 처리할 수 있으나, 어차피 stack-scan 중에 발생한 deref는 남게되고,
	  메모리 해제 시기가 늦춰지는 문제가 있다. 다만, stack-scan이 가장
	  나중에 짧은 시간 안에 이루어지므로, stack의 최근 내용을 좀 더 충실히
	  반영하는 효과를 얻을 수 있다.

	  Younger-Generation 에 대한 Marking 이후에 Stack-Scan을 하는 방법에 대해
	  고려한다(?)
    */

	//========================================================================//
	//========================================================================//

	bool removeSoftRef;
	ScanMode  dirty_G;
	{
		fox_task_setPriority(Kernel::g_pSystemTask, 10);
		
		fox_monitor_lock(g_GC.pGCFinished);
		fox_monitor_notifyAll(g_GC.pGCFinished);
		fox_monitor_release(g_GC.pGCFinished);

		fox_monitor_lock(g_GC.pTrigger);
		g_GC.resetScanPriority();
		uint curr_t = fastiva_getTickCount0();
		//KASSERT(g_GC.scanPriority >= GC_IDLE_PRIORITY);
		removeSoftRef = g_GC.nextGC_T == 0;

		//if (curr_t < g_GC.nextGC_T) {
		//	// GC-monitor를 위해 g_GC.nextGC_T를 변경한다.
		//	g_GC.nextGC_T = curr_t + GC_INTERVAL;
		while (!g_GC.isGCTriggered) {
			fox_monitor_wait(g_GC.pTrigger, g_GC.nextGC_T - curr_t);
			if (g_triggerTask == 0) {
				break;
			}
#ifdef UNLINK_GARBAGE_AFTER_SCAN
			fastiva_Task* pTask = g_triggerTask;
			// 2011.1025 localGC 수행 중에는 published 된 것 중 markong 된 것을 
			// demarking 해주어야 한다. g_triggerTask 가 NULL 인가를 검사하여 LocalGC 여부를 검사한다.
			clearLocalQ(pTask, pTask->m_pScanQ, pTask->m_pScanNFQ);
			g_triggerTask = 0;
			fox_heap_compact();
#endif
		}
		g_GC.isGCTriggered = false;
		// 2011-12-14 LocalGC 여부 결정을 위한 것.
		g_GC.cntPublishedRookie = 0;
		g_GC.cntLocalTrigger = 0;

		//	curr_t = fastiva_getTickCount0();
		//}
		g_GC.nextGC_T = curr_t + GC_INTERVAL;
		/*
		GC 초기 시작시에는 GC_IDLE_PRIORITY로 시작한다.
		시작 후, GC_INTERVAL*3/2 시간을 초과한 후에도
		GC 가 종료되지 않으면, 현재 모든 TASK-priority 들의 평균을 취해
		GC-priority를 높인다. 다음 GC-cycle은 nextGC_T가 초과되었으므로,
		wait 없이 속개되나, scanPriority는 GC_IDLE_PRIORITY로 reset된다.
		*/

		fastiva_Task* pTask = kernelData.g_pTaskQ_interlocked;
		for (; pTask != ADDR_ZERO; pTask = pTask->m_pNext0) {
			pTask->m_cntLocalRookie = 0;
		}
		gc_verbose_printf("gc[%d]\n", g_GC.cntGC); 
		g_GC.cntGC ++; 
		fox_monitor_release(g_GC.pTrigger);

		fox_task_setPriority(Kernel::g_pSystemTask, g_GC.getScanPriority());

		

		/* Local-GC 만을 수행할 것인지를 판단한다.
		*
		int cntLocalObj = 0;
		fastiva_Task* pFirstTask = kernelData.g_pTaskQ_interlocked;
		fastiva_Task* pTask;
		for (pTask = pFirstTask; pTask != ADDR_ZERO; pTask = pTask->m_pNext0) {
			cntLocalObj += pTask->m_cntLocalAlloc - pTask->m_cntLocalRemoved;
		}
		if (cntLocalObj >= g_GC.cntPublic / 2) {
			dirty_G = 0;
		}
		else {
			dirty_G = 1;
		}
		*/
		dirty_G = PUBLIC_SCANNING;
	}

	int cntLocalInstance = 0;
	int cntLocalRookie = 0;
	int cntScanThread = 0;
	int cntGlobalScan = 0;
	int cntScan = 0;
	int start_t;
#ifdef GC_DEBUG
	start_t = fastiva_getTickCount0();
#endif

	{
		// 1은 LocalGC시 stack-touch를 처리하기 위하여 예약됨.
		//int mark = (g_GC.currentMark & 0xFF) + 2;
		//if (mark >= 0xFF) {
		//	mark = 2;
		//}
		//g_GC.currentMark = HM_PUBLISHED + mark;
		// 일부 task가 publishing 중인 경우, published-tree의 일부는 
		// reachable-marking되고(leaf) 일부는 unreahcable (뿌리) 상태로
		// 남을 수 있으나, 이는 문제되지 않는다.
		// 그 역은 심각한 문제가 된다. 뿌리는 marking되고 leaf가 marking되지
		// 않은 경우, leaf를 marking할 방법이 없다.
		/*
		fastiva_Task* pFirstTask = kernelData.g_pTaskQ_interlocked;
		fastiva_Task* pTask;
		for (pTask = pFirstTask; pTask != ADDR_ZERO; pTask = pTask->m_pNext0) {
			while (pTask->inCriticalSection()) {
				// 현재 진행 중인 publishing이 마무리될 수 있도록 한다.
				sys_task_spin();
			}
		}
		//*/
	}

	//KASSERT(!g_GC.isReachable(DEBUG_INSTANCE));

	fastiva_Instance_p pFirstReference;
	fastiva_Instance_p pFirstSoftReference;
	g_GC.lockScanMode(true);
	g_GC.setScanMode(PUBLIC_SCANNING);//dirty_G);
	clearPhantomQ();
	g_GC.releaseScanMode(true);
	//========================================================================//
	//========================================================================//

	if (dirty_G > 0) {
		int cntGlobalSlot = g_GC.freeGlobalSlotOffset / sizeof(void*);

		int cntStackSlot = 512;
		int aStackBuffer[512];
		int scanStopAt = 164;

		while (cntGlobalSlot > 0) {
			int cntSlot;
			if (cntGlobalSlot > cntStackSlot) {
				cntSlot = cntStackSlot;
			}
			else {
				cntSlot = cntGlobalSlot;
			}

			lockGlobalList();
			// registerGlobal시 aGlobalSlot의 주소가 변경될 수 있다.
			// lockGlobalList() 후에 Slot 주소를 구한다.
			fastiva_Instance_p* pGlobalSlot = g_GC.aGlobalSlot + cntGlobalSlot;
			cntGlobalSlot -= cntStackSlot;

			int* pSlot = aStackBuffer;
			for (; cntSlot-- > 0; ) {
				pGlobalSlot--;
				fastiva_Instance_p pObj = *pGlobalSlot;
				if ((pObj->m_globalRef$ & 0x7FFF) > 0) {
					*pSlot ++ = (int)pObj;
				}
				else {
					GC_DEBUG_BREAK(pObj);
					pObj->m_globalRef$ = 0;
					unregisterGlobal(pGlobalSlot);
				}
			}
			if (cntGlobalSlot <= 0) {
				KASSERT(pGlobalSlot == g_GC.aGlobalSlot);
			}
			releaseGlobalList();

			// GlobalList locking 시간을 짧게하기 위하여 stackBuffer를 사용한다.
			for (; --pSlot >= aStackBuffer;) {
				fastiva_Instance_p pObj = (fastiva_Instance_p)*pSlot;
#if PROTOTYPE_GC
				markReachableTree(pObj);
#else

				IF_GC_DEBUG { cntGlobalScan ++; }

#ifdef _DEBUG
				if (cntGlobalScan == scanStopAt) {
					extern FOX_BOOL gc_markStrongReachable_debug;
					gc_markStrongReachable_debug = true;
					int a = 3;
				}
#endif
				fm::gc_markStrongReachable(pObj);
#endif
				IF_DEBUG {
					//fox_debug_printf("cnt global %d", cntGlobalScan ++);
				}
			}
		}

		//IF_DEBUG {
		//	fox_printf("\n[GC-1:%d] Obj=%d-%d=%d, Global=%d, Mark=%d, FNZ=%d, ERR=%d, FT=%d\n", 
		//				g_GC.cntGC, g_GC.cntAllocated, g_GC.cntUnlink, g_GC.cntAllocated - g_GC.cntUnlink, 
		//				cntGlobalScan, g_cntMarked, g_cntFinalizing, g_cntMemoryReadErr);
		//}
	}

	//===========================================================================//
	//===========================================================================//
	fastiva_Task* pFirstTask = kernelData.g_pTaskQ_interlocked;
	//===========================================================================//
	//===========================================================================//

	for (fastiva_Task* pTask = pFirstTask; pTask != ADDR_ZERO; ) {
		fastiva_Task* pNext = pTask->m_pNext0;
		// 2011.0911 LocalGC 모드 구현
		java_lang_Thread_p pThread = (java_lang_Thread_p)pTask->m_pUserData;
		if (pThread != NULL) {
			pTask->m_scanState = IN_NEED_SCAN;
			if (!isReachable(pThread)) {
				fox_printf("unreachable thread found");
				fm::gc_markPublicReachable(pThread);
			}
		}
		pTask = pNext;
	}

	g_GC.lockScanMode(true);
	task_interrupted_for_gc = true;
	g_GC.setScanMode(TASK_SCANNING);
	g_GC.releaseScanMode(true);
	// gc_interrupt 검사를 안전하게 할 수 있도록 잠시 sleep() 한다(?).
	fox_task_sleep(1);

	{
		/* HeapScan 이후에 생성된 task의 stack내에 reachable-marking되지 않은
		   public-instance 가 존재할 가능성은 없다. 단, 해당 Task의 Local-GC를
		   수행할 필요는 있다.
		*/
		fastiva_Task* pTask;
#ifdef UNLINK_GARBAGE_AFTER_SCAN
		pTask = pFirstTask;
		for (; pTask != ADDR_ZERO; pTask = pTask->m_pNext0) {
			scanLocalStack(pTask, false);
		}
		/** 2011.0926
		    참조되지 않은 ref->referent 들이 모두 publicQ로 옮겨진 상태다.
			해당 instance 를 unlink 할 때 반드시 ref->m_referent 를 null 로 reset하여야만 한다.
		*/

		g_GC.lockScanMode(true);
		pFirstReference = g_GC.referenceQ.removeAll();
		pFirstSoftReference = g_GC.softRefQ.removeAll();
		g_GC.releaseScanMode(true);

		fox_GC__markScanFinished();
#endif

		pTask = pFirstTask;
		fastiva_Task* pPrevTask = ADDR_ZERO;

		for (; pTask != ADDR_ZERO; ) {
			fastiva_Task* pNext = pTask->m_pNext0;
#ifdef GC_DEBUG
			cntScanThread ++;
#endif

			sys_task_interruptTask(pTask);
			if (scanLocalStack(pTask, false) || pTask->m_pFinalizeTask != NULL) {
				pPrevTask = pTask;
			}
			else if (pPrevTask != ADDR_ZERO) {
				KASSERT(pPrevTask->m_pNext0 == pTask);
				pPrevTask->m_pNext0 = pNext;
			}
			else {
				// kernelData.g_pTaskQ_interlocked 가 변경되었을 수 있다.
				fox_mutex_lock(kernelData.g_pLock);
				if (pTask == kernelData.g_pTaskQ_interlocked) { // 한번 더 확인
					kernelData.g_pTaskQ_interlocked = pNext;
				}
				else {
					pPrevTask = kernelData.g_pTaskQ_interlocked;
					while (pPrevTask->m_pNext0 != pTask) {
						pPrevTask = pPrevTask->m_pNext0;
					}
					pPrevTask->m_pNext0 = pNext;
				}
				fox_mutex_release(kernelData.g_pLock);
			}

			pTask = pNext;
		}

#ifndef UNLINK_GARBAGE_AFTER_SCAN
		/** 2011.0926
		    taskScan을 모두 마치면, 참조되지 않은 ref->referent 들이 모두 publicQ로 옮겨진 상태다.
			해당 instance 를 unlink 하기 전에 반드시 ref->m_referent 를 null 로 reset하여야만 한다.
		*/
		g_GC.lockScanMode(true);
		pFirstReference = g_GC.referenceQ.removeAll();
		pFirstSoftReference = g_GC.softRefQ.removeAll();
		g_GC.releaseScanMode(true);

		// marking 되지 않는 soft_ref/phantom_ref 를 null 로 변경한다.
		clearReference(pFirstSoftReference, pFirstReference, removeSoftRef);

		/* 2011.0108
		    pFirstReference 를 scanFinished() 이 후에 설정하면, 그 이후에 Q에 추가된 
			Reference 들이 marking 되지 않은 상태로 publicQ로 옮겨져 삭제될 수 있다.
		*/
#endif
	}



	// Public-Instance 에 대한 reachable 여부를 확인하기 전에 현재 finalize가 종료되지 
	// 않은 모든 instance들을 모두 marking해 주어야만 한다.
	// Finalizable-Instance가 Public-Lost-Instance들을 참조하고 있을 수 있기 때문이다.

	{

		fastiva_Instance_p pNext;

		QueueHeader publicNFQ[1];
		publicNFQ->m_pNext$ = g_GC.pPublishedNFQ;
		clearPublicQ(publicNFQ, false);
		g_GC.pPublishedNFQ = publicNFQ->m_pNext$;

#ifdef MARK_FINALIZING
		markFinalizing(Kernel::g_pSystemTask);
#endif
		if (publicNFQ->m_pFirstLost != ADDR_ZERO) {
			fox_GC::addFinalizing(Kernel::g_pSystemTask, publicNFQ->m_pFirstLost, publicNFQ->m_pLastLost);
		}

		fox_GC__markScanFinished();

		QueueHeader publicQ[1];
		publicQ->m_pNext$ = g_GC.pPublishedQ;
		clearPublicQ(publicQ, true);
		g_GC.pPublishedQ = publicQ->m_pNext$;
#if 0
		if (publicQ->m_pFirstLost != ADDR_ZERO) {
			addLost(Kernel::g_pSystemTask, publicQ->m_pFirstLost, publicQ->m_pLastLost);
		}
#endif
	}

	fox_heap_compact();


	int cntLocal = 0;
	int cntPublic;
	int cntSoftRef;
	int cntWeakRef;
	int cntFinalizable;

	{
		extern int g_allocated;
		extern int g_deleted;
		extern int g_totalAlloc;

		int cnt;
#ifdef GC_DEBUG
		g_cntWeakReachable = 0;
#endif

		/************************************************************************
		demarkReachable은 반드시 모든 scanning이 종료된 이후에 처리되어야 한다.
		즉, marking과 demarking이 동시에 이루어지면 절대 안된다.
		************************************************************************/

		cntPublic = demarkReachableQ(g_GC.pPublishedQ);
		cntFinalizable = demarkReachableQ(g_GC.pPublishedNFQ);
		cntSoftRef = demarkReachableQ(g_GC.softRefQ.getFirst());
		cntWeakRef = demarkReachableQ(g_GC.referenceQ.getFirst());
		
		fastiva_Task* pTask = kernelData.g_pTaskQ_interlocked;
		fastiva_Task* pPrevTask = ADDR_ZERO;
		
		while (pTask != ADDR_ZERO) {
			fastiva_Task* pNext = pTask->m_pNext0;
			// LocalGC를 대비한 동기화.
			pTask->enterCriticalSection();
			cntLocal += demarkReachableQ(pTask->m_pLocalQ);
			cntFinalizable += demarkReachableQ(pTask->m_pLocalNFQ);
			pTask->leaveCriticalSection();
			pTask = pNext;
		}

		IF_GC_DEBUG {
			lockGlobalList();
			// registerGlobal시 aGlobalSlot의 주소가 변경될 수 있다.
			// lockGlobalList() 후에 Slot 주소를 구한다.
			fastiva_Instance_p* pGlobalSlot = g_GC.aGlobalSlot;
			for (int cntSlot = g_GC.freeGlobalSlotOffset / sizeof(void*); cntSlot-- > 0; pGlobalSlot ++) {
				fastiva_Instance_p pObj = *pGlobalSlot;
				if (isReachable(pObj) && pObj->getClass$() != java_lang_Class_C$::importClass$()) {
					const fastiva_InstanceContext * pCtx = fm::getInstanceContext(pObj);
					fox_printf("Global Slot Clear Error: %s/%s(%x)(mark:%x)(gref:%x)\n", pCtx->getPackageName(), pCtx->m_pBaseName,
						pObj->m_pClass$->m_pContext$->m_accessFlags, pObj->m_mark$, pObj->m_globalRef$);
					fox_exit(-1);
				}
				//g_GC.demarkReachable(pObj);
			}
			releaseGlobalList();
		}


		IF_GC_DEBUG {
			// GC 중에 새로 생성된 Local-Instance(새로 생성된 task 포함)도 r
			// eachable marking되었을 수 있다. 이를 demarking해주어야 한다.
			pTask = kernelData.g_pTaskQ_interlocked;
			while (pTask != ADDR_ZERO) {
				fastiva_Task* pNext = pTask->m_pNext0;
				java_lang_Thread_p pThread = (java_lang_Thread_p)pTask->m_pUserData;
				if (isReachable(pThread)) {
					const fastiva_InstanceContext * pCtx = fm::getInstanceContext(pThread);
					fox_printf("Thread Clear Error: %s/%s\n", pCtx->getPackageName(), pCtx->m_pBaseName);
					fox_exit(-1);
				}
				pTask = pNext;
			}
		}
	}

	g_GC.lockScanMode(true);
	// 반드시 모든 thread의 LocalQ를 demarking 한 이후에 GC IDLE로 변경할 것.
	// LocalGC와의 동기화 때문.
	g_GC.setScanMode(GC_IDLE);
	g_GC.releaseScanMode(true);


	IF_GC_DEBUG {
		extern int g_cnt_semphore_lock;
		extern int g_cnt_monitor_lock;
		fox_printf("[GC:%d] Obj=%d-%d=%d(Pub=%d, Finalizable=%d, Local=%d, SoftRef=%d, WeakRef=%d)\n"
			"GlobalRef=%d, Thread=%d, FinalizerThread=%d cntUnlinkedWeakRef=%d\n"
			"Sem-Lock=%d, Monitor-Lock=%d\n", 
						g_GC.cntGC, g_GC.cntAllocated, g_GC.cntUnlink, g_GC.cntAllocated - g_GC.cntUnlink, 
						cntPublic, cntFinalizable, cntLocal, cntSoftRef, cntWeakRef, 
						cntGlobalScan, cntScanThread, g_cntFinalizeTask, s_cntRefUnlink,
						g_cnt_semphore_lock, g_cnt_monitor_lock);
	}

	if (dirty_G >= ROOT_G) {
		// ConstantString의 m_value(char[])가 Code-Segement에 생성된
		// 경우엔, 안전한 시점에만, package를 unload해야 한다.
		removeGarbagePackage();
	}
}

void fox_GC::clearReference(fastiva_Instance_p pFirstSoftReference, fastiva_Instance_p pFirstReference, bool removeSoftRef) {

	fastiva_Instance_p pPrev;
	fastiva_Instance_T$ header[1];

	g_GC.lockScanMode(false);
	{
		fastiva_Instance_p pObj = pFirstSoftReference;
		pPrev = header;
		pPrev->m_pNext$= pObj;
		fastiva_Instance_p pNext;
		for (; pObj != ADDR_ZERO; pObj = pNext) {

			GC_DEBUG_BREAK(pObj);
			pNext = pObj->m_pNext$;

			if (!g_GC.isReachable(pObj)
			||  (validateReferent((java_lang_ref_Reference*)pObj, !removeSoftRef) == ADDR_ZERO)) {
				pPrev->m_pNext$ = pNext;
				if (!g_GC.isFinalizable(pObj)) {
					// 아직 finalizableQ에 대한 처리가 끝나지 않았으므로,
					// 즉시 unlnk할 수 없다.
					addPublished(pObj);
				}
				else {
					// ref는 finalizable 검사가 되어 있지 않다.
					addPublishedNF(pObj);
				}
			}
			else {
				pPrev = pObj;
			}
		}
	}
	g_GC.releaseScanMode(false);

	g_GC.lockScanMode(true);
	{
		if (header->m_pNext$ != ADDR_ZERO) {
			g_GC.softRefQ.insert(header->m_pNext$, pPrev);
		}
	}
	g_GC.releaseScanMode(true);

	g_GC.lockScanMode(false);
	{
		/*
		PublicLost-Q를 만들기 전에 Reference-marking을 처리한다.
		*/

		/* Reference 처리의 편의를 위해 모든 referent 는 Published 된 상태이다.
		참조) setReferent()
		*/

		/* REFERENCE_Q에 있는 Published-Instance를 해당 Q로 옮기지 않아도 무방하다.
		minRefCount에 의해 GC 여부가 결정되는데, REFERENCE_Q에 속한 Instance의
		Generation에 관계없이 해당 Instance의 refCount가 minRefCount가 작다면,
		해당 Instance는 dirty_G 보다 높은 세대로 부터 참조되지 않고 있음을 나타낸다. 
		dirty_G 이하는 이미 모두 scan된 상태이므로 해당 instance를 GC하여도 무방하다.
		*/

		/* Reference 자체가 GC 되어 경우엔 추가적인 조치가 불필요하다.*/
		// 참고) GC Cycle 시작 직 후에 추가된 Reference는 그 해당 Referent를 marking 된
		// 상태로 추가된다. (setReferent() 참조) 따라서, 별도로 scan할 필요가 없다.
		// 이에 GC 시작 직 후에 얻은 pFirstReference 를 사용한다.
		int cntRef = 0;
		pPrev = header;
		fastiva_Instance_p pObj = pPrev->m_pNext$ = pFirstReference;
		fastiva_Instance_p pNext;
		for (; pObj != ADDR_ZERO; pObj = pNext) {

			GC_DEBUG_BREAK(pObj);
			pNext = pObj->m_pNext$;

			if (!g_GC.isReachable(pObj)) {
				IF_GC_DEBUG {
					s_cntRefUnlink ++;
				}
				pPrev->m_pNext$ = pNext;
				if (!g_GC.isFinalizable(pObj)) {
					// 아직 finalizableQ에 대한 처리가 끝나지 않았으므로,
					// 즉시 unlnk할 수 없다.
					addPublished(pObj);
				}
				else {
					// ref는 finalizable 검사가 되어 있지 않다.
					addPublishedNF(pObj);
				}
				/*
				아래의 코드는 사용할 수 없다. 여러 개의 PhantomReference가
				동일한 referent를 가지고 있으면 문제가 발생한다.
				if (type == ACC_PHANTOM_REF$) {
					pMark->demarkPhantomRefReachable();
				}
				*/
				continue;
			}
			else {
				//g_GC.demarkReachable(pObj);
			}

			java_lang_ref_Reference* pRef = (java_lang_ref_Reference*)pObj;
			int type = (pRef->getClass())->m_pContext$->m_accessFlags
				& (ACC_PHANTOM_REF$ | ACC_SOFT_REF$ | ACC_WEAK_REF$);
			KASSERT(type != 0);

			/* SUN-JDK 와의 호환성.
			PhantomReference는 Referent에 대한 unlink가 호출되는 시점에
			(finalize() 함 수 수행 이 후가 아니다. Phantom-Reassign이 발생할 수 있기
			때문이다.) ReferenceQ에 pRef를 추가한다.
			그 외는 addFinalizing과 동시에 지정된 ReferenceQ에 pRef를 추가한다.
			(Phantom-Reassign이 되는 것에는 개의치 않는다.)

			하나의 Referent를 여러개의 Reference에서 참조할 수 있음을 유의한다.
			*/
			if (type == ACC_PHANTOM_REF$) {

				java_lang_Object* pReferent = (java_lang_Object*)pRef->m_referent;
				if (pReferent == ADDR_ZERO) {
					goto remove_ref_in_ref_q;
				}
				if (!g_GC.isReachable(pReferent)) {
					{
						/** ref.clear() 가 호출되기 전까지는 referent및 그 이하 객체는
							일명 PhantomReachable 상태이다.
						*/
						AutoLocalHeapLock __critical_section(fastiva_getCurrentTask());
						fm::gc_markStrongReachable(pReferent);
					}
					if (g_GC.isFinalizable(pReferent)) {
						/**
						 * finalize() 호출이 종료되고 나서 즉, reclaim 직전에
						 * referent가 PhantomReachable 로 변경된다.
						 */
						g_GC.markPhantomRefReachable(pReferent);
#ifndef FASTIVA_ENABLE_FINALIZE
						// 2011.0112. @TODO 별도 쓰레드 분리 필요.
						pReferent->finalize();
#endif
					}
					else {
						/** ref.clear() 호출될 때까지 unlink 되지 않는다. */
						java_lang_ref_ReferenceQueue * q = pRef->m_queue;
						if (q != ADDR_ZERO) {
							q->enqueue(pRef);
						}
						goto remove_ref_in_ref_q;
					}
				}
			}
			else if (validateReferent(pRef, false) == ADDR_ZERO) {
	remove_ref_in_ref_q:
				IF_GC_DEBUG {
					s_cntRefUnlink ++;
				}
				pPrev->m_pNext$ = pNext;
				if (!g_GC.isFinalizable(pRef)) {
					addPublished(pRef);
				}
				else {
					addPublishedNF(pRef);
				}
				continue;
			}
			cntRef ++;
			pPrev = pObj;
		}

		IF_GC_DEBUG {
			fox_printf("cntRef(Weak/Phantom) %d", cntRef);
		}
	}

	g_GC.releaseScanMode(false);

	g_GC.lockScanMode(true);
	{
		if (header->m_pNext$ != ADDR_ZERO) {
			g_GC.referenceQ.insert(header->m_pNext$, pPrev);
		}
	}
	g_GC.releaseScanMode(true);
		
}

void FOX_FASTCALL(fastiva_GC_doGC)(bool fullScan) {
	fastiva_Task* pCurrTask = (fastiva_Task*)fox_task_currentTask();
	KASSERT(pCurrTask != Kernel::g_pSystemTask);

	fox_monitor_lock(g_GC.pGCFinished);
	fox_monitor_lock(g_GC.pTrigger);
	int dirtyG  = fullScan ? ROOT_G : 0;

	if (g_GC.nextGC_T != 0) {
		g_GC.nextGC_T = !fullScan;
	}
	g_GC.boostScanPriority(pCurrTask->m_priority);
	int cntOldGC = g_GC.cntGC;
	int cntLoop = 0;

	/**
	2012.0103 아래의 코드는 2개 이상의 쓰레드가 동시에 본 함수를 호출할 시 문제가 된다.
	먼저 호출되어 waiting 중인 쓰레드를 다음 쓰레드가 wake 하여 버리기 때문에 무한 Loop에 빠진다.
	do {
		fox_monitor_notify(g_GC.pTrigger);
		fox_monitor_wait(g_GC.pTrigger, -1);
	} while (fullScan && g_GC.cntGC == cntOldGC);
	대안 1) 
		monitor를 trigger와 finished 으로 나누는 방법.
		finished을 먼저 lock하고, trigger에 notify를 보낸다.
	대안 2)
		본 함수 진입을 critical-section으로 막는다.
		이 방식 사용시 다른 함수에서 trigger.notify() 시 wake되는 단점이 있다.
	*/

	g_GC.isGCTriggered = true;
	fox_monitor_notifyAll(g_GC.pTrigger);
	fox_monitor_release(g_GC.pTrigger);

	fox_monitor_wait(g_GC.pGCFinished, -1);
	fox_monitor_release(g_GC.pGCFinished);
}

void FOX_FASTCALL(fastiva_GC_triggerGC)(int maxLocalAlloc) {
	if (false) {
		// for synchronized gc for debugging;
		fastiva_GC_doGC(false);
		return;
	}

	fastiva_Task* pCurrTask = (fastiva_Task*)fox_task_currentTask();
	KASSERT(pCurrTask != Kernel::g_pSystemTask);

	if (g_GC.cntLocalTrigger < 12
	&&  pCurrTask->m_totalRookieSize > maxLocalAlloc 
	&&  g_GC.cntPublishedRookie < pCurrTask->m_cntLocalRookie) {
		g_GC.lockScanMode(false);
		if (g_GC.getScanMode() == GC_IDLE) {
			g_GC.cntLocalTrigger ++;
			pCurrTask->m_gcInterrupted = TRIGGERED_BY_SELF;
			pCurrTask->m_scanState = IN_NEED_SCAN;
			fox_GC_doLocalGC_0(true);
			fox_printf("doLocalGC finished %x(time:%dms)\n", pCurrTask, (int)(currentTimeMillis_MD()-gc_start_t));
#ifdef UNLINK_GARBAGE_AFTER_SCAN
			g_triggerTask = pCurrTask;
#else
			g_GC.releaseScanMode(false);
			fox_heap_compact();
			fox_printf("heap_compact finished %x(time:%dms)\n", pCurrTask, (int)(currentTimeMillis_MD()-gc_start_t));
			return;
#endif
		}
		g_GC.releaseScanMode(false);
#ifndef UNLINK_GARBAGE_AFTER_SCAN
		return;
#endif
	}

	if (fox_monitor_tryLock(g_GC.pTrigger)) {
		fox_monitor_notifyAll(g_GC.pTrigger);
		fox_monitor_release(g_GC.pTrigger);
	}
}







void fox_GC::removeGarbagePackage() {
#if JPP_VM_SUPPORT_PACKAGLE_LOADER
	void FOX_FASTCALL(fox_freeLibrary)(const void* hDLL);

	fastiva_PackageInfo* pSlot = kernelData.g_aPackageSlot;
	fastiva_Package* pPrevPackage = ADDR_ZERO;
#if 1
	// 구현을 미룬ㄴ다. 2007.10
	FASTIVA_DBREAK();
#else
	while (pPackage != ADDR_ZERO) {
		if (pPackage->m_pModule->m_cntMarked == 0) {
			k_debug_puts("[Fastiva] package unload : ");
			k_debug_puts(pPackage->m_pPakcageName);
			if (pPrevPackage == ADDR_ZERO) {
				kernelData.g_pPackageQ = pPackage->m_pNext;
			}
			else {
				pPrevPackage->m_pNext = pPackage->m_pNext;
			}
			java_lang_Class_p pClass = (java_lang_Class_p)pPackage->m_pClassQ;
			sys_huge_heap_lock();
			while (pClass != ADDR_ZERO) {
				// array class를 clear해주어야 한다.
				java_lang_Class_p pArrayClass = pClass->getm_pCompositeType$;
				while (pArrayClass != ADDR_ZERO) {
					fox_GC::unlinkHeapInstance(pArrayClass);
					pArrayClass = pArrayClass->getm_pCompositeType$;
				}
				pClass = (java_lang_Class_p)pClass->m_pNext$;
			}
			sys_huge_heap_unlock();
			#ifdef FASTIVA_TARGET_OS_WIN32
				// package DLL이 없어지므로 
				const void* hDLL = pPackage->m_hDLL;
				pPackage = (fastiva_Package*)pPackage->m_pNext;
				fox_freeLibrary(hDLL);
			#endif
		}
		else {
			pPrevPackage = pPackage;
			pPackage->m_cntMarked = 0;
			#ifdef FASTIVA_TARGET_OS_WIN32
				pPackage = (fastiva_Package*)pPackage->m_pNext;
			#endif
		}
		#ifndef FASTIVA_TARGET_OS_WIN32
			pPackage = (fastiva_Package*)pPackage->m_pNext;
		#endif
	}
#endif
#endif
}




#ifdef __ABOUT_FINALIZATION_000
sdfas
0. Lost-Instance의 처리 조건.
	1) FinalizeTask내의 Local-Reachable을 포함한 모든 Reachable-Object는
		Finalize 되지 않아야 한다. ddd
	2) Phantom-Rechable 은 unlink 되지 않아야 한다.
	3) Finalize() 수행 중 새로 생성된 instance가 publishing된 경우,
	해당 Instance를 finalize 해서는 안된다.


1. Finalize Loca-Stack.
	FinalizeTask의 LocalStack은 반드시 scan하여야만 한다.
	Local-FinalizeTask의 stack 내에 public-instance 가 marking 되지 않은
	채로 남아 있는 상태에서 GC가 1,2 Cycle 진행되면, 해당 instance가 
	public-finalizable-Q로 	옮겨져 finalize() 함수가 실행되어 버리고,
	심지어 unlink 되어 버리는 결과가 발생된다.
	
2. Finalize Local-Q
	Finalize() 수행 도중 생성된 Local-Instance들 중 Publishing 되지 않은
	인스탄스들은 모두 Finalize 시킨다.

3. Phantom_Publishing 의 금지
	finalizing 중에 해당 Lost-Instance 또는 그 ref-tree(Expire된)를
	Public-Reachable ref-tree에 assign하는 것을 금지하면, 
	(PhantomAssignError를 throw 한다) Application의 안정성을 
	좀 더 확보할 수 있다. (그런데, JVM은 왜 허용하고 있을까?)
	Fianlize-Task에서 생성된 Local-Instance에 대한 assign도 금지되어야
	한다. 해당 Local-Instance가 publishing될 때, Exception을 발생시킬 수도
	있으나, assign시점과 오류발생시점의 차이로 인해 개발자의 혼란이
	가중되기 때문이다.

4. Phantom_Publishing.
	1) Lost-Instance 는 Finalize Task에서만 참조 가능한 Local-Instance 이다.
    이에 Local-G와 Lost-G를 동일하게 사용하는 것이 가능하다.
	이에, Lost-Local-Instance 는 아무 변경없이 Lost-G로 이동시킬 수 있다.
	3) Publish된 Lost-Instance는 Phantom으로 변경된다.
	참고) FinalizeTask 내의 Local-Reachable은 Lost 상태와 같다.

5. Phantom_Publishing 의 문제
	1) Phantom-Publishing은 이미 Publishing된 객체의 HeapMark에는 변화를 주지 
	않는다. Phantom으로 변경된 Lost-Instance가 Public-Q로 옮겨지기 전에
	FinalizeTask가 blocking된 상태에서 그 ref-tree가 변경되고, 1회 이상의 
	GC Cycle이 수행되면, 새로 assign된 ref는 실제로는 Strong-Reachable임에도 
	불구하고, -해당 Phantom을 따로 scan하지 않으면- Exipred-Q에 추가되는 
	문제가 발생한다. 
	(참고, Phantom-Publishing 당시 Public-Reachable로 marking되었다하더라도
	그 GC Cycle에서 새로 assign된 instance는 demarking 되어 버린다.)

	2) 이 때, 해당 Ref 가 실제론 Strong-Reachble임에도 불구하고 finalize()가 
	수행되는 문제가 발생한다. (해당 Instance가 Unlink 되지는 않더라도)
	또한 Lost-Public-Instance를 Finalize-Q에 추가할 시, Generation 을
	Local 상태로 변경하는데, 이 과정에 Phantom-Rechable 상태가 해제되어
	(Phantom-Rechable임에도 불구하고) Unlink 되어버리는 문제도 아울러 발생한다. 

	3) 참고로 Local-Finalization 시에는 위의 상황이 문제되지 않는다.
	모든 Phantom-Reachble은 Published-Instance이기 때문이다.

5. Phantom_Publishing 의 잘못된 해법
	1) 현재 GC 수행 중이 아니라면, phantom-ref-Tree의 Published-flag만 복구한 후, 
	즉시 pantom-Q에 추가하여 다음 GC의 0 scan시 marking 할  수 있을 것
    같으나 실제로 불가하다. (여러 차례 검증된 결과임!!)
		a) GC 세대 이하로 Phantom-Publishing 된 Phantom-reachable ref-tree의 
		일부가 GC 세대보다 더 윗세대에 의해 참조되면서 Phantom-reachable 이 아닌
		Strong-Reachable 상태로 바뀐 경우, <해당 ref-tree는 아직 Public-Q로 옮겨진
		상태가 아니기 때문에> 해당 인스탄스에 새로 assing된 Normal-Instance는 
		scan할 방법이 없어, (Strong-Reachable임에도) finalize되어 버린다.
		b) Phantom_Publishing 시 GC 수행 여부와 관계없이 모든 phantom-reachable을 
		Public-Reachable 로 marking해 주어도 1)의 문제는 해결되지 않는다. 
		Blocking 기간 내에 두번째 GC Cycle이 수행되면 해당 Noraml-ref가 
		demarking되어 버리기 때문이다.

6. Phantom_Publishing 의 환경.
	1) GC 수행 도중 publishing된 instance는 모두 Reachable로 marking되나,
	다음 GC Cycle 이 곧바로 시작된 경우, demarking되지 않은 상태로 남는다.

	2) GC 수행 중이 아닌 경우 Phantom-Publishing된 instance는 Lost-Instance의
	Generation만 변경한다.

	3) finalize() 도중 blocking 이 발생한 경우, 해당 기간 동안 여러 차례의
	GC-Cycle이 수행될 수 있다.

7. 옳바른 해법.
	1) Finalize-Task의 local-stack을 scan하여 모든 Local-Reachable 을 
	marking 한다.
	- FinalizeTask가 Finalize() 수행 중 blocking 되었다면 m_pFinalizeQ 의 
	firstInstance가 아직 stack 내에 보관되어 있는 상태이므로, 
	현재 Finalize 되었거나 Finalizing 해야할 모든 Instance가 marking된다.
	- Task-priority상 FinalizeTask가 unlink 단계의 작업을 수행 중일 가능성은 없다.
	Loca-scan 중 또는 그 직후에 Finalizing 작업을 끝내고, publishing된
	인스탄스를 phantom_Q로 옮긴 후, 새로운 Q를 Waiting하고 있을 수 있다. 
	phantomQ에 인스탄스를 추가하는 시점에는 GC state가 변경되지 않으므로,
	모두 Reachable이거나, 또는 모두 Demarked 상태를 가지고 있게된다.
	(GC cycle 내에서 marking/demarking 한다.)

	2) Phantom-Marking.
	- GC 수행 직후, PhantomQ에 있는 Instance들을 Public-Q로 옮긴다.
		(이후엔 Phantom-Q에는 reachable phantom만이 저장된다.)
	- 
	- GC 수행 종료 직후에, public-FinalizableQ에 남겨진 Instance는 별도로 
	marking해 주어야만 한다. 해당 Q는 stack scan marking되지 않기 때문이다.
	- FinalizeTask가 Finalize() 수행 중 blocking 되었다면 m_pFinalizeQ 의 
	firstInstance가 아직 stack 내에 보관되어 있으나, 해당 Instance의
	m_pNext$ 는 scan-field가 아니므로 그 뒤에 연결된 instance는 scan되지
	않는다. 해당 Instance을 m_qFinalizing으로 보관하여 따로 can하여야 한다.
	- Phantom-Marking 후 곧바로 demarking 해버린다. (GC 종료 후 marking 하였으므로).

#endif


void fox_GC::markFinalizing(fastiva_Task* pTask) {
	pTask->lock_finalizer();

	fastiva_Instance_p pObj = pTask->m_pFinalizingQ;
	fastiva_Instance_p pFirst = pObj;
	while (pObj != ADDR_ZERO) {
		// 
		fm::gc_markFinalizerReachable(pObj);
		pObj = pObj->m_pNext$;
	}

	pObj = pTask->m_pFinalizing;
	if (pObj != ADDR_ZERO) {
		// 현재 finalize() 함수를 수행 중인 객체도 marking 해주어야만
		// 함수 수행 도중 참조 객체가 GC 되는 것을 방지할 수 있다.
		fm::gc_markFinalizerReachable(pObj);
	}

	pTask->unlock_finalizer();
}

#define NEXT_FINALIZABLE_Q(header)		((fastiva_Instance_p*)&header->m_monitor$)[0]
#define LAST_LOST_INSTANCE(LostQ)		((fastiva_Instance_p*)&LostQ->m_monitor$)[0]
#define DEPENDET_FINALIZABLE_Q(LostQ)	((fastiva_Instance_p*)&LostQ->m_monitor$)[1]

#if 0
int fox_GC::getFinalizableList(finalize_Task* pCurrTask, FinalizeInfo* pInfo) {
	fastiva_Task* pScannedTask = pCurrTask->m_pScannedTask;

	pScannedTask->lock_finalizer();

	fastiva_Instance_p pLost = pCurrTask->m_pLostQ;
	fastiva_Instance_p pFirst = pCurrTask->m_pFinalizableQ;
	if (pLost != ADDR_ZERO) {
		while (DEPENDET_FINALIZABLE_Q(pLost) == ADDR_ZERO) {
			fastiva_Instance_p pLastLost = LAST_LOST_INSTANCE(pLost);
			fastiva_Instance_p pNextLost = pLastLost->m_pNext$;
			LAST_LOST_INSTANCE(pLost)->m_pNext$ = ADDR_ZERO;
			LAST_LOST_INSTANCE(pLost) = ADDR_ZERO;
			g_GC.phantomQ.insert(pLost, pLastLost);
			pLost = pNextLost;
			if (pLost == ADDR_ZERO) {
				pCurrTask->m_pLostQ = ADDR_ZERO;
				goto check_fianlizable_q;
			}
		}
		KASSERT(pFirst != ADDR_ZERO);
#ifdef _DEBUG
		fastiva_Instance_p pHeader = pFirst;
		while (DEPENDET_FINALIZABLE_Q(pLost) != pHeader) {
			pHeader = NEXT_FINALIZABLE_Q(pHeader);
		}
#endif
		if (DEPENDET_FINALIZABLE_Q(pLost) != pFirst) {
			pCurrTask->m_pLostQ = pLost;
			pLost = ADDR_ZERO;
		}
		else {
			pCurrTask->m_pLostQ = LAST_LOST_INSTANCE(pLost)->m_pNext$;
			fastiva_Instance_p pLastLost = LAST_LOST_INSTANCE(pLost);
			pInfo->m_pLastLost = pLastLost;
			KASSERT(pLastLost != ADDR_ZERO);
			LAST_LOST_INSTANCE(pLost) = ADDR_ZERO;
			pLastLost->m_pNext$ = ADDR_ZERO;
			DEPENDET_FINALIZABLE_Q(pLost) = ADDR_ZERO;
		}
	}
	else {
check_fianlizable_q:
		if (pFirst == ADDR_ZERO) {
			pScannedTask->m_pFinalizeTask = ADDR_ZERO;
			pCurrTask->m_pFinalizingQ = ADDR_ZERO;
			pScannedTask->unlock_finalizer();
			return false;
		}
	}

	pInfo->m_pFirstLost = pLost;
	pInfo->m_pFinalizableQ = pFirst;

	pCurrTask->m_pFinalizableQ = NEXT_FINALIZABLE_Q(pFirst);
	NEXT_FINALIZABLE_Q(pFirst) = ADDR_ZERO;

	pScannedTask->unlock_finalizer();

	return true;
}
#endif

extern struct _JNIEnv* fastiva_jni_getEnv();

void fox_GC::call_finalize(fastiva_Task* pTask) {
	// just for call Env->AttachThread;
	_JNIEnv* pEnv = fastiva_jni_getEnv();

	KASSERT(pTask->m_pFinalizeTask == fastiva_getCurrentTask());

	fastiva_Instance_p pObj;
	int cntNFObject;
	while (true) {
		pTask->lock_finalizer();
		pObj = pTask->m_pFinalizingQ;
		if (pObj == NULL) {
			pTask->m_pFinalizeTask = NULL;
			pTask->m_pFinalizing = NULL;
			pTask->unlock_finalizer();
			break;
		}
		pTask->m_pFinalizing = pObj;
		pTask->m_pFinalizingQ = pObj->m_pNext$;
		pTask->unlock_finalizer();

		TRY$ {
			IF_GC_DEBUG {
				cntNFObject ++;
			}
			((java_lang_Object_p)pObj)->finalize();
		}
		CATCH_ANY$ {
			// ThreadDeathError를 무시한다.
			fox_printf("!!! Uncaught exception catched in call_finalize::run: \n");
			fm::printError(catched_ex$);
		}

		IF_GC_DEBUG {
			//const fastiva_InstanceContext * pCtx = fm::getInstanceContext(pObj);
			//fox_printf("Finalized : %s/%s\n", pCtx->getPackageName(), pCtx->m_pBaseName);
		}

		g_GC.lockScanMode(true);
		g_GC.phantomQ.insert(pObj);
		g_GC.releaseScanMode(true);
	}

	IF_GC_DEBUG {
		fox_printf("%d objects are finalized.\n", cntNFObject);
	}
	// the last instance;
	//pObj = pCurrTask->m_pFinalizingQ;
	//pCurrTask->m_pFinalizingQ = ADDR_ZERO;
	//return pLastObj; 
}


void fox_GC::runFinalize(void* param) {
#if defined(_DEBUG) && defined(__GNUC__)
	//gc_verbose_printf("[%d] runFinalize start\n", pthread_self());
#endif

	finalize_Task* pCurrTask = (finalize_Task*)fastiva_getCurrentTask();
	FASTIVA_BEGIN_MANAGED_SECTION(0);

	fox_printf("run finalizer %x\n", pCurrTask);

	fastiva_Task* pScanTask = pCurrTask->m_pScannedTask;
	call_finalize(pScanTask);

#if 0
	fastiva_Instance_p pLost;
	fastiva_Instance_p pLastObj;
	FinalizeInfo pInfo[1];
	while (getFinalizableList(pCurrTask->m_pScanTask, pInfo)) {
#if 1 // ndef UNLINK_IN_FINALIZER
		if (pInfo->m_pFinalizableQ != ADDR_ZERO) {
			pLastObj = call_finalize(pInfo->m_pFinalizableQ);
			g_GC.phantomQ.insert(pInfo->m_pFinalizableQ, pLastObj);
		}
		if (pInfo->m_pFirstLost != ADDR_ZERO) {
			g_GC.phantomQ.insert(pInfo->m_pFirstLost, pInfo->m_pLastLost);
		}
#else
		fastiva_Instance_p pFirst; 
		if ((pFirst = pCurrTask->m_pFinalizingQ) != ADDR_ZERO) {
			pLastObj = call_finalize(pCurrTask);
			fastiva_Instance_p pObj = pFirst; 
			sys_huge_heap_lock();
			while (pObj != ADDR_ZERO) {
				fastiva_Instance_p pNext = pObj->m_pNext$;

				if (g_GC.demarkPhantomRefReachable(pObj)
				||  g_GC.isPublished(pObj)) {
					g_GC.demarkReachable(pObj);
					g_GC.phantomQ.insert(pObj);
				}
				else {
					unlinkHeapInstance(pObj);
				}
				pObj = pNext;
			}
			sys_huge_heap_unlock();
		}
		else if (pLost == ADDR_ZERO) {
			break;
		}
		fastiva_Instance_p pObj = pLost; 
		sys_huge_heap_lock();
		while (pObj != ADDR_ZERO) {
			fastiva_Instance_p pNext = pObj->m_pNext$;

			if (g_GC.isPublished(pObj)) {
				g_GC.demarkReachable(pObj);
				g_GC.phantomQ.insert(pObj);
			}
			else {
				unlinkHeapInstance(pObj);
			}
			pObj = pNext;
		}
		sys_huge_heap_unlock();
#endif
	}


	if (pCurrTask->m_pUserData != ADDR_ZERO) {
		fastiva_releaseGlobalRef((java_lang_Thread_p)pCurrTask->m_pUserData);
		pCurrTask->m_pUserData = ADDR_ZERO;
	}
#endif
	FASTIVA_END_MANAGED_SECTION();
			fox_printf("run finalizer done %x\n", pCurrTask);

	IF_GC_DEBUG {
		g_cntFinalizeTask --;
	}
}

/*
void fox_GC::markPhantomReachable(fastiva_Task* pFinalizeTask) {
	KASSERT(pFinalizeTask != ADDR_ZERO);

	fastiva_Instance_p pLost = pFinalizeTask->m_pFinalizableQ->getFirst();
	while (pLost != ADDR_ZERO) {
		HeapMark* pMark = getHeapMark(pLost);
		// 현재 GC 가 종료된 상태이므로, 이미 Rechable 로 marking된 것은 
		// 모두 PublicReachble marking이다.
		if (pMark->isPublished() && !g_GC.isReachable()) {
			// pLost는 자체는 marking할 필요는 없다.
			fox_GC::visitFields(pLost, fox_GC::markReachableTree);
		}
	}
	pLost = pFinalizeTask->m_qFinalizing;
	while (pLost != ADDR_ZERO) {
		HeapMark* pMark = getHeapMark(pLost);
		if (pMark->isPublished() && !g_GC.isReachable()) {
			// pLost 자체는 marking할 필요는 없다.
			fox_GC::visitFields(pLost, fox_GC::markReachableTree);
		}
	}

	demarkQueue(pFinalizeTask->m_pFinalizableQ->getFirst());
	demarkQueue(pFinalizeTask->m_qFinalizing);
}
*/

/*
void fox_GC::addPublicLost(fastiva_Instance_p pFirstObj) {
	// 현재 Kernel::g_pSystemTask->m_pFinalizeTask 는 null이거나
	// suspend된 상태에서 단 한차례만 호출되어야 한다.

	fastiva_Instance_p pObj = pFirstObj;
	fastiva_Instance_p pLastLost;// = header;
	fastiva_Instance_p pNext;

	for (; pObj != ADDR_ZERO; pObj = pNext) {
		pNext = pObj->m_pNext$;

		GC_DEBUG_BREAK(pObj);
		KASSERT(pObj->m_cntRecursion$ == 0);
		//KASSERT(pObj->m_globalRef$ < g_GC.minRefCount);
		// 현재 추가 중인 Lost-Instance는 이미 추가된 Lost-Instance의
		// finalize 시에 Phantom-Reachable로 marking될 수 있다.
		// setField의 충돌을 방지하기 위해 mutex-lock을 사용한다.
		// 초기 mutex 충돌은 finalize-task와만 발생하므로
		// performance 영향은 미미하다.
		g_GC.demarkPublished(pObj);
		pLastLost = pObj;
	}

	//pFirstObj = header->m_pNext$;
	if (pFirstObj == ADDR_ZERO) {
		return;
	}

	fox_GC::addFinalizing(Kernel::g_pSystemTask, pFirstObj, pLastLost);

}
*/


fastiva_Instance_p fox_GC::getLastInstance(fastiva_Instance_p pObj) {
	fastiva_Instance_p pNext;
	while ((pNext = pObj->m_pNext$) != ADDR_ZERO) {
		pObj = pNext;
	}
	return pObj;
}

bool fox_GC::addFinalizing(fastiva_Task* pTask, fastiva_Instance_p pFirstObj, fastiva_Instance_p pLastObj) {

	int cntNFObject = 0;
	IF_GC_DEBUG {
		fastiva_Instance_p pObj = pFirstObj;
		for (; pObj != ADDR_ZERO; pObj = pObj->m_pNext$) {
			// 현재 pObj는 완전한 unreachable이다.
			// unreachable 검사 이 전에 미리 markFinalizing이 호출된
			// 상태이므로, 현재는 모든 잠재적인 rechable도 처리된 상태이다.
			GC_DEBUG_BREAK(pObj);
			KASSERT(!g_GC.isReachable(pObj));
		}
	}
	
	{
		fastiva_Instance_p pObj = pFirstObj;
		for (; pObj != ADDR_ZERO; pObj = pObj->m_pNext$) {
			// 현재 pObj는 완전한 unreachable이다.
			// unreachable 검사 이 전에 미리 markFinalizing이 호출된
			// 상태이므로, 현재는 모든 잠재적인 rechable도 처리된 상태이다.
			GC_DEBUG_BREAK(pObj);
			KASSERT(g_GC.isFinalizable(pObj));
			KASSERT(pObj->m_monitor$.m_cntRecursion$ == 0);
			KASSERT(pObj->m_globalRef$ == 0);

			/**
			2011.02.01 NF 객체를 publishing 한다.
			NF 객체의 subtree 는 적어도 2개 이상의 thread에 의해 참조가능하기 때문.
			*/
#ifdef MARK_FINALIZING
			if (!isPublished(pObj)) {
				publishInstance(pObj);
			}
			else {
				fm::gc_markFinalizerReachable(pObj);
			}
#endif
			/**
			2012.02.01 publish 상태를 해제한다.
			subtree 는 published 된 상태이므로,
			subtree 내의 객체 또는 다른 global 객체의 field에 assign 하는 경우,
			published 도 상태가 변경된다. (그렇지 않으면 전체 call_finalize() 종료 후 삭제 가능)
			참고) NFObject 에서 NFObject 를 참조하는 경우, 해당 객체는 다시 publish 될 수 있다.
			해당 문제는 무시한다.
			*/
			g_GC.demarkFinalizable(pObj);
			// while loop 내에서 위의 명령을 사용할 수 없다.
			// 이미 re-assign에 의해 reachable 상태로 바뀐 뒤 일 수 있기 때문이다.
			IF_GC_DEBUG {
				cntNFObject ++;
			}
		}
	}

	IF_GC_DEBUG {
		gc_debug_printf("add Finalizing (%d)\n", cntNFObject);
	}

	// 나중에 추가된 Lost-Instance들은 이 전에 추가된 Lost-Instance에 의해
	// 참조되고 있을 수 있다. (그 역은 성립하지 않는다.) 따라서, 
	// 이전에 추가된 Lost-Instance는 새로 추가된 Instance에 관계없이 
	// unlink할 수 있으나, 새로 추가된 lost-instance 는 
	// 이전에 추가된 instance에 대한 finalizing이 끝나기 전까지 
	// unlink할 수 없다.


	// Finalize-Task는 높은 priority(8)를 가진다. (10은 real-time task를 위해 예약)
	// 따라서 Finalizable-Instance 는 finalize-task 생성 직후, 해제되어버릴 가능성이 높다.
	// 그러나, Finalize() 함수 내에서 task가 blocking 될 가능성은 항상 있으므로,
	// addFinalizing() 수행 직 후, 잠시 기다린 뒤에 task 수행이 종료되지 않았으면,
	// 해당 Finalizable-Instance 의 ref-tree를 Reachable-marking 해주고, 
	// 모든 Unrechable-Instance를 즉시 unlink한다.
	// finalizing이 모두 끝난 후, Lost-Instance를 해제하는 방식을 취할 수도 있으나,
	// 그러기 위해선 Lost-Instance-Q를 별도로 생성해주어야 하는 부담이 있다.
	// 인스탄스의 개수가 매우 많은 경우엔, Lost-Instance-Q를 별도로 만드는 것 자체가
	// 상당한 부담이 된다.

	KASSERT(pFirstObj == ADDR_ZERO || pLastObj->m_pNext$ == ADDR_ZERO);
	KASSERT(pFirstObj == ADDR_ZERO || NEXT_FINALIZABLE_Q(pFirstObj) == ADDR_ZERO);

	finalize_Task* pFinalizeTask = pTask->m_pFinalizeTask;
	if (pFinalizeTask != ADDR_ZERO) {
		pTask->lock_finalizer();
		pFinalizeTask = pTask->m_pFinalizeTask;
		if (pFinalizeTask != ADDR_ZERO) { // 한 번 더 검사.
			KASSERT(pFinalizeTask->m_pScannedTask == pTask);
			fastiva_Instance_p pHeader;

			pLastObj->m_pNext$ = pTask->m_pFinalizingQ;
			pTask->m_pFinalizingQ = pFirstObj;
		}
		pTask->unlock_finalizer();
		return true;
	}

	// finalizer-tak priority
	// 10은 real-time performance를 위해 예약된 것이므로,
	// priority 8을 사용한다.
	pFinalizeTask = (finalize_Task*)fox_kernel_createTaskContext(sizeof(finalize_Task));
	gc_debug_printf("finalizing task created %x\n", pFinalizeTask);
	fox_task_create_ex(fm::thread_crt0, (void*)-1, 8, pFinalizeTask, 0);
	//fox_task_create(fm::thread_crt0, , 8);
	//sys_task_initGCTask(pFinalizeTask);
	pTask->m_pFinalizeTask = pFinalizeTask;
	pTask->m_pFinalizingQ = pFirstObj;
	pFinalizeTask->m_pScannedTask = pTask;
	fox_task_resume(pFinalizeTask);
	return true;
}

#if 0
bool fox_GC::addLost(fastiva_Task* pTask, fastiva_Instance_p pFirstLost, fastiva_Instance_p pLastLost) {

#ifdef _DEBUG
	int cntLost = 0;
	{
		fastiva_Instance_p pObj = pFirstLost;
		for (; pObj != ADDR_ZERO; pObj = pObj->m_pNext$) {
			// 현재 pObj는 완전한 unreachable이다.
			// unreachable 검사 이 전에 미리 markFinalizing이 호출된
			// 상태이므로, 현재는 모든 잠재적인 rechable도 처리된 상태이다.
			GC_DEBUG_BREAK(pObj);
			KASSERT(!g_GC.isReachable(pObj));
			KASSERT(!g_GC.isFinalizable(pObj));
			KASSERT(pObj->m_monitor$.m_cntRecursion$ == 0);
			KASSERT((pObj->m_globalRef$ & 0x7FFF) == 0);
			// while loop 내에서 위의 명령을 사용할 수 없다.
			// 이미 re-assign에 의해 reachable 상태로 바뀐 뒤 일 수 있기 때문이다.
			cntLost ++;
		}
	}
#endif

	// 나중에 추가된 Lost-Instance들은 이 전에 추가된 Lost-Instance에 의해
	// 참조되고 있을 수 있다. (그 역은 성립하지 않는다.) 따라서, 
	// 이전에 추가된 Lost-Instance는 새로 추가된 Instance에 관계없이 
	// unlink할 수 있으나, 새로 추가된 lost-instance 는 
	// 이전에 추가된 instance에 대한 finalizing이 끝나기 전까지 
	// unlink할 수 없다.


	// Finalize-Task는 높은 priority(8)를 가진다. (10은 real-time task를 위해 예약)
	// 따라서 Finalizable-Instance 는 finalize-task 생성 직후, 해제되어버릴 가능성이 높다.
	// 그러나, Finalize() 함수 내에서 task가 blocking 될 가능성은 항상 있으므로,
	// addFinalizing() 수행 직 후, 잠시 기다린 뒤에 task 수행이 종료되지 않았으면,
	// 해당 Finalizable-Instance 의 ref-tree를 Reachable-marking 해주고, 
	// 모든 Unrechable-Instance를 즉시 unlink한다.
	// finalizing이 모두 끝난 후, Lost-Instance를 해제하는 방식을 취할 수도 있으나,
	// 그러기 위해선 Lost-Instance-Q를 별도로 생성해주어야 하는 부담이 있다.
	// 인스탄스의 개수가 매우 많은 경우엔, Lost-Instance-Q를 별도로 만드는 것 자체가
	// 상당한 부담이 된다.

	KASSERT(pFirstLost == ADDR_ZERO || pLastLost->m_pNext$ == ADDR_ZERO);
	KASSERT(pFirstLost == ADDR_ZERO || LAST_LOST_INSTANCE(pFirstLost) == ADDR_ZERO);
	KASSERT(pFirstLost == ADDR_ZERO || DEPENDET_FINALIZABLE_Q(pFirstLost) == ADDR_ZERO);

	finalize_Task* pFinalizeTask = pTask->m_pFinalizeTask;
	if (pFinalizeTask != ADDR_ZERO) {
		pTask->lock_finalizer();
		pFinalizeTask = pTask->m_pFinalizeTask;
		if (pFinalizeTask != ADDR_ZERO) { // 한 번 더 검사.
			KASSERT(pFinalizeTask->m_pScannedTask == pTask);
			fastiva_Instance_p pHeader;

			pHeader = pFinalizeTask->m_pLostQ;
			if (pHeader != ADDR_ZERO) {
				while (LAST_LOST_INSTANCE(pHeader)->m_pNext$ != ADDR_ZERO) {
					pHeader = LAST_LOST_INSTANCE(pHeader)->m_pNext$;
				}
				LAST_LOST_INSTANCE(pHeader)->m_pNext$ = pFirstLost;
				LAST_LOST_INSTANCE(pHeader) = pLastLost;
			}
			else {
				// call_finalize()를 수행 중이다.
				pFinalizeTask->m_pLostQ = pFirstLost;
				LAST_LOST_INSTANCE(pFirstLost) = pLastLost;
				pHeader = pFinalizeTask->m_pFinalizableQ;
				if (pHeader != ADDR_ZERO) {
					while (NEXT_FINALIZABLE_Q(pHeader) != ADDR_ZERO) {
						pHeader = NEXT_FINALIZABLE_Q(pHeader);
					}
				}
				DEPENDET_FINALIZABLE_Q(pFirstLost) = pHeader;
			}
		}
		pTask->unlock_finalizer();
	}

	if (pFinalizeTask != ADDR_ZERO) {
		return true;
	}

fast_unlink:
	// clearQueue 도중에 FinalizerTask가 종료된 경우이다.
	sys_huge_heap_lock();
	while (pFirstLost != ADDR_ZERO) {
		fastiva_Instance_p pNext = pFirstLost->m_pNext$;
		fox_GC::unlinkHeapInstance(pFirstLost);
		pFirstLost = pNext;
	}
	sys_huge_heap_unlock();
	return true;
}
#endif

void fox_GC::doLocalGC(fastiva_Task* pCurrTask) {

    return;
	if (GC_ENABLE_LOCAL_REF) {

	KASSERT(pCurrTask == fox_task_currentTask());

	pCurrTask->m_cntLocalRookie = 0;
	if (pCurrTask->m_pLocalQ == ADDR_ZERO) {
		return;
	}

	AutoLocalHeapLock __critical_section(pCurrTask);
	
	fastiva_Instance_p pFirstRookie = pCurrTask->m_pLocalQ;
	fastiva_Instance_T$ header;
	header.m_pNext$ = pFirstRookie;

	fastiva_StackContext* pStackContext = pCurrTask->m_pStackContext;
	pStackContext->m_pStack = &pCurrTask;

	while (pStackContext != ADDR_ZERO) {
		KASSERT(pStackContext->isClosed());
		int* stack_bottom = (int*)pStackContext->getBottom();
		int* stack_top    = (int*)pStackContext->getTop();
		while (stack_top < stack_bottom) {
			fastiva_Instance_p pObj = (fastiva_Instance_p)*stack_top ++;
			pObj = fox_GC::getHeapInstance(pObj);
			if (pObj != ADDR_ZERO && !g_GC.isPublished(pObj)) {
				/* 현재는 stack-marking을 하기 전 또는 그 이 후이다.
				   GC-task에 의한 stack-marking과 충돌되지 않으므로
				   mark를 thread-safe하게 바꿀 수 있다.
				*/
				*(unsigned char*)&pObj->m_mark$ = 1;
		}
		}
		pStackContext = (fastiva_StackContext*)pStackContext->m_pPrev;
	}

	fastiva_Instance_p pRookie = pFirstRookie;
	while (pRookie != ADDR_ZERO) {
		/*
		참고. finalizable Object 들에 대해서는 dereferece가 이루어지지 않으므로
		finalizable에 의해 참조되는 Object는 Unlink되지 않는다.
		*/
		fastiva_Instance_p pNext = pRookie->m_pNext$;
		if (pRookie->m_localRef$ == 0 
		&&  !g_GC.isPublished(pRookie) 
		&&  (pRookie->m_mark$ & 0xFF) != 1) {
#if PROTOTYPE_GC
			java_lang_Class_p pClass = pRookie->getClass$();
			pClass->derefLocalTree$(pRookie);
#else
			fm::gc_derefLocalRef(pRookie);
#endif
		}
		pRookie = pNext;
	}

	fastiva_Instance_p pPrev = &header;
	pRookie = pFirstRookie;
	int cntUnlink = 0;
	sys_huge_heap_lock();
	while (pRookie != ADDR_ZERO) {
		fastiva_Instance_p pNext = pRookie->m_pNext$;
		if (pRookie->m_localRef$ == 0 && !g_GC.isPublished(pRookie) && (pRookie->m_mark$ & 0xFF) != 1) {
#ifdef _DEBUG
			pRookie->m_localRef$ = -1;
#endif
			fox_GC::unlinkHeapInstance(pRookie);
			pPrev->m_pNext$ = pNext;
		}
		else {
			pPrev = pRookie;
		}
		pRookie = pNext;
	}
	sys_huge_heap_unlock();

	pCurrTask->setLocalQ_unsafe(header.m_pNext$);
}
}



// ========================================================================= //
// java_Thread crt0
// ========================================================================= //
void FOX_FASTCALL(sys_monitor_destroySyncData)(fox_Task* pTask);

void sys_task_detachNativeTask(fox_Task* pTask) {

	java_lang_Thread_p pJavaThread = (java_lang_Thread_p)pTask->m_pUserData;
	fox_printf("thread terminated %x, %x\n", pJavaThread, pTask);
	if (pJavaThread != ADDR_ZERO) {
		pJavaThread->m_hTask$ = ADDR_ZERO;
		{	SYNCHRONIZED$(pJavaThread)
			// 누군가 wait하고 있다면 unlink되지 않는다.
			fox_util_dec32((uint*)&kernelData.g_activeThreadCount_interlocked);
			//notifyAll을 하지 않으면 join에서 계속 wait를 하고 있으므로
			//wait가 깨어나지 않는다. 따라서 notifyAll을 호출한다.
			//pJavaThread->notify();
			pJavaThread->notifyAll();
		}

		fastiva_releaseGlobalRef(pJavaThread);
		//pCurrTask->setTerminated();

		if (!pJavaThread->isDaemon()) {
			fox_util_dec32((uint*)&kernelData.g_userThreadCount_interlocked);
		}
	}

	pTask->setTerminated();
	sys_monitor_destroySyncData(pTask);

	// fox_task_wake(Kernel::g_pSystemTask);
}

void fm::thread_crt0(void* param) {
#if defined(_DEBUG) && defined(__GNUC__)
	//gc_verbose_printf("[%d] fm::thread_crt0 start\n", pthread_self());
#endif
	java_lang_Thread_p volatile pJavaThread = (java_lang_Thread_p)param;
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	pCurrTask->setState(RUNNABLE);

	//fox_GC::markPublished(GC_FIELD_REF_OF(pJavaThread));

	FASTIVA_BEGIN_MANAGED_SECTION(0);

	bool isFinalizer = pJavaThread == (void*)-1;
	if (isFinalizer) {
		pCurrTask->m_pUserData = 0;
		pJavaThread = FASTIVA_NEW(java_lang_Thread)();
		pJavaThread->m_hTask$ = pCurrTask;
	}

	KASSERT(pCurrTask == (fastiva_Task*)pJavaThread->m_hTask$);

	fm::attachThread(pJavaThread, pCurrTask);//

	TRY$ {
		/*
		fox_scheduler_lock();	// suspend()상태 해결.
		// 현재 TASK는 SUSPEND 상태일 수 있다. (Non-STARTED)
		// Scheduler-Lock 호출시 이문제가 해소된다.
		jbool isTerminatedBeforeStart = pCurrTask->m_pMonitor == (void*)-1;
		KASSERT(pCurrTask->m_pMonitor == (void*)1 || pCurrTask->m_pMonitor == (void*)-1);
		pCurrTask->m_pMonitor = ADDR_ZERO;
		fox_scheduler_unlock();
		*/
		//KASSERT(!fox_scheduler_isLocked());

		if (!pCurrTask->isTerminated()) { //isTerminatedBeforeStart) {
			if (isFinalizer) {
				fox_GC::call_finalize(pCurrTask->m_pScannedTask);
			}
			else {
				pJavaThread->run();
			}
		}
		catched_ex$ = ADDR_ZERO;
	}
	CATCH_ANY$ {
		fox_printf("!!! Uncaught exception catched in thread_crt0: \n");
		fm::printError(catched_ex$);
		java_lang_Throwable_p pUncaught = catched_ex$;
		#if defined(FASTIVA_J2SE) 
			TRY$ {
				java_lang_ThreadGroup_p pGroup = pJavaThread->getThreadGroup();
				pGroup->uncaughtException(pJavaThread, pUncaught);
			}
			CATCH_ANY$ {
				fm::printError(catched_ex$);
			}
		#endif
	}


	#if !JPP_JDK_IS_ANDROID() // JPP_ENABLE_THREAD_GROUP
	{
		//java_lang_Thread의 exit를 호출해야만 ThreadGroup에서 현 Thread를 제거한다.
		TRY$ {
			//CDC에서는 exit에서 ThreadGroup을 제거하는것 뿐만 아니라,
			//java_lang_ThreadGroup의 uncaughtException를 호출하여 준다.
			pJavaThread->exit((java_lang_Throwable_p)catched_ex$);
		}
		CATCH_ANY$ {
			catched_ex$->printStackTrace((java_io_PrintStream_p)ADDR_ZERO);
		}
	}
	#endif

#ifdef _WIN32
	sys_task_detachNativeTask(pCurrTask);
#else
	// pthread_key_destroyed() 에 의해 호출된다.
#endif
	// v3 임시로 막음. g_GC.resumeScanning();

	FASTIVA_END_MANAGED_SECTION();

	//fox_task_stop(pCurrTask);
}

/*
void fox_GC::visitFields(fastiva_Instance_p pObj, FIELD_VISITOR pfnVisitor) {

	const fastiva_Class_p pClass = pObj->getClass();
	if (fm::getArrayDimension(pClass) == 0) {
		const fastiva_InstanceContext* pCtx = fm::getInstanceContext(pClass);

		const unsigned short* pOffset = pCtx->m_aScanOffset;
		for (int offset; (offset = *pOffset ++) != 0; ) {
			int offset = offset & ~3;
			fastiva_Instance_p pValue = *(fastiva_Instance_p*)((int)pObj + offset);
			if (pValue != FASTIVA_NULL) {
				if (offset != offset) {
					pValue = ((fastiva_Interface_p)pValue)->getInstance$();
				}
				pfnVisitor(pValue);
#ifdef _DEBUG
				/*
				if (g_GC.isPublished(pObj) > g_GC.isPublished(pValue)) {
					// 진행중인 PUBLSHING을 종료할 여유를 준다.
					sys_task_spin();
					pValue = *(fastiva_Instance_p*)((int)pObj + offset);
					if (offset != offset) {
						pValue = ((fastiva_Interface_p)pValue)->getInstance$();
					}
					KASSERT(g_GC.isPublished(pObj) <= g_GC.isPublished(pValue));
				}
				*
#endif
			}
		}
	}
	else if (!pClass->m_pContext$->isPrimitive()) {
		java_lang_Object_ap pArray = (java_lang_Object_ap)pObj;
		fastiva_Instance_p* ppValue = fm::getPointerBuffer(pArray);
		for (int i = pArray->length(); i -- > 0; ) {
			fastiva_Instance_p pValue = *ppValue ++;
			if (pValue != FASTIVA_NULL) {
				pfnVisitor(pValue);
#ifdef _DEBUG
				/*
				if (g_GC.isPublished(pObj) > g_GC.isPublished(pValue)) {
					// 진행중인 PUBLSHING을 종료할 여유를 준다.
					sys_task_spin();
					pValue = ppValue[-1];
					KASSERT(g_GC.isPublished(pObj) <= g_GC.isPublished(pValue));
				}
				*
#endif
			}
		}
	}
}

void fox_GC::visitFieldsEx(fastiva_Instance_p pObj, int param, FIELD_VISITOR_EX pfnVisitor) {
	const fastiva_Class_p pClass = pObj->getClass();
	if (fm::getArrayDimension(pClass) == 0) {
		const fastiva_InstanceContext* pCtx = fm::getInstanceContext(pClass);

		const unsigned short* pOffset = pCtx->m_aScanOffset;
		for (int offset; (offset = *pOffset ++) != 0; ) {
			int offset = offset & ~3;
			fastiva_Instance_p pValue = *(fastiva_Instance_p*)((int)pObj + offset);
			if (pValue != FASTIVA_NULL) {
				if (offset != offset) {
					pValue = ((fastiva_Interface_p)pValue)->getInstance$();
				}
				pfnVisitor(pValue, param);
				KASSERT(g_GC.isPublished(pObj) == g_GC.isPublished(pValue));
			}
		}
	}
	else if (!pClass->m_pContext$->isPrimitive()) {
		java_lang_Object_ap pArray = (java_lang_Object_ap)pObj;
		fastiva_Instance_p* ppValue = fm::getPointerBuffer(pArray);
		for (int i = pArray->length(); i -- > 0; ) {
			fastiva_Instance_p pValue = *ppValue ++;
			if (pValue != FASTIVA_NULL) {
				pfnVisitor(pValue, param);
				KASSERT(g_GC.isPublished(pObj) == g_GC.isPublished(pValue));
			}
		}
	}
}


void fox_GC::visitFieldsEx3(fastiva_Instance_p pObj, int param1, int param2, FIELD_VISITOR_EX3 pfnVisitor) {
	const fastiva_Class_p pClass = pObj->getClass();
	if (fm::getArrayDimension(pClass) == 0) {
		const fastiva_InstanceContext* pCtx = fm::getInstanceContext(pClass);

		const unsigned short* pOffset = pCtx->m_aScanOffset;
		for (int offset; (offset = *pOffset ++) != 0; ) {
			int offset = offset & ~3;
			fastiva_Instance_p pValue = *(fastiva_Instance_p*)((int)pObj + offset);
			if (pValue != FASTIVA_NULL) {
				if (offset != offset) {
					pValue = ((fastiva_Interface_p)pValue)->getInstance$();
				}
				pfnVisitor(pValue, param1, param2);
				KASSERT(g_GC.isPublished(pObj) == g_GC.isPublished(pValue));
			}
		}
	}
	else if (!pClass->m_pContext$->isPrimitive()) {
		java_lang_Object_ap pArray = (java_lang_Object_ap)pObj;
		fastiva_Instance_p* ppValue = fm::getPointerBuffer(pArray);
		for (int i = pArray->length(); i -- > 0; ) {
			fastiva_Instance_p pValue = *ppValue ++;
			if (pValue != FASTIVA_NULL) {
				pfnVisitor(pValue, param1, param2);
				KASSERT(g_GC.isPublished(pObj) == g_GC.isPublished(pValue));
			}
		}
	}
}
*/


#ifdef FASTIVA_SUPPORT_WEAK_REF

void fastiva_vm_Reference__clear(java_lang_ref_Reference_p self) {
	self->m_referent = (jint)FASTIVA_NULL;
}

java_lang_Object* fastiva_vm_Reference__get(java_lang_ref_Reference_p self) {
	java_lang_Object* pReferent;

	pReferent = (java_lang_Object*)self->m_referent;

	if (pReferent == ADDR_ZERO) {
		return pReferent;
	}

	int type = self->getClass$()->m_pContext$->m_accessFlags
		& (ACC_PHANTOM_REF$ | ACC_SOFT_REF$ | ACC_WEAK_REF$);
	// phantom-ref는 get() 함수 호출시 현재 함수로 절대 진입하지 않는다.
	KASSERT(type != 0 && type != ACC_PHANTOM_REF$);

	fastiva_Task* pCurrTask = fastiva_getCurrentTask();

	g_GC.lockScanMode(false);
	// 한 번 더 검사.
	pReferent = (java_lang_Object*)self->m_referent;
	if (pReferent != NULL && g_GC.getScanMode() >= TASK_SCANNING) {
		// scan 중인, 또는 이미 scan이 끝난 task의 stack으로 
		// 해당 referent가 넘겨질 수 있다.
		AutoLocalHeapLock __critical_section(pCurrTask);
//		if (pCurrTask->m_scanState == IN_NEED_SCAN) {
#if PROTOTYPE_GC
			fox_GC::markReachableTree(pReferent);
#else
			fm::gc_markStrongReachable(pReferent);
#endif
//		}
	}
	g_GC.releaseScanMode(false);

	return pReferent;
}

void fastiva_vm_Reference__setReferent(java_lang_ref_Reference_p self, java_lang_Object* pReferent) {
	// 본 함수는 constructor에서만 호출되므로, old-value는 존재하지 않느다.
	KASSERT(self->m_referent == ADDR_ZERO);
	if (pReferent == ADDR_ZERO) {
		fox_GC::registerLocalInstance(fastiva_getCurrentTask(), self, 100);
		return;
	}

 	// 2011.0111 본 함수는 Constructor에 의해서만 호출된다.
	// self 를 reachable marking 할 필요는 없으나 구현 편의상 publishInstance()를 호출한다.
	fox_GC::publishInstance(self);

	fox_GC::publishInstance(pReferent);

	// publishing 직 후에 this의 reachable 상태가 변경되었을 수 있으나,
	// 현재 Reference를 생성 중이므로, stack-marking에 의해서만 가능하다.
	// stack-markin 시에는 pReferent도 동시에 marking되므로 염려할 필요가 없다.
	self->m_referent = (int)pReferent;

	int ref_type = self->getClass$()->m_pContext$->m_accessFlags & (ACC_PHANTOM_REF$ | ACC_SOFT_REF$ | ACC_WEAK_REF$);
	KASSERT (ref_type != 0);
	/*
	referent 값이 설정되기 전에 scan되는 것을 방지하기 위하여
	referent 값을 assign 할 때, Reference-Q에 추가한다.
	*/
	KASSERT(self->m_pNext$ == ADDR_ZERO);

	g_GC.lockScanMode(true);
	if (ref_type == ACC_SOFT_REF$) {
		g_GC.softRefQ.insert(self);
	}
	else {
		g_GC.referenceQ.insert(self);
	}
	g_GC.releaseScanMode(true);

}


void* fox_GC::validateReferent(java_lang_ref_Reference* pRef, bool markNow) {

	fastiva_Instance_p pReferent = (fastiva_Instance_p)pRef->m_referent;
	if (pReferent == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	
	if (g_GC.isReachable(pReferent)) {
		return pReferent;
	}

	/*
	하나의 Referent를 여러개의 Reference에서 참조할 수 있음을 유의한다.
	validate 검사 직전에 referent 값이 참조될 수 있다. 
	scanMode Lock을 통해 이를 방지한다.
	*/
	KASSERT(g_GC.isScanModeLocked());
	if (!g_GC.isReachable(pReferent)) { // 한번 더 검사.
		KASSERT(g_GC.isPublished(pReferent));
		if (!markNow) {
			pRef->m_referent = (int)(pReferent = ADDR_ZERO);
			if (pRef->m_queue != ADDR_ZERO) {
				pRef->m_queue->enqueue(pRef);
			}
		}
		else {
			AutoLocalHeapLock __critical_section(fastiva_getCurrentTask());
#if PROTOTYPE_GC
			fox_GC::markReachableTree(pReferent);
#else
			fm::gc_markStrongReachable(pReferent);
#endif
		}
	}
	return pReferent;
}
#endif

class WeakGlobalRef : public java_lang_ref_WeakReference {
	virtual void just_for_vtable_generation() {};
};
WeakGlobalRef g_weak_global_ref;

java_lang_Object_p fm::createGlobalRef(java_lang_Object_p pObj) {
	if (pObj != FASTIVA_NULL) {
		fastiva_lockGlobalRef(pObj);
	}
	return pObj;
}

java_lang_Object_p fm::deleteGlobalRef(java_lang_Object_p pObj) {
	if (pObj != FASTIVA_NULL) {
		fastiva_releaseGlobalRef(pObj);
	}
	return pObj;
}

jint fm::EnsureLocalCapacity(jint capacity) {
	return 0;
}

jint fm::PushLocalFrame(jint capacity) {
	return 0;
}

java_lang_Object_p fm::popLocalFrame(java_lang_Object_p pObj) {
	return fm::createLocalRef(pObj);
}

java_lang_Object_p fm::createWeakGlobalRef(java_lang_Object_p pObj) {
	// vtable을 이용하여 일반 WeakReference와 GlobalWeakRefernece를 구별한다.
	java_lang_ref_WeakReference_p pRef = FASTIVA_NEW(WeakGlobalRef)(pObj);
	fastiva_lockGlobalRef(pRef);
	return pRef;
}

java_lang_Object_p fm::deleteWeakGlobalRef(java_lang_Object_p pObj) {
	fastiva_releaseGlobalRef(pObj);
	return pObj;
}

jbool fm::isSameObject(fastiva_Instance_p pObj1, fastiva_Instance_p pObj2) {
	if (pObj1 == 0) {
		return pObj2 == 0;
	}
	if (*(int*)pObj1 == *(int*)&g_weak_global_ref) {
		pObj1 = ((java_lang_ref_Reference_p)pObj1)->get();
		// vtable 이 서로 같으면,
	}
	if (*(int*)pObj2 == *(int*)&g_weak_global_ref) {
		pObj2 = ((java_lang_ref_Reference_p)pObj2)->get();
	}
	//pObj1 = fm::getRealInstance(pObj1);
	//pObj2 = fm::getRealInstance(pObj2);
	return pObj1 == pObj2;
}

java_lang_Object_p fm::createLocalRef(java_lang_Object_p pObj) {
	if (pObj == FASTIVA_NULL) {
	}
	else if (*(int*)pObj == *(int*)&g_weak_global_ref) {
		pObj = ((java_lang_ref_Reference_p)pObj)->get();
		// vtable 이 서로 같으면,
	}
	return pObj;
}
java_lang_Object_p fm::deleteLocalRef(java_lang_Object_p pObj) {
	return pObj;
}

inline bool IsPrimitiveProperty(fastiva_Instance_p addr) {
	return ((int)addr <= (int)0x10001 || (uint)addr >= 0xFFFF0000); 
}

void fm::markStackTouched(fastiva_Instance_p pObj) {
	if (g_GC.isStackTouched(pObj)) {
#if PROTOTYPE_GC
		fastiva_Prototype* pClass = 0;
#endif
		if (g_GC.isPublished(pObj)) {
#if PROTOTYPE_GC
			pClass->fastiva_Prototype::markPublishedReachableBabyTree$(pObj);
#else
			g_GC.markStrongReachable(pObj);
			fm::gc_markPublicReachable(pObj);
			//fastiva_Instance::markPublicReachable(pObj);
#endif
		}
		else {
#if PROTOTYPE_GC
			pClass->fastiva_Prototype::markReachableTree$(pObj);
#else
			fm::gc_markStrongReachable(pObj);
#endif
		}
	}
}



#if PROTOTYPE_GC
void fastiva_Prototype::markReachableTree$(fastiva_Instance_p pObj) {
	g_GC.markStrongReachable(pObj);
	const fastiva_InstanceContext* pCtx = fm::getInstanceContext((java_lang_Class_p)this);

	const unsigned short* pOffset = pCtx->m_aScanOffset;
	for (int offset; (offset = *pOffset ++) != 0; ) {
		//int offset = offset & ~3;
		fastiva_Instance_p pValue = *(fastiva_Instance_p*)((int)pObj + offset);
		if (pValue != FASTIVA_NULL) {
			//if (offset != offset) {
			//	pValue = ((fastiva_Interface_p)pValue)->getInstance$();
			//}
			fox_GC::markReachableTree(pValue);
		}
	}
	return;
}


void fastiva_Prototype::derefLocalTree$(fastiva_Instance_p pObj) {
	const fastiva_InstanceContext* pCtx = fm::getInstanceContext((java_lang_Class_p)this);

	const unsigned short* pOffset = pCtx->m_aScanOffset;
	for (int offset; (offset = *pOffset ++) != 0; ) {
		fastiva_Instance_p pValue = *(fastiva_Instance_p*)((int)pObj + offset);
		if (pValue != FASTIVA_NULL) {
			fox_GC::derefLocalTree(pValue);
		}
	}
	return;
}


void fastiva_Prototype::markPublishedBabyTree$(fastiva_Instance_p pObj) {
	g_GC.markPublished(pObj);
	const fastiva_InstanceContext* pCtx = fm::getInstanceContext((java_lang_Class_p)this);

	const unsigned short* pOffset = pCtx->m_aScanOffset;
	for (int offset; (offset = *pOffset ++) != 0; ) {
		//int offset = offset & ~3;
		fastiva_Instance_p pValue = *(fastiva_Instance_p*)((int)pObj + offset);
		if (pValue != FASTIVA_NULL) {
			//if (offset != offset) {
			//	pValue = ((fastiva_Interface_p)pValue)->getInstance$();
			//}
			if (!g_GC.isPublished(pValue)) {
				fox_GC::markPublishedBabyTree(pValue);
			}
		}
	}
	return;
}


void fastiva_Prototype::markPublishedReachableBabyTree$(fastiva_Instance_p pObj) {
	g_GC.markPublishedReachable(pObj);
	const fastiva_InstanceContext* pCtx = fm::getInstanceContext((java_lang_Class_p)this);

	const unsigned short* pOffset = pCtx->m_aScanOffset;
	for (int offset; (offset = *pOffset ++) != 0; ) {
		//int offset = offset & ~3;
		fastiva_Instance_p pValue = *(fastiva_Instance_p*)((int)pObj + offset);
		if (pValue != FASTIVA_NULL) {
			//if (offset != offset) {
			//	pValue = ((fastiva_Interface_p)pValue)->getInstance$();
			//}
			if (!g_GC.isPublished(pValue)) {
				fox_GC::markPublishedReachableBabyTree(pValue);
			}
			else if (!g_GC.isReachable(pValue)) {
				fox_GC::markReachableTree(pValue);
			}
		}
	}
	return;
}





#endif

// 참고) PrimitiveArray는 내부 item을 marking할 필요가 없으므로,
// java_lang_Object의 scanInstance를 inherit하여 사용한다.
void fm::scanPointerArray(java_lang_Object_ap pArray, FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner) { 
	java_lang_Object_p* ppValue = fm::getUnsafeBuffer(pArray);
	GC_DEBUG_BREAK(pArray);

	for (int i = pArray->length(); i -- > 0; ) {
		java_lang_Object_p pValue = *ppValue ++;
		if (pValue != FASTIVA_NULL) {
			method(pValue, scanner);
		}
	}
	
}

#if FASTIVA_SUPPORTS_JAVASCRIPT
static bool __inLocalScanTask() {
	return g_GC.getScanMode() == TASK_SCANNING
		&& fox_task_currentTask() == Kernel::g_pSystemTask;
}

여기도 수정!!
void com_wise_jscript_JsActivation::scanInstance$(FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner) {

	com_wise_jscript_JsActivation_p pObj = this;
	if (pObj->m_pLocalFrame == ADDR_ZERO
	||  t == MARK_PUBLISHED || t == DEC_LOCAL_REF
	||  __inLocalScanTask()) {
		// pObj->m_pLocalFrame == ADDR_ZERO => stack 참조가 끝난 이후이다.
		// LocalStackScan 중에는 Instance를 모두 마킹한다.
		switch (t) {
			case SCAN_TYPE_A:
				VTABLE$::JPP_SCAN_TYPE_A(this);
				break;
			case SCAN_TYPE_B:
				VTABLE$::JPP_SCAN_TYPE_B(this);
				break;
			case SCAN_TYPE_C:
				VTABLE$::JPP_SCAN_TYPE_C(this);
				break;
			case SCAN_TYPE_D:
				VTABLE$::JPP_SCAN_TYPE_D(this);
				break;
		}
	}
	else { 
		// 이미 stack 내에서 해당 Pointer가 존재하는 상태이다.
		// 이미 Reachable로 marking된 상태이다.
		if (pObj->m_pLocalFrame == (void*)-1) {
			// Cache에 보관중이다. 
			// 내부 참조값은 아무런 의미가 없으므로 별도 marking할 필요가 없다.
		}
		else {
			// scanInstance()는 Unreachable-Instance를 Reachable로
			// 변경한 이후에 호출된다. 즉, 본 함수가 호출되기 직전에
			// this 는 Unreachable 상태이었다. (따라서, 정상적인
			// stackScan 동안 Reachable 로 marking된 상태라면 
			// 본 함수는 결코 호출되지 않는다.)
			g_GC.markStackTouched(pObj);

			/** 2008.11.14
			 현 Object 를 PUBLISHED로 MARKING할 경우,
			 sub-field 의 Object 또는 Array의 elelement를
			 변경할 경우에도 Publshing 해주어야만 한다
			 현재 구현상 jscript 내에서 LocalVar 변경시
			 Publishing 처리를 하지 않고 있으므로,
			 문제가 발생한다.
		    */
			//KASSERT(t == MARK_STRONG_REACHABLE);
		}
	}
	return;
}

#endif





#if FASTIVA_SUPPORTS_JAVASCRIPT
void JsValueSet::scanInstance$(FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner) {
	Variant_ap pArray = (Variant_ap)this;
	JsVariant* pVar = (JsVariant*)fm::getUnsafeBuffer(pArray);
	for (int i = pArray->length(); i -- > 0; pVar++) {
		fastiva_Instance_p pValue = pVar->asObject();
		if (pValue != FASTIVA_NULL) {
			method(pValue);
		}
	}
	return;
}
#endif




