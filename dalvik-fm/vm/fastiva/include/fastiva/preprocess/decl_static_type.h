#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_static_type.h")
#endif

#define JPP_PREPROCESS_STATIC_DATA
#define JPP_PREPROCESS_STATIC_METHOD
#define JPP_PREPROCESS_SYSTEM_METHOD

#undef	JPP_CLASS
#define JPP_CLASS	FASTIVA_DECL_STATIC_TYPE
#define FASTIVA_END_OF_PREPROCESS			};

/**
  2012.11.14 macro 처리를 방지하기 위하여, prefix 를 직접 처리하여야 한다.
*/

#ifdef ANDROID
#define FASTIVA_FIELD_STATIC(acc, name, type, cls, accFlag)					\
	FASTIVA_DECL_FIELD_##type(acc, FASTIVA_JNI_FLAG_JNI$ accFlag, FASTIVA_FIELD_ACCESS_TYPE_##type, sfields[FASTIVA_JNI_SFIELD_INDEX_JNI$ accFlag].value, get__##name, set__##name)
#else
#define FASTIVA_FIELD_STATIC(acc, name, type, cls, accFlag)					\
	FASTIVA_DECL_STATIC_FIELD(acc, FASTIVA_FIELD_TYPE_##type, m_##name)			\
	FASTIVA_DECL_FIELD_##type(acc, FASTIVA_JNI_FLAG_JNI$ accFlag, FASTIVA_FIELD_ACCESS_TYPE_##type, m_##name, get__##name, set__##name)
#endif

#define FASTIVA_METHOD_STATIC(acc, cnt, accFlag, ret_t, slot, fn, args)									\
	FASTIVA_ACC_##acc FASTIVA_RETURN_TYPE_##ret_t fn (FASTIVA_PARAM_PAIRS_##cnt args);

#if (JPP_JNI_EXPORT_LEVEL > 0)
#ifdef ANDROID
#define FASTIVA_FIELD_CONSTANT(acc, name, type, cls, accFlag)					\
	FASTIVA_DECL_CONSTANT_FIELD(acc, FASTIVA_JNI_FLAG_JNI$ accFlag, FASTIVA_FIELD_ACCESS_TYPE_##type, sfields[FASTIVA_JNI_SFIELD_INDEX_JNI$ accFlag].value, get__##name, set__##name, cls##__##name)
	//FASTIVA_DECL_FIELD_##type(acc, FASTIVA_JNI_FLAG_JNI$ accFlag, FASTIVA_FIELD_ACCESS_TYPE_##type, sfields[FASTIVA_JNI_SFIELD_INDEX_JNI$ accFlag].value, get__##name, set__##name)
#else
#define FASTIVA_FIELD_CONSTANT(acc, name, type, cls, accFlag)					\
	FASTIVA_DECL_STATIC_FIELD(acc, FASTIVA_FIELD_TYPE_##type, m_##name)			\
	FASTIVA_DECL_FIELD_##type(acc, FASTIVA_JNI_FLAG_JNI$ accFlag, FASTIVA_FIELD_ACCESS_TYPE_##type, m_##name, get__##name, set__##name)
#endif
#else
	// ignore;
#endif

