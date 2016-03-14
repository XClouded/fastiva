#include <precompiled_libcore.h>

#include <kernel/fastiva_GC.h>

#include <fastiva/Heap.h>

#include <fox/kernel/sys_heap.h>

#include <fastiva_malloc.h>
// 웹

#if 0
	int FOX_FASTCALL(sys_heap_tryReadRW32)(const void* ptr);
	int FOX_FASTCALL(sys_heap_tryReadRD32)(const void* ptr);
#endif

#ifdef _DEBUG
	// #define GC_DISABLE_REAL_UNLINK
#endif

//java_lang_Object_p fastiva.allocate(const fastiva_ClassMoniker* pMoniker) {
//	const fastiva_ClassContext* pContext = fm::loadMoniker(pMoniker);
//	return allocate(pContext);
//}

java_lang_Object_p fastiva_Runtime::allocate(const fastiva_ClassContext* pRawContext) {
	const fastiva_InstanceContext* pContext = pRawContext->toInstanceContext();
	return allocateEx(pContext, pContext->m_sizInstance);
}

struct fastiva_ArrayInstance : fastiva_ArrayHeader {
#if FASTIVA_SUPPORTS_DYNAMIC_ARRAY
	void* m_aItem;
#endif
};


extern const int MAX_SMALL_ALLOC_SIZ;

fastiva_ArrayHeader* fastiva_allocArrayInstance(uint item_siz, java_lang_Class* pClass, int length, void* handle, void* data) {
	fastiva_ArrayInstance* pRookie;
	KASSERT(item_siz <= 8);
    int siz = item_siz * length;
	if (item_siz <= 2 && siz < 4096 - item_siz) {
		// byte array 의 경우에 null-termination을 위하여 여백을 둔다.
		siz += item_siz;
        
	}
	else if (pClass->m_pContext$->isInterface() && fm::getArrayDimension(pClass) == 1) {
		siz *= 2;
	}

	if (data != 0) {
		void* ptr = (void*)fastiva_GC_malloc(sizeof(fastiva_ArrayInstance), OBJECT_INSTANCE);
		pRookie = (fastiva_ArrayInstance*)ptr;
		pRookie->m_aItem = data;
		pRookie->m_javaRef$ = handle;
	}
	else if (FASTIVA_SUPPORTS_DYNAMIC_ARRAY) {
		int alloc_siz = sizeof(fastiva_ArrayInstance) + siz;
		if (alloc_siz <= MAX_SMALL_ALLOC_SIZ) {
			void* ptr = (void*)fastiva_GC_malloc(alloc_siz, OBJECT_INSTANCE);
			pRookie = (fastiva_ArrayInstance*)ptr;
			pRookie->m_javaRef$ = (void*)1;
			pRookie->m_aItem = pRookie + 1;
		}
		else {
			void* ptr = (void*)fastiva_GC_malloc(sizeof(fastiva_ArrayInstance), OBJECT_INSTANCE);
			pRookie = (fastiva_ArrayInstance*)ptr;
			pRookie->m_aItem = fastiva_GC_malloc(siz, ARRAY_BUFFER);
		}
	}
	else {
		int alloc_siz = siz + sizeof(fastiva_ArrayHeader);
		pRookie = (fastiva_ArrayInstance*)fastiva_GC_malloc(alloc_siz, OBJECT_INSTANCE);
	}
	//fastiva_Task* pCurrTask = g_GC.tryLocalGC();
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();//g_GC.tryLocalGC();

	// @ v-table 설정
	void* pVTable = pClass->obj.vtable$;
	*(void**)pRookie = pVTable;
	*(int*)&pRookie->m_length = (length);
	if (DEBUG_LOCAL_REF_AS_GC_GENERATION) {
		// 아직 객체 등록 전이다. 만일 등록 직후, scan이 발생하면 해당 g_GC.cntGC 로 변경된다.
		pRookie->m_localRef$ = g_GC.cntGC;
	}
	fox_heap_registerObject(pRookie);

	// local-stack 내에서 scan 되지 않도록 가장 나중에 m_marked를 설정한다.
	*(const void**)&pRookie->m_pClass$ = pClass;

	// 반드시 solt_id 를 부여한 이후(registerHeapInstance() 호출 이후) 에 Local-Q에 추가하여야 한다.
	fox_GC::registerLocalInstance(pCurrTask, pRookie, sizeof(fastiva_ArrayInstance) + siz);

	return pRookie;
}

java_lang_Object_p fastiva_Runtime::allocateEx(const fastiva_InstanceContext* pContext, int siz) {

	//fastiva_Task* pCurrTask = g_GC.tryLocalGC();
	KASSERT(siz >= pContext->m_sizInstance);

	java_lang_Object_p pRookie = (java_lang_Object_p)fastiva_GC_malloc(siz, OBJECT_INSTANCE);

	fastiva.initObject(pContext, pRookie);

	fm::registerHeapInstance(pContext, pRookie);

	return pRookie;
}

fastiva_ArrayHeader* fm::allocArrayInstance(uint item_siz, java_lang_Class* pClass, int length) {
	fastiva_ArrayHeader* pArray = fastiva_allocArrayInstance(item_siz, pClass, length, 0, 0);
	return pArray;
}


void fastiva_Runtime::initObject(const fastiva_InstanceContext* pContext, fastiva_Instance_p pRookie) {
	CHECK_STACK_FAST();

	// 참고) IMPORT CLASS에 실패할 수 있다.
	java_lang_Class* pClass = pContext->m_pClass;
	if (!pClass->isLinked$()) {
		fm::linkClass(pClass);
	}

	void* pVTable = ((fastiva_Class_p)pClass)->obj.vtable$;
	*(void**)pRookie = pVTable;
	*(const void**)&pRookie->m_pClass$ = (const void*)pClass;
}


fastiva_Task* fox_GC::tryLocalGC() {
	FASTIVA_DBREAK();
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
#if 0
	int cntRookie = pCurrTask->m_cntLocalRookie;
	if (cntRookie < 2048) {
		if (&pCurrTask < pCurrTask->m_lowSPonMalloc) {
			pCurrTask->m_lowSPonMalloc = &pCurrTask;
		}
	}
	else 
	if (cntRookie > 2048 * 2
	||  &pCurrTask < pCurrTask->m_lowSPonMalloc) {
		doLocalGC(pCurrTask);
	}
#endif
	return pCurrTask;
}

void fm::registerHeapInstance(const fastiva_InstanceContext* pContext, fastiva_Instance_p pRookie) {
	CHECK_STACK_FAST();

	// Class 는 GC되지 않는다. fox_heap_alloc등을 통해 GC되지 않는 pointer를 생성하여야 한다.
	// 특히, Class의 m_pNext는 GC에서 활용할 수 없다. 그러나, SCRIPT에서 Class를 활용하기 위하여
	// 허용한다. (2008.05)
	// KASSERT((void*)pContext != (void*)java_lang_Class_T$::getRawContext$());

	fastiva_Task* pCurrTask = fastiva_getCurrentTask();

//	java_lang_Object_p pRookie = (java_lang_Object_p)fastiva_GC_malloc(siz, OBJECT_INSTANCE);
/*
	fastiva_Class_p pClass = pContext->m_pClass;
	void* pVTable = pClass->obj.vtable$;
	*(void**)pRookie = pVTable;
	*(const void**)&pRookie->m_pClass$ = pClass;
*/
#if 0 //ndef FASTIVA_NO_CPP_MULTI_INHERIT
	const fastiva_ClassContext* pCtx = pContext;
	while (pCtx != ADDR_ZERO) {
		if (pCtx == java_lang_Thread::getRawContext$()) {
			int a = 3;
		}
		const fastiva_IVTable** ppIVT = pCtx->toInstanceContext()->m_ppIVT;
		if (ppIVT != ADDR_ZERO) {
			const fastiva_IVTable* pIVT;
			for (; (pIVT = *ppIVT) != ADDR_ZERO; ppIVT++) {
				KASSERT(pIVT->m_offset < (int)0xFFFF && (int)siz >= pIVT->m_offset + 4);
				void* refIVT = (char*)pRookie + pIVT->m_offset;
				KASSERT(sys_heap_tryReadRD32((int*)pIVT) != -1);
				KASSERT(sys_heap_tryReadRD32((int*)pIVT) != -1);
				*(const void**)refIVT = pIVT;
			}
		}
		pCtx = pCtx->toInstanceContext()->m_pSuperContext;
	}
#endif


	//fox_HeapSlot * pSlot22 = fox_HeapSlot::getHeapSlotEx(m_mark);
	//KASSERT(pSlot22->m_pObject == pRookie);
	int finalize_slot_offset = FASTIVA_VIRTUAL_SLOT_OFFSET(java_lang_Object, void, finalize, 0, ());

	// 반드시 solt_id 를 부여한 이후(registerHeapInstance() 호출 이후) 에 Local-Q에 추가하여야 한다.
	// local-stack 내에서 scan 되지 않도록 가장 나중에 m_marked를 설정한다.

	if (DEBUG_LOCAL_REF_AS_GC_GENERATION) {
		// 아직 객체 등록 전이다. 만일 등록 직후, scan이 발생하면 해당 g_GC.cntGC 로 변경된다.
		pRookie->m_localRef$ = g_GC.cntGC;
	}
	fox_heap_registerObject(pRookie);

	// Array그 자체는 WeakRef가 아니다.
	// Reference는 해당 referent가 finalizing 되기 전에 ReferenceQueue에 추가된다.
	// 단 ReferenceQueue에 추가되는 과정에서 지연이 생겨, finalizing 이 먼저
	// 이루어질 수도 있다.
	// Reference 그 자체가 GC 되면 ReferenceQueue에 추가되지 않는다.(당연한 얘기)
	fastiva_Class_p pClass = pContext->m_pClass;
	void* pVTable = pClass->obj.vtable$;
	int ref_type = pContext->m_accessFlags & (ACC_PHANTOM_REF$ | ACC_SOFT_REF$ | ACC_WEAK_REF$);
	if (ref_type != 0) {
		pRookie->m_pNext$ = ADDR_ZERO;
		if (((int*)pVTable)[finalize_slot_offset] != g_GC.pfnPsuedoFinalize) {
			int a = 3;
		}
		// Reference는 LocalQ에 추가하지 않는다.
		// 생성자 내부에서 호출되는 setRefenerent() 함수가 호출될 때,
		// 적절한 Q에 추가된다.
	}
	else 
	// 반드시 reference-Q 처리를 먼저 한 후에, fianlize() 함수 여부를 검사한다.
#ifdef FASTIVA_ENABLE_FINALIZE
	if (((int*)pVTable)[finalize_slot_offset/sizeof(int)] != g_GC.pfnPsuedoFinalize) {
		// registerHeapInstance 이 후에 finalize-flag를 처리한다.
		// demarkFinalized()를 thread-safe 하게 처리하기 위하여
		// LocalQ 등록 전에 처리한다
		g_GC.markFinalizable(pRookie);

		pCurrTask->enterCriticalSection();
		if (pCurrTask->m_scanState != IN_SCANNING && *(char*)&pRookie->m_mark$ != 0) {
			// local-object marking은 stack-scan 시에만 이루어지고,
			// stack-scan 시에는 CriticalSection 내부에 있다.
			// 즉, LocalQ에 추가되기 전이라, demarking되지 않은 상태이다.
			*(char*)&pRookie->m_mark$ = 0;
		}
		pRookie->m_pNext$ = pCurrTask->m_pLocalNFQ;
		pCurrTask->setLocalNFQ_unsafe(pRookie);
		pCurrTask->leaveCriticalSection();
		//fox_printf("Non finalized registerered %s/%s\n", pContext->getPackageName(), pContext->m_pBaseName);
	}
	else 
#endif
	{
		// 반드시 solt_id 를 부여한 이후에 Local-Q에 추가하여야 한다.
		fox_GC::registerLocalInstance(pCurrTask, pRookie, pContext->m_sizInstance);
	}

	KASSERT(pCurrTask->m_pStackContext != ADDR_ZERO);

}

void fox_GC::registerLocalInstance(fastiva_Task* pTask, fastiva_Instance_p pRookie, int siz) {
	pTask->enterCriticalSection_GCI();
	if (pTask->m_scanState != IN_SCANNING && *(char*)&pRookie->m_mark$ != 0) {
		// local-object marking은 stack-scan 시에만 이루어지고,
		// stack-scan 시에는 CriticalSection 내부에 있다.
		// 즉, LocalQ에 추가되기 전이라, demarking되지 않은 상태이다.
		*(char*)&pRookie->m_mark$ = 0;
	}
	pTask->m_cntLocalRookie ++;
	pTask->m_cntLocalInstance ++;
	pTask->m_totalRookieSize += siz;
#if defined(_DEBUG) && defined(_WIN32)
	pRookie->m_idx$ = g_GC.cntAllocated;
	if (pRookie->m_idx$ == 0xa46d10) {
		int a = 3;
	}
#endif
	pRookie->m_pNext$ = pTask->m_pLocalQ;
	pTask->setLocalQ_unsafe(pRookie);
	pTask->leaveCriticalSection();
}


void fox_heap_notifyAvailable() {
	fox_monitor_lock(g_GC.pTrigger);
	fox_monitor_notify(g_GC.pTrigger);
	fox_monitor_release(g_GC.pTrigger);
}

void fastiva_GC_triggerGC(int maxLocalAlloc);

static int allocSizeOfCurrentGeneration = 64 * 1024 * 1024 / 8 * 4;

void* sys_heap_malloc_array_buffer(int siz);

FASTIVA_HMEM fastiva_GC_malloc(uint siz, FASTIVA_MEM_TYPE type) {
	if ((allocSizeOfCurrentGeneration -= siz) < 0) {
		allocSizeOfCurrentGeneration = 16 * 1024 * 1024;//fox_heap_getTotalSize() / 8;
		fastiva_GC_triggerGC(allocSizeOfCurrentGeneration/2);
	}


	siz = (siz + 3) & ~0x3;

	int cntRetry = 0;
	void* pHeap;
	while (true) {
		if (type != ARRAY_BUFFER) {
			pHeap = fox_heap_malloc(siz);
		}
		else {
			pHeap = sys_heap_malloc_array_buffer(siz);
		}

		if (pHeap != ADDR_ZERO) {
			goto allocated;
		}
		else if (++cntRetry > 2) {
			fastiva_throwOutOfMemoryError();
		}

		fastiva_GC_doGC(cntRetry > 0);
	}

allocated:
	GC_DEBUG_BREAK(pHeap);
	if (type == ARRAY_BUFFER) {
		// array-buffer는 allocation 시 memory 를 초기화하여 return한다.
	}
	else {
		memset(pHeap, 0, siz);
	}

	fox_util_inc32(&g_GC.cntAllocated);
	return (FASTIVA_HMEM)pHeap;
}

extern void fastiva_jni_cloneJavaRef(fastiva_Instance_p pObj);

java_lang_Object_p fm::clone(
	fastiva_Instance_p pObj
) {
	
	java_lang_Class_p pClass = pObj->getClass$();
	const fastiva_InstanceContext* pContext = fm::getInstanceContext(pClass);
	
	int siz = pContext->m_sizInstance;
	java_lang_Object_p pClone = fastiva.allocateEx(pContext->toInstanceContext(), siz);

	int* pSrc = (int*)((char*)pObj + sizeof(fastiva_Instance_T$));
	int* pDst = (int*)((char*)pClone + sizeof(fastiva_Instance_T$));
	for (int i = (siz - sizeof(fastiva_Instance_T$)) / sizeof(int); i -- > 0; ) {
		*pDst++ = *pSrc++;
	}

#if FASTIVA_SUPPORT_JNI_STUB
	{
		pClone->m_javaRef$ = ADDR_ZERO;
		fastiva_jni_cloneJavaRef(pClone);
	}
#endif

	// 새로이 clone 된 instance는 LOCAL_G에 속한다.
	// 따라서 그 field-ref-count를 변경할 필요가 없다.
	return (java_lang_Object_p)(pClone);
}

fastiva_ArrayHeader* fm::cloneArray(
	fastiva_ArrayHeader* pArray
) {
	int item_size = sizeof(void*);
	java_lang_Class_p pClass = pArray->getClass();
	KASSERT(fm::getArrayDimension(pClass) >= 1);
	if (fm::getArrayDimension(pClass) == 1) {
		const fastiva_ClassContext* pContext = pClass->m_pContext$;
		if (pContext->isPrimitive()) {
			item_size = fm::getPrimitiveArrayItemSize(pContext);
		}
	}

	fastiva_ArrayHeader* pClone = fm::allocArrayInstance(item_size, pArray->getClass(), pArray->m_length);

#if FASTIVA_SUPPORT_JNI_STUB
	// 불필요한 코드.
	// pClone->m_javaRef$ = ADDR_ZERO;
#endif

	void* pSrc = pArray->getBuffer_unsafe$();
	void* pDst = pClone->getBuffer_unsafe$();
	int buf_siz = pArray->m_length * item_size;
	memcpy(pDst, pSrc, buf_siz);

	return pClone;
}



void FOX_FASTCALL(sys_heap_setHeapType)(void* pRookie, int type);


fastiva_Instance_p fox_GC::getHeapInstance(fastiva_Instance_p pUnknown) {

	if (!fox_heap_checkObject(pUnknown)) {
		GC_DEBUG_BREAK(pUnknown);
		return ADDR_ZERO;
	}
	return pUnknown;
}

void fastiva_Instance::operator delete(void* pObj) {
	sys_huge_heap_lock();
	fox_GC::unlinkHeapInstance((fastiva_Instance_p) pObj);
	sys_huge_heap_unlock();
}

void sys_heap_free_array_buffer(void* ptr, int alloac_size);
void fastiva_jni_releaseJavaArray(fastiva_ArrayHeader* pArray);
void fastiva_jni_releaseJavaRef(fastiva_Instance_p pObj);


void fox_GC::unlinkHeapInstance(fastiva_Instance_p pObj) {
	// slotTable realign 중에 unlink가 호출될 수 있다. 이를
	// 동기화 한다.

	GC_UNLINK_BREAK(pObj);
	KASSERT(fox_heap_checkObject(pObj));

#ifndef UNLINK_IN_FINALIZER
	//KASSERT(fastiva_getCurrentTask() == Kernel::g_pSystemTask || pObj->m_localRef$ == (ushort)-1);
#endif
	if (pObj->m_globalRef$ == 0x8000) {
		// marking되지는 않았으나, global-list에 추가되어 있는 상태이다.
		// unlink를 다음 기회로 미룬다.
		
		return;
	}

	KASSERT(pObj->m_globalRef$ == 0);
	KASSERT(!g_GC.isReachable(pObj));

	fox_util_inc32(&g_GC.cntUnlink);

#ifdef KERNEL_DEBUG
	#define MARK_UNLINKED(pObj)									\
		KASSERT(*(int*)pObj != 0xDC00DC00);						\
		*(int*)pObj = 0xDC00DC00;
#else
	#define MARK_UNLINKED(pObj)									\
		pObj->m_mark$ = 0xDCDC;
#endif

	bool GC_DISABLE_REAL_UNLINK = false;
	if (GC_DISABLE_REAL_UNLINK) {
		MARK_UNLINKED(pObj);
		pObj->m_globalRef$ = -1;
		return;
	}

	if (FASTIVA_SUPPORTS_DYNAMIC_ARRAY) {
		java_lang_Class* pClass = pObj->m_pClass$;
		int dimension = fm::getArrayDimension(pClass);
		if (dimension == 0) {
			fastiva_jni_releaseJavaRef(pObj);
		}
		else if (((fastiva_ArrayInstance*)pObj)->m_javaRef$ == 0) {
			int item_size = 4;
			fastiva_ArrayInstance* pArray = (fastiva_ArrayInstance*)pObj;
			int cntItem = pArray->m_length;
			if (dimension == 1) {
				if (pClass->m_pContext$->isPrimitive()) {
					item_size = fm::getPrimitiveArrayItemSize(pClass->m_pContext$);
					if (item_size <= 2 && (item_size * cntItem) < 4096 - item_size) {
						// byte 와 jchar array 뒤에는 '\0'이 추가되어 있다.
						cntItem ++;
					}
				}
				else if (pClass->m_pContext$->isInterface()) {
					item_size = 8;
				}
			}
			sys_heap_free_array_buffer(pArray->m_aItem, item_size * cntItem);
		}
		else if (((fastiva_ArrayInstance*)pObj)->m_javaRef$ == (void*)1) {
			// constant array 다.
		}
		else {
			fastiva_jni_releaseJavaArray((fastiva_ArrayInstance*)pObj);
		}
	}
	MARK_UNLINKED(pObj);
	fox_heap_free_ex(pObj, FOX_MEM_OBJECT);
}


void fastiva_jni_exchangeBuffer(fastiva_ArrayHeader* pArrayHeader, void* javaArray, void* data) {
	fastiva_ArrayInstance* pArray = (fastiva_ArrayInstance*)pArrayHeader;
	KASSERT(fm::getArrayDimension(pArray->getClass()));
	KASSERT(pArray->getClass()->m_pContext$->isPrimitive());

	void* old_buf = pArray->m_aItem;
	pArray->m_aItem = data;
	if (pArray->m_javaRef$ == (void*)0) {
		int item_size = fm::getPrimitiveArrayItemSize(pArray->getClass()->m_pContext$);
		int cntItem = pArray->m_length;
		if (item_size <= 2 && (item_size * cntItem) < 4096 - item_size) {
			// byte 와 jchar array 뒤에는 '\0'이 추가되어 있다.
			cntItem ++;
		}
		sys_huge_heap_lock();
		sys_heap_free_array_buffer(old_buf, item_size * cntItem);
		sys_huge_heap_unlock();
	}
	pArray->m_javaRef$ = javaArray;
}



class SystemHashTable : public java_util_Hashtable {
public:
	void init$() {
		this->m_pClass$ = java_util_Hashtable::getRawContext$()->m_pClass;
		java_util_Hashtable::init$();
	}
};
static SystemHashTable g_sysHashTable;

java_lang_String_p fm::getInternedString(java_lang_String_p pRookie) {
	java_lang_String_p res = (java_lang_String_p)kernelData.g_pInternedString->get(pRookie);

	return res;
}

java_lang_String_p fm::internString(java_lang_String_p pRookie) {
	if (kernelData.g_pInternedString == ADDR_ZERO) {
		//memset((int*)&g_sysHashTable + 1, 0, sizeof(g_sysHashTable) - sizeof(int));  
		//g_sysHashTable.init$();
		kernelData.g_pInternedString = FASTIVA_NEW(java_util_Hashtable)();
			//&g_sysHashTable;
		//fastiva_lockGlobalRef(kernelData.g_pInternedString);
		//}
	}
	java_lang_String_p ret = (java_lang_String_p)kernelData.g_pInternedString->get(pRookie);

	if (ret != ADDR_ZERO)
		return ret;
	kernelData.g_pInternedString->put(pRookie, pRookie);
	return pRookie;
}

java_lang_String_p fm::createStringConstant(fastiva_ConstantString* pCS) {
/* CLDC는 intern을 사용하지 않는다(?)
#ifdef FASTIVA_CLDC
	#ifdef KERNEL_DEBUG
		if (isLinked(java_lang_String_T$::getRawContext$()->m_pClass)) {
			fastiva_Task* pCurrTask = fox_task_currentTask();
			fox_scheduler_boostPriority(pCurrTask);	// disable local-monitor-lock & GC.
		}
	#else
		fastiva_Task* pCurrTask = fox_task_currentTask();
		fox_scheduler_boostPriority(pCurrTask);	// disable local-monitor-lock & GC.
	#endif
	fastiva_Task* pCurrTask = fox_task_currentTask();
	fastiva_Instance_p pLastRookieQ = pCurrTask->m_pLocalQ;
#endif
*/
	
	java_lang_String_p pStr;
	if (pCS->m_len < 0) {
		pStr = fastiva.createAsciiStringA(pCS->m_aByte, -pCS->m_len);
	}
	else {
		pStr = fastiva.createStringW(pCS->m_aUnicod, pCS->m_len);
	}

/* CLDC는 intern을 사용하지 않는다(?)
#ifdef FASTIVA_CLDC
	pCurrTask->m_pLocalQ = pLastRookieQ;
	fox_GC::unregisterHeapInstance(pStr);
	fox_GC::unregisterHeapInstance(pStr->m_value);
	#ifdef KERNEL_DEBUG
		if (isLinked(java_lang_String_T$::getRawContext$()->m_pClass)) {
			fox_scheduler_normalize(pCurrTask);
		}
	#else
		fox_scheduler_normalize(pCurrTask);
	#endif
#else
*/
	pStr = fm::internString(pStr);
//#endif
	return pStr;
}


void* __cdecl operator new(uint siz) {
	void* ptr = fastiva_malloc(siz);
	if (ptr == NULL) {
		fastiva_throwOutOfMemoryError();
	}
	return ptr;
}


void __cdecl operator delete(void* ptr) {
	fastiva_free(ptr);
	//DEBUG_MSG_OUT("delete called");
}

void *operator new[] (uint siz) {
	void* ptr = fastiva_malloc(siz);
	if (ptr == NULL) {
		fastiva_throwOutOfMemoryError();
	}
	return ptr;
}

void operator delete[] (void* ptr) {
	fastiva_free(ptr);
}

void* fastiva_malloc(unsigned int size) {
	void* ptr = fox_heap_malloc(size);
	if (ptr == NULL) {
		fox_debug_printf("!!!GC called in malloc");
		fastiva_GC_doGC(true);
		ptr = fox_heap_malloc(size);
	}
	return ptr;
}

void fastiva_free(void* mem) {
	if (mem != NULL) {
		fox_heap_free(mem);
	}
}

void* fastiva_calloc(size_t num, size_t size) {
	int s = num * size;
	void * ptr = fastiva_malloc(s);
	if (ptr != NULL) {
		memset(ptr, 0, s);
	}
	return ptr;
}

char* fastiva_strdup(const char *strSource) {
	int len = strlen(strSource);
	char * ptr = (char*)fastiva_malloc(len+1);
	if (ptr != NULL) {
		memcpy(ptr, strSource, len);
		ptr[len] = 0;
	}
	return ptr;
}

