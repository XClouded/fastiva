#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("pointer_thunk.h")
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD

#ifndef JPP_PREPROCESS_POINTER_THUNK
	#define JPP_PREPROCESS_POINTER_THUNK
	#if 0 // (FASTIVA_BUILD_TARGET != FASTIVA_BUILD_TARGET_RUNTIME_LIBCORE) 
		#define FASTIVA_STATIC_VIRTUAL_METHOD_ADDR(CLASS, ret_t, fn, cnt, args)	\
			(*(FASTIVA_RETURN_TYPE_##ret_t (*) FASTIVA_STATIC_PARAM_TYPES(cnt, args))((FASTIVA_MERGE_TOKEN(CLASS, _EXVT$)*)obj.vtable$)->FASTIVA_METHOD_SLOT_NAME(ret_t, fn##_S$, cnt, args))
	#else
		#define FASTIVA_STATIC_VIRTUAL_METHOD_ADDR(CLASS, ret_t, fn, cnt, args)	\
			CLASS::fn 
	#endif

	#ifdef FASTIVA_STATIC_METHOD_HAS_THIS_ARG
	#define FASTIVA_STATIC_PARAM_TYPES(cnt, args) \
			(FASTIVA_INSTANCE_PARAM(TYPES, STATIC$*, cnt, args))
	#define FASTIVA_STATIC_PARAM_NAMES(cnt, args) \
			FASTIVA_INSTANCE_PARAM(NAMES, this, cnt, args)
	#else
	#define FASTIVA_STATIC_PARAM_TYPES(cnt, args) \
			args
	#define FASTIVA_STATIC_PARAM_NAMES(cnt, args) \
			FASTIVA_PARAM_NAMES_##cnt args
	#endif

	#define FASTIVA_INLINE_PROXY_THUNK(CLASS, ...) \
		// ignore

	#define FASTIVA_VIRTUAL_PROXY_THUNK(CLASS, ret_t, slot, fn, cnt, args) \
		inline FASTIVA_RETURN_TYPE_##ret_t FASTIVA_MERGE_TOKEN(CLASS, _I$)::fn \
				(FASTIVA_INSTANCE_PARAM(PAIRS, CLASS* self, cnt, args)) { \
			void** vtable = (void**)(void*)FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_vtable$; \
			void* pfn = vtable[FASTIVA_VIRTUAL_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args)]; \
			typedef FASTIVA_RETURN_TYPE_##ret_t (*FUNC_T) (		\
				FASTIVA_INSTANCE_PARAM(TYPES, void*, cnt, args));	\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t	((FUNC_T)pfn) \
				(FASTIVA_INSTANCE_PARAM(NAMES, self, cnt, args)); \
		} 

	#define FASTIVA_FINAL_PROXY_THUNK(CLASS, ret_t, slot, fn, cnt, args) \
		inline FASTIVA_RETURN_TYPE_##ret_t FASTIVA_MERGE_TOKEN(CLASS, _I$)::fn \
				(FASTIVA_INSTANCE_PARAM(PAIRS, CLASS* self, cnt, args)) { \
			FASTIVA_MERGE_TOKEN(CLASS, _EXVT$)* vtable = FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_vtable$; \
			void* pfn = vtable->FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args);	\
			typedef FASTIVA_RETURN_TYPE_##ret_t (*FUNC_T) (		\
				FASTIVA_INSTANCE_PARAM(TYPES, void*, cnt, args));	\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t	((FUNC_T)pfn) \
				(FASTIVA_INSTANCE_PARAM(NAMES, self, cnt, args)); \
		} 

	#define FASTIVA_STATIC_PROXY_THUNK(CLASS, ret_t, slot, fn, cnt, args) \
		inline FASTIVA_RETURN_TYPE_##ret_t CLASS##_C$::fn (FASTIVA_PARAM_PAIRS_##cnt args) { \
			FASTIVA_MERGE_TOKEN(CLASS, _EXVT$)* vtable = FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_vtable$; \
			void* pfn = vtable->FASTIVA_METHOD_SLOT_NAME(ret_t, slot##_S$, cnt, args);	\
			typedef FASTIVA_RETURN_TYPE_##ret_t (*FUNC_T)(FASTIVA_PARAM_TYPES_##cnt args);	\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t	((FUNC_T)pfn)(FASTIVA_PARAM_NAMES_##cnt args); \
		} 

	#define FASTIVA_STATIC_THUNK(CLASS, ret_t, slot, fn, cnt, args) \
		inline FASTIVA_RETURN_TYPE_##ret_t \
		FASTIVA_MERGE_TOKEN(CLASS, _C$)::fn (FASTIVA_PARAM_PAIRS_##cnt args) {	\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t	(FASTIVA_RETURN_TYPE_##ret_t) \
				FASTIVA_STATIC_VIRTUAL_METHOD_ADDR(CLASS, ret_t, fn, cnt, args) \
					(FASTIVA_STATIC_PARAM_NAMES(cnt, args)); \
		} \

#endif // JPP_PREPROCESS_POINTER_THUNK



#define FASTIVA_METHOD_OVERRIDE(acc, cnt, sig, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)

#define FASTIVA_METHOD_INTERFACE(acc, cnt, sig, ret_t, slot, fn, args)			\
	FASTIVA_INTERFACE_THUNK(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)

#if FASTIVA_IS_IN_PREPROCESS_LIBCORE_CLASS && FASTIVA_BUILD_TARGET != FASTIVA_BUILD_TARGET_RUNTIME_LIBCORE

	#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
		FASTIVA_VIRTUAL_THUNK(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args) \
		FASTIVA_JNI_VIRTUAL_PROXY_THUNK_##sig(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)

	#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
		FASTIVA_FINAL_THUNK(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args) \
		FASTIVA_JNI_FINAL_PROXY_THUNK_##sig(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)

	#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
		FASTIVA_JNI_STATIC_PROXY_THUNK_##sig(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)


#else

	#define FASTIVA_METHOD_VIRTUAL(acc, cnt, sig, ret_t, slot, fn, args)		\
		FASTIVA_VIRTUAL_THUNK(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)

	#define FASTIVA_METHOD_FINAL(acc, cnt, sig, ret_t, slot, fn, args)				\
		FASTIVA_FINAL_THUNK(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)

	#define FASTIVA_METHOD_STATIC(acc, cnt, sig, ret_t, slot, fn, args)				\
		FASTIVA_JNI_STATIC_THUNK_##sig(FASTIVA_PREPROCESS_CLASS, ret_t, slot, fn, cnt, args)


#endif // FASTIVA_IS_IN_PREPROCESS_LIBCORE_CLASS && FASTIVA_BUILD_TARGET == FASTIVA_BUILD_TARGET_EXTERNAL_COMPONENT



