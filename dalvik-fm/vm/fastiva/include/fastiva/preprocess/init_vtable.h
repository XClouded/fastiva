#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("init_vtable.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
	void** vtable = (void**)(void*)FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _G$)::g_vtable$;

#ifndef ANDROID
#if FASTIVA_SUPPORT_JNI_STUB
	#define FASTIVA_END_OF_PREPROCESS										\
		vtable->scanInstance_$$ =										\
			(void*)FASTIVA_PREPROCESS_CLASS::VTABLE$::scanInstance$;		\
		vtable->scanJavaProxyFields_$$ =									\
			(void*)FASTIVA_PREPROCESS_CLASS::VTABLE$::scanJavaProxyFields$; 
#else
	#define FASTIVA_END_OF_PREPROCESS										\
		vtable->scanInstance_$$ =											\
			(void*)FASTIVA_PREPROCESS_CLASS::VTABLE$::scanInstance$;		
#endif
#endif

#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)	{	\
	typedef FASTIVA_RETURN_TYPE_##ret_t (*FN)(FASTIVA_INSTANCE_PARAM(TYPES, \
		FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _p), cnt, args));		\
	vtable[FASTIVA_VIRTUAL_SLOT_OFFSET(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args)] = \
		(void*)(FN)FASTIVA_METHOD_ADDR_NON_VIRTUAL(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args); }


#undef FASTIVA_METHOD_ABSTRACT
#define FASTIVA_METHOD_ABSTRACT(acc, cnt, sig, ret_t, slot, fn, args)	{	\
	vtable[FASTIVA_VIRTUAL_SLOT_OFFSET(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args)] = \
		(void*)fastiva.throwAbstractMethodError; }

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)

#if FASTIVA_USE_CPP_VTABLE
#undef  FASTIVA_METHOD_OVERRIDE_EXPORT
#define FASTIVA_METHOD_OVERRIDE_EXPORT(acc, cnt, sig, ret_t, slot, fn, args)		\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)			\
	{ typedef FASTIVA_RETURN_TYPE_##ret_t (*FN)(FASTIVA_INSTANCE_PARAM(TYPES, \
		FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _p), cnt, args));		\
		vtable[FASTIVA_VIRTUAL_SLOT_OFFSET(FASTIVA_PREPROCESS_CLASS, ret_t, slot, cnt, args)] = \
		(void*)(FN)FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _I$)::fn##_override$); }
#endif

#undef FASTIVA_METHOD_OVERRIDE_IFC
#define FASTIVA_METHOD_OVERRIDE_IFC(acc, cnt, sig, ret_t, slot, fn, args)			\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)


#define FASTIVA_METHOD_INTERFACE(acc, cnt, IFC, ret_t, slot, fn, args)			\
	"error!! invalid-method-type"


#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	// ignore


