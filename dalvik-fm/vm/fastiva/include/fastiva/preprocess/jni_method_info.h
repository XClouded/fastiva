#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("jni_method_info.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD


#ifndef FASTIVA_JNI_METHOD_INFO
#define FASTIVA_JNI_METHOD_INFO
	#define FASTIVA_IS_FAST_STATIC		true
	#define FASTIVA_IS_FAST_VIRTUAL		false
	#define FASTIVA_IS_FAST_SYNTHETIC	false
	#define FASTIVA_IS_FAST_NON_VIRTUAL	false
	#define FASTIVA_IS_FAST_OVERRIDE	false
	#define FASTIVA_IS_FAST_ABSTRACT	false

#if ANDROID
	extern "C" void fastiva_BridgeFunc(const u4* argv, JValue* pResult, const Method* method, Thread* self);

	#define FASTIVA_INIT_METHOD_INFO(CLASS, acc, cnt, jni_acc, TYPE, ret, slot, fn, real_ret, args)  \
		FASTIVA_RAW_CLASS_PTR(CLASS), \
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_##jni_acc | FASTIVA_JNI_FLAG_##TYPE  | ACC_NATIVE$ | ACC_FASTIVA_METHOD, \
		FASTIVA_##TYPE##_SLOT_OFFSET(FASTIVA_PREPROCESS_CLASS, ret, slot, cnt, args), \
		(FASTIVA_ARGLIST_SIZE_##cnt args) + (FASTIVA_IS_FAST_##TYPE ? 0 : 1), 0, (FASTIVA_ARGLIST_SIZE_##cnt args) + (FASTIVA_IS_FAST_##TYPE ? 0 : 1), \
		FASTIVA_JNI_UTF_NAME_##jni_acc, \
		{ (DexFile*)FASTIVA_ARGLIST_ID_##cnt args, (u4)FASTIVA_JNI_TYPE_ID_##ret }, \
		FASTIVA_JNI_SHORT_SIG_##jni_acc, \
		NULL, \
		FASTIVA_JNI_ARG_INFO_##jni_acc, \
		FASTIVA_JNI_BRIDGE_##jni_acc, \
		FASTIVA_IS_FAST_##TYPE, true, false, false, \
		{ 	(void*)FASTIVA_METHOD_ADDR_##TYPE(CLASS, real_ret, slot, fn, cnt, args) }, \
		FASTIVA_JNI_GENERIC_SIG_##jni_acc, \
		FASTIVA_JNI_ANNOTATIONS_##jni_acc, \
		FASTIVA_JNI_PARAM_ANNOTATIONS_##jni_acc

	#define FASTIVA_INIT_ABSTRACT_METHOD_INFO(CLASS, acc, cnt, jni_acc, TYPE, ret, slot, fn, real_ret, args)  \
		FASTIVA_RAW_CLASS_PTR(CLASS), \
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_##jni_acc | FASTIVA_JNI_FLAG_##TYPE | ACC_NATIVE$ | ACC_FASTIVA_METHOD, \
		FASTIVA_##TYPE##_SLOT_OFFSET(FASTIVA_PREPROCESS_CLASS, ret, slot, cnt, args), \
		FASTIVA_ARGLIST_SIZE_##cnt args + 1, 0, FASTIVA_ARGLIST_SIZE_##cnt args + 1, \
		FASTIVA_JNI_UTF_NAME_##jni_acc, \
		{ (DexFile*)FASTIVA_ARGLIST_ID_##cnt args, (u4)FASTIVA_JNI_TYPE_ID_##ret }, \
		FASTIVA_JNI_SHORT_SIG_##jni_acc, \
		NULL , \
		FASTIVA_JNI_ARG_INFO_##jni_acc, \
		(DalvikBridgeFunc)fastiva.throwAbstractMethodError, \
		FASTIVA_IS_FAST_##TYPE, true, false, false, \
		{ 	(void*)NULL }, \
		FASTIVA_JNI_GENERIC_SIG_##jni_acc, \
		FASTIVA_JNI_ANNOTATIONS_##jni_acc, \
		FASTIVA_JNI_PARAM_ANNOTATIONS_##jni_acc

#else
	#define FASTVA_JNI_METHOD_SLOT_PTR(CLASS, jni_acc, fn, cnt, args)			\
		(void*)FASTIVA_MERGE_TOKEN(CLASS, _EXVT$)::								\
			FASTIVA_METHOD_SLOT_NAME(ret_t, FASTIVA_MERGE_TOKEN(fn_, fn), cnt, args),	

	#define FASTIVA_INIT_METHOD_INFO(CLASS, acc, cnt, jni_acc, TYPE, ret, slot, fn, real_ret, args)  \
		FASTIVA_JNI_METHOD_NAME_ID(fn),										\
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_##jni_acc | FASTIVA_JNI_FLAG_##TYPE | ACC_NATIVE$ | ACC_FASTIVA_METHOD,\
		FASTIVA_JNI_##TYPE##_SLOT_OFFSET(CLASS, ret, fn, cnt, args),				\
		FASTIVA_JNI_TYPE_ID_##ret,											\
		FASTIVA_JNI_ARG_LIST_ID(cnt, args),									\
		cnt, FASTIVA_ARGLIST_SIZE_##cnt args,								\
		FASTIVA_JNI_GENERIC_SIG_##jni_acc,									\
		FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS),								\
		FASTVA_JNI_METHOD_SLOT_PTR(CLASS, jni_acc, fn, cnt, args),			\
		FASTIVA_JNI_ANNOTATIONS_##jni_acc

	#define FASTIVA_INIT_ABSTRACT_METHOD_INFO(CLASS, acc, cnt, jni_acc, TYPE, ret, slot, fn, real_ret, args)  \
	FASTIVA_JNI_METHOD_NAME_ID(fn),										\
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_##jni_acc | FASTIVA_JNI_FLAG_##TYPE | ACC_NATIVE$ | ACC_FASTIVA_METHOD,\
		FASTIVA_JNI_##TYPE##_SLOT_OFFSET(CLASS, ret, fn, cnt, args),				\
		FASTIVA_JNI_TYPE_ID_##ret,											\
		FASTIVA_JNI_ARG_LIST_ID(cnt, args),									\
		cnt, FASTIVA_ARGLIST_SIZE_##cnt args,								\
		FASTIVA_JNI_GENERIC_SIG_##jni_acc,									\
		FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS),								\
		0,			\
		FASTIVA_JNI_ANNOTATIONS_##jni_acc
#endif

	#define MK_STR_EX(T)		#T
	#define MK_STR(T)			MK_STR_EX(T)

	#if 0 // defined(_DEBUG) && defined(_WIN32)
		#define FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)	, MK_STR(FASTIVA_MERGE_3TOKEN(ret, fn, FASTIVA_ARGLIST_NAME_##cnt args))
	#else
		#define FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)	// ignore
	#endif
#endif


#ifdef ANDROID
static fastiva_MethodInfo//fastiva_MethodInfo 
#else
static const fastiva_MethodInfo//fastiva_MethodInfo 
#endif
	FASTIVA_MERGE_3TOKEN(aMethodInfo_, FASTIVA_PREPROCESS_CLASS, $)[] = {

#define FASTIVA_END_OF_PREPROCESS									};




#define FASTIVA_METHOD_VIRTUAL(acc, cnt, jni_acc, ret, slot, fn, args) {		\
		FASTIVA_INIT_METHOD_INFO(FASTIVA_PREPROCESS_CLASS, acc, cnt, jni_acc,\
			VIRTUAL, ret, slot, fn, ret, args)								\
			FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)				\
	},

#undef FASTIVA_METHOD_ABSTRACT
#define FASTIVA_METHOD_ABSTRACT(acc, cnt, jni_acc, ret, slot, fn, args) {		\
		FASTIVA_INIT_ABSTRACT_METHOD_INFO(FASTIVA_PREPROCESS_CLASS, acc, cnt, jni_acc,\
			ABSTRACT, ret, slot, fn, ret, args)								\
			FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)				\
	},


// getDeclaredMethods 동작을 정확히 하기 위해, Override 된 함수도 MethodInfo 생성.
#define FASTIVA_METHOD_OVERRIDE(acc, cnt, jni_acc, ret, slot, fn, args)  {		\
		FASTIVA_INIT_METHOD_INFO(FASTIVA_PREPROCESS_CLASS, acc, cnt, jni_acc,\
			OVERRIDE, ret, slot, fn, ret, args)								\
			FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)				\
	},


// super 또는 anscent 의 함수를 재정의 (interface 상속)
#undef FASTIVA_METHOD_OVERRIDE_IFC
#define FASTIVA_METHOD_OVERRIDE_IFC(acc, cnt, jni_acc, ret, slot, fn, args)		\
		FASTIVA_METHOD_OVERRIDE(public, cnt, jni_acc, ret, slot, fn, args)

#undef FASTIVA_METHOD_OVERRIDE_ACC
#define FASTIVA_METHOD_OVERRIDE_ACC(acc, cnt, jni_acc, ret, slot, fn, args)		\
		FASTIVA_METHOD_OVERRIDE(acc, cnt, jni_acc, ret, slot, fn, args)

#undef	FASTIVA_METHOD_ABSTRACT_SYNTHETIC
#define FASTIVA_METHOD_ABSTRACT_SYNTHETIC(acc, cnt, jni_acc, ret, fn, real_ret, args)	{	\
		FASTIVA_INIT_ABSTRACT_METHOD_INFO(FASTIVA_PREPROCESS_CLASS, acc, cnt, jni_acc,\
			VIRTUAL, ret, slot, fn, real_ret, args)							\
			FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)				\
	},

#define FASTIVA_METHOD_INTERFACE(acc, cnt, jni_acc, ret, slot, fn, args) {			\
		FASTIVA_INIT_ABSTRACT_METHOD_INFO(FASTIVA_PREPROCESS_CLASS, acc, cnt, jni_acc,\
			VIRTUAL, ret, slot, fn, ret, args)								\
			FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)				\
	},

#define FASTIVA_METHOD_FINAL(acc, cnt, jni_acc, ret, slot, fn, args) {				\
		FASTIVA_INIT_METHOD_INFO(FASTIVA_PREPROCESS_CLASS, acc, cnt, jni_acc,\
			NON_VIRTUAL, ret, slot, fn, ret, args)								\
			FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)				\
	},

#define FASTIVA_METHOD_STATIC(acc, cnt, jni_acc, ret, slot, fn, args) {			\
		FASTIVA_INIT_METHOD_INFO(FASTIVA_PREPROCESS_CLASS, acc, cnt, jni_acc,\
			STATIC, ret, slot, fn, ret, args)											\
			FASTIVA_DEBUG_JNI_METHOD_NAME(ret, fn, cnt, args)				\
	},










