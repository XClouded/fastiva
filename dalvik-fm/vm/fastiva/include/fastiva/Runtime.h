#ifndef __FASTIVA_FASTVM_H__
#define __FASTIVA_FASTVM_H__

#include <fastiva/Config.h>
#include <fastiva/preprocess/util.h>
#include <fastiva/Types.h>
#include <fastiva/preprocess/params.h>

#define FASTIVA_STRING_POOL		FASTIVA_MODULE_NAME(stringPool)

#ifdef FASTIVA_PRELOAD_STRING_CONSTANTS
	#define PSTR$(idx, str)	FASTIVA_STRING_POOL[idx]
#else
	#define PSTR$(idx, str)	fastiva.getImmortalString(&FASTIVA_MODULE_NAME(ModuleInfo), idx)
#endif

extern java_lang_String_p FASTIVA_STRING_POOL[];

struct fastiva_CallStack;
struct fastiva_CallerClassStack;

#if (FASTIVA_BUILD_TARGET == FASTIVA_BUILD_TARGET_RUNTIME_KERNEL) 
	#define FASTIVA_DECL_VM_RUNTIME_CLASS_NAME fastiva_Runtime

	#define FASTIVA_DECL_VM_RUNTIME_API(ret_type, name, cnt, attr, params) \
		static attr ret_type name (FASTIVA_PARAM_PAIRS_##cnt params); 

	#define FASTIVA_DECL_VM_RUNTIME_API_VOID(name, cnt, attr, params) \
		FASTIVA_DECL_VM_RUNTIME_API(void, name, cnt, attr, params)


	#define FASTIVA_DECL_VM_RUNTIME_EXTRA() \
		static fastiva_PrimitiveClass[fastiva_Primitives:cntType][4] primitiveClasses;
		

	#include <fastiva/RuntimeAPI.h>

#endif

#if 1
#ifdef __GNUC__
	#define FASTIVA_GNUC_ATTR(attr, garbage)	attr
#else
	#define FASTIVA_GNUC_ATTR(attr, garbage)	
#endif

	#define FASTIVA_DECL_VM_RUNTIME_CLASS_NAME fastiva_RuntimeProxy 

	#define FASTIVA_DECL_VM_RUNTIME_API_VOID(name, cnt, attr, params) \
		void FASTIVA_GNUC_ATTR(attr, 0) (*m_##name) params; \
		inline static attr void name(FASTIVA_PARAM_PAIRS_##cnt params) { \
			g_instance.m_##name(FASTIVA_PARAM_NAMES_##cnt params); \
		}

	#define FASTIVA_DECL_VM_RUNTIME_API(ret_type, name, cnt, attr, params) \
		ret_type (*m_##name) params; \
		inline static attr ret_type name (FASTIVA_PARAM_PAIRS_##cnt params) { \
			return g_instance.m_##name(FASTIVA_PARAM_NAMES_##cnt params); \
		}

	#include <fastiva/RuntimeAPI.h>

#endif

#if (FASTIVA_BUILD_TARGET == FASTIVA_BUILD_TARGET_RUNTIME_KERNEL) 
	extern fastiva_Runtime fastiva;
#else
	extern fastiva_RuntimeProxy fastiva;
#endif

extern int fastiva_AnnotationCache$[];
extern int fastiva_ParameterAnnotationCache$[];

#if (FASTIVA_BUILD_TARGET == FASTIVA_BUILD_TARGET_EXTERNAL_COMPONENT) 
	#define fastiva_ArgListPool$  fastiva_component_ArgListPool
#else
	#define fastiva_ArgListPool$  fastiva_libcore_ArgListPool
#endif

struct fastiva_CallStack {
	const char* m_strMethod;

	fastiva_CallStack(const char* strMethod) {
		this->m_strMethod = strMethod;
		fastiva.pushCallStack(this);
	}
	~fastiva_CallStack() {
		fastiva.popCallStack(this);
	}
};

#ifdef FASTIVA_DUMP_JAVA_CALL_LOG
	#define JPP_PROLOGUE(CLASS, fn, cnt, args)	\
		fastiva_CallStack fastiva_call_stack$$ (__FUNCTION__);
		//fastiva_CallStack fastiva_call_stack$$ (aMethodInfo_##CLASS##$ + CLASS##_jni_method_table_index$::FASTIVA_METHOD_SLOT_NAME(ret_t, fn, cnt, args));
#else
	#define JPP_PROLOGUE(CLASS, fn, cnt, args)	// IGNORE
#endif

#define JPP_PUSH_CALLER_CLASS$(pClass)  fastiva_CallerClassStack  fastiva_caller_stack$(pClass);

inline void FOX_NO_RETURN fastiva_throwNoClassDefFoundErr(const char* className) {
	fastiva.throwNoClassDefFoundError(className);
}


#endif // __FASTIVA_FASTVM_H__
