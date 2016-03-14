// Debug Message Output Options
// #define FASTIVA_SHOW_PREPROCESS_MESSAGE
// #define FASTIVA_SHOW_COMPILE_SOURCE

// inherited-interface method is overriden by super-final method
// or just add ACC_FINAL flag onto super virtual method
#undef  FASTIVA_METHOD_OVERRIDE_IFC
#define FASTIVA_METHOD_OVERRIDE_IFC(acc, cnt, acc_ex, ret_t, slot, fn, args)		
	// IGNORE

#undef  FASTIVA_METHOD_OVERRIDE_ACC
#define FASTIVA_METHOD_OVERRIDE_ACC(acc, cnt, acc_ex, ret_t, slot, fn, args)		
	// IGNORE


#undef  FASTIVA_METHOD_ABSTRACT_SYNTHETIC
#define FASTIVA_METHOD_ABSTRACT_SYNTHETIC(acc, cnt, acc_ex, ret_t, fn, real_ret_t, args)		
	// IGNORE


#undef  FASTIVA_METHOD_OVERRIDE_EXPORT
#if FASTIVA_USE_CPP_VTABLE
#define FASTIVA_METHOD_OVERRIDE_EXPORT FASTIVA_METHOD_VIRTUAL // export super package-private method via new virtual method (in same package)
#else
#define FASTIVA_METHOD_OVERRIDE_EXPORT FASTIVA_METHOD_OVERRIDE // override and export super package-private method (in same package)
#endif

#undef  FASTIVA_METHOD_ABSTRACT
#define FASTIVA_METHOD_ABSTRACT		FASTIVA_METHOD_VIRTUAL

#undef  FASTIVA_METHOD_CONSTRUCTOR
#define FASTIVA_METHOD_CONSTRUCTOR		FASTIVA_METHOD_FINAL

#undef  JPP_CLASS
#define JPP_CLASS(TYPE, CLASS, SUPER)	// IGNORE

#undef  JPP_VIRTUAL_SLOT_ALIAS
#define JPP_VIRTUAL_SLOT_ALIAS(t, slot, ret_new, cnt, args, ret_org)	
	// IGNORE
