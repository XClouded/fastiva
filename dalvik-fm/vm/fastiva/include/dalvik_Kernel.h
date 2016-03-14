#ifndef __FASTIVA_KERNEL_H__
#define __FASTIVA_KERNEL_H__

#include <Dalvik.h>
#include <kernel/Module.h>
#include <string.h>
#include <fastiva/JppException.h>

#include <java/lang/ClassLoader$SystemClassLoader.h>
#include <java/util/ArrayList.h>
#include <java/util/Hashtable.h>

#undef FASTIVA_NO_DALVIK_INTERNAL_NATIVE

#define FASTIVA_SET_NATIVE_STACK_POINTER(thread, sp)	\
	if (!FASTIVA_SCAN_INTO_JNI_STACK) { thread->m_pNativeStackPointer = (sp); }

static const uint ACC_FASTIVA_DIRECT				= ACC_FASTIVA_METHOD;
static const uint ACC_FASTIVA_FNI					= 0x08000000;
static const uint ACC_FASTIVA_CPP_NO_FLOAT_ARGS		= 0x04000000;
static const uint ACC_FASTIVA_CPP_NO_JNI_STACK_SHIFT    = 0x02000000;
static const int OVERRIDING_VIRTUAL_PROXY_COUNT = 256*4;

extern bool g_fastiva_enable_call_log;
extern bool g_fastiva_enable_jni_call_log;


extern void d2f_initStatic(Thread*, ClassObject* clazz);
extern bool fastiva_isSystemClass(fastiva_Class_p pClass);

extern bool fastiva_dvmIsValidDataObject(const Object* obj);
extern bool d2f_isFastivaClassObject(const Object*);
extern int  d2f_isFastivaClass(const ClassObject* clazz);
extern int  d2f_isFastivaMethod(const Method* method);

extern void fastiva_addReference(Object *obj);

extern void d2f_registerInternalNativeMethods(DalvikNativeClass* pClass);
extern int d2f_computeInterpreterArgInfo(Method* meth);
extern bool d2f_inFastivaFNI(Thread* self);
extern void d2f_executeJppMethod(Object* obj, const Method* method, JValue* pResult, va_list arg_list);

extern void d2f_initObject(Object* newObj, ClassObject* clazz);
extern ArrayObject* d2f_getClassAnnotations(const ClassObject* clazz);
extern Object* d2f_getClassAnnotation(const ClassObject* clazz, const ClassObject* annotationClazz);
extern ArrayObject* d2f_getMethodAnnotations(const Method* method);
extern ArrayObject* d2f_getParameterAnnotations(const Method* method);
extern Object* d2f_getMethodAnnotation(const Method* method, const ClassObject* annotationClazz);
extern ArrayObject* d2f_getFieldAnnotations(const Field* field);
extern Object* d2f_getFieldAnnotation(const Field* field, const ClassObject* annotationClazz);
extern Object* d2f_getDefaultAnnotationValue(const Method* method, const ClassObject* annotationClazz);
extern ArrayObject* d2f_getMethodSignatureAnnotation(const Method* method);
extern ArrayObject* d2f_getFieldSignatureAnnotation(const Field* field);
extern ArrayObject* d2f_getClassSignatureAnnotation(const ClassObject* clazz);
extern ArrayObject* d2f_getDeclaredClasses(const ClassObject* clazz);
extern void fastiva_popDalvikException();
extern "C" jlonglong fastiva_popDalvikException_Ex(jlonglong v);
extern void fastiva_popDalvikException(Thread* self);

extern ClassObject* d2f_getEnclosingClass(const ClassObject* clazz);
extern ClassObject* d2f_getDeclaringClass(const ClassObject* clazz);
extern Object* d2f_getEnclosingMethod(const ClassObject* clazz);
extern StringObject* d2f_getInnerClassName(const ClassObject* clazz);

extern void fastiva_dvmPopFrame(Thread* self);
extern bool fastiva_dvmPushInterpFrame(Thread* self, const Method* method);
extern "C" {
	void* fastiva_lockStack(Thread* currThread, jmp_buf* fastiva_buf$);
	Thread* fastiva_lockCurrentStack(jmp_buf* fastiva_buf$);
	void fastiva_releaseStack(void* currThread, void* old_sp);
	void FOX_NO_RETURN fastiva_dvmCatchFastivaException(Thread* pCurrTask);
};


struct fastiva_StackLock {
	Thread* self;
	void* old_sp;
#ifdef _DEBUG
        ThreadStatus oldStatus;
#endif
	fastiva_StackLock(Thread* self) {
		this->self = self;
#ifdef _DEBUG
                oldStatus = self->status;
		if (!FASTIVA_SCAN_INTO_JNI_STACK && self->status == THREAD_RUNNING) {
			assert(self->m_pNativeStackPointer == 0);
		}
#endif
	}
	~fastiva_StackLock() {
#ifdef _DEBUG
                assert(oldStatus == self->status);
#endif
                self->m_pNativeStackPointer = old_sp;
	}
};

#define FASTIVA_IS_STACK_CLOSED(thread) (thread->m_pNativeStackPointer != NULL)

#define FASTIVA_IS_PRECOMPILED(fileName)  (strstr(gDvm.bootClassPathStr, fileName) != 0)
// for full_precompile 					 (strncmp(fileName, "/system/framework/", 18) == 0) 

#if FASTIVA_SCAN_INTO_JNI_STACK // FASTIVA_SUSPEND_BY_SIGNAL
#define FASTIVA_SUSPEND_STACK(self) // IGNORE

#define FASTIVA_RESUME_STACK(self)  // IGNORE

#else
#define FASTIVA_SUSPEND_STACK(self)											\
	FASTIVA_SUSPEND_STACK_unsafe(self)										\

#define FASTIVA_RESUME_STACK(self)											\
	FASTIVA_RESUME_STACK_unsafe(self)

#endif

#define FASTIVA_SUSPEND_STACK_safe(self) \
	FASTIVA_SUSPEND_STACK_unsafe(self);	\

#define FASTIVA_SUSPEND_STACK_unsafe(self) {	\
	jmp_buf* fastiva_buf$ = (jmp_buf*)alloca(sizeof(jmp_buf));			\
		fastiva_StackLock fastiva_StackLock_$$(self);						\
		fastiva_StackLock_$$.old_sp = fastiva_lockStack(self, fastiva_buf$);\

#define oooooo \
	void* old_native_sp$ = self->m_pNativeStackPointer; \
	if (old_native_sp$ == NULL) { \
		fastiva_lockStack(self, fastiva_buf$); \
	}

#define FASTIVA_RESUME_STACK_safe(self) \
	FASTIVA_RESUME_STACK_unsafe(self);

#define FASTIVA_RESUME_STACK_unsafe(self)									\
	                assert(fastiva_StackLock_$$.oldStatus == self->status); \
	} 

#define oooooooooooooooooooooooooo2 \
self->m_pNativeStackPointer = old_native_sp$; \
	assert (self->status != THREAD_RUNNING || self->m_pNativeStackPointer == 0); \


class Kernel {
public:
	enum { MAX_PACKAGE_COUNT = 512 };

	static const fastiva_ClassContext* primitiveContexts[fastiva_Primitives::cntType];
	static fastiva_PrimitiveClass_p* primitiveClasses;
	//static java_lang_Thread_p	g_pGCThread;
	//static jbool g_isSystemRunning;
	java_util_Hashtable_p g_pInternedString;

	java_lang_Object g_pPackageListLock[1];
	java_util_ArrayList_p g_pAnnotationsList;

	fastiva_Module g_interpreterModule;
	fastiva_Module* g_appModule;

	const char** g_typeSigs;

	int g_cntPackage;
	fastiva_PackageSlot* g_aPackageSlot;
	int* g_ivtableAllocTop;
	int* g_ivtableAllocBase;
	Object* g_classLow;
	Object* g_classHigh;
	volatile int cntPendingScanThread;
	volatile int cntUnfinishedScanThread;
	volatile int classTableItemIndex;
	volatile int gcGeneration;
	int vmInitialized;

	pthread_mutex_t initClassLock;

	pthread_cond_t	allThreadScanStartedCond;
	pthread_cond_t  gcFinshinedCond;
	pthread_cond_t  allThreadScanFinishedCond;

	static void beginManagedSection_$$(
		struct fastiva_ManagedSection*
	);

	static void endManagedSection_$$(
		struct fastiva_ManagedSection*
	);
};

extern Kernel kernelData;

namespace fm {
	bool isLinked$(fastiva_MetaClass*);

	java_lang_Class_p loadClass(unicod* pName, int ucs_len, java_lang_ClassLoader_p pLoader);

	java_lang_Class_p getRawClass(const char* sig, java_lang_ClassLoader_p pLoader = NULL);

	java_lang_String_p createStringConstant(const unicod* str);


	fastiva_Class_p initRawClass(const ClassObject* pClass);

	inline fastiva_Class_p getRawClass(const ClassObject* pClass) {
#ifdef FASTIVA_PRELOAD_STATIC_INSTANCE
		assert(pClass->status >= CLASS_VERIFIED || pClass->status == CLASS_ERROR);
		return (fastiva_Class_p)pClass;
#else
		return && obsolete && initRawClass(pClass); 
#endif
	}


	/*java_lang_Object_p allocateEx(
		const fastiva_InstanceContext*,
		int // sizInstance
	)*/

	void initObject(
		fastiva_Class_p,
		fastiva_Instance_p
	);


	java_lang_String_p internString(java_lang_String_p pRookie);

	java_lang_Class_p getPrimitiveClassBySig(unicod signature);

	// return end_of_utf_string;
	char* getUTFChars(
		char*, // buff,
		const unicod*, 
		int // ucs_len
	);

	java_lang_String_p createString(
		const char*
	);

	java_lang_String_p createStringA(
		const char*, 
		int // str_len
	);

	java_lang_String_p createStringW(
		const unicod*, 
		int // ucs_len
	);

	java_lang_String_p createUTFString(
		const char*
	);

	int getUnicodeLengthOfUTF8(
		const char*,
		int // str_len
	);

	java_lang_String_p createUTFStringA(
		const char*, 
		int // str_len
	);

	int getUTFLength(
		const unicod*, 
		int // ucs_len
	);

	java_lang_String_p createAsciiString(
		const char*
	);

	java_lang_String_p createAsciiStringA(
		const char*, 
		int // str_len
	);
};

#define FASTIVA_PRIMITIVE_CLASS_OF(T)							\
	(Kernel::primitiveClasses[fastiva_Primitives::T])

#define FASTIVA_PRIMITIVE_CONTEXT_OF(T)									\
	(Kernel::primitiveContexts[fastiva_Primitives::T])


#endif