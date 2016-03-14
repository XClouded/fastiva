#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("native_to_java_init.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_INSTANCE_DATA
#define JPP_PREPROCESS_STATIC_DATA
#define JPP_PREPROCESS_STATIC_METHOD

//#define FASTIVA_JNI_FIELD_NAME$(name)										\
//	FASTIVA_MODULE_NAME(ModuleInfo)->m_aRefName[FASTIVA_JNI_FIELD_NAME_ID(name)].m_pszToken

#define FASTIVA_JNI_METHOD_NAME$(name, sig)						name
#define FASTIVA_JNI_METHOD_SIG$(name, sig)						sig
	//FASTIVA_MODULE_NAME(ModuleInfo)->m_aRefName[FASTIVA_JNI_METHOD_NAME_ID(name)].m_pszToken

	JNIEnv* pEnv = (JNIEnv*)pStatic;
	g_javaClass = jni_getGlobalClass(pEnv, FASTIVA_MERGE_3TOKEN(FASTIVA_SIG_, FASTIVA_PREPROCESS_CLASS, _CLASSNAME), true);
	
	if (g_javaClass == ADDR_ZERO) {
		return;
	}
	fox_debug_printf("Proxy class registered %s, %x\n",
			FASTIVA_MERGE_3TOKEN(FASTIVA_SIG_, FASTIVA_PREPROCESS_CLASS,_CLASSNAME),
			g_javaClass);

#define FASTIVA_FIELD(acc, name, type, cls, jni_acc)						\
	m_jniFieldID_##name = pEnv->GetFieldID((jclass)g_javaClass, #name, FASTIVA_SIG_##type);

#define FASTIVA_FIELD_CONSTANT(acc, name, type, cls, jni_acc)				\
	m_jniFieldID_##name = pEnv->GetStaticFieldID((jclass)g_javaClass, #name, FASTIVA_SIG_##type);

#define FASTIVA_FIELD_STATIC(acc, name, type, cls, jni_acc)					\
	m_jniFieldID_##name = pEnv->GetStaticFieldID((jclass)g_javaClass, #name, FASTIVA_SIG_##type);


#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	JPP_JAVA_JNI_METHOD_ID(fn, cnt, args) = pEnv->GetMethodID((jclass)g_javaClass, FASTIVA_JNI_METHOD_NAME$ sig, FASTIVA_JNI_METHOD_SIG$ sig);

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	JPP_JAVA_JNI_METHOD_ID(fn, cnt, args) = pEnv->GetMethodID((jclass)g_javaClass, FASTIVA_JNI_METHOD_NAME$ sig, FASTIVA_JNI_METHOD_SIG$ sig);


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	JPP_JAVA_JNI_METHOD_ID(fn, cnt, args) = pEnv->GetMethodID((jclass)g_javaClass, FASTIVA_JNI_METHOD_NAME$ sig, FASTIVA_JNI_METHOD_SIG$ sig);

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	JPP_JAVA_JNI_METHOD_ID(fn, cnt, args) = pEnv->GetStaticMethodID((jclass)g_javaClass, FASTIVA_JNI_METHOD_NAME$ sig, FASTIVA_JNI_METHOD_SIG$ sig);



#undef FASTIVA_METHOD_CONSTRUCTOR
#define FASTIVA_METHOD_CONSTRUCTOR(acc, cnt, sig, ret_t, slot, fn, args)			\
	JPP_JAVA_JNI_METHOD_ID(fn, cnt, args) = pEnv->GetMethodID((jclass)g_javaClass, FASTIVA_JNI_METHOD_NAME$ sig, FASTIVA_JNI_METHOD_SIG$ sig);
