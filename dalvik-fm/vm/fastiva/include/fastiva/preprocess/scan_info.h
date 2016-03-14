#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("scan_info.h")
#endif

#define __FASTIVA_PREPROCESS_SUPER__

FASTIVA_INIT_SCAN_OFFSET_TABLE(FASTIVA_PREPROCESS_CLASS) = {

#define FASTIVA_END_OF_PREPROCESS					0  };

//#undef FASTIVA_DECL_STATIC_METHODS
//#define FASTIVA_DECL_STATIC_METHODS(CLASS)	0, 


/**
  ���� class �� ������ field ���� ������ �Ϳ� ����Ͽ� �ݵ�� declared-class��
  �̿��Ͽ� field-offset�� ����Ѵ�.
*/
#define FASTIVA_FIELD(acc, name, type, declrared_cls, jni_acc)			\
	FASTIVA_FIELD_SCAN_OFFSET_##type(declrared_cls, m_##name)

#define FASTIVA_FIELD_CONSTANT(acc, name, type, declrared_cls, jni_acc)	\
	// IGNORE

#define FASTIVA_FIELD_STATIC(acc, name, type, declrared_cls, jni_acc)	\
	// FASTIVA_STATIC_FIELD_SCAN_OFFSET_##type(cls, name, value_type)



#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	// IGNORE

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	// IGNORE


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE

