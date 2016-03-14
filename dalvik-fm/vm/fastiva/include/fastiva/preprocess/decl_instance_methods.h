
#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_instance_methods.h")
#endif

#define JPP_PREPROCESS_INHERITED_INTERFACES
#define JPP_PREPROCESS_INSTANCE_DATA
#define JPP_PREPROCESS_INSTANCE_METHOD

/*
class FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _I$) : public FASTIVA_PREPROCESS_CLASS::SUPER_I$ {
	public:
	typedef FASTIVA_PREPROCESS_CLASS::SUPER_I$ SUPER$;
	typedef FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _I$) THIS$;
	typedef FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _C$) STATIC$;
	typedef struct FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _G$) G$;									

#define FASTIVA_END_OF_PREPROCESS		};
*/

#define FASTIVA_FIELD(acc, name, type, cls, jni_acc)			\
	// IGNORE

#define FASTIVA_FIELD_CONSTANT(acc, name, type, cls, jni_acc)	\
	// IGNORE

#define FASTIVA_FIELD_STATIC(acc, name, type, cls, jni_acc)	\
	// IGNORE


#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	"error!! interface methods"

#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE "error!! interface can't have non-abstract methods"




