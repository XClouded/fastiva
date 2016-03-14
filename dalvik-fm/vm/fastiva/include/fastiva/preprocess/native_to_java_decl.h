#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("native_to_java_decl.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_INSTANCE_DATA
#define JPP_PREPROCESS_STATIC_DATA
#define JPP_PREPROCESS_STATIC_METHOD


#define FASTIVA_FIELD(acc, name, type, cls, jni_acc)						\
	static void* FASTIVA_MERGE_TOKEN(m_jniFieldID_, name);					\
	FASTIVA_RETURN_TYPE_##type get__##name();								\
	void set__##name(FASTIVA_RETURN_TYPE_##type v);

#define FASTIVA_FIELD_CONSTANT(acc, name, type, cls, jni_acc)				\
	static void* FASTIVA_MERGE_TOKEN(m_jniFieldID_, name);					\
	FASTIVA_RETURN_TYPE_##type get__##name();								\
	void set__##name(FASTIVA_RETURN_TYPE_##type v);

#define FASTIVA_FIELD_STATIC(acc, name, type, cls, jni_acc)					\
	static void* FASTIVA_MERGE_TOKEN(m_jniFieldID_, name);					\
	FASTIVA_RETURN_TYPE_##type get__##name();								\
	void set__##name(FASTIVA_RETURN_TYPE_##type v);


#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	static void* JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);					\
	FASTIVA_DECL_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);				   \


#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	static void* JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);					\
	FASTIVA_DECL_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);   \


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	static void* JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);					\
	FASTIVA_DECL_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);				   \

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	static void* JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);					\
	static FASTIVA_DECL_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);				   \


#undef FASTIVA_METHOD_CONSTRUCTOR
#define FASTIVA_METHOD_CONSTRUCTOR(acc, cnt, sig, ret_t, slot, fn, args)			\
	static void* JPP_JAVA_JNI_METHOD_ID(fn, cnt, args);					\
	FASTIVA_DECL_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);\

