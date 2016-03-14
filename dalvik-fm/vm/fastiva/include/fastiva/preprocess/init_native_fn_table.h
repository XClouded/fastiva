#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("init_native_fn_table.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD

#undef  JPP_CLASS
#define JPP_CLASS(TYPE, CLASS, SUPER)	struct CLASS##_jni_method_table_index$ { enum {
#define FASTIVA_END_OF_PREPROCESS		}; };



#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args),


#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args),


#define FASTIVA_METHOD_INTERFACE(acc, cnt, IFC, ret_t, slot, fn, args)			\
	FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args),


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args),


#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args),
