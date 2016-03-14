#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("native_to_java_impl.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_INSTANCE_DATA
#define JPP_PREPROCESS_STATIC_DATA
#define JPP_PREPROCESS_STATIC_METHOD


	#include <jni.h>
	void* FASTIVA_PREPROCESS_CLASS::g_javaClass = 0;
	void* FASTIVA_PREPROCESS_CLASS::getJavaClass() {
		if (g_javaClass == ADDR_ZERO) {
			initStatic$();
		}
		return g_javaClass;
	}

	//FASTIVA_REGISTER_PROXY(FASTIVA_PREPROCESS_CLASS);

#define FASTIVA_FIELD(acc, name, type, cls, jni_acc)						\
	void* FASTIVA_PREPROCESS_CLASS::m_jniFieldID_##name;                   \
	FASTIVA_RETURN_TYPE_##type FASTIVA_PREPROCESS_CLASS::get__##name() { return JNI_GET_FIELD_##type((jobject)this, (jfieldID)FASTIVA_MERGE_TOKEN(m_jniFieldID_, name)); }\
	void FASTIVA_PREPROCESS_CLASS::set__##name(FASTIVA_RETURN_TYPE_##type v) { JNI_SET_FIELD_##type((jobject)this, (jfieldID)FASTIVA_MERGE_TOKEN(m_jniFieldID_, name), (JNI_ARG_TYPE_##type)v); }

#define FASTIVA_FIELD_CONSTANT(acc, name, type, cls, jni_acc)				\
	void* FASTIVA_PREPROCESS_CLASS::m_jniFieldID_##name; \
	FASTIVA_RETURN_TYPE_##type FASTIVA_PREPROCESS_CLASS::get__##name() { return JNI_GET_STATIC_FIELD_##type((jclass)g_javaClass, (jfieldID)FASTIVA_MERGE_TOKEN(m_jniFieldID_, name)); }\
	void FASTIVA_PREPROCESS_CLASS::set__##name(FASTIVA_RETURN_TYPE_##type v) { JNI_SET_STATIC_FIELD_##type((jclass)g_javaClass, (jfieldID)FASTIVA_MERGE_TOKEN(m_jniFieldID_, name), (JNI_ARG_TYPE_##type)v); }

#define FASTIVA_FIELD_STATIC(acc, name, type, cls, jni_acc)					\
	void* FASTIVA_PREPROCESS_CLASS::m_jniFieldID_##name; \
	FASTIVA_RETURN_TYPE_##type FASTIVA_PREPROCESS_CLASS::get__##name() { return JNI_GET_STATIC_FIELD_##type((jclass)g_javaClass, (jfieldID)FASTIVA_MERGE_TOKEN(m_jniFieldID_, name)); }\
	void FASTIVA_PREPROCESS_CLASS::set__##name(FASTIVA_RETURN_TYPE_##type v) { JNI_SET_STATIC_FIELD_##type((jclass)g_javaClass, (jfieldID)FASTIVA_MERGE_TOKEN(m_jniFieldID_, name), (JNI_ARG_TYPE_##type)v); }




#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	void* FASTIVA_PREPROCESS_CLASS::JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);						\

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, 0, 0, ret_t, slot, fn, args)
	
#define FASTIVA_METHOD_INTERFACE(acc, cnt, IFC, ret_t, slot, fn, args)			\
	void* FASTIVA_PREPROCESS_CLASS::JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);						\


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	void* FASTIVA_PREPROCESS_CLASS::JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);						\

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	void* FASTIVA_PREPROCESS_CLASS::JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);						\



#undef FASTIVA_METHOD_CONSTRUCTOR
#define FASTIVA_METHOD_CONSTRUCTOR(acc, cnt, sig, ret_t, slot, fn, args)			\
	void* FASTIVA_PREPROCESS_CLASS::JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);						\

