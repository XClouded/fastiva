#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_vtable.h")
#endif


#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD


struct FASTIVA_PREPROCESS_CLASS::VTABLE$
	: public FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _I$) {
public:

#define FASTIVA_END_OF_PREPROCESS	};



#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)	\
	enum { \
		FASTIVA_MERGE_TOKEN(OFFSET_, FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args)) = \
			FASTIVA_OFFSETOF(FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _I$), \
			FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args))  / sizeof(void*) \
	  };


#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)
	// IGNORE

#undef FASTIVA_METHOD_OVERRIDE_IFC
#define FASTIVA_METHOD_OVERRIDE_IFC(acc, cnt, sig, ret_t, slot, fn, args)			\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)


#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args) \
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args) 


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args) \
	// IGNORE


#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args) \
	// IGNORE


#undef  JPP_VIRTUAL_SLOT_ALIAS
#define JPP_VIRTUAL_SLOT_ALIAS(t, slot, ret_t, cnt, args, ret_org)	\
	enum { FASTIVA_MERGE_TOKEN(OFFSET_, FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args)) = \
		FASTIVA_MERGE_TOKEN(OFFSET_, FASTIVA_METHOD_SLOT_NAME(ret_org, slot, cnt, args)) \
	  };




