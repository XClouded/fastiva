

#ifdef WIN32_MSG
	#pragma message ("--------- prolog " __FILE__)
#endif


#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	typedef 

#define FASTIVA_METHOD_CREATE(acc, cnt, sig, args)							\
	FASITVA_JNI_METHOD_INFO_##acc(acc, VAL(FASTIVA_VOID_p), init$, cnt, args, FINAL)

#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASITVA_JNI_METHOD_INFO_##acc(acc, ret_t, fn, cnt, args, VIRTUAL)

#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASITVA_JNI_METHOD_INFO_##acc(acc, ret_t, fn, cnt, args, FINAL)

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)			\
	// NOTHING

#define FASTIVA_METHOD_OVERRIDE_IFC(acc, cnt, sig, ret_t, slot, fn, args)			\
	FASITVA_JNI_METHOD_INFO_##acc(acc, ret_t, fn, cnt, args, VIRTUAL)


