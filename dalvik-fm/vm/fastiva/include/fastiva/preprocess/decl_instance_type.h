#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_instance_type.h")
#endif

#define JPP_PREPROCESS_INHERITED_INTERFACES
#define JPP_PREPROCESS_INSTANCE_DATA
#define JPP_PREPROCESS_INSTANCE_METHOD
#define JPP_PREPROCESS_STATIC_METHOD
#define JPP_PREPROCESS_SYSTEM_METHOD

#undef	JPP_CLASS
#define JPP_CLASS	FASTIVA_DECL_POINTER_CLASS
#define FASTIVA_END_OF_PREPROCESS			};



#define JPP_INHERITS_INTERFACE(IFC)										\
	public: IFC##_p as__##IFC##$() { return (IFC##_p)this; }				\


#define JPP_INHERITS_EMPTY_INTERFACE(IFC)									\
	public: IFC##_p as__##IFC##$() { return (IFC##_p)this; }				\


#define JPP_INHERITS_BYTECODE_INTERFACE(IFC)								\
	public: IFC##_p as__##IFC##$() {										\
		return (IFC##_p)(void*)this->as__BytecodeObject$();					\
	}


#define FASTIVA_FIELD(acc, name, type, cls, accFlag)							\
	FASTIVA_ACC_##acc FASTIVA_FIELD_TYPE_##type m_##name;						\
	FASTIVA_DECL_FIELD_##type(acc, FASTIVA_JNI_FLAG_JNI$ accFlag, FASTIVA_FIELD_ACCESS_TYPE_##type, m_##name, get__##name, set__##name)


#define FASTIVA_METHOD_VIRTUAL(acc, cnt, accFlag, ret_t, slot, fn, args)		\
	FASTIVA_ACC_##acc FASTIVA_VIRTUAL FASTIVA_RETURN_TYPE_##ret_t fn args;


// @TODO: If using SUPER$::fn 사용시에는 불필요할 수 있다.
#define FASTIVA_METHOD_OVERRIDE(acc, cnt, accFlag, ret_t, slot, fn, args)				\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, accFlag, ret_t, slot, fn, args)


#define FASTIVA_METHOD_INTERFACE(acc, cnt, accFlag, ret_t, slot, fn, args)			\
	FASTIVA_METHOD_VIRTUAL(acc, cnt, accFlag, ret_t, slot, fn, args)


#define FASTIVA_METHOD_FINAL(acc, cnt, accFlag, ret_t, slot, fn, args)				\
	FASTIVA_ACC_##acc FASTIVA_RETURN_TYPE_##ret_t  fn args;

#ifdef FASTIVA_STATIC_METHOD_HAS_THIS_ARG
#define FASTIVA_METHOD_STATIC(acc, cnt, accFlag, ret_t, slot, fn, args)				\
	FASTIVA_ACC_##acc static FASTIVA_RETURN_TYPE_##ret_t fn (FASTIVA_INSTANCE_PARAM(TYPES, STATIC$*, cnt, args));
#else
#define FASTIVA_METHOD_STATIC(acc, cnt, accFlag, ret_t, slot, fn, args)				\
	FASTIVA_ACC_##acc static FASTIVA_RETURN_TYPE_##ret_t fn args;
#endif


