#ifndef __KERNEL_RUNNABLE_H__
#define __KERNEL_RUNNABLE_H__

#include <fox/kernel/TaskContext.h>
#include <kernel/instance_Q.h>
#include <fastiva/JppException.h>

//#define UNLINK_IN_FINALIZER
//#define ENABLE_TYPE_CHECK_CACHE
//#define UNLINK_GARBAGE_AFTER_SCAN

struct fastiva_StackContext : public fastiva_ManagedSection {
	void* getTop() {
		return m_pStack;
	}
	
	void* getBottom() { 
		return this; 
	}
	
	bool isClosed() {
		return m_pStack != ADDR_ZERO;
	}
};

enum fastiva_ScanState {
	NOT_SCANNED = 0,
	IN_NEED_SCAN = -1,
	IN_SCANNING = 1,
	SCANNED = 2,
};

#ifdef _DEBUG
extern int g_cntObjAlloc;
#endif


#define  CNT_STACK_CYCLE 10

#define CNT_GLOBAL_JNI_ARRAY_CACHE 8
//LocalGC를 임의의 순간에 수행하기 위해서는 (m_marked$ 임의 변경)
// Stack-Scan(pLocalQ 값 수정 직후 까지)과정에서 해당 Thread가 Suspend되어 있어야 한다.
//#define UNLINK_GARBAGE_AFTER_SCAN 1

struct finalize_Task;

struct fastiva_Task : public fox_Task {

	fastiva_Task*	m_pNext0;
    fastiva_Rewinder*	m_pTopRewinder;
    void*			m_pTopHandler;
#if FASTIVA_SUPPORTS_JAVASCRIPT
	com_wise_jscript_JsPrimitive*	m_pTempJsObject;
	JsLocalFrame* m_pCurrJsContext;
#endif
	fastiva_StackFrame*		m_pTopStackFrame;

	const fastiva_Instance_p	m_pLocalQ;
	const fastiva_Instance_p	m_pLocalNFQ;
	fastiva_Instance_p	m_pFinalizingQ;  // 현재 finalize()를 수행 중인 인스탄스의 Q.
	fastiva_Instance_p	m_pFinalizing;  // 현재 finalize()를 수행 중인 인스탄스.


#ifdef UNLINK_GARBAGE_AFTER_SCAN
	fastiva_Instance_p	m_pScanQ;
	fastiva_Instance_p	m_pLastScan;
	fastiva_Instance_p	m_pScanNFQ;
	fastiva_Instance_p	m_pLastScanNF;
#endif

	int						m_cntLocalRookie;
	int						m_totalRookieSize;
	int						m_cntLocalInstance;
	int						m_exceptionContextDepth;
	fastiva_ScanState		m_scanState;
	void*					m_lowSPonMalloc;

	fastiva_StackContext*	m_pStackContext;
    java_lang_Throwable_p	m_pJniException;
	int						m_invokeJNI_retAddr;
	
	finalize_Task*			m_pFinalizeTask; // Finalizer of this task;
	fastiva_Task*			m_pScannedTask;	 // Stack Scaned Task of Finalizer Task (0 = normal-task)
	fox_Mutex				m_pFinalizerLock[1];

	fox_Semaphore*  m_gcStack;
#if FASTIVA_SUPPORT_JNI_STUB
	bool					m_jniJavaThreadAttached;
	void *					m_pDebugTrace;
	void *					m_pJNIEnv0;
	int						m_jniLocalFrameDepth;
	int						m_cntJniCall;
	fastiva_ArrayHeader*	m_globalArrayCache[CNT_GLOBAL_JNI_ARRAY_CACHE];
#endif
	fastiva_Task() : m_pLocalQ(0), m_pLocalNFQ(0) {}

	void init() {
		m_gcStack = fox_semaphore_create();
		fox_mutex_init(m_pFinalizerLock);
		fox_Task::init();
	}
	
	void lock_finalizer() {
		fox_mutex_lock(m_pFinalizerLock);
	}

	void unlock_finalizer() {
		fox_mutex_release(m_pFinalizerLock);
	}

#if FASTIVA_SUPPORT_JNI_STUB
	void setCachedGlobalArray(int jArray, fastiva_ArrayHeader* fmArray) {
		// jobject 는 ref 이므로 직접 비교할 수 없다. 
		// 현재의 hash 값은 ref 그 자체와는 아무 관계가 없다.
		m_globalArrayCache[((uint)jArray / 16) % CNT_GLOBAL_JNI_ARRAY_CACHE] = fmArray;
	}
	void clearGlobalArrayCache() {
		for (int i = 0; i < CNT_GLOBAL_JNI_ARRAY_CACHE; i ++) {
			m_globalArrayCache[i] = 0;
		}
	}
#endif
	// doLocalGC() 에서만 호출된다.
	void setLocalQ_unsafe(fastiva_Instance_p pLocalQ) {
		*(fastiva_Instance_p*)&this->m_pLocalQ = pLocalQ;
	}

	// doLocalGC() 에서만 호출된다.
	void setLocalNFQ_unsafe(fastiva_Instance_p pLocalNFQ) {
		*(fastiva_Instance_p*)&this->m_pLocalNFQ = pLocalNFQ;
	}

	fastiva_Instance_p resetLocalQ() {
		fastiva_Instance_p pObj;
		enterCriticalSection();
		pObj = this->m_pLocalQ;
		m_totalRookieSize = 0;
		setLocalQ_unsafe(ADDR_ZERO);
		leaveCriticalSection();
		return pObj;
	}

	fastiva_Instance_p resetLocalNFQ() {
		fastiva_Instance_p pObj;
		enterCriticalSection();
		pObj = this->m_pLocalNFQ;
		setLocalNFQ_unsafe(ADDR_ZERO);
		leaveCriticalSection();
		return pObj;
	}

	void notifyLocalInstanceUnlinked(fastiva_Instance_p pRetiree) {
#ifdef _DEBUG
		enterCriticalSection();
		this->m_cntLocalInstance --;
		leaveCriticalSection();
#endif
	}

	void insertLocalQ(fastiva_Instance_p pFirstLocal, fastiva_Instance_p pLastLocal, 
						   fastiva_Instance_p pFirstNF, fastiva_Instance_p pLastNF) {
		enterCriticalSection();
		fastiva_Instance_p pObj;
		if (pFirstLocal != ADDR_ZERO) {
			fastiva_Instance_p pOldLocalQ = this->m_pLocalQ;
			*(fastiva_Instance_p*)&this->m_pLocalQ = pFirstLocal;
			KASSERT(pLastLocal->m_pNext$ == ADDR_ZERO);
			pLastLocal->m_pNext$ = pOldLocalQ;
#if _DEBUG 
			for (pObj = pFirstLocal; pObj != pLastLocal; pObj = pObj->m_pNext$) {
				KASSERT(pObj != pOldLocalQ);
			}
#endif
		}
		if (pFirstNF != ADDR_ZERO) {
			fastiva_Instance_p pOldNFQ = this->m_pLocalNFQ;
			*(fastiva_Instance_p*)&this->m_pLocalNFQ = pFirstNF;
			KASSERT(pLastNF->m_pNext$ == ADDR_ZERO);
			pLastNF->m_pNext$ = pOldNFQ;
#if _DEBUG 
			for (pObj = pFirstNF; pObj != pLastNF; pObj = pObj->m_pNext$) {
				KASSERT(pObj != pOldNFQ);
			}
#endif
		}
		leaveCriticalSection();
	}

#if FASTIVA_SUPPORTS_JAVASCRIPT
	com_wise_jscript_JsPrimitive* getTempJsObject();
	JsLocalFrame* getCurrJsContext() { return m_pCurrJsContext; }
	com_wise_jscript_JsGlobal* getJsGlobal();// { return m_pJsGlobal; }
	JsLocalFrame* setCurrJsContext(JsLocalFrame* pContext) { 
		JsLocalFrame* pLastContext = m_pCurrJsContext; 
		m_pCurrJsContext = pContext;
		return pLastContext;
	}
#endif


#ifdef ENABLE_TYPE_CHECK_CACHE
	const fastiva_Class_p			m_pCachedInstanceClass;
	const fastiva_InstanceContext* m_pCachedInstanceType;

	const fastiva_Class_p			m_pCachedInterfaceClass;
	const fastiva_ClassContext* m_pCachedInterfaceType;
	int								m_cachedInterfaceOffset;

	const fastiva_ClassContext*	m_pCachedArrayClass; // 변수명과 달리 Context를 사용한다.
	const fastiva_ClassContext*	m_pCachedArrayType;
#endif

};

struct finalize_Task : public fastiva_Task {
	//fastiva_Task*		m_pScannedTask;	 // Stack Scaned Task of Finalizer Task (0 = normal-task)
	//fastiva_Instance_p	m_pFinalizableQ; // finalize-task의 경우, 새로 추가된 finalize-list 이다
	//fastiva_Instance_p	m_pLostQ; 
};

#define fastiva_getCurrentTask()  ((fastiva_Task*)fox_task_currentTask())



#endif // __KERNEL_RUNNABLE_H__



/**====================== end ========================**/