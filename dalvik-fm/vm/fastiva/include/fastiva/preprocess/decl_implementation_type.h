
#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_implementation_type.h")
#endif

#define JPP_PREPROCESS_INHERITED_INTERFACES
#define JPP_PREPROCESS_INSTANCE_METHOD


#undef	JPP_CLASS
#define JPP_CLASS	FASTIVA_DECL_IMPLEMENTATION_CLASS

#define FASTIVA_END_OF_PREPROCESS		};	


#define JPP_INHERITS_INTERFACE(IFC)										\
	public: static const int FASTIVA_INTERFACE_TABLE_FIELD_NAME(IFC)[];


#define JPP_INHERITS_EMPTY_INTERFACE(IFC)									\
	public: enum { FASTIVA_INTERFACE_TABLE_FIELD_NAME(IFC) = FASTIVA_EMPTY_VTABLE };


#define JPP_INHERITS_BYTECODE_INTERFACE(IFC)		// ignore


#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	FASTIVA_DECL_VIRTUAL_SLOT(FASTIVA_PREPROCESS_CLASS, acc, ret_t, slot, cnt, args, ((int)c+1-i-i))\
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);


#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_DECL_METHOD_SLOT_OWNER_TYPE(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args); \
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);

#if FASTIVA_USE_CPP_VTABLE
#undef  FASTIVA_METHOD_OVERRIDE_EXPORT
#define FASTIVA_METHOD_OVERRIDE_EXPORT(acc, cnt, sig, ret_t, slot, fn, args)		\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn##_override$, cnt, args) {	\
		FASTIVA_RETURN_INSTRUCUTION_##ret_t	fn								\
			(FASTIVA_INSTANCE_PARAM(NAMES, self, cnt, args));						\
	}
#endif

#undef FASTIVA_METHOD_OVERRIDE_IFC
// slot 만 선언. slot_owner 는 변경하지 않는다.
#define FASTIVA_METHOD_OVERRIDE_IFC(acc, cnt, sig, ret_t, slot, fn, args)		\
	void* FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args); 

#define oooooooo \
	FASTIVA_DECL_VIRTUAL_SLOT(FASTIVA_PREPROCESS_CLASS, acc, ret_t, slot, cnt, args, ((int)0+1-0-0))\
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args) {	\
		typedef SUPER$::FASTIVA_METHOD_SLOT_OWNER_TYPE(ret_t, slot, cnt, args) TARGET; \
		FASTIVA_RETURN_INSTRUCUTION_##ret_t	TARGET::fn	\
			(FASTIVA_INSTANCE_PARAM(NAMES, self, cnt, args)); \
	}


#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	FASTIVA_DECL_VIRTUAL_SLOT(FASTIVA_PREPROCESS_CLASS, acc, ret_t, slot, cnt, args, 0)


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_DECL_METHOD_SLOT_OWNER_TYPE(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args); \
	FASTIVA_DECL_INSTANCE_METHOD(FASTIVA_PREPROCESS_CLASS, ret_t, fn, cnt, args);




