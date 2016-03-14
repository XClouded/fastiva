#include <Dalvik.h>
#include <dalvik_Kernel.h>
#include <kernel/Module.h>
#include <string.h>
#include <sys/mman.h>
#include <java/util/ArrayList.inl>
#include <cutils/atomic.h>
#include <pthread.h>
#include "mterp/common/FindInterface.h"
#include "alloc/HeapInternal.h"

#ifdef _DEBUG
#include <java/lang/Integer.inl>
#include <java/lang/Long.inl>
#include <java/lang/Enum.inl>
#include <java/lang/String.inl>
#include <java/lang/IntegralToString.inl>
#include <java/io/PrintStream.inl>
#include <java/util/HugeEnumSet.inl>
#include <java/util/Collections.inl>
#include <java/util/Map.inl>
#include <java/lang/ClassLoader.inl>
#endif

#define kalloc(item_type, cnt)	(item_type*)malloc(sizeof(item_type) * cnt)

Kernel kernelData;

extern void fastiva_dvmFullSuspendCheck(Thread* self);


#if 0 // def __GNUC__
#include <sys/atomics.h>
//#include <private/bionic_atomic_inline.h>
//#include <private/bionic_futex.h>
//#include <private/bionic_pthread.h>
//#include <private/bionic_tls.h>
#include <bionic/pthread_internal.h>
//#include <thread_private.h>


#ifdef USE_PRIVAYE_FUTEX

inline u2 __getKernelThreadId()  {
	//void**  tls = (void**)__get_tls();

    pthread_t thread = pthread_self();
	return ((pthread_internal_t*)(int)thread)->kernel_id;

}
//#include <private/bionic_futex.h>
//#define android_atomic_add _sync_fetch_and_add
//#define android_atomic_and _sync_fetch_and_and
#define USE_FUTEX 1
#else 
#define __getKernelThreadId()  dvmGetSysThreadId()
#endif

#include <private/bionic_futex.h>
inline int __futex_syscall3(volatile void *ftx, int op, int val)
{
    return futex(ftx, op, val, NULL, NULL, 0);
}

inline int __futex_syscall4(volative void *ftx, int op, int val, const struct timespec *timeout)
{
    return futex(ftx, op, val, (void *)timeout, NULL, 0);
}

inline int __futex_wait(volatile void *ftx, int val, const struct timespec *timeout)
{
    return futex(ftx, FUTEX_WAIT, val, (void *)timeout, NULL, 0);
}

inline int __futex_wake(volatile void *ftx, int count)
{
    return futex(ftx, FUTEX_WAKE, count, NULL, NULL, 0);
}

inline int  __futex_wake_ex(volatile void *ftx, int pshared, int val)
{
    return __futex_syscall3(ftx, pshared ? FUTEX_WAKE : FUTEX_WAKE_PRIVATE, val);
}

inline int  __futex_wait_ex(volatile void *ftx, int pshared, int val, const struct timespec *timeout)
{
    return __futex_syscall4(ftx, pshared ? FUTEX_WAIT : FUTEX_WAIT_PRIVATE, val, timeout);
}

void dvmInitMutex(fastiva_recursive_mutex* mutex) {
	mutex->lockOwner = 0;
	mutex->cntLock = 0;
#ifdef USE_FUTEX
	mutex->signal1 = 0;
	mutex->signal2 = 0;
#else
    int cc __attribute__ ((__unused__));
#ifdef CHECK_MUTEX
    pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
    cc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
    assert(cc == 0);
	pthread_mutex_init((pthread_mutex_t*)(void*)&mutex->signal2, &attr);
    pthread_mutexattr_destroy(&attr);
#else
	pthread_mutex_init((pthread_mutex_t*)(void*)&mutex->signal2, NULL);
#endif
	cc = pthread_mutex_lock((pthread_mutex_t*)(void*)&mutex->signal2);
    assert(cc == 0);
#endif
}

inline void waitBroadcastLock(volatile int* signal, int value) {
#if USE_FUTEX
	int cntInnerLoop  __attribute__ ((__unused__)) = 0;
	while (*signal == value) {
		assert(++cntInnerLoop < 1000);
		__futex_wait(signal, value, NULL);
	};
#else
	int res = pthread_mutex_lock((pthread_mutex_t*)(void*)&signal);
	assert(res == 0);
#endif
}

inline void waitSingle(volatile int* signal) {
#if USE_FUTEX
	int cntLoop  __attribute__ ((__unused__)) = 0;
	int cntInnerLoop  __attribute__ ((__unused__)) = 0;
	while (true) {
		while (*signal == 0) {
			assert(++cntInnerLoop < 1000);
			__futex_wait(signal, 0, NULL);
		};
		assert(++cntLoop < 1000);
		
        int err  __attribute__ ((__unused__)) ;
		err = android_atomic_release_cas(1, 0, signal);
		if (err == 0) {
			break;
		}
        //assert(err == 0);
	}
#else
	int res __attribute__ ((__unused__));
    res = pthread_mutex_lock((pthread_mutex_t*)(void*)&signal);
	assert(res == 0);
#endif
}


inline u2 getOwner(fastiva_recursive_mutex* mutex) {
	return (u2)mutex->lockOwner;
}

inline void setOwner(fastiva_recursive_mutex* mutex, int owner) {
    int v __attribute__ ((__unused__));
	assert(mutex->lockOwner > 0xFFFF);
	v = android_atomic_add(owner - 0x10000, &mutex->lockOwner);
	assert((v & 0xFFFF) == 0);
	assert(v > 0xFFFF);
}

inline int addBlocked(fastiva_recursive_mutex* mutex) {
	return android_atomic_add(0x10000, &mutex->lockOwner);
}

inline void release(fastiva_recursive_mutex* mutex) {
    int v __attribute__ ((__unused__));
#ifdef _DEBUG
    mutex->cntLock = getOwner(mutex);
#endif

	v = android_atomic_and(~0xFFFF, &mutex->lockOwner);
	if (v > 0xFFFF) {
#ifdef _DEBUG
 		mutex->cntLock |= 0x1000;
#endif
#if USE_FUTEX
        mutex->signal1 = 1;
		android_memory_barrier();
		__futex_wake(&mutex->signal1, 1);
#else
		int res __attribute__ ((__unused__));
		res = pthread_mutex_unlock((pthread_mutex_t*)(void*)&mutex->signal2);
		assert(res == 0);
#endif
	}
	//sched_yield();
}


int dvmTryLockMutex(fastiva_recursive_mutex* mutex) {
	u2 self = (u2)__getKernelThreadId();
	assert(self != 0);
	if (getOwner(mutex) == self) {
		mutex->cntLock ++;
		return 0;
	}
	if (android_atomic_release_cas(0, self, &mutex->lockOwner) == 0) {
		mutex->cntLock = 1;
		assert(getOwner(mutex) == self);
		return 0;
    }
	return EPERM;
}


void dvmLockMutex(fastiva_recursive_mutex* mutex) {
	u2 self = (u2)__getKernelThreadId();
	assert(self != 0);
	if (getOwner(mutex) == self) {
		mutex->cntLock ++;
		return;
	}

	if (addBlocked(mutex) != 0){
		waitSingle(&mutex->signal1);
	}
	setOwner(mutex, self);
	assert(getOwner(mutex) == self);
	mutex->cntLock = 1;
}

void dvmUnlockMutex(fastiva_recursive_mutex* mutex) {
	assert(getOwner(mutex) == (u2)__getKernelThreadId() && mutex->cntLock > 0);
	if (--mutex->cntLock == 0) {
		release(mutex);
	}
}

void dvmWaitCond(pthread_cond_t* pCond, fastiva_recursive_mutex* mutex) {
	assert(getOwner(mutex) == (u2)__getKernelThreadId() && mutex->cntLock > 0);
	int oldCount = mutex->cntLock;
	/* ����, ���� blocked �� thread �� ������ sysMutex�� loked �� ���·� ��ȯ�ǰ�,
	   blocked thread �� ������ pthread_mutex_unlock �� ȣ��ｿ��?��ȯ�ȴ�.
	   -- �̹� mutex-owner�� �ٲ� �����̰�, sysMutex�� lock �� ���´� �����ȴ�.
	*/
#ifdef _DEBUG
    mutex->cntLock = getOwner(mutex);
#endif 
	int sig = mutex->signal2;
	release(mutex);
#ifdef USE_FUTEX
	waitBroadcastLock(&mutex->signal2, sig);
#else
    int cc __attribute__ ((__unused__));
	cc = pthread_cond_wait(pCond, (pthread_mutex_t*)(void*)&mutex->signal2);
    assert(cc == 0);
#endif
	dvmLockMutex(mutex);
	mutex->cntLock = oldCount;
}

void dvmNotify(pthread_cond_t* pCond, fastiva_recursive_mutex* mutex) {
	assert(getOwner(mutex) == (u2)__getKernelThreadId() && mutex->cntLock > 0);
#ifdef USE_FUTEX
	mutex->signal2 ++;
	android_memory_barrier();
	__futex_wake(&mutex->signal2, 0xFFFF);
    // ALOGE("Notity cntWait: %d", mutex->signal2);
#else
	dvmBroadcastCond(pCond);
#endif
}

bool fastiva_mutex_isLockedBySelf(fastiva_recursive_mutex* mutex) {
	return getOwner(mutex) == (u2)__getKernelThreadId();
}
#endif



#if !defined __arm__ || !defined __thumb__
Thread* fastiva_lockCurrentStack(jmp_buf* fastiva_buf$) {
	Thread* pCurrTask = dvmThreadSelf();
	fastiva_lockStack(pCurrTask, fastiva_buf$);
	return pCurrTask;
}

void* fastiva_lockStack(Thread* self, jmp_buf* fastiva_buf$) {
	FASTIVA_ASSERT(self != NULL);
	void* old_sp = self->m_pNativeStackPointer;
	self->m_pNativeStackPointer = fastiva_buf$;				
	return old_sp;
}

void fastiva_releaseStack(void* self, void* old_buf) {
	((Thread*)self)->m_pNativeStackPointer = old_buf;
}
#endif

java_lang_String_p 
fm::createStringConstant(const unicod* str) {
	int len = str[0];
	str ++;
	java_lang_String_p pStr = fm::createStringW(str, len);
#ifdef ANDROID	
	pStr = (java_lang_String_p)dvmLookupImmortalInternedString(pStr);
#endif
	return pStr;
}


java_lang_String_p 
fm::internString(java_lang_String_p pRookie) {
	/**
		Do not usr pRookie->intern_();
		It does not immotal.
	*/
	return (java_lang_String_p)dvmLookupInternedString(pRookie);
}

java_lang_Class_p 
fm::getPrimitiveClassBySig(unicod signature) {
	switch (signature) {
		case 'Z':
			return FASTIVA_PRIMITIVE_CLASS_OF(jbool);
		case 'B':
			return FASTIVA_PRIMITIVE_CLASS_OF(jbyte);
		case 'C':
			return FASTIVA_PRIMITIVE_CLASS_OF(unicod);
		case 'S':
			return FASTIVA_PRIMITIVE_CLASS_OF(jshort);
		case 'I':
			return FASTIVA_PRIMITIVE_CLASS_OF(jint);
		case 'J':
			return FASTIVA_PRIMITIVE_CLASS_OF(jlonglong);
		case 'F':
			return FASTIVA_PRIMITIVE_CLASS_OF(jfloat);
		case 'D':
			return FASTIVA_PRIMITIVE_CLASS_OF(jdouble);
		case 'V':
			return FASTIVA_PRIMITIVE_CLASS_OF(jvoid);
	}
	return ADDR_ZERO;
}

ClassObject* g_arrayInterfaces[2];
InterfaceEntry g_arrayIftable[2];
extern bool (*fastiva_isMultiArgList_fn)(u4 argsId);

bool fastiva_isMultiArgList(u4 argsId) {
	u4* argTypeList = (u4*)argsId;
	bool res = (argTypeList[0] > JPP_ARG_CNT_FLAG);
	if (res) {
		int a = 3;
	}
	return res;
}

volatile void* dvmFindInterfaceMethodInCache_fn;
extern FASTIVA_RUNTIME_EXPORT void fastiva_libcore_initModuleInfo();
void fastiva_initKernel() {
	extern void fastiva_initInterpreterOperationTable();

	assert(sizeof(Object) % 8 == 0);
	assert(sizeof(fastiva_Class) % 8 == 0);

	// �Լ� ������ ������ ����..
	dvmFindInterfaceMethodInCache_fn = (void*)dvmFindInterfaceMethodInCache;
	fastiva_libcore_initModuleInfo();

	ClassObject* d2f_getRawStaticOfClass();
	kernelData.g_classHigh = kernelData.g_classLow = d2f_getRawStaticOfClass();
	assert(kernelData.g_classLow != NULL);
	g_pLibcoreModule->prepareRawClasses();
	fastiva_isMultiArgList_fn = fastiva_isMultiArgList;

	g_arrayIftable[0].clazz = g_arrayInterfaces[0] = FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Cloneable);//->getClass();
	g_arrayIftable[1].clazz = g_arrayInterfaces[1] = FASTIVA_RAW_CLASS_CONTEXT_PTR(java_io_Serializable);//->getClass();

	kernelData.g_ivtableAllocTop = (int*)mmap(NULL, 1024*1024*4, PROT_READ | PROT_WRITE,
            /*MAP_SHARED |*/ MAP_PRIVATE | MAP_ANON, -1, 0);
	kernelData.g_ivtableAllocBase = kernelData.g_ivtableAllocTop;
#ifdef _WIN32
	fastiva_initInterpreterOperationTable();
#endif
	return;
}



void fastiva_initRuntime() {

#ifdef NO_FASTIVA
	return;
#endif

#if 0
	extern void check_fni();
	check_fni();
#endif

    assert((int)CLASS_ISFINALIZABLE$$        == (int)CLASS_ISFINALIZABLE);
    assert((int)CLASS_ISCLASS$$              == (int)CLASS_ISCLASS);

    assert((int)CLASS_ISREFERENCE$$          == (int)CLASS_ISREFERENCE);
                                                               
    assert((int)CLASS_ISWEAKREFERENCE$$      == (int)CLASS_ISWEAKREFERENCE);
    assert((int)CLASS_ISFINALIZERREFERENCE$$ == (int)CLASS_ISFINALIZERREFERENCE);
    assert((int)CLASS_ISPHANTOMREFERENCE$$   == (int)CLASS_ISPHANTOMREFERENCE);

    assert((int)CLASS_ISOPTIMIZED$$          == (int)CLASS_ISOPTIMIZED);  
    assert((int)CLASS_ISPREVERIFIED$$        == (int)CLASS_ISPREVERIFIED);


	#undef FASTIVA_DECL_VM_RUNTIME_FIELD
	#define FASTIVA_DECL_VM_RUNTIME_FIELD(ret_type, name, suffix)	// igore

	#undef FASTIVA_DECL_VM_RUNTIME_API
	#define FASTIVA_DECL_VM_RUNTIME_API(ret_type, name, cnt, attr, params)	\
		fastiva_RuntimeProxy::g_instance.m_##name = fastiva_Runtime::name;

	#undef FASTIVA_DECL_VM_RUNTIME_API_VOID
	#define FASTIVA_DECL_VM_RUNTIME_API_VOID(name, cnt, attr, params)	\
		FASTIVA_DECL_VM_RUNTIME_API(void, name, cnt, attr, params);

	#include <fastiva/RuntimeAPI.h>

#ifdef _WIN32
#else
	int __android_log_print(int prio, const char *tag, const char *fmt, ...);
	fastiva_RuntimeProxy::g_instance.log = __android_log_print;
#endif
	extern void fastiva_initPrimitiveClasses();

#ifdef FASTIVA 
#ifdef _WIN32
	_putenv("JAVA_HOME=/system");
	_putenv("LD_LIBRARY_PATH=/system");
	_putenv("ANDROID_ROOT=/system");
	_putenv("FANDROID_ROOT=/vendor");
#endif
#endif


	Kernel::primitiveClasses = kalloc(fastiva_PrimitiveClass*, fastiva_Primitives::cntType*(FASTIVA_MAX_ARRAY_DIMENSION+1));
	//ALOGD("classID pool %p: cnt:%d", Kernel::primitiveClasses, JPP_EXTERNAL_CLASS_ID_START);
	//kernelData.g_typeSigs =  kalloc(const char*, JPP_EXTERNAL_CLASS_ID_START);
	dvmInitMutex(&kernelData.initClassLock);
	kernelData.vmInitialized = false;
	pthread_cond_init(&kernelData.allThreadScanStartedCond, NULL);
	pthread_cond_init(&kernelData.allThreadScanFinishedCond, NULL);
	pthread_cond_init(&kernelData.gcFinshinedCond, NULL);

	fastiva_initPrimitiveClasses();
	g_pLibcoreModule->initAllClasses();


    if (dvmReferenceTableEntries(&dvmThreadSelf()->internalLocalRefTable) != 0)
    {
        ALOGW("Warning: tracked references remain post-initialization");
        dvmDumpReferenceTable(&dvmThreadSelf()->internalLocalRefTable, "MAIN");
    }
	return;
}



void fastiva_initRuntime_ex() {
#ifdef NO_FASTIVA
	return;
#endif


	void fastiva_initPrimitiveArrayClasses();
	fastiva_initPrimitiveArrayClasses();
#ifdef FASTIVA_GC_DEBUG
	//java_lang_String_C$::getRawStatic$()->objectSize = java_lang_String_C$::getRawContext$()->toInstanceContext()->m_sizInstance;
#endif
	g_pLibcoreModule->initStringPool();
	//g_pLibcoreModule->linkPreloadableClasses(true);
	g_pLibcoreModule->internStringPool();
	kernelData.g_pAnnotationsList = FASTIVA_NEW(java_util_ArrayList)();

    if (dvmReferenceTableEntries(&dvmThreadSelf()->internalLocalRefTable) != 0)
    {
        ALOGW("Warning: tracked references remain post-initialization");
        dvmDumpReferenceTable(&dvmThreadSelf()->internalLocalRefTable, "MAIN");
    }

#if 0 //def _DEBUG
	volatile int loop = true;
	for (; loop; ) {
		ALOGD("DEBUG BREAK == NULL");
		usleep(1000*1000);
	}
#endif

#ifndef _WIN32 
	bool fastiva_sysInit();
	fastiva_sysInit();
#endif

#if 0 //def _WIN32
				int v = 2;
				switch (v) {
					case 0: while (true) {
						ALOGW("0");
						break;
					case 1:
						ALOGW("1");
						break;
					case 2:
						ALOGW("2");
						break;
					}
					default:
						ALOGW("finally");
						break;
				}
#endif
}

extern "C" Thread* fastiva_dvmThreadSelf()
{
    return (Thread*) pthread_getspecific(gDvm.pthreadKeySelf);
}

extern void fastiva_dvmMarkThread(Thread* self, bool withLock);
extern void fastiva_dvmFullSuspendCheck(Thread* self);

void fastiva_doSuspend(void* sp) {
	Thread* self = fastiva_dvmThreadSelf();
	fastiva_dvmFullSuspendCheck(self);
}

#ifdef FASTIVA_CONCURRENT_STACK_SCAN
void fastiva_doScanThread(void* sp) {
	Thread* self = fastiva_dvmThreadSelf();
	self->m_pNativeStackPointer = sp;
	fastiva_dvmMarkThread(self, true);
}
#endif


#ifdef _DEBUG
static void test();
void ex_test(int a, int b, jlonglong c, jlonglong c1, jlonglong c2, jlonglong c3) {
	ALOGD("ex_test called");
}

extern "C" void call_ex_test();
#endif

void fastiva_check_runtime() {
#ifdef _DEBUG
	test();

    ALOGE("fastiva_check_runtime started");
	TRY$ {
	java_lang_String_p pStr = java_lang_IntegralToString_C$::importClass$()->intToString_((int)0x80000000);
	assert(pStr->charAt_(0) == '-');
	assert(java_lang_Integer_C$::importClass$()->parseInt_(pStr) == (int)0x80000000);
	java_util_ArrayList_C$::importClass$()->getGenericInterfaces_();
	pStr = java_lang_IntegralToString_C$::importClass$()->longToString_((jlonglong)0x8000000000000000);
	assert(pStr->charAt_(0) == '-');
	assert(java_lang_Long_C$::importClass$()->parseLong_(pStr) == (jlonglong)0x8000000000000000);
	assert(((ArrayObject*)0)->contents == ((fastiva_ArrayHeader*)0)->getBuffer_unsafe$());
	assert(((StringObject*)pStr)->array() == (void*)pStr->get__value());

	//java_lang_Class_p c2 = (java_lang_Class_p)dvmFindClassNoInit("Lcom/android/server/SystemServer;", NULL);
    //java_lang_Class_ap aClass6 = java_lang_Class_A::create$(1);
    //aClass6->set$(0, java_lang_String_A::importClass$());
    //void* pMethod3 = c2->getMethod_(fm::createAsciiString("main"), aClass6);

	d2f_getInnerClassName(dvmFindClassNoInit("Ljava/lang/Daemons$ReferenceQueueDaemon;", NULL));
	d2f_getInnerClassName(dvmFindClassNoInit("Ljava/util/AbstractMap$2;", NULL));

	java_util_HugeEnumSet_p pHes = FASTIVA_NEW(java_util_HugeEnumSet)(NULL, java_lang_Enum_A::create$(0));
	java_util_EnumSet_p pEs = (java_util_HugeEnumSet_p)pHes;
	pHes->complement_();
	pEs->complement_();

	//c = dvmFindClassNoInit("Landroid/widget/GridLayout$2;", NULL);
	//android_widget_GridLayout_0Alignment_p pObj = FASTIVA_NEW(android_widget_GridLayout_02)();
	//pObj->getAlignmentValue_(0, 0, 0);

	//for (int i = 0; i < 2205; i ++) {
	//	g_pLibcoreModule->getAnnotations(i);
	//}
    ALOGE("fastiva_check_runtime end");


	ulonglong ALL_ONES = ~0L;
		int a = 64;
		jlonglong  v1 = (ALL_ONES >> (-5 & 63));
		jlonglong  v2 = (ALL_ONES >> a);
		jlonglong  v3 = (ALL_ONES >> a);
		jlonglong  v4 = (ALL_ONES << a);

		call_ex_test();
	}
	CATCH_ANY$ {
	}

#endif
}

#ifdef _DEBUG

struct TTKMP2 {
	~TTKMP2() {
		ALOGD("Unwind by longjmp in MempryException");
	}
};

void test() {
	pthread_mutex_t mutex;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	int res;
	res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	assert(res == 0);
    res = pthread_mutex_init(&mutex, &attr);
	assert(res == 0);
    pthread_mutexattr_destroy(&attr);

	pthread_mutex_lock(&mutex);
	pthread_mutex_lock(&mutex);
        ALOGE("multi lock success");
	pthread_mutex_unlock(&mutex);
	pthread_mutex_unlock(&mutex);
        ALOGE("multi unlock success");
//*
#ifndef _WIN32
	volatile int zero = 0;
	{
		TRY$ {
			TTKMP2 tt;
			int* a = (int*)+3;
			*a = 3;
		}
		CATCH_ANY$ {
			fox_printf("First null pointer exception catched");
		}
	}
	{
		TRY$ {
			TTKMP2 tt;
			int* a = (int*)+3;
			*a = 3;
		}
		CATCH_ANY$ {
			fox_printf("second null pointer exception catched");
		}
	}
	{
		TRY$ {
			TTKMP2 tt;
			zero = 254/ zero;
		}
		CATCH_ANY$ {
			fox_printf("first div by zero exception catched");
		}
	}
	{
		TRY$ {
			TTKMP2 tt;
			int a = 0;
			zero = 254/ zero;
		}
		CATCH_ANY$ {
			fox_printf("second div by zero exception catched");
		}
	}
#if 0
	fox_printf("second div by zero exception catched %d", zero);
	{
		TRY$ {
			test_stackOverflow();
		}
		CATCH_ANY$ {
			fox_printf("Stack overflow Exception catched");
		}
	}
	{
		TRY$ {
			test_stackOverflow();
		}
		CATCH_ANY$ {
			fox_printf("Stack overflow Exception catched");
		}
	}
#endif
#endif
//*/
}


#endif

#ifdef _WIN32
void test_run(JNIEnv* env, jclass startClass, jmethodID startMeth, jarray strArray) {


	Thread* self = dvmThreadSelf();
	for (int i = 0; i < 200; i ++) {
		env->CallStaticVoidMethod(startClass, startMeth, strArray);
		env->ExceptionClear();

		dvmChangeStatus(NULL, THREAD_RUNNING);

		TRY$ {
		void* bootClassLoader = java_lang_Object_C$::getRawStatic$()->getClassLoader_();
		if (bootClassLoader != NULL) {
			void* systemClassLoader = java_lang_ClassLoader_C$::getRawStatic$()->getSystemClassLoader_();
			systemClassLoader = 0;
		}
		}
		CATCH_ANY$ {
		}



		void dvmCollectGarbage();
		dvmCollectGarbage();

		int size = java_util_Collections_C$::importClass$()->emptyMap_()->size_();
		assert(size == 0);
		FASTIVA_SET_NATIVE_STACK_POINTER(self, self->m_pNativeStackBottom);
		dvmChangeStatus(NULL, THREAD_NATIVE);
        //dvmClassShutdown();
	}
}
#endif
