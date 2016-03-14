#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("init_ivtable.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD


#define __FASTIVA_PREPROCESS_SUPER__

#ifndef __JPP_INIT_IVTABLE_H__
#define __JPP_INIT_IVTABLE_H__

	#define JPP_BEGIN_INTERFACE_SLOT_TABLE(IFC, CLASS)								\
		const int FASTIVA_INTERFACE_TABLE_NAME(IFC, CLASS)[] = {		\

	#define JPP_END_INTERFACE_SLOT_TABLE(IFC, CLASS)							\
		};																		\

#endif // __JPP_INIT_IVTABLE_H__




#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	"error!! invalid-method-type"

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! invalid-method-type"

// 하위 class 가 vtable을 overrider 할 수 있으므로 EXVT에서 final 함수를 참조하면 안된다.
#define FASTIVA_METHOD_INTERFACE(acc, cnt, IFC, ret_t, slot, fn, args)			\
	FASTIVA_VIRTUAL_SLOT_OFFSET(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args),

#define FASTIVA_METHOD_INTERFACE_22(acc, cnt, IFC, ret_t, slot, fn, args)			\
	(void*)(FASTIVA_RETURN_TYPE_##ret_t (*)(FASTIVA_INTERFACE_IMPL_SELF(cnt)		\
		FASTIVA_PARAM_PAIRS_##cnt args))									\
		FASTIVA_MERGE_3TOKEN(												\
		FASTIVA_MERGE_3TOKEN(FASTIVA_PREPROCESS_CLASS, __, FASTIVA_IFC_NAME_##IFC),__,fn),

#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! invalid-method-type"

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! invalid-method-type"





