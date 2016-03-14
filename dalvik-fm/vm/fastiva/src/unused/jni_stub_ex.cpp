#include <precompiled_libcore.h>

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

extern void fastiva_GC_jni_clearWeakGlobalRef(void* env0);
extern jobject fastiva_jni_NewWeakGlobalRef$(JNIEnv* pEnv, jobject obj);
extern void fastiva_jni_JavaProxyFinalized(JNIEnv* pEnv, java_lang_Object_p pObj);
extern void fastiva_jni_setMaxStrongGlobalRefCount(int count);


extern "C"
/*JNIEXPORT*/jobject JNICALL Java_fastiva_jni_FastivaStub_toStringNative(
	JNIEnv* env0, jclass c, jobject proxySelf
) {
	FASTIVA_BEGIN_JNI_SECTION(env0)
	JPP_STUB_TO_FASTIVA_OBJECT_ARG(self, java_lang_Object, proxySelf);
	java_lang_String_p res = self->toString();
	JPP_FASTIVA_TO_JOBJECT_PRIMIVTIVES(jvmRes, java_lang_String, res);
	return jvmRes;
	FASTIVA_END_JNI_SECTION()
}

extern "C"
/*JNIEXPORT*/void JNICALL Java_fastiva_jni_FastivaStub_setSafeJniGlobalRefMaxCount(
	JNIEnv* env0, jclass c, int count
) {
	fastiva_jni_setMaxStrongGlobalRefCount(count);
}

extern "C"
/*JNIEXPORT*/void JNICALL Java_fastiva_jni_FastivaStub_setMaxNativeHeapSize(
	JNIEnv* env0, jclass c, int maxHeapSize
) {
	fox_heap_setMaxHeapSize(maxHeapSize);
}


extern "C"
/*JNIEXPORT*/jboolean JNICALL Java_fastiva_jni_FastivaStub_eqaulsNative(
	JNIEnv* env0, jclass c, jobject proxySelf, jobject arg1
) {
	// @todo 적절한 시기에 함수명 오타 처리. ^_^
	FASTIVA_BEGIN_JNI_SECTION(env0)
	JPP_STUB_TO_FASTIVA_OBJECT_ARG(self, java_lang_Object, proxySelf);
	JPP_STUB_TO_FASTIVA_OBJECT_ARG(obj1, java_lang_Object, arg1);
	jboolean jvmRes = self->equals(obj1);
	//JPP_FASTIVA_TO_JOBJECT_PRIMIVTIVES(jvmRes, java_lang_String, res);
	return jvmRes;
	FASTIVA_END_JNI_SECTION()
}


extern "C"
/*JNIEXPORT*/jint JNICALL Java_fastiva_jni_FastivaStub_hashCodeNative(
	JNIEnv* env0, jclass c, jobject proxySelf
) {
	FASTIVA_BEGIN_JNI_SECTION(env0)
	JPP_STUB_TO_FASTIVA_OBJECT_ARG(self, java_lang_Object, proxySelf);
	int jvmRes = self->hashCode();
	return jvmRes;
	FASTIVA_END_JNI_SECTION()
}


extern "C"
/*JNIEXPORT*/void JNICALL Java_fastiva_jni_FastivaStub_detachNativeObject(
	JNIEnv* pEnv, jclass c, int handle
) {
	java_lang_Object_p pObj = (java_lang_Object_p)handle;
	if (pObj == NULL) {
		fox_debug_printf("!!! NativeStub is not created in Native area.\n");
		fox_debug_printf("!!! NativeStub is not created in Native area.\n");
		fox_debug_printf("!!! NativeStub is not created in Native area.\n");
		return;
	}
	fastiva_jni_JavaProxyFinalized(pEnv, pObj);
}

/*
extern "C"
void JNICALL Java_fastiva_jni_FastivaStub_doNativeGC(
	JNIEnv* pEnv, jclass c, int handle
) {
	fastiva_GC_jni_clearWeakGlobalRef(pEnv);
	fastiva_GC_doGC(true);
}
*/


extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_getFastivaBoolean(
	JNIEnv* pEnv, jclass c, jobject jobj, jboolean v
) {
	if (v) {
		return (int)(void*)java_lang_Boolean_C$::importClass$()->get__TRUE();
	}
	else {
		return (int)(void*)java_lang_Boolean_C$::importClass$()->get__FALSE();
	}
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_newFastivaByte(
	JNIEnv* pEnv, jclass c, jobject jobj, jbyte v
) {
	java_lang_Byte_p res = FASTIVA_NEW(java_lang_Byte)(v);
	// res 는 Fastiva 내에서만 참조되므로, GlobalLock을 할 필요가 없다.
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
	return (int)(void*)res;
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_newFastivaCharacter(
	JNIEnv* pEnv, jclass c, jobject jobj, jchar v
) {
	java_lang_Character_p res = FASTIVA_NEW(java_lang_Character)(v);
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
	return (int)(void*)res;
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_newFastivaShort(
	JNIEnv* pEnv, jclass c, jobject jobj, jshort v
) {
	java_lang_Short_p res = FASTIVA_NEW(java_lang_Short)(v);
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
	return (int)(void*)res;
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_newFastivaInteger(
	JNIEnv* pEnv, jclass c, jobject jobj, jint v
) {
	java_lang_Integer_p res = FASTIVA_NEW(java_lang_Integer)(v);
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
	return (int)(void*)res;
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_newFastivaLong(
	JNIEnv* pEnv, jclass c, jobject jobj, jlonglong v
) {
	java_lang_Long_p res = FASTIVA_NEW(java_lang_Long)(v);
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
	return (int)(void*)res;
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_newFastivaFloat(
	JNIEnv* pEnv, jclass c, jobject jobj, jfloat v
) {
	java_lang_Float_p res = FASTIVA_NEW(java_lang_Float)(v);
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
	return (int)(void*)res;
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaStub_newFastivaDouble(
	JNIEnv* pEnv, jclass c, jobject jobj, jdouble v
) {
	java_lang_Double_p res = FASTIVA_NEW(java_lang_Double)(v);
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, jobj);
	return (int)(void*)res;
}


/*JNIEXPORT*/int JNICALL fastiva_vm_FastivaUtil_C$__getNativeGlobalString(java_lang_String_p pString) {
	fox_printf("this must not be called");
	fox_exit(-1);
	return 0;
}

extern "C"
/*JNIEXPORT*/int JNICALL Java_fastiva_jni_FastivaUtil_getNativeGlobalString(
	JNIEnv* pEnv, jclass c, jstring s
) {
	java_lang_String_p pStr = toFastivaObject((java_lang_String_p)pEnv, s, true);
	java_lang_String_p res = fm::getInternedString(pStr);
	res->m_javaRef$ = (void*)fastiva_jni_NewWeakGlobalRef$(pEnv, s);
	return (int)(void*)res;
}


extern "C"
/*JNIEXPORT*/void JNICALL Java_fastiva_jni_Runnable_jni_run(
	JNIEnv* env0, jobject self
) {
	FASTIVA_BEGIN_JNI_SECTION(env0)
	java_lang_Object_p pObj = nativeStubToFastivaObject(pEnv, java_lang_Object::getRawContext$(), self);
	java_lang_Runnable_p pRunnable = java_lang_Runnable::ptr_cast$(pObj);
	pRunnable->run();
	FASTIVA_END_JNI_SECTION()
}






void fastiva_jni_initFastivaStub(JNIEnv* pEnv, jclass fastivaStub_class) {

	JNINativeMethod m[] = {
		{	(char*)"detachNativeObject", (char*)"(I)I", 
			(void*)Java_fastiva_jni_FastivaStub_detachNativeObject },

		{	(char*)"toStringNative", (char*)"(Ljava/lang/Object;)Ljava/lang/String;", 
			(void*)Java_fastiva_jni_FastivaStub_toStringNative },

		{	(char*)"eqaulsNative", (char*)"(Ljava/lang/Object;Ljava/lang/Object;)Z", 
			(void*)Java_fastiva_jni_FastivaStub_eqaulsNative },

		{	(char*)"hashCodeNative", (char*)"(Ljava/lang/Object;)I", 
			(void*)Java_fastiva_jni_FastivaStub_hashCodeNative},

		{	(char*)"setSafeJniGlobalRefMaxCount", (char*)"(I)V", 
			(void*)Java_fastiva_jni_FastivaStub_setSafeJniGlobalRefMaxCount},

		/*
		{	(char*)"doNativeGC", (char*)"()V", 
			(void*)Java_fastiva_jni_FastivaStub_doNativeGC},
		*/
#ifdef JPP_AUTO_CONVERT_CORE_CLASS

		{	(char*)"getFastivaBoolean", (char*)"(Ljava/lang/Object;Z)I", 
			(void*)Java_fastiva_jni_FastivaStub_getFastivaBoolean },

		{	(char*)"newFastivaByte", (char*)"(Ljava/lang/Object;B)I", 
			(void*)Java_fastiva_jni_FastivaStub_newFastivaByte },

		{	(char*)"newFastivaCharacter", (char*)"(Ljava/lang/Object;C)I", 
			(void*)Java_fastiva_jni_FastivaStub_newFastivaCharacter },

		{	(char*)"newFastivaShort", (char*)"(Ljava/lang/Object;S)I", 
			(void*)Java_fastiva_jni_FastivaStub_newFastivaShort },

		{	(char*)"newFastivaInteger", (char*)"(Ljava/lang/Object;I)I", 
			(void*)Java_fastiva_jni_FastivaStub_newFastivaInteger },

		{	(char*)"newFastivaLong", (char*)"(Ljava/lang/Object;J)I", 
			(void*)Java_fastiva_jni_FastivaStub_newFastivaLong },

		{ 	(char*)"newFastivaFloat", (char*)"(Ljava/lang/Object;F)I", 
			(void*)Java_fastiva_jni_FastivaStub_newFastivaFloat },

		{	(char*)"newFastivaDouble", (char*)"(Ljava/lang/Object;D)I", 
			(void*)Java_fastiva_jni_FastivaStub_newFastivaDouble },
#endif
	};

	int result;
	result = pEnv->RegisterNatives(fastivaStub_class, m, sizeof(m) / sizeof(m[0]));
	if (result < 0) {
		pEnv->ExceptionClear();
		fox_printf("++ Fail RegisterNatives for FastivaStub.nativeMethods\n");
		fox_exit(-1);
	}

#ifdef JPP_AUTO_CONVERT_CORE_CLASS
	{
		jclass fastivaUtil_class = pEnv->FindClass("com/tf/thinkdroid/common/util/FastivaUtil");
		JNINativeMethod m2[] = {
			{	(char*)"getNativeGlobalString", (char*)"(Ljava/lang/String;)I", 
				(void*)Java_fastiva_jni_FastivaUtil_getNativeGlobalString },
		};
		result = pEnv->RegisterNatives(fastivaUtil_class, m2, sizeof(m2) / sizeof(m2[0]));
		if (result < 0) {
			pEnv->ExceptionClear();
			fox_printf("++ Fail RegisterNatives for FastivaUtil.nativeMethods\n");
			fox_exit(-1);
		}
	}

	{
		jclass Runnable_jni_class = pEnv->FindClass("fastiva/jni/lang/Runnable_jni");
		JNINativeMethod m2[] = {
			{	(char*)"run", (char*)"()V", 
				(void*)Java_fastiva_jni_Runnable_jni_run },
		};

		result = pEnv->RegisterNatives(Runnable_jni_class, m2, sizeof(m2) / sizeof(m2[0]));
		if (result < 0) {
			pEnv->ExceptionClear();
			fox_printf("++ Fail RegisterNatives for Runnable_jni.nativeMethods\n");
			fox_exit(-1);
		}
	}
#endif
}

