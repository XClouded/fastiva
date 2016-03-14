#ifndef __FOX_JNI_H
#define __FOX_JNI_H



/**
* JNI function들을 모아놓았다.
* 함수이름은 검색등의 편의를 위해서 모두 JNI헤더에 있는 함수들과 동일한
* 이름을 사용하였다.
* 나중에 이 스트럭처를 그대로 ENV에 대입하도록 하자.
* 이들을 스턱처로 넣은 이유는? 시스템 클래스의 프랜드로 하기위해서이다.
*/


class fox_JNI {
public:

    //NULL,//void *reserved0;
    //NULL,//void *reserved1;
    //NULL,//void *reserved2;

    //NULL,//void *reserved3;

    static jint JNICALL GetVersion(JNIEnv *);
    static jclass JNICALL DefineClass(JNIEnv *, const char *name, jobject loader, const jbyte *buf, jsize len);
	static jclass JNICALL FindClass(JNIEnv *, const char *name);

    static jmethodID JNICALL FromReflectedMethod(JNIEnv *, jobject method);
    static jfieldID JNICALL FromReflectedField(JNIEnv *, jobject field);
    static jobject JNICALL ToReflectedMethod(JNIEnv *, jclass cls, jmethodID methodID, jboolean isStatic);
	static jclass JNICALL GetSuperclass(JNIEnv *, jclass sub);

    static jboolean JNICALL IsAssignableFrom(JNIEnv *, jclass sub, jclass sup);
    static jobject JNICALL ToReflectedField(JNIEnv *, jclass cls, jfieldID fieldID, jboolean isStatic);

    static jint JNICALL Throw(JNIEnv *, jthrowable obj);
    static jint JNICALL ThrowNew(JNIEnv *, jclass clazz, const char *msg);
    static jthrowable JNICALL ExceptionOccurred(JNIEnv *);
    static void JNICALL ExceptionDescribe(JNIEnv *);
    static void JNICALL ExceptionClear(JNIEnv *);
    static void JNICALL FatalError(JNIEnv *, const char *msg);

    static jint JNICALL PushLocalFrame(JNIEnv *, jint capacity);
    static jobject JNICALL PopLocalFrame(JNIEnv *, jobject result);

    static jobject JNICALL NewGlobalRef(JNIEnv *, jobject lobj);
    static void JNICALL DeleteGlobalRef(JNIEnv *, jobject gref);
    static void JNICALL DeleteLocalRef(JNIEnv *, jobject obj);
    static jboolean JNICALL IsSameObject(JNIEnv *, jobject obj1, jobject obj2);
    static jobject JNICALL NewLocalRef(JNIEnv *, jobject ref);
    static jint JNICALL EnsureLocalCapacity(JNIEnv *, jint capacity);

	static jobject JNICALL AllocObject(JNIEnv *, jclass clazz);
    static jobject JNICALL NewObject(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jobject JNICALL NewObjectV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jobject JNICALL NewObjectA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);

    static jclass JNICALL GetObjectClass(JNIEnv *, jobject obj);
    static jboolean JNICALL IsInstanceOf(JNIEnv *, jobject obj, jclass clazz);

    static jmethodID JNICALL GetMethodID(JNIEnv *, jclass clazz, const char *name, const char *sig);
    static jmethodID JNICALL GetStaticMethodID(JNIEnv *, jclass clazz, const char *name, const char *sig);

    static jlong JNICALL CallMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jlong JNICALL CallMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jlong JNICALL CallMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue * args);

    static jlong JNICALL CallNonvirtualMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jlong JNICALL CallNonvirtualMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jlong JNICALL CallNonvirtualMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue * args);

    static jlong JNICALL CallStaticMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jlong JNICALL CallStaticMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jlong JNICALL CallStaticMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);

    static jfieldID JNICALL GetFieldID(JNIEnv *, jclass clazz, const char *name, const char *sig);
    static jfieldID JNICALL GetStaticFieldID(JNIEnv *, jclass clazz, const char *name, const char *sig);

    static jlong JNICALL GetField(JNIEnv *, jobject obj, jfieldID fieldID);
    static void JNICALL SetField(JNIEnv *, jobject obj, jfieldID fieldID, jint val);
    static void JNICALL SetLongField(JNIEnv *, jobject obj, jfieldID fieldID, jlong val);

    //static jlong JNICALL GetStaticField(JNIEnv *, jclass clazz, jfieldID fieldID);
    //static void JNICALL SetStaticField(JNIEnv *, jclass clazz, jfieldID fieldID, jobject value);
  
    
    
    static jstring JNICALL NewString(JNIEnv *, const jchar *unicode, jsize len);
    static jsize JNICALL GetStringLength(JNIEnv *, jstring str);
    static const jchar *JNICALL GetStringChars(JNIEnv *, jstring str, jboolean *isCopy);
    static void JNICALL ReleaseStringChars(JNIEnv *, jstring str, const jchar *chars);
    
    static jstring JNICALL NewStringUTF(JNIEnv *, const char *utf);
    static jsize JNICALL GetStringUTFLength(JNIEnv *, jstring str);
    static const char* JNICALL GetStringUTFChars(JNIEnv *, jstring str, jboolean *isCopy);
    static void JNICALL ReleaseStringUTFChars(JNIEnv *, jstring str, const char* chars);
    
    static jsize JNICALL GetArrayLength(JNIEnv *, jarray array);

    static jobjectArray JNICALL NewObjectArray(JNIEnv *, jsize len, jclass clazz, jobject init);
	static jarray NewPrimitiveArray(fastiva_PrimitiveClass_p primitiveType, int len);

    static jobject JNICALL GetObjectArrayElement(JNIEnv *, jobjectArray array, jsize index);
    static void JNICALL SetObjectArrayElement(JNIEnv *, jobjectArray array, jsize index, jobject val);

    static jbooleanArray JNICALL NewBooleanArray(JNIEnv *, jsize len);
    static jbyteArray JNICALL NewByteArray(JNIEnv *, jsize len);
    static jcharArray JNICALL NewCharArray(JNIEnv *, jsize len);
    static jshortArray JNICALL NewShortArray(JNIEnv *, jsize len);
    static jintArray JNICALL NewIntArray(JNIEnv *, jsize len);
    static jlongArray JNICALL NewLongArray(JNIEnv *, jsize len);
    static jfloatArray JNICALL NewFloatArray(JNIEnv *, jsize len);
    static jdoubleArray JNICALL NewDoubleArray(JNIEnv *, jsize len);

    static void* JNICALL GetArrayElements(JNIEnv *, jarray array, jboolean *isCopy);
    static void JNICALL ReleaseArrayElements(JNIEnv *, jarray array, void *elems, jint mode);
    static void JNICALL GetArrayRegion(JNIEnv *, jarray array, jsize start, jsize l, void *buf);
    static void JNICALL SetArrayRegion(JNIEnv *, jarray array, jsize start, jsize l, void *buf);


	static jint JNICALL RegisterNatives(JNIEnv *, jclass clazz, const JNINativeMethod *methods,       jint nMethods);
    static jint JNICALL UnregisterNatives(JNIEnv *, jclass clazz);
    
	static jint JNICALL MonitorEnter(JNIEnv *, jobject obj);
    static jint JNICALL MonitorExit(JNIEnv *, jobject obj);
    
    static jint JNICALL GetJavaVM(JNIEnv *, JavaVM **vm);
    
    static void JNICALL GetStringRegion(JNIEnv *, jstring str, jsize start, jsize len, jchar *buf);
    static void JNICALL GetStringUTFRegion(JNIEnv *, jstring str, jsize start, jsize len, char *buf);
    
    static void * JNICALL GetPrimitiveArrayCritical(JNIEnv *, jarray array, jboolean *isCopy);
    static void JNICALL ReleasePrimitiveArrayCritical(JNIEnv *, jarray array, void *carray, jint mode);
    	
	static const jchar * JNICALL GetStringCritical(JNIEnv *, jstring string, jboolean *isCopy);
    static void JNICALL ReleaseStringCritical(JNIEnv *, jstring string, const jchar *cstring);
    
    static jweak JNICALL NewWeakGlobalRef(JNIEnv *, jobject obj);
    static void JNICALL DeleteWeakGlobalRef(JNIEnv *, jweak ref);
    
    static jboolean JNICALL ExceptionCheck(JNIEnv *);

    static jobject JNICALL NewDirectByteBuffer(JNIEnv* env, void* address, jlong capacity);
    static void* JNICALL GetDirectBufferAddress(JNIEnv* env, jobject buf);
    static jlong JNICALL GetDirectBufferCapacity(JNIEnv* env, jobject buf);


	// for array locking.
	//static void* lockArray(fastiva_ArrayHeader* obj, int itemSize);
	//static void unlockArray(void* lockedP);

	// for GetStaticFieldID, GetFieldID
	static jfieldID JNICALL getFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig, int nonStatic);

	// for GetStaticMethodID, GetMethodID
	static jmethodID JNICALL GetMethodID(jclass clazz, const char *name, const char *sig, jboolean isStatic);
	
	
	/*
	Method Call을 위해서 정의 한다.
	Parameter를 통일 시키기 위해서 사용되는 function이다.
	만약 parameter가 jlong형이면 jint Type으로 aParam에 두개로 나누어서 들어간다.
	*/
	static jlong InvokeMethod_0(void *fn, jint *aParams);
	static jlong InvokeMethod_1(void *fn, jint *aParams);
	static jlong InvokeMethod_2(void *fn, jint *aParams);
	static jlong InvokeMethod_3(void *fn, jint *aParams);
	static jlong InvokeMethod_4(void *fn, jint *aParams);
	static jlong InvokeMethod_5(void *fn, jint *aParams);
	static jlong InvokeMethod_6(void *fn, jint *aParams);
	static jlong InvokeMethod_7(void *fn, jint *aParams);
	static jlong InvokeMethod_8(void *fn, jint *aParams);
	static jlong InvokeMethod_9(void *fn, jint *aParams);
	static jlong InvokeMethod_10(void *fn, jint *aParams);
	static jlong InvokeMethod_11(void *fn, jint *aParams);
	static jlong InvokeMethod_12(void *fn, jint *aParams);
	static jlong InvokeMethod_13(void *fn, jint *aParams);
	static jlong InvokeMethod_14(void *fn, jint *aParams);
	static jlong InvokeMethod_15(void *fn, jint *aParams);
	static jlong InvokeMethod_16(void *fn, jint *aParams);
	static jlong InvokeMethod_17(void *fn, jint *aParams);
	static jlong InvokeMethod_18(void *fn, jint *aParams);
	static jlong InvokeMethod_19(void *fn, jint *aParams);
	static jlong InvokeMethod_20(void *fn, jint *aParams);
	static jlong InvokeMethod_21(void *fn, jint *aParams);
	static jlong InvokeMethod_22(void *fn, jint *aParams);
	static jlong InvokeMethod_23(void *fn, jint *aParams);
	static jlong InvokeMethod_24(void *fn, jint *aParams);
	static jlong InvokeMethod_25(void *fn, jint *aParams);
	static jlong InvokeMethod_26(void *fn, jint *aParams);
	
	static fastiva_ArrayHeader* InvokeMethodA_0(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_1(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_2(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_3(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_4(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_5(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_6(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_7(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_8(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_9(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_10(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_11(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_12(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_13(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_14(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_15(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_16(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_17(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_18(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_19(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_20(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_21(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_22(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_23(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_24(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_25(void *fn, jint *aParams);
	static fastiva_ArrayHeader* InvokeMethodA_26(void *fn, jint *aParams);
	

	// va_list와 jvalue를 aParams에 assign한다.
	static int ParseParams(const fastiva_Method *pMethod, va_list args, jint *aParams);
	static int ParseParams(const fastiva_Method *pMethod, jvalue *args, jint *aParams);
	
	// 현재 Method가 pObj에 속해있냐를 체크한다.
	static void JNICALL	ensureMethodIn(java_lang_Object_p pObj, const fastiva_Method *pMethod);
	// 현재 Method가 pClazz에 속해 있냐를 체크한다.
	static void JNICALL	ensureMethodIn(java_lang_Class_p pClazz, const fastiva_Method *pMethod);
	
	static jlonglong CallMethod_ex(
		va_list args,
		const fastiva_Method *pMethod, 
		java_lang_Object_p pThis,
		java_lang_Class_p pClass = ADDR_ZERO
	);
	static jlonglong CallMethod_ex(
		jvalue* args,
		const fastiva_Method *pMethod, 
		java_lang_Object_p pThis,
		java_lang_Class_p pClass = ADDR_ZERO
	);
};




/**
* NULL포인터는 하드웨어 방식을 사용하게 될수도 있기때문에 
* 굳이 return boolean 방식으로 구현하지 않아도 된다.
* 대신에 이부분은 TRY...CATCH_ANY_EX의 안쪽에 있어야 한다.
*/
inline void CHECK_NULL(const void* ptr, void* ret) {
	if (ptr == ADDR_ZERO || ptr == FASTIVA_NULL) {
		fox_JNI::FatalError(0, "invalid args (null)\n");// fastiva_throwNullPointerException();
	}
}

inline void CHECK_ZERO(const void* ptr, void* ret) {
	if (ptr == ADDR_ZERO || ptr == FASTIVA_NULL) {
		fox_JNI::FatalError(0, "invalid args (zero)\n");// fastiva_throwNullPointerException();
	}
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEnv* getJNIEnv();
jint __stdcall getVMEnv(JavaVM *vm, void **penv, jint version);

#ifdef __cplusplus
};
#endif

/*
    static jobject JNICALL CallObjectMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jobject JNICALL CallObjectMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jobject JNICALL CallObjectMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue * args);

    static jboolean JNICALL CallBooleanMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jboolean JNICALL CallBooleanMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jboolean JNICALL CallBooleanMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue * args);

    static jbyte JNICALL CallByteMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jbyte JNICALL CallByteMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jbyte JNICALL CallByteMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue *args);

    static jchar JNICALL CallCharMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jchar JNICALL CallCharMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jchar JNICALL CallCharMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue *args);

    static jshort JNICALL CallShortMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jshort JNICALL CallShortMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jshort JNICALL CallShortMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue *args);

    static jint JNICALL CallIntMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jint JNICALL CallIntMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jint JNICALL CallIntMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue *args);

    static jlong JNICALL CallLongMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jlong JNICALL CallLongMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jlong JNICALL CallLongMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue *args);

    static jfloat JNICALL CallFloatMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jfloat JNICALL CallFloatMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jfloat JNICALL CallFloatMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue *args);

    static jdouble JNICALL CallDoubleMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static jdouble JNICALL CallDoubleMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static jdouble JNICALL CallDoubleMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue *args);

    static void JNICALL CallVoidMethod(JNIEnv *, jobject obj, jmethodID methodID, ...);
    static void JNICALL CallVoidMethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args);
    static void JNICALL CallVoidMethodA(JNIEnv *, jobject obj, jmethodID methodID, jvalue * args);

    static jobject JNICALL CallNonvirtualObjectMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jobject JNICALL CallNonvirtualObjectMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jobject JNICALL CallNonvirtualObjectMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue * args);

    static jboolean JNICALL CallNonvirtualBooleanMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jboolean JNICALL CallNonvirtualBooleanMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jboolean JNICALL CallNonvirtualBooleanMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue * args);

    static jbyte JNICALL CallNonvirtualByteMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jbyte JNICALL CallNonvirtualByteMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jbyte JNICALL CallNonvirtualByteMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue *args);

    static jchar JNICALL CallNonvirtualCharMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jchar JNICALL CallNonvirtualCharMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jchar JNICALL CallNonvirtualCharMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue *args);

    static jshort JNICALL CallNonvirtualShortMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jshort JNICALL CallNonvirtualShortMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jshort JNICALL CallNonvirtualShortMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue *args);

    static jint JNICALL CallNonvirtualIntMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jint JNICALL CallNonvirtualIntMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jint JNICALL CallNonvirtualIntMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue *args);

	static jlong JNICALL CallNonvirtualLongMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jlong JNICALL CallNonvirtualLongMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jlong JNICALL CallNonvirtualLongMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue *args);

    static jfloat JNICALL CallNonvirtualFloatMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jfloat JNICALL CallNonvirtualFloatMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jfloat JNICALL CallNonvirtualFloatMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue *args);

    static jdouble JNICALL CallNonvirtualDoubleMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static jdouble JNICALL CallNonvirtualDoubleMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static jdouble JNICALL CallNonvirtualDoubleMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue *args);

    static void JNICALL CallNonvirtualVoidMethod(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, ...);
    static void JNICALL CallNonvirtualVoidMethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       va_list args);
    static void JNICALL CallNonvirtualVoidMethodA(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID,       jvalue * args);

    static jobject JNICALL GetObjectField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jboolean JNICALL GetBooleanField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jbyte JNICALL GetByteField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jchar JNICALL GetCharField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jshort JNICALL GetShortField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jint JNICALL GetIntField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jlong JNICALL GetLongField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jfloat JNICALL GetFloatField(JNIEnv *, jobject obj, jfieldID fieldID);
    static jdouble JNICALL GetDoubleField(JNIEnv *, jobject obj, jfieldID fieldID);

    static void JNICALL SetObjectField(JNIEnv *, jobject obj, jfieldID fieldID, jobject val);
    static void JNICALL SetBooleanField(JNIEnv *, jobject obj, jfieldID fieldID, jboolean val);
    static void JNICALL SetByteField(JNIEnv *, jobject obj, jfieldID fieldID, jbyte val);
    static void JNICALL SetCharField(JNIEnv *, jobject obj, jfieldID fieldID, jchar val);
    static void JNICALL SetShortField(JNIEnv *, jobject obj, jfieldID fieldID, jshort val);
    static void JNICALL SetIntField(JNIEnv *, jobject obj, jfieldID fieldID, jint val);
    static void JNICALL SetLongField(JNIEnv *, jobject obj, jfieldID fieldID, jlong val);
    static void JNICALL SetFloatField(JNIEnv *, jobject obj, jfieldID fieldID, jfloat val);
    static void JNICALL SetDoubleField(JNIEnv *, jobject obj, jfieldID fieldID, jdouble val);

    static jobject JNICALL CallStaticObjectMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jobject JNICALL CallStaticObjectMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jobject JNICALL CallStaticObjectMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);
    
    static jboolean JNICALL CallStaticBooleanMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jboolean JNICALL CallStaticBooleanMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jboolean JNICALL CallStaticBooleanMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);

    static jbyte JNICALL CallStaticByteMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jbyte JNICALL CallStaticByteMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jbyte JNICALL CallStaticByteMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);
    
    static jchar JNICALL CallStaticCharMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jchar JNICALL CallStaticCharMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jchar JNICALL CallStaticCharMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);
    
    static jshort JNICALL CallStaticShortMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jshort JNICALL CallStaticShortMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jshort JNICALL CallStaticShortMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);
    
    static jint JNICALL CallStaticIntMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jint JNICALL CallStaticIntMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jint JNICALL CallStaticIntMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);
    
    static jlong JNICALL CallStaticLongMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jlong JNICALL CallStaticLongMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jlong JNICALL CallStaticLongMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);
    
    static jfloat JNICALL CallStaticFloatMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jfloat JNICALL CallStaticFloatMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jfloat JNICALL CallStaticFloatMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);

    static jdouble JNICALL CallStaticDoubleMethod(JNIEnv *, jclass clazz, jmethodID methodID, ...);
    static jdouble JNICALL CallStaticDoubleMethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args);
    static jdouble JNICALL CallStaticDoubleMethodA(JNIEnv *, jclass clazz, jmethodID methodID, jvalue *args);

    static void JNICALL CallStaticVoidMethod(JNIEnv *, jclass cls, jmethodID methodID, ...);
    static void JNICALL CallStaticVoidMethodV(JNIEnv *, jclass cls, jmethodID methodID, va_list args);
    static void JNICALL CallStaticVoidMethodA(JNIEnv *, jclass cls, jmethodID methodID, jvalue * args);



    static jobject JNICALL GetStaticObjectField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jboolean JNICALL GetStaticBooleanField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jbyte JNICALL GetStaticByteField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jchar JNICALL GetStaticCharField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jshort JNICALL GetStaticShortField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jint JNICALL GetStaticIntField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jlong JNICALL GetStaticLongField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jfloat JNICALL GetStaticFloatField(JNIEnv *, jclass clazz, jfieldID fieldID);
    static jdouble JNICALL GetStaticDoubleField(JNIEnv *, jclass clazz, jfieldID fieldID);

    static void JNICALL SetStaticObjectField(JNIEnv *, jclass clazz, jfieldID fieldID, jobject value);
    static void JNICALL SetStaticBooleanField(JNIEnv *, jclass clazz, jfieldID fieldID, jboolean value);
    static void JNICALL SetStaticByteField(JNIEnv *, jclass clazz, jfieldID fieldID, jbyte value);
    static void JNICALL SetStaticCharField(JNIEnv *, jclass clazz, jfieldID fieldID, jchar value);
    static void JNICALL SetStaticShortField(JNIEnv *, jclass clazz, jfieldID fieldID, jshort value);
    static void JNICALL SetStaticIntField(JNIEnv *, jclass clazz, jfieldID fieldID, jint value);
    static void JNICALL SetStaticLongField(JNIEnv *, jclass clazz, jfieldID fieldID, jlong value);
    static void JNICALL SetStaticFloatField(JNIEnv *, jclass clazz, jfieldID fieldID, jfloat value);
    static void JNICALL SetStaticDoubleField(JNIEnv *, jclass clazz, jfieldID fieldID, jdouble value);

    static jboolean * JNICALL GetBooleanArrayElements(JNIEnv *, jbooleanArray array, jboolean *isCopy);
    static jbyte * JNICALL GetByteArrayElements(JNIEnv *, jbyteArray array, jboolean *isCopy);
    static jchar * JNICALL GetCharArrayElements(JNIEnv *, jcharArray array, jboolean *isCopy);
    static jshort * JNICALL GetShortArrayElements(JNIEnv *, jshortArray array, jboolean *isCopy);
    static jint * JNICALL GetIntArrayElements(JNIEnv *, jintArray array, jboolean *isCopy);
    static jlong * JNICALL GetLongArrayElements(JNIEnv *, jlongArray array, jboolean *isCopy);
    static jfloat * JNICALL GetFloatArrayElements(JNIEnv *, jfloatArray array, jboolean *isCopy);
    static jdouble * JNICALL GetDoubleArrayElements(JNIEnv *, jdoubleArray array, jboolean *isCopy);

    static void JNICALL ReleaseBooleanArrayElements(JNIEnv *, jbooleanArray array, jboolean *elems, jint mode);
    static void JNICALL ReleaseByteArrayElements(JNIEnv *, jbyteArray array, jbyte *elems, jint mode);
    static void JNICALL ReleaseCharArrayElements(JNIEnv *, jcharArray array, jchar *elems, jint mode);
    static void JNICALL ReleaseShortArrayElements(JNIEnv *, jshortArray array, jshort *elems, jint mode);
    static void JNICALL ReleaseIntArrayElements(JNIEnv *, jintArray array, jint *elems, jint mode);
    static void JNICALL ReleaseLongArrayElements(JNIEnv *, jlongArray array, jlong *elems, jint mode);
    static void JNICALL ReleaseFloatArrayElements(JNIEnv *, jfloatArray array, jfloat *elems, jint mode);
    static void JNICALL ReleaseDoubleArrayElements(JNIEnv *, jdoubleArray array, jdouble *elems, jint mode);

    static void JNICALL GetBooleanArrayRegion(JNIEnv *, jbooleanArray array, jsize start, jsize l, jboolean *buf);
    static void JNICALL GetByteArrayRegion(JNIEnv *, jbyteArray array, jsize start, jsize len, jbyte *buf);
    static void JNICALL GetCharArrayRegion(JNIEnv *, jcharArray array, jsize start, jsize len, jchar *buf);
    static void JNICALL GetShortArrayRegion(JNIEnv *, jshortArray array, jsize start, jsize len, jshort *buf);
    static void JNICALL GetIntArrayRegion(JNIEnv *, jintArray array, jsize start, jsize len, jint *buf);
    static void JNICALL GetLongArrayRegion(JNIEnv *, jlongArray array, jsize start, jsize len, jlong *buf);
    static void JNICALL GetFloatArrayRegion(JNIEnv *, jfloatArray array, jsize start, jsize len, jfloat *buf);
    static void JNICALL GetDoubleArrayRegion(JNIEnv *, jdoubleArray array, jsize start, jsize len, jdouble *buf);


    static void JNICALL SetBooleanArrayRegion(JNIEnv *, jbooleanArray array, jsize start, jsize l, jboolean *buf);
    static void JNICALL SetByteArrayRegion(JNIEnv *, jbyteArray array, jsize start, jsize len, jbyte *buf);
    static void JNICALL SetCharArrayRegion(JNIEnv *, jcharArray array, jsize start, jsize len, jchar *buf);
    static void JNICALL SetShortArrayRegion(JNIEnv *, jshortArray array, jsize start, jsize len, jshort *buf);
    static void JNICALL SetIntArrayRegion(JNIEnv *, jintArray array, jsize start, jsize len, jint *buf);
    static void JNICALL SetLongArrayRegion(JNIEnv *, jlongArray array, jsize start, jsize len, jlong *buf);
    static void JNICALL SetFloatArrayRegion(JNIEnv *, jfloatArray array, jsize start, jsize len, jfloat *buf);
    static void JNICALL SetDoubleArrayRegion(JNIEnv *, jdoubleArray array, jsize start, jsize len, jdouble *buf);
    
*/
#endif __FOX_JNI_H
