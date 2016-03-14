
#ifdef WIN32_MSG
	#pragma message ("--------- prolog " __FILE__)
#endif


//#define FASTIVA_METHOD_CREATE(acc, cnt, sig, args)							\
//	FASTIVA_JNI_CONSTRUCTOR_INFO(acc, cnt, args)

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_JNI_METHOD_INFO(acc, ret_t, fn, cnt, args, STATIC, sig)

#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_JNI_METHOD_INFO(acc, ret_t, fn, cnt, args, VIRTUAL, sig)

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE

#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_JNI_METHOD_INFO(acc, ret_t, fn, cnt, args, FINAL, sig)

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_JNI_NATIVE_FILTER_##acc(FASTIVA_JNI_METHOD_INFO_1)				\
		((acc, ret_t, fn, cnt, args, VIRTUAL, sig))




static const Fastiva::MethodInfo FASTIVA_REVERSE_MERGE(FASTIVA_REVERSE_MERGE(
								 $, FASTIVA_THIS_CLASS), aMethodInfo_)[] = {


