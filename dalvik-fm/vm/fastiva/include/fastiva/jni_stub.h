#ifndef __FASTIVA_JNI_STUB_H__
#define __FASTIVA_JNI_STUB_H__

#include <jni.h>
#include <fastiva/JppException.h>
#include <java/io/FileDescriptor.h>
#include <java/io/InputStream.h>
#include <java/io/OutputStream.h>
#include <java/lang/Runnable.h>
#include <java/lang/Enum.h>


#define FASTIVA_JNI_STUB(NAME, SIG, FN) { (char*)NAME, (char*)SIG, (void*)FN }


#ifdef _WIN32
	#pragma warning(disable : 4715) // not all control paths return a value
#endif

struct fastiva_JniExceptionCheck;
extern JNIEnv* fastiva_jni_getEnv();
extern jobject fastiva_jni_NewGlobalRef$(JNIEnv* pEnv, jobject obj);
extern jclass object_class;
extern jclass jni_getGlobalClass(JNIEnv* pEnv, const char* classPath, bool ignoreException = false);
extern jobject jni_popLocalFrame(JNIEnv* pEnv, fastiva_JniExceptionCheck* pchk, jobject res);

void fastiva_debug_out(java_lang_String_p pStr);

//#define fastiva_jniEnv		fastiva_jni_getEnv()

#define JPP_JNI_POP_LOCAL_FRAME(res)  jni_popLocalFrame(pEnv, &fastiva_JniExceptionCheck$$, res)

inline bool IS_VALID_JAVA_REF(void* obj) {
	return obj > (void*)0xFFFF;
}


class fastiva_JNIStubInfo {
public:
	fastiva_JNIStubInfo* m_pNext;
	const fastiva_ClassContext* m_pContext;
	const JNINativeMethod* m_qMethod;
	int m_cntMethod;


	fastiva_JNIStubInfo(
		const fastiva_ClassContext* pContext, 
		const JNINativeMethod* qMethod, 
		int cntMethod
	) {
		this->m_pContext = pContext;
		this->m_qMethod = qMethod;
		this->m_cntMethod = cntMethod;
		this->m_pNext = g_qStub;
		g_qStub = this;
	}

	static fastiva_JNIStubInfo* g_qStub;
};

class fastiva_ProxyRegister {
public:
	void (*m_initProxy)(void*);
	fastiva_ProxyRegister* m_pNext;

	fastiva_ProxyRegister(const void *fn) {
		m_initProxy = (void (*)(void*))fn;
		this->m_pNext = g_qProxyRegister;
		g_qProxyRegister = this;
	}
	static fastiva_ProxyRegister* g_qProxyRegister;
};

#define FASTIVA_REGISTER_PROXY(CLASS)															\
	extern fastiva_ProxyRegister FASTIVA_MERGE_TOKEN(CLASS, _info);								\
	fastiva_ProxyRegister FASTIVA_MERGE_TOKEN(CLASS, _info)((void*)FASTIVA_MERGE_TOKEN(CLASS, _C$)::initStatic$);	\
	JNIEXPORT void* FASTIVA_MERGE_TOKEN(get_JNI_REGISTER_, CLASS)() { return &FASTIVA_MERGE_TOKEN(CLASS, _info); }	\


struct JNI_DEBUG_TRACE {
	fox_Task* pTask;
	int jniDepth;
	const char* func;
	const char* file; int line;
	JNI_DEBUG_TRACE(const char* func, const char* file, int line, bool J2N) {
#if defined(_DEBUG) || defined(_DEBUG_TRACE)
		dumpIn(func, file, line, J2N);
#endif
	}

	~JNI_DEBUG_TRACE() {
#if defined(_DEBUG) || defined(_DEBUG_TRACE)
		dumpOut();
#endif
	}

	void dumpIn(const char* func, const char* file, int line, bool J2N);
	void dumpOut();
};

struct fastiva_JniExceptionCheck {
	JNIEnv* m_pEnv;
	bool hasLocalFrame;
	fastiva_JniExceptionCheck(JNIEnv* pEnv) { m_pEnv = pEnv; hasLocalFrame = false; }
	~fastiva_JniExceptionCheck();
};

#define FASTIVA_REGISTER_NATIVE_METHODS(CLASS)								\
	extern fastiva_JNIStubInfo s_JNI_REGISTER_##CLASS;					\
	JNIEXPORT void* get_JNI_REGISTER_##CLASS() { return &s_JNI_REGISTER_##CLASS; } \
	fastiva_JNIStubInfo s_JNI_REGISTER_##CLASS(CLASS::getRawContext$(),	\
		gMethods, FASTIVA_ARRAY_ITEM_COUNT(gMethods))


#define JPP_BEGIN_JAVA_CALL(CLASS)									\
	JNI_DEBUG_TRACE _jni_debug_trace_(__FUNCTION__, __FILE__, __LINE__, false);	\
	JNIEnv* pEnv = fastiva_jni_getEnv();							\
	JPP_JNI_PUSH_LOCAL_FRAME(pEnv);						\
	FASTIVA_ENTER_NATIVE_SECTION()	

#define JPP_BEGIN_JAVA_CALL_EX(CLASS, useLocalFrame)								\
	JNI_DEBUG_TRACE _jni_debug_trace_(__FUNCTION__, __FILE__, __LINE__, false);	\
	JNIEnv* pEnv = fastiva_jni_getEnv();							\
	if (useLocalFrame && pEnv->PushLocalFrame(16) != 0) { fox_printf("PushLocalFrame fail"); fox_exit(-1); }					\
	FASTIVA_ENTER_NATIVE_SECTION()									\
	fastiva_JniExceptionCheck fastiva_JniExceptionCheck$$(pEnv);


#define JPP_END_JAVA_CALL()											\
	FASTIVA_LEAVE_NATIVE_SECTION()									


#define FASTIVA_BEGIN_JNI_SECTION(pJniEnv)									\
	FASTIVA_BEGIN_MANAGED_SECTION(pJniEnv)								\
	JNIEnv* volatile pEnv = env0;													\
	JNI_DEBUG_TRACE _jni_debug_trace_(__FUNCTION__, __FILE__, __LINE__, true);		\
	TRY$ {																\


#define FASTIVA_END_JNI_SECTION()												\
	} CATCH_ANY$ {															\
		fastiva_jni_throwJavaException(pEnv, catched_ex$);						\
	}																		\
	FASTIVA_END_MANAGED_SECTION()


#define JPP_JNI_VALIDATE_JAVACALL_RESULT(res)							\
	if (pEnv->ExceptionCheck()) { res = 0; }


fastiva_ArrayHeader* fastiva_allocArrayInstance(
	uint item_siz, java_lang_Class* pClass, int length, void* handle, void* data);

void fastiva_jni_exchangeBuffer(fastiva_ArrayHeader* pArray, void* javaArray, void* data);

void fastiva_jni_releaseJavaArray(fastiva_ArrayHeader* pArray);

//void fastiva_jni_throwFastivaException(JNIEnv* pEnv, jthrowable ex);

void fastiva_jni_throwJavaException(JNIEnv* pEnv, java_lang_Throwable_p);

// 2011.1011 // �׻� ptr_cast$�� �ϸ� �ӵ��� ��������.
#define JPP_STUB_TO_FASTIVA_OBJECT(var, type, arg)				\
	type##_p var = type::ptr_cast$(nativeStubToFastivaObject(pEnv, type::getRawContext$(), arg)) // (type##_p)
#define JPP_STUB_TO_FASTIVA_OBJECT_ARG		JPP_STUB_TO_FASTIVA_OBJECT


#define JPP_STUB_TO_FASTIVA_INTERFACE(var, type, arg)				\
	type##_p var = type::ptr_cast$(nativeStubToFastivaObject(pEnv, type::getRawContext$(), arg))
#define JPP_STUB_TO_FASTIVA_INTERFACE_ARG	JPP_STUB_TO_FASTIVA_INTERFACE


#define JPP_STUB_TO_FASTIVA_ARRAY(var, type, arg)				\
	type##_ap var = (type##_ap)nativeStubToFastivaArray(pEnv, type::getRawContext$(), arg)
#define JPP_STUB_TO_FASTIVA_ARRAY_ARG		JPP_STUB_TO_FASTIVA_ARRAY


#define JPP_JOBJECT_TO_FASTIVA_OBJECT(var, type, arg)				\
	type##_p var = (type##_p)toFastivaObject((type##_p)pEnv, arg)
#define JPP_JOBJECT_TO_FASTIVA_OBJECT_ARG	JPP_JOBJECT_TO_FASTIVA_OBJECT


#define JPP_JOBJECT_TO_FASTIVA_INTERFACE(var, type, arg)				\
	&& not supported &&
#define JPP_JOBJECT_TO_FASTIVA_INTERFACE_ARG	JPP_JOBJECT_TO_FASTIVA_INTERFACE


#define JPP_JOBJECT_TO_FASTIVA_ARRAY(var, type, arg)				\
	type##_ap var = (type##_ap)toFastivaArray((type##_p)pEnv, type::getJavaClass(), arg)
#define JPP_JOBJECT_TO_FASTIVA_ARRAY_ARG		JPP_JOBJECT_TO_FASTIVA_ARRAY


#define JPP_JOBJECT_TO_FASTIVA_PRIMIVTIVES(var, type, arg)				\
	type##_p var = (type##_p)toFastivaObject((type##_p)pEnv, arg, false)
#define JPP_JOBJECT_TO_FASTIVA_PRIMIVTIVES_ARG(var, type, arg)				\
	type##_p var = (type##_p)toFastivaObject((type##_p)pEnv, arg, true)


#define JPP_FASTIVA_TO_STUB_OBJECT(var, type, arg)				\
	jobject var = toNativeStub(pEnv, type::getRawContext$(), arg)
#define JPP_FASTIVA_TO_STUB_OBJECT_ARG		JPP_FASTIVA_TO_STUB_OBJECT


#define JPP_FASTIVA_TO_STUB_ARRAY(var, type, arg)				\
	jobject var = toNativeStubArray(pEnv, type::getRawContext$(), arg)
#define JPP_FASTIVA_TO_STUB_ARRAY_ARG		JPP_FASTIVA_TO_STUB_ARRAY

#define JPP_FASTIVA_TO_JOBJECT_OBJECT(var, type, arg)				\
	jobject var = toJavaObject(pEnv, arg)
#define JPP_FASTIVA_TO_JOBJECT_OBJECT_ARG	JPP_FASTIVA_TO_JOBJECT_OBJECT

#define JPP_FASTIVA_TO_JOBJECT_ARRAY(var, type, arg)				\
	jobject var = toJavaArray((fastiva_BytecodeProxy_p)pEnv, type::getJavaClass(), arg)
#define JPP_FASTIVA_TO_JOBJECT_ARRAY_ARG	JPP_FASTIVA_TO_JOBJECT_ARRAY


#define JPP_FASTIVA_TO_JOBJECT_PRIMIVTIVES(var, type, arg)				\
	jobject var = toJavaObject(pEnv, arg, false)
#define JPP_FASTIVA_TO_JOBJECT_PRIMIVTIVES_ARG(var, type, arg)				\
	jobject var = toJavaObject(pEnv, arg, true)



Byte_ap toFastivaObject(Byte_ap, jobject obj, bool isArg);

Unicod_ap toFastivaObject(Unicod_ap, jobject obj, bool isArg);

Short_ap toFastivaObject(Short_ap, jobject obj, bool isArg);

Int_ap toFastivaObject(Int_ap, jobject obj, bool isArg);

Longlong_ap toFastivaObject(Longlong_ap, jobject obj, bool isArg);

Float_ap toFastivaObject(Float_ap, jobject obj, bool isArg);

Double_ap toFastivaObject(Double_ap, jobject obj, bool isArg);

java_lang_Throwable_p toFastivaObject(java_lang_Throwable_p, jobject obj);

java_lang_String_p toFastivaObject(java_lang_String_p, jobject obj, bool isArg);

java_io_FileDescriptor_p toFastivaObject(java_io_FileDescriptor_p, jobject obj, bool isArg);

java_lang_String_ap toFastivaObject(java_lang_String_ap, jobject value, bool isArg);

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
java_lang_Enum_p toFastivaObject(java_lang_Enum_p, jobject obj);

java_lang_Object_p toFastivaObject(java_lang_Object_p, jobject obj);
#endif

inline 
fastiva_BytecodeProxy_p toFastivaObject(fastiva_BytecodeProxy_p, jobject obj) {
	return (fastiva_BytecodeProxy_p)obj;
}

fastiva_ArrayHeader* toFastivaArray(fastiva_BytecodeProxy_p, void* jclass, jobject);

java_lang_Object_p nativeStubToFastivaObject(JNIEnv* pEnv, const fastiva_ClassContext* pContext, jobject proxy);

java_lang_Object_ap nativeStubToFastivaArray(JNIEnv* pEnv, const fastiva_ClassContext* pContext, jobject proxy);





jbyteArray toJavaObject(JNIEnv* pEnv, Byte_ap array, bool isArg);

jcharArray toJavaObject(JNIEnv* pEnv, Unicod_ap array, bool isArg);

jshortArray toJavaObject(JNIEnv* pEnv, Short_ap array, bool isArg);

jintArray toJavaObject(JNIEnv* pEnv, Int_ap array, bool isArg);

jlongArray toJavaObject(JNIEnv* pEnv, Longlong_ap array, bool isArg);

jfloatArray toJavaObject(JNIEnv* pEnv, Float_ap array, bool isArg);

jdoubleArray toJavaObject(JNIEnv* pEnv, Double_ap array, bool isArg);

jobject toJavaObject(JNIEnv* pEnv, java_lang_Throwable_p);

jstring toJavaObject(JNIEnv* pEnv, java_lang_String_p pStr, bool isArg);

jobject toJavaObject(JNIEnv* pEnv, java_io_FileDescriptor_p pStr, bool isArg);

jobjectArray toJavaObject(JNIEnv* pEnv, java_lang_String_ap array, bool isArg);

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
jobject toJavaObject(JNIEnv* pEnv, java_io_InputStream_p inp);

jobject toJavaObject(JNIEnv* pEnv, java_io_OutputStream_p out);

jobject toJavaObject(JNIEnv* pEnv, java_lang_Runnable_p v);

jobject toJavaObject(JNIEnv* pEnv, java_lang_Enum_p obj);

jobject toJavaObject(JNIEnv* pEnv, java_lang_Object_p obj);
#endif

inline 
jobject toJavaObject(JNIEnv* pEnv, fastiva_BytecodeProxy_p obj) {
	return (jobject)obj;
}

jobjectArray toJavaArray(fastiva_BytecodeProxy_p, void* jclass, fastiva_ArrayHeader* fmArray);

jobject toNativeStub(JNIEnv* pEnv, const fastiva_ClassContext* pRetTypeContext, java_lang_Object_p fmObj);

jobjectArray toNativeStubArray(JNIEnv* pEnv, const fastiva_ClassContext* pRetTypeContext, fastiva_ArrayHeader* fmArray);

//jobjectArray toJavaObject(fastiva_ArrayHeader* jproxyArray);





#endif // __FASTIVA_JNI_STUB_H__

