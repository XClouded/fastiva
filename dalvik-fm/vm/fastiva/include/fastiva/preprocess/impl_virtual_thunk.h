#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("impl_virtual_thunk.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD


	class VIRTUAL_T : public FASTIVA_PREPROCESS_CLASS { 

#define FASTIVA_END_OF_PREPROCESS		};

//

#define FASTIVA_FIELD(acc, name, type, cls, accFlag)			\
	// IGNORE

#define FASTIVA_FIELD_CONSTANT(acc, name, type, cls, accFlag)	\
	// IGNORE

#define FASTIVA_FIELD_STATIC(acc, name, type, cls, accFlag)	\
	// IGNORE



#define FASTIVA_METHOD_VIRTUAL(acc, cnt, accFlag, ret_t, slot, fn, args)		\
	// IGNORE

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, accFlag, ret_t, slot, fn, args)				\
	// IGNORE

#define FASTIVA_METHOD_INTERFACE(acc, cnt, accFlag, ret_t, slot, fn, args)			\
	"error!! invalid-method-type"

#define FASTIVA_METHOD_FINAL(acc, cnt, accFlag, ret_t, slot, fn, args)				\
	FASTIVA_IMPL_NON_VIRTUAL_THUNK(ret_t, fn, cnt, args)

#define FASTIVA_METHOD_STATIC(acc, cnt, accFlag, ret_t, slot, fn, args)				\
	FASTIVA_IMPL_STATIC_VIRTUAL_THUNK(ret_t, fn, cnt, args) && obsolete



