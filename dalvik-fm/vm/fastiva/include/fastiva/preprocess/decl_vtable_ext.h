#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_vtable_ext.h")
#endif


#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD


class FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _EXVT$)
	: public FASTIVA_PREPROCESS_CLASS::VTABLE$ {
public:

#define FASTIVA_END_OF_PREPROCESS	};



#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)	\
	// IGNORE


#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)
	// IGNORE


#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args) \
	"error!! invalid-method-type"


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args) \
	FASTIVA_DECL_NON_VIRTUAL_SLOT(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args) \


#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args) \
	FASTIVA_DECL_STATIC_SLOT(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args) \






