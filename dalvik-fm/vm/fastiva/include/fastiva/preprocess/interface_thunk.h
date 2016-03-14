#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("interface_thunk.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD


#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	"error!! interface can't have non-abstract methods"

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! interface can't have non-abstract methods"

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	FASTIVA_INTERFACE_THUNK(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! interface can't have non-abstract methods"

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	"error!! interface can't have non-abstract methods"




