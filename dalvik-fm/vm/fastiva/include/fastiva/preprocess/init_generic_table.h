#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("init_generic_table.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD


void** cvt = ((void***)c2)[0];
void** class_vt = ((void***)java_lang_Class_C$::getRawStatic$())[0];
static const void* generic_vtable[] = {
	class_vt[0],	// scanInstance
	class_vt[1],	// clone
	class_vt[2],	// equals
	class_vt[3],	// finalize
	class_vt[4],	// hashCode
	class_vt[5],	// toString

#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
#define FASTIVA_END_OF_PREPROCESS	};	\
	*(void**)&g_class$[0]	= generic_vtable;
#else
#define FASTIVA_END_OF_PREPROCESS	};	\
	*(void**)&g_class$	= generic_vtable;
#endi


#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
	// IGNORE

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	cvt[FASTIVA_VIRTUAL_SLOT_OFFSET(FASTIVA_PREPROCESS_CLASS,ret_t,fn,cnt,args)],\

#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
	// IGNORE

#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
	(void*)FASTIVA_PREPROCESS_CLASS::fn,\


