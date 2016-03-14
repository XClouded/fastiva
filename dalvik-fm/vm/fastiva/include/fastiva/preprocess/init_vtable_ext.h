#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("init_vtable_ext.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD

	FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _EXVT$)* extable = FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _G$)::g_vtable$;

#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	// IGNORE

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	"error!! invalid-method-type"


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args) {				\
	extable->FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args) = \
		(void*)FASTIVA_METHOD_ADDR_NON_VIRTUAL(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args); }


#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args) {				\
	extable->FASTIVA_METHOD_SLOT_NAME(ret_t, slot##_S$, cnt, args) = \
		(void*)FASTIVA_METHOD_ADDR_STATIC(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args); }
	
#define FASTIVA_METHOD_STATIC_obsolete(acc, cnt, sig, ret_t, slot, fn, args) {				\
	pClass->FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args) = addr;				

