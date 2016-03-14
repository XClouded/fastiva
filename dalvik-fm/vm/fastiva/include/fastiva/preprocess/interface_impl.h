#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("interface_impl.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD

#ifndef __JPP_INTERFACE_IMPL_H__

#define FASTIVA_INTERFACE_IMPL(CLASS, IFC, ret_t, slot, cnt, args)					\
	static FASTIVA_RETURN_TYPE_##ret_t										\
	FASTIVA_MERGE_3TOKEN(FASTIVA_MERGE_3TOKEN(CLASS, __, IFC),__,slot)(	\
		FASTIVA_INTERFACE_IMPL_SELF(cnt)									\
		FASTIVA_PARAM_PAIRS_##cnt args) {									\
		FASTIVA_RETURN_INSTRUCUTION_##ret_t	(FASTIVA_RETURN_TYPE_##ret_t)	\
			((FASTIVA_METHOD_SLOT_OWNER(CLASS, slot, cnt, args)*)self)->slot	\
							(FASTIVA_PARAM_NAMES_##cnt args);				\
    }																		\

#endif // __JPP_INTERFACE_IMPL_H__




#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	"error!! interface can't have non-abstract methods"

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! interface can't have non-abstract methods"

#define FASTIVA_METHOD_INTERFACE(acc, cnt, IFC, ret_t, slot, fn, args)			\
	FASTIVA_INTERFACE_IMPL(FASTIVA_PREPROCESS_CLASS, FASTIVA_IFC_NAME_##IFC,					\
			ret_t, fn, cnt, args)

#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! interface can't have non-abstract methods"

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! interface can't have non-abstract methods"




