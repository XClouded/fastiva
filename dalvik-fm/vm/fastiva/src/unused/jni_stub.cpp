#include <precompiled_libcore.h>

#if 1 || FASTIVA_SUPPORT_JNI_STUB

#include <fastiva/jni_stub.h>
#include <kernel/Kernel.h>
#include <kernel/Runnable.h>
#include <fox/Atomic.h>

#ifdef _WIN32
	#include <fastiva_malloc.h>
	#include <string.h>
#else 
	#include <alloca.h>
#endif

#define ENABLE_JNI_ARRAY_CACHE  false
/****
  LocalRef 처占쏙옙
  1. Java -> Native : 
      GlobalRef 전환이 불필요하다. 
  2. Native -> Java : 
      인자나 ReturnValue에 대한 LocalRef는 Push/PopLocalFrame()을 이용하여 제거한다.
      Return Value에 한하여 GlobalRef로 전환하고, 해당 GlobalRef는 별도 Q에 저장하였다가,
	  Jni함수 (Java->Native) 종료시 DeleteGlobalRef 를 호출하여 삭제한다.
	  % 주의 : 해당 ReturnValue가 field에 assign된 경우를 고려하여야한다. 
	           동일 LocalRef에 대한 NewGlobalRef() 중복 호출시 각각 다른 Ref가 생성되는지 확인할 것.
  3. Primitive Array의 변환
     J->N, N->J 변환시 모두 javaArray의 GlobalRef를 생성하고, fastivaArray이 unlink시 해제한다.
	 <<MemoryLeak 없슴>>
  4. 객체 Array의 변환
      객체 Array의 경우, buffer를 공유할 수 없으므로, 무조건 상호 복사하여 주고 받는다.
	 <<MemoryLeak 없슴>>
  6. String의 변환.
     Fastiva 객체는 WeakGlobalRef를 통해서 NativeStub을 참조한다.
	 변환된 또는 변환하는 String 객체의 LocalRef는 삭제하지 않는다.
	 <<MemoryLeak 없슴>>
  5. Fastiva -> NativeStub (역변환은 항상 nativeHandle 을 통해서만 이루어짐);
     Fastiva 객체는 WeakGlobalRef를 통해서 NativeStub을 참조하고,
	 nativeHandle 은 GlobalLock된 객체를 반환한다.
	 <<MemoryLeak 없슴>>
*/


JavaVM* g_pJavaVM;
//JNIEnv* fastiva_jniEnv_root;
extern fox_Semaphore* g_globalListLock;

class AutoSem {
	fox_Semaphore* sem;
public:
	AutoSem(fox_Semaphore* sem) {
		this->sem = sem;
		fox_semaphore_lock(sem);
	}
	~AutoSem() {
		fox_semaphore_release(sem);
	}
};

#define AutoGlobalLock()	AutoSem global_list_lock_$$(g_globalListLock)

extern void fastiva_GC_jni_clearWeakGlobalRef(void* env0);

//static jfieldID stub_nativeHandle;
static jfieldID string_value;
static jfieldID string_offset;
static jfieldID thread_contextClassLoader;

static jmethodID string_new;
//static jmethodID object_getClass;
//static jmethodID proxy_init;

jclass object_class;
static jclass fastivaStub_class;
static jclass string_class;
static jclass class_class;
static jclass thread_class;
#ifdef JPP_AUTO_CONVERT_CORE_CLASS
static jclass runnableStub_class;
#endif

static jclass inpStream_class;
static jclass outStream_class;
static jclass ioExStream_class;
//static jmethodID class_getClassLoader;
static jmethodID thread_currentThread;
static jmethodID fastivaStub_toEnum;
#ifdef JPP_AUTO_CONVERT_CORE_CLASS
	static jmethodID enum_ordinal;
	static jmethodID fastivaStub_getNativeHandle;
#endif
//static jclass class_FastivaRuntimeException;

static jclass javaLangClassLoader;
static jmethodID getSystemClassLoader, loadClass;
static jmethodID fastivaStub_loadClass;
static jmethodID fastivaStub_doJavaGC;
static jmethodID fastivaStub_createPrimitive;
#if JPP_ENABLE_J2N_SHARED_STRING
static jclass fastivaUtil_class;
static jmethodID fastivaUtil_getSharedFastivaString;
#endif
static jmethodID class_getName;
static jobject systemClassLoader = 0;

static jclass fileDescriptor_class;
static jmethodID fileDescriptor_init$;
static jfieldID fileDescriptor_descriptor;
// static jfieldID fileDescriptor_readOnly;



JNIEnv* fastiva_jni_getEnv() {
	fastiva_Task* pTask = fastiva_getCurrentTask();
	KASSERT(pTask != NULL);// && pTask->m_pStackContext != NULL);
	JNIEnv* pEnv = (JNIEnv*)pTask->m_pJNIEnv0;
	if (pEnv == NULL) {
		g_pJavaVM->AttachCurrentThread((void**)&pEnv, NULL);
		pTask->m_jniJavaThreadAttached = true;
		pTask->m_pJNIEnv0 = pEnv;

		jobject jThread = pEnv->CallStaticObjectMethod(thread_class, thread_currentThread);
		pEnv->SetObjectField(jThread, thread_contextClassLoader, systemClassLoader);
	}
	return (pEnv);
}

fastiva_JNIStubInfo* fastiva_JNIStubInfo::g_qStub = ADDR_ZERO;
fastiva_ProxyRegister* fastiva_ProxyRegister::g_qProxyRegister = ADDR_ZERO;
static int cntGlobalRef = 0;
static int cntStrongGlobalRef = 0;

static void dumpStackTrace() {
	fastiva_Task* pTask = fastiva_getCurrentTask();
	if (pTask != NULL) {
		pTask->m_pDebugTrace = (void*)1;
	}
}

// 51200 에 못미치는 지점에서 오류가 발생한다. (충분히 작게 잡을 것)
static const int MAX_GLOBAL_REF = 51200 - 2500;
static int s_currMaxGlobalRef = MAX_GLOBAL_REF;
static int s_MAX_STRONG_GLOBAL_REF = 1900;

void fastiva_jni_setMaxStrongGlobalRefCount(int count) {
	s_MAX_STRONG_GLOBAL_REF = count;
}

jobject fastiva_jni_NewWeakGlobalRef$(JNIEnv* pEnv, jobject obj) {
	dumpStackTrace();
	if ((fox_util_inc32(&cntGlobalRef) & 0xFFF) == 0) {
		fox_debug_printf("cntGlobalRef: %d", cntGlobalRef);
	}

	if (cntGlobalRef > s_currMaxGlobalRef) {
		pEnv->ExceptionClear();
		fox_printf("!!! Collecting WeakGlobal Garbage Ref!!");
		pEnv->CallStaticVoidMethod(fastivaStub_class, fastivaStub_doJavaGC);

		fastiva_GC_jni_clearWeakGlobalRef(pEnv);
		if (cntGlobalRef >= MAX_GLOBAL_REF - 100) {
			// 너무 자주 실행되지 않도록 
			s_currMaxGlobalRef = cntGlobalRef + 100;
		}
		else {
			s_currMaxGlobalRef = MAX_GLOBAL_REF;
		}
	}
	jobject res = pEnv->NewWeakGlobalRef(obj);
	return res;
}

static void* _getCachedGlobalArray(JNIEnv* pEnv, jobject jArray) {
	fastiva_Task* pCurrTask = fastiva_getCurrentTask(); 
	pCurrTask->enterCriticalSection();
	fastiva_ArrayHeader* pRes = NULL;
	for (int i = 0; i < CNT_GLOBAL_JNI_ARRAY_CACHE; i ++) {
		fastiva_ArrayHeader* pArray = pCurrTask->m_globalArrayCache[i];
		if (pArray != NULL && pEnv->IsSameObject((jobject)pArray->m_javaRef$, jArray)) {
			pRes = pArray;
			break;
		}
	}
	pCurrTask->leaveCriticalSection();
	return pRes;
}

#if _DEBUG
static bool print_cntStrongGlobalRef = false;
#endif

jobject fastiva_jni_NewGlobalRef$(JNIEnv* pEnv, jobject obj) {
	dumpStackTrace();

	if ((fox_util_inc32(&cntGlobalRef) & 0xFFF) == 0) {
		fox_debug_printf("cntGlobalRef: %d", cntGlobalRef);
	}

#if _DEBUG
	if (print_cntStrongGlobalRef) {
		fox_printf("current cntStrongGlobalRef is : %d", cntStrongGlobalRef);
		print_cntStrongGlobalRef = false;
	}
#endif

#ifndef _DEBUG
	if (fox_util_inc32(&cntStrongGlobalRef) > s_MAX_STRONG_GLOBAL_REF) {
		fox_printf("cntStrongGlobalRef(%d) > %d", cntStrongGlobalRef, s_MAX_STRONG_GLOBAL_REF);
		pEnv->CallStaticVoidMethod(fastivaStub_class, fastivaStub_doJavaGC);
		fastiva_GC_doGC(true);
#if _DEBUG
		print_cntStrongGlobalRef = true;
#endif
	}
#endif

	if (cntGlobalRef > s_currMaxGlobalRef) {
		fox_printf("!!! Collecting Global Garbage Ref!!");
		pEnv->CallStaticVoidMethod(fastivaStub_class, fastivaStub_doJavaGC);

		fastiva_GC_jni_clearWeakGlobalRef(pEnv);
		if (cntGlobalRef >= MAX_GLOBAL_REF - 100) {
			// 너무 자주 실행되지 않도록 
			s_currMaxGlobalRef = cntGlobalRef + 100;
		}
		else {
			s_currMaxGlobalRef = MAX_GLOBAL_REF;
		}
	}
	jobject res = pEnv->NewGlobalRef(obj);
	return res;
}

void debug_out_class_type(JNIEnv* pEnv, const char* format, jobject object) {
#ifdef _DEBUG
	jclass  proxy_class = (jclass)pEnv->GetObjectClass(object);
	jstring className = (jstring)pEnv->CallObjectMethod(proxy_class, class_getName);
	unicod* pUcsName = (unicod* )pEnv->GetStringCritical(className, NULL);

	char clsName[1024];
	fastiva.getUTFChars(clsName, pUcsName, pEnv->GetStringLength(className));
	fox_debug_printf(format, clsName);

	//java_lang_Class_p pClass = fm::loadClass(pUcsName, pEnv->GetStringLength(className), NULL);
	pEnv->ReleaseStringCritical(className, (jchar*)pUcsName);
	pEnv->DeleteLocalRef(className);
	pEnv->DeleteLocalRef(proxy_class);
#endif
}

void fastiva_jni_JavaProxyFinalized(JNIEnv* pEnv, java_lang_Object_p pObj) {
	{ 
		AutoGlobalLock(); 
		void* jref = (void*)fox_util_xchg32(&pObj->m_javaRef$, 0);
		if (IS_VALID_JAVA_REF(jref)) {
			//fox_debug_printf("!!! NativeStub detached\n");
			((JNIEnv*)pEnv)->DeleteWeakGlobalRef((jobject)jref);
			fox_util_dec32(&cntGlobalRef);
		}
	}
	fm::deleteGlobalRef(pObj);
}

void fastiva_jni_DeleteGlobalRef$(JNIEnv* pEnv, jobject retiree) {
	//debug_out_class_type(pEnv, "! Global Ref Deleted : %d\n", retiree);
	pEnv->DeleteGlobalRef(retiree);
	fox_util_dec32(&cntGlobalRef);
	fox_util_dec32(&cntStrongGlobalRef);
}

typedef Unicod_A Char_A;
typedef Unicod_ap Char_ap;
typedef Longlong_A Long_A;
typedef Longlong_ap Long_ap;

jclass getNativeStubClass(const fastiva_ClassContext* pContext, const fastiva_ClassContext* pAccessType, JNIEnv* pEnv = NULL);


#define CREATE_FASTIVA_ARRAY(pEnv, Type, jtype, jArray)										\
	Type##_ap res = NULL;																	\
	dumpStackTrace();																		\
	jboolean isCopy;																		\
	if (jArray == NULL) {																	\
	}																						\
	else if (!isArg) {																		\
		int len = pEnv->GetArrayLength((jarray)jArray);										\
		java_lang_Class_p pClass = Type##_A::importClass$();								\
		res = (Type##_ap)fastiva_allocArrayInstance(sizeof(jtype), pClass, len, 0, 0);	\
		pEnv->Get##Type##ArrayRegion((jtype##Array)jArray, 0, len, (jtype*)res->getBuffer_unsafe$());	\
	}																						\
	else if ((res = (Type##_ap)_getCachedGlobalArray(pEnv, jArray)) == NULL) {		\
		jtype##Array javaArray =  (jtype##Array)fastiva_jni_NewGlobalRef$(pEnv, jArray);		\
		pEnv->DeleteLocalRef(jArray);														\
		int len = pEnv->GetArrayLength((jarray)javaArray);									\
		java_lang_Class_p pClass = Type##_A::importClass$();								\
		void* pData = pEnv->Get##Type##ArrayElements(javaArray, &isCopy);					\
		res = (Type##_ap)fastiva_allocArrayInstance(sizeof(jtype), pClass, len, javaArray, pData);	\
		pEnv->Release##Type##ArrayElements(javaArray, (jtype*)pData, JNI_ABORT);			\
		fastiva_getCurrentTask()->setCachedGlobalArray((int)javaArray, res);		\
	}

Byte_ap toFastivaObject(Byte_ap env0, jobject obj, bool isArg) {
	JNIEnv* pEnv = (JNIEnv*)(void*)env0;
	CREATE_FASTIVA_ARRAY(pEnv, Byte, jbyte, obj);
	return res;
}

Unicod_ap toFastivaObject(Unicod_ap env0, jobject obj, bool isArg) {
	JNIEnv* pEnv = (JNIEnv*)(void*)env0;
	CREATE_FASTIVA_ARRAY(pEnv, Char, jchar, obj);
	return res;
}

Short_ap toFastivaObject(Short_ap env0, jobject obj, bool isArg) {
	JNIEnv* pEnv = (JNIEnv*)(void*)env0;
	CREATE_FASTIVA_ARRAY(pEnv, Short, jshort, obj);
	return res;
}

Int_ap toFastivaObject(Int_ap env0, jobject obj, bool isArg) {
	JNIEnv* pEnv = (JNIEnv*)(void*)env0;
	CREATE_FASTIVA_ARRAY(pEnv, Int, jint, obj);
	return res;
}

Longlong_ap toFastivaObject(Longlong_ap env0, jobject obj, bool isArg) {
	JNIEnv* pEnv = (JNIEnv*)(void*)env0;
	CREATE_FASTIVA_ARRAY(pEnv, Long, jlong, obj);
	return res;
}


Float_ap toFastivaObject(Float_ap env0, jobject obj, bool isArg) {
	JNIEnv* pEnv = (JNIEnv*)(void*)env0;
	CREATE_FASTIVA_ARRAY(pEnv, Float, jfloat, obj);
	return res;
}


Double_ap toFastivaObject(Double_ap env0, jobject obj, bool isArg) {
	JNIEnv* pEnv = (JNIEnv*)(void*)env0;
	CREATE_FASTIVA_ARRAY(pEnv, Double, jdouble, obj);
	return res;
}


void fastiva_jni_releaseJavaArray(fastiva_ArrayHeader* pArray) {
	jobject javaArray = (jobject)pArray->m_javaRef$;
		fastiva_Task* pCurrTask = fastiva_getCurrentTask(); 
		pCurrTask->enterCriticalSection();
		for (int i = 0; i < CNT_GLOBAL_JNI_ARRAY_CACHE; i ++) {
			if (pArray == pCurrTask->m_globalArrayCache[i]) {
				pCurrTask->m_globalArrayCache[i] = NULL;
				break;
			}
		}
		pCurrTask->leaveCriticalSection();
	fastiva_jni_DeleteGlobalRef$(fastiva_jni_getEnv(), (jobject)javaArray);
}

jclass findDalvikSystemClass(JNIEnv* pEnv, const char* className) {
    jclass cls = NULL;
    cls = pEnv->FindClass(className);
    if (cls != ADDR_ZERO) {
    	return cls;
    }
	pEnv->ExceptionClear();

    jstring strClassName;

    /* create an object for the class name string; alloc could fail */
    strClassName = pEnv->NewStringUTF(className);
    if (strClassName == ADDR_ZERO) {
        fox_printf("ERROR: unable to convert '%s' to string\n", className);
        goto bail;
    }


    /* try to find the named class */
    cls = (jclass) pEnv->CallStaticObjectMethod(fastivaStub_class, fastivaStub_loadClass,
                        strClassName);
    if (pEnv->ExceptionOccurred()) {
        fox_printf("ERROR: unable to load class by StubClassLoader '%s' from %p\n",
            className, systemClassLoader);
		return 0;
    }
    else {
    	fox_debug_printf("Stub class loader is %p, loading %p (%s)\n",
    			systemClassLoader, strClassName, className);
    	goto bail;
    }

    if (systemClassLoader == 0) {
    	return 0;
    }

    /* try to find the named class */
    cls = (jclass) pEnv->CallObjectMethod(systemClassLoader, loadClass,
                        strClassName, true);
    if (pEnv->ExceptionOccurred()) {
        fox_printf("ERROR: unable to load class '%s' from %p\n",
            className, systemClassLoader);
    }
    else {
    	fox_debug_printf("system class loader is %p, loading %p (%s)\n",
    			systemClassLoader, strClassName, className);
    }

bail:
	pEnv->ExceptionClear();
	pEnv->DeleteLocalRef(strClassName);
    return cls;
}

jobject toJavaObject(JNIEnv* pEnv, java_lang_Throwable_p ex) {
	char name[512];
	const fastiva_ClassContext* pContext = ex->getClass()->m_pContext$;
	fox_debug_printf("create Java exception\n");
	jclass clazz = (jclass)pContext->m_pClass->javaClass$;

	if (clazz != ADDR_ZERO) {
		jobject jEx = toNativeStub(pEnv, pContext, ex);
		return jEx;
	}


	clazz = getNativeStubClass(pContext, NULL, pEnv);

	char* str = 0;
	java_lang_String_p message = ex->getMessage();
	if (message != ADDR_ZERO) {
		str = (char*)alloca(message->length() * 3);
		char* end = fastiva.getUTFChars(str, fm::getUnsafeStringBuffer(message), message->length());
		*end = 0;
	}
	if (pEnv->ExceptionCheck()) {
		fox_debug_printf("create Java exception w\n");
	}

	while (true) {
		jmethodID init_ex = pEnv->GetMethodID(clazz, "<init>", "()V");
		if (init_ex == 0) {
			pEnv->ExceptionClear();
			clazz = pEnv->GetSuperclass(clazz);
			continue;
		}
		jobject throwable = pEnv->NewObject(clazz, init_ex);
		return throwable;
	}
	/*
	int res2 = pEnv->ThrowNew(clazz, str);
	jobject throwable = pEnv->ExceptionOccurred();
	pEnv->ExceptionClear();
	*/
}

void fastiva_jni_throwJavaException(JNIEnv* pEnv, java_lang_Throwable_p ex) {
	fox_debug_printf("fastiva throw exception to java \n");

	jthrowable jex = (jthrowable)toJavaObject(pEnv, ex);
	pEnv->Throw(jex);
	fox_debug_printf("fastiva throw pending exception to java \n");
}

jclass getNativeStubClass(const fastiva_ClassContext* pContext0, const fastiva_ClassContext* pAccessType, JNIEnv* pEnv) {
	const fastiva_ClassContext* pContext = pContext0;
	while (pContext != java_lang_Object::getRawContext$()) {
		jclass clazz = (jclass)pContext->m_pClass->javaClass$;
		if (clazz != NULL) {
#ifdef _DEBUG
			if (pAccessType != NULL && pAccessType != pContext) {
				if(!pAccessType->isAssignableFrom(pContext)) {
					char name1[512];
					int len1 = fm::getClassNameUTF8(name1, sizeof(name1), pContext);
					char name2[512];
					int len2 = fm::getClassNameUTF8(name2, sizeof(name2), pAccessType);
					fox_printf("Can't cast %s to %s\n", name1, name2);
                    fox_exit(-1);
				}
			}
#endif
			return clazz;
		}
#ifdef _DEBUG
		if (!pContext->isInstance()) {
			char name[512];
			int len = fm::getClassNameUTF8(name, sizeof(name), pContext);
			fox_printf("Wrong interface %s\n", name);
			return NULL;
		}
#endif
		if (pEnv != NULL) {
			char name[512];
			int len = fm::getClassNameUTF8(name, sizeof(name), pContext);
			clazz = findDalvikSystemClass(pEnv, name);
			if (clazz != NULL) {
				//Exception 占쏙옙 占쏙옙占쌔쇽옙 占쏙옙占싫댐옙. NativeStub 占쏙옙 占싣니므뤄옙 javaClass$ 占쏙옙 占쏙옙占쏙옙占쌔쇽옙占쏙옙 占싫된댐옙.
				//pContext->m_pClass->javaClass$ = clazz;
				fox_debug_printf("Exception class load : %s:%x", name, clazz);
				return clazz;
			}
			pEnv->ExceptionClear();
		}

		pContext = pContext->toInstanceContext()->m_pSuperContext;
	}

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
	if (pAccessType == java_lang_Runnable::getRawContext$()) {
		return runnableStub_class;
	}
#endif

	return NULL;
/*
	char name[512];
	int len = fm::getClassNameUTF8(name, sizeof(name), pContext);
	clazz = findDalvikSystemClass(name);
	if (clazz == NULL) {
		pEnv->ExceptionClear();
		fox_debug_printf("fail FindClass %s\n", name);
		return NULL;
	}
	jclass g_clazz = (jclass)fastiva_jni_NewGlobalRef$(clazz);
	pContext->m_pClass->javaClass$ = g_clazz;

	pEnv->DeleteLocalRef(clazz);

	return g_clazz;
*/
}

java_lang_String_p toFastivaObject(java_lang_String_p env0, jobject obj, bool isArg) {
	if (obj == NULL) {
		return NULL;
	}
	dumpStackTrace();
	JNIEnv* pEnv = (JNIEnv*)(void*)env0; 
#if JPP_ENABLE_J2N_SHARED_STRING
	int handle = pEnv->CallStaticIntMethod(fastivaUtil_class, fastivaUtil_getSharedFastivaString, obj);
	if (handle != 0) {
		return (java_lang_String_p)handle;
	}
#endif
	int len = pEnv->GetStringLength((jstring)obj);
	const jchar* pText =  pEnv->GetStringCritical((jstring)obj, NULL);
	Unicod_ap txtBuf = (Unicod_ap)fastiva_allocArrayInstance(sizeof(jchar), Char_A::importClass$(), len, 0, 0);//glock, pText);
	memcpy(txtBuf->getBuffer_unsafe$(), pText, len*2);
	java_lang_String_p pStr = FASTIVA_NEW(java_lang_String)(0, len, txtBuf);
	pEnv->ReleaseStringCritical((jstring)obj, (jchar*)pText);//, JNI_ABORT);
	return pStr;
}

java_io_FileDescriptor_p toFastivaObject(java_io_FileDescriptor_p env0, jobject obj, bool isArg) {
	if (obj == NULL) {
		return NULL;
	}
	dumpStackTrace();
	JNIEnv* pEnv = (JNIEnv*)(void*)env0; 
	int descriptor = pEnv->GetIntField(obj, fileDescriptor_descriptor);
	// jbool readOnly = pEnv->GetBooleanField(obj, fileDescriptor_readOnly);

	java_io_FileDescriptor_p fd = FASTIVA_NEW(java_io_FileDescriptor)();
	fd->m_descriptor = descriptor;
	// fd->m_readOnly = readOnly;
	return fd;
}

#ifdef JPP_AUTO_CONVERT_CORE_CLASS

int jni_get_enum_ordinal(JNIEnv* pEnv, fastiva_BytecodeProxy_p pObj) {
	return pEnv->CallIntMethod((jobject)pObj, enum_ordinal);
}

java_lang_Enum_p toFastivaObject(java_lang_Enum_p env0, jobject enumObj) {
	if (enumObj == NULL) {
		return NULL;
	}
	JNIEnv* pEnv = (JNIEnv*) env0;
	int enumId = pEnv->CallIntMethod(enumObj, enum_ordinal);

	jclass  proxy_class = (jclass)pEnv->GetObjectClass(enumObj);
	jstring className = (jstring)pEnv->CallObjectMethod(proxy_class, class_getName);
	unicod* pUcsName = (unicod* )pEnv->GetStringCritical(className, NULL);
	java_lang_Class_p pClass = fm::loadClass(pUcsName, pEnv->GetStringLength(className), NULL);
	pEnv->ReleaseStringCritical(className, (jchar*)pUcsName);
	pEnv->DeleteLocalRef(className);
	pEnv->DeleteLocalRef(proxy_class);
	if (pClass == NULL || !pClass->isAssignableFrom(java_lang_Enum_C$::importClass$())) {
		THROW_EX_NEW$(java_lang_ClassNotFoundException, ());
	}

	java_lang_Enum_p v = ((fastiva_EnumClass_p)pClass)->getEnumValue$(enumId);
	return v;
}

#endif

java_lang_String_ap toFastivaObject(java_lang_String_ap env0, jobject javaArray, bool isArg) {
	if (javaArray == NULL) {
		return NULL;
	}
	JNIEnv* pEnv = (JNIEnv*) env0;
	int len = pEnv->GetArrayLength((jarray)javaArray);						
	java_lang_String_ap fmArray = java_lang_String_A::create$(len);
	for (int i = len; --i >= 0; ) {
		jstring obj = (jstring)pEnv->GetObjectArrayElement((jobjectArray)javaArray, i);
		java_lang_String_p pStr = toFastivaObject((java_lang_String_p)env0, obj, isArg);

		fmArray->set$(i, pStr);
	}
	return fmArray;
}

fastiva_ArrayHeader* toFastivaArray(fastiva_BytecodeProxy_p env0, void* javaClass, jobject javaArray) {
	if (javaArray == NULL) {
		return NULL;
	}
	JNIEnv* pEnv = (JNIEnv*) env0;
	int len = pEnv->GetArrayLength((jarray)javaArray);						
	fastiva_BytecodeClass_ap fmArray = fastiva_BytecodeClass_A::create$(len);
	for (int i = len; --i >= 0; ) {
		jobject obj = pEnv->GetObjectArrayElement((jobjectArray)javaArray, i);
		fmArray->set$(i, (fastiva_BytecodeProxy_p)obj);
		pEnv->DeleteLocalRef(obj);
	}
	return fmArray;
}

/**
  to do : fasiva-stub 종류 별로, nativeHandle fieldID를 관리하면 access 속도 향상 가능 !!!
*/
java_lang_Object_p nativeStubToFastivaObject(JNIEnv* pEnv, const fastiva_ClassContext* pContext, jobject proxy) {
	if (proxy == NULL) {
		return NULL;
	}
	jclass  proxy_class = (jclass)pEnv->GetObjectClass(proxy);
						  //getNativeStubClass(pContext);
	jfieldID nativeHandle = pEnv->GetFieldID(proxy_class, "nativeHandle", "I");
	java_lang_Object_p res = (java_lang_Object_p)pEnv->GetIntField(proxy, nativeHandle);
	pEnv->DeleteLocalRef(proxy_class);
	pEnv->DeleteLocalRef(proxy);
	// 원인모르나, DeleteLocalRef 를 할 필요가 없다. pEnv->DeleteLocalRef(proxy);
	if (0  && !(pContext == res->getClass()->getContext$() || pContext->isAssignableFrom(res->getClass()->getContext$()))) {
		char name1[512], name2[512];
		int len1 = fm::getClassNameUTF8(name1, sizeof(name1), pContext);
		int len2 = fm::getClassNameUTF8(name2, sizeof(name2), res->getClass()->getContext$());
		fox_printf("Invalid jniStub casting %s to %s\n", name2, name1);
		fox_exit(-1);
	}
	KASSERT(res != NULL && pContext == res->getClass()->getContext$() || pContext->isAssignableFrom(res->getClass()->getContext$()));
	return res;
}

#ifndef _WIN32
class fastiva_InterfaceStub : public fastiva_Interface {
public:
	fastiva_InterfaceStub(java_lang_Object_p pObj, const fastiva_ClassContext* pContext) {
			const void** pIVT = fastiva.checkImplemented(pObj, pContext->toInterfaceContext());
			this->init(pObj, pIVT);
	}
};
#endif 

java_lang_Object_ap nativeStubToFastivaArray(JNIEnv* pEnv, const fastiva_ClassContext* pContext, jobject javaArray) {
	if (javaArray == NULL) {
		return NULL;
	}
	int len = pEnv->GetArrayLength((jarray)javaArray);						
	java_lang_Object_ap fmArray = (java_lang_Object_ap)fastiva.allocatePointerArray(pContext, len);
	java_lang_Object_p* pItems = pContext->isInterface() ? (java_lang_Object_p*)fmArray->getBuffer_unsafe$() : NULL;
	for (int i = len; --i >= 0; ) {
		jobject item = pEnv->GetObjectArrayElement((jobjectArray)javaArray, i);
		java_lang_Object_p pObj = nativeStubToFastivaObject(pEnv, pContext, item);
		if (pItems != NULL) {
#ifndef _WIN32
			fastiva_InterfaceStub ifc(pObj, pContext);
			fastiva.setInterfaceArrayItem(fmArray, pContext, pItems + i, ifc);
#endif
		}
		else {
			fmArray->set$(i, pObj);
		}
	}
	return fmArray;
}

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
java_lang_Object_p toFastivaObject(java_lang_Object_p env0, jobject jobj) {
	if (jobj == NULL) {
		return NULL;
	}
	JNIEnv* pEnv = (JNIEnv*) env0;
	int handle = pEnv->CallStaticIntMethod(fastivaStub_class, fastivaStub_getNativeHandle, jobj);
	if (handle != 0) {
		return (java_lang_Object_p)handle;
	}
	else {
		java_lang_Object_p res = FASTIVA_NEW(java_lang_Object)();
		res = fm::createGlobalRef(res);
		res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
		return res;
	}
}
#endif


#define CREATE_JAVA_ARRAY_0(pEnv, Type, jtype, array)															\
	jtype##Array res;																					\
    if (array == NULL) {																				\
    	res = NULL;																						\
    }																									\
    else {																								\
			int len = array->length();																	\
			res = pEnv->New##Type##Array(len);												\
			pEnv->Set##Type##ArrayRegion(res, 0, len, (jtype*)array->getBuffer_unsafe$());	\
	}																									\


#define CREATE_JAVA_ARRAY(pEnv, Type, jtype, array)														\
	jtype##Array res;																					\
		dumpStackTrace();																				\
    if (array == NULL) {																				\
    	return NULL;																					\
    }																									\
    res = (jtype##Array)array->m_javaRef$;																\
    if (IS_VALID_JAVA_REF(res)) {																		\
		return res;																						\
	}																									\
	int len = array->length();																			\
    if (!isArg) {																			\
			res = pEnv->New##Type##Array(len);															\
			pEnv->Set##Type##ArrayRegion(res, 0, len, (jtype*)array->getBuffer_unsafe$());				\
	}																									\
	else {																								\
			jtype##Array local_res = pEnv->New##Type##Array(len);										\
			res = (jtype##Array)fastiva_jni_NewGlobalRef$(pEnv, local_res);								\
			pEnv->DeleteLocalRef(local_res);															\
			pEnv->Set##Type##ArrayRegion(res, 0, len, (jtype*)array->getBuffer_unsafe$());				\
			const jtype* pData = pEnv->Get##Type##ArrayElements(res, NULL);								\
			fastiva_jni_exchangeBuffer(array, (void*)res, (void*)pData);								\
			pEnv->Release##Type##ArrayElements(res, (jtype*)pData, JNI_ABORT);							\
	}																									\


jbyteArray toJavaObject(JNIEnv* pEnv, Byte_ap array, bool isArg) {
	CREATE_JAVA_ARRAY(pEnv, Byte, jbyte, array);
	return res;
}

jcharArray toJavaObject(JNIEnv* pEnv, Unicod_ap array, bool isArg) {
	CREATE_JAVA_ARRAY(pEnv, Char, jchar, array);
	return res;
}

jshortArray toJavaObject(JNIEnv* pEnv, Short_ap array, bool isArg) {
	CREATE_JAVA_ARRAY(pEnv, Short, jshort, array);
	return res;
}

jintArray toJavaObject(JNIEnv* pEnv, Int_ap array, bool isArg) {
	CREATE_JAVA_ARRAY(pEnv, Int, jint, array);
	return res;
}

jlongArray toJavaObject(JNIEnv* pEnv, Longlong_ap array, bool isArg) {
	CREATE_JAVA_ARRAY(pEnv, Long, jlong, array);
	return res;
}


jfloatArray toJavaObject(JNIEnv* pEnv, Float_ap array, bool isArg) {
	CREATE_JAVA_ARRAY(pEnv, Float, jfloat, array);
	return res;
}


jdoubleArray toJavaObject(JNIEnv* pEnv, Double_ap array, bool isArg) {
	CREATE_JAVA_ARRAY(pEnv, Double, jdouble, array);
	return res;
}

java_lang_Throwable_p toFastivaObject(java_lang_Throwable_p env0, jobject javaException) {
	FASTIVA_DBREAK();
	if (javaException == NULL) {
		return NULL;
	}
	JNIEnv* pEnv = (JNIEnv*) env0;
	jclass  proxy_class = (jclass)pEnv->GetObjectClass(javaException);
	jstring className = (jstring)pEnv->CallObjectMethod(proxy_class, class_getName);
	unicod* pUcsName = (unicod* )pEnv->GetStringCritical(className, NULL);
	java_lang_Class_p pClass = fm::loadClass(pUcsName, pEnv->GetStringLength(className), NULL);
	pEnv->ReleaseStringCritical(className, (jchar*)pUcsName);
	pEnv->DeleteLocalRef(className);
	pEnv->DeleteLocalRef(proxy_class);
	pEnv->DeleteLocalRef(javaException);
	java_lang_Throwable_p ex = java_lang_Throwable::ptr_cast$(pClass->newInstance());
	return ex;
}


jobject toJavaObject(JNIEnv* pEnv, java_io_FileDescriptor_p fd, bool isArg) {
	if (fd == NULL) {
		return NULL;
	}
	jobject obj = pEnv->NewObject(fileDescriptor_class, fileDescriptor_init$);
	pEnv->SetIntField(obj, fileDescriptor_descriptor, fd->m_descriptor);
	// pEnv->SetBooleanField(obj, fileDescriptor_readOnly, fd->m_readOnly);
	return obj;
}



jstring toJavaObject(JNIEnv* pEnv, java_lang_String_p pStr, bool isArg) {
	if (pStr == NULL) {
		return NULL;
	}
	{ AutoGlobalLock(); 
		jstring jstr = (jstring)pStr->m_javaRef$;
		if (IS_VALID_JAVA_REF(jstr)) {
			jstring res = (jstring)pEnv->NewLocalRef(jstr); 
			if (res != NULL) {
				return res;
			}
			else {
				void* jref = (void*)fox_util_xchg32(&pStr->m_javaRef$, 0);
				if (IS_VALID_JAVA_REF(jref)) {
					pEnv->DeleteWeakGlobalRef(jstr);
					fox_util_dec32(&cntGlobalRef);
				}
			}
		}
	}
	int len = pStr->m_count;
	jobject obj = pEnv->NewString((jchar*)(fm::getUnsafeStringBuffer(pStr)), len);
	return (jstring)obj;
}


jobjectArray toJavaObject(JNIEnv* pEnv, java_lang_String_ap array, bool isArg) {
	if (array == NULL) {
		return NULL;
	}

	int len = array->length();																	
	jobjectArray javaArray = pEnv->NewObjectArray(len, string_class, NULL);										

	for (int i = array->length(); --i >= 0; ) {
		java_lang_String_p pStr = array->get$(i);
		jstring obj = toJavaObject(pEnv, pStr, isArg);
		pEnv->SetObjectArrayElement(javaArray, i, obj);
		pEnv->DeleteLocalRef(obj);
	}
	return javaArray;
}

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
jobject toJavaObject(JNIEnv* pEnv, java_io_InputStream_p inp) {
	return toNativeStub(pEnv, java_io_InputStream::getRawContext$(), inp);
}

jobject toJavaObject(JNIEnv* pEnv, java_io_OutputStream_p out) {
	return toNativeStub(pEnv, java_io_OutputStream::getRawContext$(), out);
}

jobject toJavaObject(JNIEnv* pEnv, java_lang_Runnable_p obj) {
	return toNativeStub(pEnv, java_lang_Runnable::getRawContext$(), obj);
}

jobject toJavaObject(JNIEnv* pEnv, java_lang_Enum_p obj) {
	fastiva_Class* pClass = obj->m_pClass$;
	jclass clazz = (jclass)pClass->javaClass$;
	if (clazz == NULL) {
		const fastiva_ClassContext* pContext = pClass->m_pContext$;
		char name[512];
		int len = fm::getClassNameUTF8(name, sizeof(name), pContext);
	 	clazz = findDalvikSystemClass(pEnv, name);
	 	if (clazz == NULL) {
			fox_printf("Enum class(%s) not found", name);
			fox_exit(-1);
	 	}
		pClass->javaClass$ = fastiva_jni_NewGlobalRef$(pEnv, clazz);
	}
	jobject v = pEnv->CallStaticObjectMethod(fastivaStub_class, fastivaStub_toEnum, clazz, obj->ordinal());
	return v;
}

jobject toJavaObject(JNIEnv* pEnv, java_lang_Object_p fmObj) {
	{ AutoGlobalLock(); 
	jobject obj = (jobject)fmObj->m_javaRef$;
	if (IS_VALID_JAVA_REF(obj)) {
		jobject res = pEnv->NewLocalRef(obj); 
		if (res != NULL) {
			return res;
		}
		else {
			void* jref = (void*)fox_util_xchg32(&fmObj->m_javaRef$, 0);
			if (IS_VALID_JAVA_REF(jref)) {
				pEnv->DeleteWeakGlobalRef((jobject)jref);
				fox_util_dec32(&cntGlobalRef);
			}
		}
	}
	}
	int id = fm::getPrimitiveReflectionType(fmObj->getClass$());
	if (id < 0) {
		const fastiva_ClassContext* pContext = fmObj->getClass$()->m_pContext$;
		char name[512];
		int len = fm::getClassNameUTF8(name, sizeof(name), pContext);
		fox_printf("Fastiva class(%s) can not convert to FastivaStub", name);
		fox_exit(-1);
	}
	jlonglong slot;
	void* pSlot = &slot;
	switch (id) {
	case fastiva_Primitives::jbool: 
		*(jbool*)pSlot = ((java_lang_Boolean_p)fmObj)->booleanValue();
		break;;

	case fastiva_Primitives::jbyte:
		*(jbyte*)pSlot = ((java_lang_Byte_p)fmObj)->intValue();
		break;

	case fastiva_Primitives::unicod:
		*(unicod*)pSlot = ((java_lang_Character_p)fmObj)->charValue();
		break;

	case fastiva_Primitives::jshort:
		*(jshort*)pSlot = ((java_lang_Short_p)fmObj)->intValue();
		break;

	case fastiva_Primitives::jint:
		*(jint*)pSlot = ((java_lang_Integer_p)fmObj)->intValue();
		break;

	case fastiva_Primitives::jlonglong:
		*(jlonglong*)pSlot = (jlonglong)((java_lang_Long_p)fmObj)->longValue();
		break;

	case fastiva_Primitives::jfloat:
		*(jfloat*)pSlot = (jfloat)((java_lang_Float_p)fmObj)->doubleValue();
		break;

	case fastiva_Primitives::jdouble:
		*(jdouble*)pSlot = ((java_lang_Double_p)fmObj)->doubleValue();
		break;
	};
	jobject res = pEnv->CallStaticObjectMethod(fastivaStub_class, fastivaStub_createPrimitive, id, slot);
	// obj 에 대한 handle 이 외부에 등록되지 않으므로 global lock 을 할 필요가 없다.
	fmObj->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, res);
}
#endif


jobjectArray toJavaArray(fastiva_BytecodeProxy_p env0, void* javaClass, fastiva_ArrayHeader* array) {
	if (array == NULL) {
		return NULL;
	}
	JNIEnv* pEnv = (JNIEnv*)env0;
	int len = array->length();																	
	jobjectArray javaArray = pEnv->NewObjectArray(len, (jclass)javaClass, NULL);										

	for (int i = array->length(); --i >= 0; ) {
		jobject item = (jobject)((fastiva_BytecodeClass_ap)array)->get$(i);
		pEnv->SetObjectArrayElement(javaArray, i, item);
	}
	return javaArray;
}

void* toJ2NProxy(java_lang_Object_p fmObj) {
	return toNativeStub(fastiva_jni_getEnv(), java_lang_Object::getRawContext$(), fmObj);
}


jobject toNativeStub(JNIEnv* pEnv, const fastiva_ClassContext* pRetTypeContext, java_lang_Object_p fmObj) {
	if (fmObj == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	jobject obj;
	{ AutoGlobalLock(); 
	obj = (jobject)fmObj->m_javaRef$;
	if (IS_VALID_JAVA_REF(obj)) {
		jobject res = pEnv->NewLocalRef(obj); 
		if (res != NULL) {
			return res;
		}
		else {
			void* jref = (void*)fox_util_xchg32(&fmObj->m_javaRef$, 0);
			if (IS_VALID_JAVA_REF(jref)) {
				pEnv->DeleteWeakGlobalRef((jobject)jref);
				fox_util_dec32(&cntGlobalRef);
			}
		}
	}
	}

	jclass c;
	c = getNativeStubClass(fmObj->getClass$()->m_pContext$, pRetTypeContext);
	if (c == NULL) {
		c = (jclass)pRetTypeContext->m_pClass->javaClass$;
		char name[512];
		int len = fm::getClassNameUTF8(name, sizeof(name), fmObj->getClass$()->m_pContext$);
		if (c != NULL) {
			fox_debug_printf("!!! Hidden Implemetatin Found : %s", name);
		}
		else {
			fox_printf("!!! Jni Stub class Not Found : %s", name);
			fox_exit(-1);
		}
	}

	if (pEnv->ExceptionCheck()) {
		fox_debug_printf("create Java exception 22 w\n");
	}

	jmethodID proxy_init = pEnv->GetMethodID(c, "<init>", "()V");
	if (proxy_init == NULL) {
		char name[512];
		int len = fm::getClassNameUTF8(name, sizeof(name), fmObj->getClass$()->m_pContext$);
		if (c != NULL) {
			fox_debug_printf("!!! NativeStub <init>()V method not found !!! : %s", name);
			fox_exit(-1);
		}
		fastiva_throwOutOfMemoryError();
	}

	obj = pEnv->NewObject(c, proxy_init, fmObj);
	if (obj == NULL) {
		fox_printf("!!! Can't create FastivaStub. maybe OutOfMemry Error");
		pEnv->ExceptionDescribe();
		fastiva_throwOutOfMemoryError();
	}

	jfieldID nativeHandle = pEnv->GetFieldID(c, "nativeHandle", "I");
	if (nativeHandle == NULL) {
		char name[512];
		int len = fm::getClassNameUTF8(name, sizeof(name), fmObj->getClass$()->m_pContext$);
		fox_printf("Fatal!! nativeHandle field not found%s\n", name);

		jstring className = (jstring)pEnv->CallObjectMethod(c, class_getName);
		const char* pszName = pEnv->GetStringUTFChars(className, NULL);
		fox_printf("Fatal!! nativeHandle field not found in %s\n", pszName);
		pEnv->ReleaseStringUTFChars(className, pszName);

		fastiva_throwNoClassDefFoundError();
	}
	fmObj = fm::createGlobalRef(fmObj);
	pEnv->SetIntField(obj, nativeHandle, (jint)fmObj);

	fmObj->m_javaRef$ = fastiva_jni_NewWeakGlobalRef$(pEnv, obj);
	//obj = pEnv->NewLocalRef((jobject)fmObj->m_javaRef$);
	KASSERT(obj != NULL);
	return obj;
}

jobjectArray toNativeStubArray(JNIEnv* pEnv, const fastiva_ClassContext* pRetTypeContext, fastiva_ArrayHeader* array) {
	if (array == NULL) {
		return NULL;
	}

	jclass javaClass = getNativeStubClass(array->getClass$()->m_pContext$, NULL);
	if (javaClass == NULL) {
		char name[512];
		int len = fm::getClassNameUTF8(name, sizeof(name), array->getClass$()->m_pContext$);
		fox_printf("!!! fail Find NativeStub Class %s\n", name);
		fox_exit(-1);
	}

	int len = array->length();																	
	jobjectArray javaArray = pEnv->NewObjectArray(len, (jclass)javaClass, NULL);										

	for (int i = array->length(); --i >= 0; ) {
		java_lang_Object_p pObj = ((java_lang_Object_ap)array)->get$(i);
		jobject item = toNativeStub(pEnv, pRetTypeContext, pObj);
		pEnv->SetObjectArrayElement(javaArray, i, item);
		pEnv->DeleteLocalRef(item);
	}
	return javaArray;
}



#define jni_log fox_debug_printf

bool fastiva_jni_DeleteWeakGlobalRef(void *env0, fastiva_Instance_p pObj) {
	jobject jref = (jobject)pObj->m_javaRef$; 
	if (IS_VALID_JAVA_REF(jref) && ((JNIEnv*)env0)->IsSameObject(jref, NULL)) {
		// 사용이 종료된 WeakGlobalRef 를 삭제한다.
		jref = (jobject)fox_util_xchg32(&pObj->m_javaRef$, 0);
		if (IS_VALID_JAVA_REF(jref)) {
			((JNIEnv*)env0)->DeleteWeakGlobalRef((jobject)jref);
			fox_util_dec32(&cntGlobalRef);
			return true;
		}
	}
	return false;
}

jclass jni_getGlobalClass(JNIEnv* pEnv, const char* classPath, bool ignoreException) {
    jclass c = pEnv->FindClass(classPath);
	if (c == NULL) {
		fox_printf("class not found : %s", classPath);
		if (ignoreException) {
			pEnv->ExceptionDescribe();
			pEnv->ExceptionClear();
			return NULL;
		}
        fox_exit(-1);
	}
	jclass c2 = (jclass)fastiva_jni_NewGlobalRef$(pEnv, c);
	pEnv->DeleteLocalRef(c);

	fox_util_inc32(&cntStrongGlobalRef);

	return c2;
}

void fastiva_jni_initStub0(void* pJniEnv0) {
	/**
	 Android의 버그인지 JNI_OnLoad 외의 다른 thread에서 
	 java/lang/String을 찾지 못하는 문제가 있다.
	 JNI_OnLoad에서 반드시 호출되어야 한다.
	*/
	JNIEnv* pJniEnv = (JNIEnv*) pJniEnv0;

    /* find the "system" class loader; none of this is expected to fail */
    javaLangClassLoader = jni_getGlobalClass(pJniEnv, "java/lang/ClassLoader");
    loadClass = pJniEnv->GetMethodID(javaLangClassLoader,
        "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
    KASSERT(loadClass != NULL);

    fileDescriptor_class = jni_getGlobalClass(pJniEnv, "java/io/FileDescriptor");
    fileDescriptor_init$ = pJniEnv->GetMethodID(fileDescriptor_class,
        "<init>", "()V");
    KASSERT(fileDescriptor_init$ != NULL);
	fileDescriptor_descriptor = pJniEnv->GetFieldID(fileDescriptor_class, "descriptor", "I");
	// fileDescriptor_readOnly = pJniEnv->GetFieldID(fileDescriptor_class, "readOnly", "Z");
    KASSERT(fileDescriptor_descriptor != NULL);
	// KASSERT(fileDescriptor_readOnly != NULL);
    
	//systemClassLoader = pJniEnv->CallStaticObjectMethod(javaLangClassLoader,
    //    getSystemClassLoader);
    //KASSERT(systemClassLoader != NULL);

    //getSystemClassLoader = pJniEnv->GetStaticMethodID(javaLangClassLoader,
    //    "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    //KASSERT(getSystemClassLoader != NULL);
    //systemClassLoader = pJniEnv->CallStaticObjectMethod(javaLangClassLoader,
    //    getSystemClassLoader);
    //KASSERT(systemClassLoader != NULL);

	class_class = jni_getGlobalClass(pJniEnv, "java/lang/Class");
	class_getName = pJniEnv->GetMethodID(class_class,
        "getName", "()Ljava/lang/String;");
	KASSERT(class_getName != NULL);


	thread_class = jni_getGlobalClass(pJniEnv, "java/lang/Thread");
	thread_currentThread = pJniEnv->GetStaticMethodID(thread_class,
        "currentThread", "()Ljava/lang/Thread;");
	KASSERT(thread_currentThread != NULL);
	thread_contextClassLoader = pJniEnv->GetFieldID(thread_class,
        "contextClassLoader", "Ljava/lang/ClassLoader;");
	KASSERT(thread_contextClassLoader != NULL);

	object_class = jni_getGlobalClass(pJniEnv, "java/lang/Object");
	//object_getClass = pJniEnv->GetMethodID(object_class, "getClass", "()Ljava/lang/Class;");
    //KASSERT(object_getClass != NULL);


#ifdef JPP_AUTO_CONVERT_CORE_CLASS
	jclass enum_class = jni_getGlobalClass(pJniEnv, "java/lang/Enum");
	fox_debug_printf("java.lang.Enum class %x", string_class);

	enum_ordinal = pJniEnv->GetMethodID(enum_class, "ordinal", "()I");
    KASSERT(enum_ordinal != NULL);
#endif

	string_class = jni_getGlobalClass(pJniEnv, "java/lang/String");
	fox_debug_printf("java.lang.String class %x", string_class);
	if (string_class == 0) {
		return;
	}

	fox_debug_printf("java.lang.String global class %x", string_class);
	string_value = pJniEnv->GetFieldID(string_class, "value", "[C");
	string_offset = pJniEnv->GetFieldID(string_class, "offset", "I");
	string_new = pJniEnv->GetMethodID(string_class, "<init>", "(II[C)V");
	fox_debug_printf("java.lang.String.new %x, value %x, offset %d", string_new, string_value, string_offset);

	inpStream_class = jni_getGlobalClass(pJniEnv, "fastiva/jni/io/InputStream_jni");
	fox_debug_printf("inpStream_class %x", inpStream_class);

	outStream_class = jni_getGlobalClass(pJniEnv, "fastiva/jni/io/OutputStream_jni");
	fox_debug_printf("outStream_class %x", outStream_class);

	ioExStream_class = jni_getGlobalClass(pJniEnv, "fastiva/jni/io/IOException_jni");
	fox_debug_printf("ioExStream_class %x", ioExStream_class);

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
	runnableStub_class = jni_getGlobalClass(pJniEnv, "fastiva/jni/lang/Runnable_jni");
	fox_debug_printf("runnableStub_class %x", runnableStub_class);
#endif

}

static FOX_BOOL s_jni_initilaized = false;

extern void fastiva_init_JavaProxies(JNIEnv* pEnv);

/*
 * Get a property by calling System.getProperty(key).
 *
 * Returns a newly-allocated string, or NULL on failure or key not found.
 * (Unexpected failures will also raise an exception.)
 */
static void initSystemProperty(JNIEnv* pEnv, const char* key)
{
    jclass system;
    jmethodID getProp;
    jstring keyObj = NULL;
    jstring valueObj;
    char* result = NULL;

    KASSERT(key != NULL);

    system = pEnv->FindClass("java/lang/System");
    KASSERT(system != NULL);

    getProp = pEnv->GetStaticMethodID(system, "getProperty",
        "(Ljava/lang/String;)Ljava/lang/String;");
    KASSERT(getProp != NULL);

    keyObj = pEnv->NewStringUTF(key);
    KASSERT(keyObj != NULL);

	fox_debug_printf("Sys property: %s = ", key);
    valueObj = (jstring)pEnv->CallStaticObjectMethod(system, getProp, keyObj);
    if (valueObj == NULL) {
		fox_debug_printf("NULL");
        return;
	}

	java_lang_String_p fmVal = toFastivaObject((java_lang_String_p)pEnv, valueObj, false);
	fastiva_debug_out(fmVal);
	java_lang_System_C$::importClass$()->setProperty(fastiva.createAsciiString(key), fmVal);
}


static void initDefaultLocale(JNIEnv* pEnv)
{
    jclass clsLocale;
    jmethodID getDefault, getCountry, getLanguage;
    jstring strCountry, strLanguage;
    jobject valueObj;
    char* result = NULL;

    clsLocale = pEnv->FindClass("java/util/Locale");
    KASSERT(clsLocale != NULL);

    getDefault = pEnv->GetStaticMethodID(clsLocale, "getDefault",
        "()Ljava/util/Locale;");
    KASSERT(getDefault != NULL);

    getCountry = pEnv->GetMethodID(clsLocale, "getCountry",
        "()Ljava/lang/String;");
    KASSERT(getCountry != NULL);

    getLanguage = pEnv->GetMethodID(clsLocale, "getLanguage",
        "()Ljava/lang/String;");
    KASSERT(getLanguage != NULL);

    valueObj = pEnv->CallStaticObjectMethod(clsLocale, getDefault);
    KASSERT(valueObj != NULL);


    strCountry = (jstring)pEnv->CallObjectMethod(valueObj, getCountry);
    KASSERT(strCountry != NULL);

    strLanguage = (jstring)pEnv->CallObjectMethod(valueObj, getLanguage);
    KASSERT(strLanguage != NULL);


	java_lang_String_p fmCountry = toFastivaObject((java_lang_String_p)pEnv, strCountry, false);
	fastiva_debug_out(fmCountry);

	java_lang_String_p fmLanguage = toFastivaObject((java_lang_String_p)pEnv, strLanguage, false);
	fastiva_debug_out(fmLanguage);

	java_lang_System_C$::importClass$()->setProperty(fastiva.createAsciiString("user.region"), fmCountry);
	java_lang_System_C$::importClass$()->setProperty(fastiva.createAsciiString("user.language"), fmLanguage);
}


static void initDefaultTimeZone(JNIEnv* pEnv)
{
    jclass clsTimeZone;
    jmethodID getDefault, getID;
    jstring strID;
    jobject valueObj;
    char* result = NULL;

    clsTimeZone = pEnv->FindClass("java/util/TimeZone");
    KASSERT(clsTimeZone != NULL);

    getDefault = pEnv->GetStaticMethodID(clsTimeZone, "getDefault",
        "()Ljava/util/TimeZone;");
    KASSERT(getDefault != NULL);

    getID = pEnv->GetMethodID(clsTimeZone, "getID",
        "()Ljava/lang/String;");
    KASSERT(getID != NULL);

    valueObj = pEnv->CallStaticObjectMethod(clsTimeZone, getDefault);
    KASSERT(valueObj != NULL);


    strID = (jstring)pEnv->CallObjectMethod(valueObj, getID);
    KASSERT(strID != NULL);

	java_lang_String_p fmID = toFastivaObject((java_lang_String_p)pEnv, strID, false);
	fastiva_debug_out(fmID);

	java_util_TimeZone_p pTZ = java_util_TimeZone_C$::importClass$()->getTimeZone(fmID);
    KASSERT(pTZ != NULL);
	java_util_TimeZone_C$::importClass$()->setDefault(pTZ);

}

extern void fastiva_jni_initFastivaStub(JNIEnv* pEnv, jclass fastivaStub_class);

void fastiva_jni_initStub(void* pJniEnv0, jobject classLoader) {
	if (s_jni_initilaized) {
		return;
	}

	JNIEnv* pEnv = (JNIEnv*) pJniEnv0;
	systemClassLoader = NULL;
	
	java_io_InputStream_C$::importClass$()->javaClass$ = inpStream_class;
	java_io_OutputStream_C$::importClass$()->javaClass$ = outStream_class;
	java_io_IOException_C$::importClass$()->javaClass$ = ioExStream_class;
	
	fastivaStub_class = jni_getGlobalClass(pEnv, "fastiva/jni/FastivaStub");

	fastiva_jni_initFastivaStub(pEnv, fastivaStub_class);
		
	fastivaStub_loadClass = pEnv->GetStaticMethodID(fastivaStub_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	KASSERT(fastivaStub_loadClass != NULL);
	fastivaStub_doJavaGC = pEnv->GetStaticMethodID(fastivaStub_class, "doJavaGC", "()V");
	KASSERT(fastivaStub_doJavaGC != NULL);

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
	fastivaStub_getNativeHandle = pEnv->GetStaticMethodID(fastivaStub_class, "getNativeHandle", "(Ljava/lang/Object;)I");
	KASSERT(fastivaStub_getNativeHandle != NULL);
	fastivaStub_createPrimitive = pEnv->GetStaticMethodID(fastivaStub_class, "createPrimitive", "(IJ)Ljava/lang/Object;");
	KASSERT(fastivaStub_createPrimitive != NULL);
	fastivaStub_toEnum = pEnv->GetStaticMethodID(fastivaStub_class, "toEnum", "(Ljava/lang/Class;I)Ljava/lang/Enum;");
	KASSERT(fastivaStub_loadClass != NULL);
#endif
#if JPP_ENABLE_J2N_SHARED_STRING
	fastivaUtil_class = pEnv->FindClass("com/tf/thinkdroid/common/util/FastivaUtil");
	fastivaUtil_getSharedFastivaString = pEnv->GetStaticMethodID(fastivaUtil_class, "getSharedFastivaString", "(Ljava/lang/String;)I");
	KASSERT(fastivaUtil_getSharedFastivaString != NULL);
#endif
	//initSystemProperty(pEnv, "user.language");
	//initSystemProperty(pEnv, "user.region");
	initDefaultLocale(pEnv);
	initDefaultTimeZone(pEnv);
    //class_FastivaRuntimeException = findDalvikSystemClass("fastiva/jni/FastivaRuntimeException");

	//nativeHandle = pEnv->GetFieldID(fastivaStub_class, "nativeHandle", "I");
	//proxy_init = pEnv->GetMethodID(fastivaStub_class, "<init>", "(I)V");
	//fox_debug_printf("FastivaStub.nativeHandle %x", nativeHandle);

	for (fastiva_JNIStubInfo* pStub = fastiva_JNIStubInfo::g_qStub; pStub != ADDR_ZERO; pStub = pStub->m_pNext) {
	 	const fastiva_ClassContext* pContext = pStub->m_pContext;

		jclass clazz = (jclass)pContext->m_pClass->javaClass$;
		if (clazz == NULL){
			char name[512];
			int len = fm::getClassNameUTF8(name, sizeof(name), pContext);
#ifdef _DEBUG
			if (strcmp(name, "com/tf/common/imageutil/DrawingRenderingCanceledException") == 0) {
				int a = 3;
			}
#endif
	 		clazz = findDalvikSystemClass(pEnv, name);
	 		if (clazz == NULL) {
	 			continue;
	 		}
			pContext->m_pClass->javaClass$ = fastiva_jni_NewGlobalRef$(pEnv, clazz);
			pEnv->DeleteLocalRef(clazz);
			clazz = (jclass)pContext->m_pClass->javaClass$;
		}
		else {
			/**
			  InputStream_jni, Runnable_jni, OutputStram_jni 등 미리 등록된 Shared-class 이다.
			*/
		}
		if (pStub->m_qMethod == NULL)  {
			continue;
		}
	 	int result = pEnv->RegisterNatives(clazz, pStub->m_qMethod, pStub->m_cntMethod);
	 	if (result < 0) {
	 		fox_debug_printf("!! Fail RegisterNatives %s/%s\n", pContext->getPackageName(), pContext->m_pBaseName);
	 	}
	 	else {
	 		fox_debug_printf("!! RegisterNatives %s/%s:%d\n", pContext->getPackageName(), pContext->m_pBaseName, pStub->m_cntMethod);
	 	}
	}

	FASTIVA_ENTER_NATIVE_SECTION()
	// 반드시 jni 함수 등록 후에 수행.
	fastiva_init_JavaProxies(pEnv);

	FASTIVA_LEAVE_NATIVE_SECTION()


	pEnv->ExceptionClear();
	//return;

}




void fastiva_Runtime::setJavaInstanceField(
	fastiva_Instance_p pSelf,
	fastiva_BytecodeProxy_p pNewValue,
	void * pField
) {
	JNIEnv* pEnv = fastiva_jni_getEnv();
	jobject newRef = pNewValue == NULL ? NULL : fastiva_jni_NewGlobalRef$(pEnv, (jobject)pNewValue);
	jobject oldRef = (jobject)fox_util_xchg32(pField, (int)newRef);
	if (pNewValue != NULL) {
		//pEnv->DeleteLocalRef((jobject)pNewValue);
	}

	if (oldRef != NULL) {
		KASSERT(oldRef != newRef);
		fastiva_jni_DeleteGlobalRef$(pEnv, oldRef);
	}
}

void	fastiva_jni_attachJavaTask() {
	void* env;
	//g_pJavaVM->AttachCurrentThread(&env, NULL);
	//KASSERT(env == pEnv);
}


void	fastiva_jni_detachJavaTask() {

	fastiva_Task* pTask = fastiva_getCurrentTask();
	fox_debug_printf("detach java thread %x", pTask);
	if (g_pJavaVM == NULL || pTask == NULL) {
		return;
	}

	fox_debug_printf("detach java thread called : %x", pTask->m_jniJavaThreadAttached);
	if (pTask->m_jniJavaThreadAttached) {
		g_pJavaVM->DetachCurrentThread();
	}
}

void fastiva_jni_DeleteJavaRefMethod(void* pEnv0, fastiva_BytecodeProxy_p* jjproxy) {
	fastiva_BytecodeProxy_p jproxy = *jjproxy;
	if (jproxy != NULL) {
		jproxy->releaseJavaProxy$(pEnv0);
	}
}

void fastiva_jni_releaseJavaRef(fastiva_Instance_p pObj) {
    // this function can be called gc-thred. So, do not use local jniEnv.
	JNIEnv* pEnv = fastiva_jni_getEnv();
	void* jref = (void*)fox_util_xchg32(&pObj->m_javaRef$, 0);
	if (IS_VALID_JAVA_REF(jref)) {
		pEnv->DeleteWeakGlobalRef((jobject)jref);
		fox_util_dec32(&cntGlobalRef);
	}
	pObj->scanJavaProxyFields$(pEnv, fastiva_jni_DeleteJavaRefMethod);
}
	
void fastiva_jni_CloneJavaRefMethod(void* pEnv0, fastiva_BytecodeProxy_p* jjproxy) {
	fastiva_BytecodeProxy_p jproxy = *jjproxy;
	if (jproxy != NULL) {
		*jjproxy = (fastiva_BytecodeProxy_p)fastiva_jni_NewGlobalRef$((JNIEnv*)pEnv0, (jobject)jproxy);
		KASSERT(*jjproxy != jproxy);
	}
}

void fastiva_jni_cloneJavaRef(fastiva_Instance_p pObj) {
    // this function can be called gc-thred. So, do not use local jniEnv.
	JNIEnv* pEnv = fastiva_jni_getEnv();
	KASSERT(pObj->m_javaRef$ == NULL);
	pObj->scanJavaProxyFields$(pEnv, fastiva_jni_CloneJavaRefMethod);
}

void fastiva_BytecodeProxy::releaseJavaProxy$(void* pEnv0) {
    // this function can be called gc-thred. So, do not use local jniEnv.
	if (IS_VALID_JAVA_REF(this)) {
		fastiva_jni_DeleteGlobalRef$((JNIEnv*)pEnv0, (jobject)this);
	}
}

void fastiva_jni_PushLocalFrame(int cntSlot) {
//	int res = fastiva_jniEnv->PushLocalFrame(256);
//	KASSERT(res == 0);
}
void fastiva_jni_PopLocalFrame() {
//	fastiva_jniEnv->PopLocalFrame(NULL);
}

bool s_enableJniTrace = false;
bool s_enableJniTracePrint = false;
const char* space_end = "                                       "
			+ sizeof("                                       ") - 1;

void JNI_DEBUG_TRACE::dumpIn(const char* func, const char* file, int line, bool J2N) {
	if (!s_enableJniTrace) {
		return;
	}
	this->file = file;
	this->line = line;
	fastiva_Task* pTask = fastiva_getCurrentTask();
	this->jniDepth = pTask->m_pStackContext->m_depth;
	this->pTask = pTask;
	this->func = func;
	if (s_enableJniTracePrint) {
		const char* tab = space_end - (short)jniDepth * 2;
		fox_printf("%s<< %d:[%x:%x]\t%s\n", tab, (short)jniDepth, ++pTask->m_cntJniCall, pTask, func);//, file, line);
	}
	pTask->m_pDebugTrace = 0;
}

void JNI_DEBUG_TRACE::dumpOut() {
	if (!s_enableJniTrace) {
		return;
	}
    const char* tab = space_end - (short)jniDepth * 2;
	/*
	if (((fastiva_Task*)pTask)->m_pDebugTrace != 0) {
		fox_printf("%s## %d:[%x] %s\n", tab, (short)jniDepth, pTask, func);//, file, line);
	}
	else if (s_enableJniTracePrint) {
		fox_printf("%s>> %d:[%x] %s\n", tab, (short)jniDepth, pTask, func);//, file, line);
	}
	*/
}

fastiva_jni_LocalFrame::fastiva_jni_LocalFrame(void* env0) {
	JNIEnv* pEnv = (JNIEnv*)env0;
	if (pEnv == NULL) {
		pEnv = fastiva_jni_getEnv();
	}
	this->m_res = NULL;
	this->m_pEnv = pEnv;
    int res = pEnv->PushLocalFrame(16);
	if(res != 0) {
		fox_printf("LocalFrame overflow 512");
		fox_exit(-1);
	}
	fastiva_Task* pCurrThread = fastiva_getCurrentTask();
	pushRewinder(pCurrThread);
}

fastiva_jni_LocalFrame::~fastiva_jni_LocalFrame() {
	JNIEnv* pEnv = (JNIEnv*)m_pEnv;
	if (pEnv != NULL) {
   		pEnv->PopLocalFrame((jobject)m_res);
	}
}

static bool check_ex = true;

jobject jni_popLocalFrame(JNIEnv* pEnv, fastiva_JniExceptionCheck* pExCheck, jobject res) {
	if (pEnv->ExceptionCheck()) {
		/** 2012.0512 Exception 占쌩삼옙 占쏙옙占승울옙占쏙옙占쏙옙 PopLocalFrame 占쏙옙 占쏙옙 占쏙옙 占쏙옙占쏙옙.
		*/
		check_ex = true;
		pExCheck->hasLocalFrame = true;
		fox_debug_printf("Java Exception occurred in Push/PopLocalFrame");
#ifdef _DEBUG
	fastiva_getCurrentTask()->dumpDebugStackTrace();
#endif
		return 0;
	}

	/*
	jobject res2 = pEnv->ExceptionOccurred();
	if (res2 != NULL) {
		fox_debug_printf("Java Exception occurred in Push/PopLocalFrame");
		pEnv->ExceptionClear();
		pEnv->PopLocalFrame(res2);
		pEnv->Throw((jthrowable)res2);
	}
	*/
	jobject res2 = pEnv->PopLocalFrame((jobject)res); 
	return res2;
}

int fastiva_jni_LocalFrame::setResult(void* res) {
	JNIEnv* pEnv = (JNIEnv*)m_pEnv;
	jobject res2 = pEnv->ExceptionOccurred();
	if (res2 != NULL) {
		res = res2;
		fox_debug_printf("Java Exception occurred");
	}
	res2 = pEnv->PopLocalFrame((jobject)res); 
	this->m_pEnv = 0;
	return (int)res2;
}

fastiva_JniExceptionCheck::~fastiva_JniExceptionCheck() { 
	JNIEnv* pEnv = this->m_pEnv;
	jthrowable javaException = pEnv->ExceptionOccurred();
	if (check_ex) {
		check_ex = 0;
		fox_debug_printf("Java Exception occurred %x : ", javaException);
	}

	if (javaException != ADDR_ZERO) {
		fox_debug_printf("fastiva_jni_throwFastivaException 0");
		// pEnv->ExceptionDescribe() 占쌉쇽옙 호占쏙옙占?FastivaStub.toStringNative() 占쌉쇽옙占쏙옙 호占쏙옙홱占?
		// 占쏙옙 占쏙옙, ManageedStack 占쏙옙 pair 占쏙옙 占쏙옙占쌩깍옙 占쏙옙占쏙옙 FASTIVA_ENTER_NATIVE_SECTION() 호占쏙옙.
		FASTIVA_ENTER_NATIVE_SECTION()	
		pEnv->ExceptionDescribe(); 
		// 占쏙옙占쏙옙) 2012.05.08 API 占쏙옙占쏙옙占쏙옙占쏙옙灌占?ExceptionDescribe() 호占쏙옙占?PendingException 占쏙옙 占쏙옙占쏙옙占쏙옙鳴占?占실억옙 占쏙옙占쏙옙占쏙옙
		// 占쏙옙占쏙옙占싸댐옙 占쌓뤄옙占쏙옙 占십댐옙. 占쌕몌옙 JNI 占쌉쇽옙 호占쏙옙 占쏙옙占쏙옙 占쌥듸옙占?ExceptionClear() 占쏙옙 호占쏙옙占쏙옙 占쌍억옙占?占싼댐옙.
		pEnv->ExceptionClear();
		FASTIVA_LEAVE_NATIVE_SECTION()
		fox_debug_printf("fastiva_jni_throwFastivaException 1");
		java_lang_Throwable_p ex = toFastivaObject((java_lang_Throwable_p)pEnv, (jobject)javaException);
		fox_debug_printf("fastiva_jni_throwFastivaException 2");
		if (this->hasLocalFrame) {
			pEnv->PopLocalFrame(0);
		}

		fastiva.throwException(ex);
	}
}


void fm::scanJavaProxyArray(Int_ap pArray, void* pEnv0, FASTIVA_JPROXY_SCAN_METHOD method) {
	fastiva_BytecodeProxy_p* pValue = (fastiva_BytecodeProxy_p*)pArray->getBuffer_unsafe$();
	JNIEnv* pEnv = (JNIEnv*)pEnv0;

	for (int i = pArray->length(); i -- > 0; ) {
		method(pEnv, pValue);
	}
}

java_lang_Object_p fm::cloneJavaProxyArray(Int_ap pArray) {
	JNIEnv* pEnv = fastiva_jni_getEnv();
	pArray = (Int_ap)pArray->clone();
	fastiva_BytecodeProxy_p* pValue = (fastiva_BytecodeProxy_p*)pArray->getBuffer_unsafe$();

	for (int i = pArray->length(); i -- > 0; ) {
		fastiva_jni_CloneJavaRefMethod(pEnv, pValue);
	}
	return pArray;
}

fastiva_BytecodeProxy_p fastiva_Instance::as__BytecodeObject$() {
	return (fastiva_BytecodeProxy_p) toNativeStub(fastiva_jni_getEnv(), NULL, (java_lang_Object_p)this);
}

#endif
