#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("jni_field_info.h")
#endif

#define JPP_PREPROCESS_INSTANCE_DATA
#define JPP_PREPROCESS_STATIC_DATA

#ifndef __JNI_FIELD_INFO__
#define __JNI_FIELD_INFO__
# define FASTIVA_JNI_TYPE(type)	FASTIVA_JNI_TYPE_ID_##type
#endif


#if FASTIVA_USE_TOKENIZED_FIELD_INFO
static const fastiva_FieldInfo 
	FASTIVA_MERGE_3TOKEN(aFieldInfo_, FASTIVA_PREPROCESS_CLASS, $)[] = {
#else
static fastiva_FieldInfo 
	FASTIVA_MERGE_3TOKEN(aFieldInfo_, FASTIVA_PREPROCESS_CLASS, $)[] = {
#endif

#define FASTIVA_END_OF_PREPROCESS									};



#ifdef ANDROID

#define FASTIVA_FIELD(acc, name, type, decl_cls, jni_acc) {	\
		FASTIVA_RAW_CLASS_PTR(decl_cls), \
		FASTIVA_JNI_UTF_NAME_JNI$ jni_acc,	\
		FASTIVA_SIG_##type, \
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_JNI$ jni_acc,	\
		FASTIVA_JNI_GENERIC_SIG_JNI$ jni_acc, \
		FASTIVA_JNI_ANNOTATIONS_JNI$ jni_acc, \
		((int)&((decl_cls*)0x1000)->m_##name - 0x1000)\
	},

#define FASTIVA_FIELD_CONSTANT(acc, name, type, decl_cls, jni_acc) {	\
		FASTIVA_RAW_CLASS_PTR(decl_cls), \
		FASTIVA_JNI_UTF_NAME_JNI$ jni_acc,	\
		FASTIVA_SIG_##type, \
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_JNI$ jni_acc | ACC_STATIC$ | ACC_FINAL$,	\
		FASTIVA_JNI_GENERIC_SIG_JNI$ jni_acc, \
		FASTIVA_JNI_ANNOTATIONS_JNI$ jni_acc, \
		0\
	},

#define FASTIVA_FIELD_STATIC(acc, name, type, decl_cls, jni_acc)	{	\
		FASTIVA_RAW_CLASS_PTR(decl_cls), \
		FASTIVA_JNI_UTF_NAME_JNI$ jni_acc,	\
		FASTIVA_SIG_##type, \
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_JNI$ jni_acc | ACC_STATIC$,		\
		FASTIVA_JNI_GENERIC_SIG_JNI$ jni_acc, \
		FASTIVA_JNI_ANNOTATIONS_JNI$ jni_acc, \
		0\
	},

#else

#define FASTIVA_FIELD(acc, name, type, decl_cls, jni_acc) {					\
		FASTIVA_JNI_FIELD_NAME_ID(name##_$),									\
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_JNI$ jni_acc,	\
		((int)&((decl_cls*)0x1000)->m_##name - 0x1000),						\
		FASTIVA_JNI_TYPE(type),											\
		FASTIVA_JNI_ANNOTATIONS_JNI$ jni_acc									\
	},

#define FASTIVA_FIELD_CONSTANT(acc, name, type, decl_cls, jni_acc) {	\
		FASTIVA_JNI_FIELD_NAME_ID(name##_$),									\
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_JNI$ jni_acc | ACC_STATIC$ | ACC_FINAL$,	\
		((int)&((decl_cls##_C$*)0x1000)->m_##name - 0x1000),					\
		FASTIVA_JNI_TYPE(type),											\
		FASTIVA_JNI_ANNOTATIONS_JNI$ jni_acc									\
	},

#define FASTIVA_FIELD_STATIC(acc, name, type, decl_cls, jni_acc)	{	\
		FASTIVA_JNI_FIELD_NAME_ID(name##_$),									\
		FASTIVA_JNI_ACC_##acc | FASTIVA_JNI_FLAG_JNI$ jni_acc | ACC_STATIC$,		\
		((int)&((decl_cls##_C$*)0x1000)->m_##name - 0x1000),					\
		FASTIVA_JNI_TYPE(type),											\
		FASTIVA_JNI_ANNOTATIONS_JNI$ jni_acc									\
	},


#endif