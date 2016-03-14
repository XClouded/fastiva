#include <fastiva/preprocess/prolog.h>

#ifdef WIN32_MSG
	#pragma message ("--------- prolog " __FILE__)
#endif

#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD

#undef  FASTIVA_METHOD_OVERRIDE_IFC
#define FASTIVA_METHOD_OVERRIDE_IFC(acc, cnt, acc_ex, ret_t, slot, fn, args)		\
	&& not implemented && // 상위 class에 상속된 Interface 함수가 이미 구현된 경우.

//#undef  FASTIVA_METHOD_OVERRIDE_ACC
//#define FASTIVA_METHOD_OVERRIDE_ACC(acc, cnt, acc_ex, ret_t, slot, fn, args)		\
//	&& not implemented && // 상위 메쏘드의 access-flag만 수정.

#ifndef FASTIVA_PROXY_THUNK_H__
#define FASTIVA_PROXY_THUNK_H__
	#define FASTIVA_INSTANCE_PROXY_THUNK(CLASS, ret_t, fn, cnt, args) \
		FASTIVA_RETURN_TYPE_##ret_t FASTIVA_MERGE_TOKEN(CLASS, _I$)::fn \
				(FASTIVA_INSTANCE_PARAM(PAIRS, CLASS* self, cnt, args)) { \
			FASTIVA_MERGE_TOKEN(CLASS, _EXVT$)* vtable = FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_vtable$; \
			void* pfn = vtable->FASTIVA_METHOD_SLOT_NAME(ret_t, fn, cnt, args);	\
			typedef FASTIVA_RETURN_TYPE_##ret_t (*FUNC_T) (		\
				FASTIVA_INSTANCE_PARAM(TYPES, void*, cnt, args));	\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t	((FUNC_T)pfn) \
				(FASTIVA_INSTANCE_PARAM(NAMES, self, cnt, args)); \
		} 

	#define FASTIVA_STATIC_PROXY_THUNK(CLASS, ret_t, fn, cnt, args) \
		FASTIVA_RETURN_TYPE_##ret_t CLASS::fn args { \
			FASTIVA_MERGE_TOKEN(CLASS, _EXVT$)* vtable = FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_vtable$; \
			void* pfn = vtable->FASTIVA_METHOD_SLOT_NAME(ret_t, fn##_S$, cnt, args);	\
			typedef FASTIVA_RETURN_TYPE_##ret_t (*FUNC_T) (		\
				FASTIVA_INSTANCE_PARAM(TYPES, void*, cnt, args));	\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t	((FUNC_T)pfn) args; \
		} 
#endif // FASTIVA_PROXY_THUNK_H__

#define FASTIVA_GET_CLASS_NAME(CLASS)	CLASS

#define FASTIVA_METHOD_VIRTUAL(acc, cnt, acc_ex, ret_t, slot, fn, args)		\
	FASTIVA_INSTANCE_PROXY_THUNK(FASTIVA_THIS_CLASS, ret_t, fn, cnt, args)

#define FASTIVA_METHOD_OVERRIDE(acc, cnt, acc_ex, ret_t, slot, fn, args)			\
	FASTIVA_INSTANCE_PROXY_THUNK(FASTIVA_THIS_CLASS, ret_t, fn, cnt, args)

#define FASTIVA_METHOD_INTERFACE(acc, cnt, acc_ex, ret_t, slot, fn, args) 			\
	// IGNORE

#define FASTIVA_METHOD_FINAL(acc, cnt, acc_ex, ret_t, slot, fn, args) 					\
	FASTIVA_INSTANCE_PROXY_THUNK(FASTIVA_THIS_CLASS, ret_t, fn, cnt, args)

#define FASTIVA_METHOD_STATIC(acc, cnt, acc_ex, ret_t, slot, fn, args)				\
	FASTIVA_STATIC_PROXY_THUNK(FASTIVA_THIS_CLASS, ret_t, fn, cnt, args)



