#ifndef __FASTIVA_CLASS_DEF_H__
#define __FASTIVA_CLASS_DEF_H__


#define FASTIVA_BEGIN_CLASS_DECL(CLASS, SUPER)	\
FASTIVA_CLASS CLASS : public SUPER {										\
	friend class CLASS##_C$;												\
	friend struct CLASS##_G$;												\
	public: struct VTABLE$;													\
	public: typedef struct SUPER##_G$ SUPER_G$;								\
	public: typedef class CLASS##_C$ STATIC$;								\
	public: static CLASS##_p ptr_cast$(CLASS##_p ptr) {	return ptr; }		\
	public: static const fastiva_ClassContext* getRawContext$() { 			\
		return (fastiva_ClassContext*)(void*)FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS);	\
	}																		



#define FASTIVA_DECL_INSTANCE_CLASS(CLASS, SUPER)				\
	FASTIVA_BEGIN_CLASS_DECL(CLASS, SUPER)						\
	public: static FOX_RESTRICT_API CLASS##_p allocate$() {					\
		return (CLASS##_p)fastiva.allocate(getRawContext$());				\
    }																		\
	FASTIVA_IMPL_INSTANCE_CLASS_EX(CLASS, SUPER)						



#define FASTIVA_DECL_ENUM_CLASS(CLASS, SUPER)					\
	FASTIVA_DECL_INSTANCE_CLASS(CLASS, SUPER)


#define FASTIVA_DECL_ABSTRACT_CLASS(CLASS, SUPER)				\
	FASTIVA_BEGIN_CLASS_DECL(CLASS, SUPER)						\
	private: static FOX_RESTRICT_API CLASS##_p allocate$();					\
	FASTIVA_IMPL_INSTANCE_CLASS_EX(CLASS, SUPER)						



#define FASTIVA_DECL_INTERFACE_CLASS(CLASS, SUPER)				\
	FASTIVA_BEGIN_CLASS_DECL(CLASS, SUPER)						\
	private: CLASS() {  }												\
	public: static CLASS##_p ptr_cast$(java_lang_Object_p ptr) {			\
		fastiva.checkImplemented(ptr,										\
			(fastiva_InterfaceContext*)getRawContext$());					\
		return (CLASS##_p)(void*)ptr;										\
	}																		\
	public: static CLASS##_p isImplemented$(java_lang_Object_p ptr) {		\
		const void** pIVT = fastiva.isImplemented(ptr,						\
			(fastiva_InterfaceContext*)getRawContext$());					\
		if (!pIVT) { ptr = (java_lang_Object_p)FASTIVA_NULL; }				\
		return (CLASS##_p)(void*)ptr;										\
	}																		\
	public: static CLASS##_p isImplemented$(CLASS##_p ptr) {				\
		return ptr;															\
	}																		\
	public: static CLASS##_p Null$() {										\
		return (CLASS##_p)(void*)FASTIVA_NULL;								\
	}																		\




#define FASTIVA_DECL_JAVA_PROXY_CLASS(CLASS, SUPER)							\
	FASTIVA_BEGIN_CLASS_DECL(CLASS, SUPER)									\
	public: static FOX_RESTRICT_API CLASS##_p allocate$() {					\
		return (CLASS##_p)&g_debugInstance;									\
    }																		\
	public: static void initStatic$();										\
	public: static CLASS##_p ptr_cast$(fastiva_BytecodeProxy_p p) {			\
		return (CLASS##_p)p;												\
	}																		\
	private: static void* g_javaClass;										\
	public: static void* getJavaClass();



#define FASTIVA_DECL_GENERIC_CLASS(CLASS, SUPER)					\
	FASTIVA_BEGIN_CLASS_DECL(CLASS, SUPER)						\
	public: static FOX_RESTRICT_API CLASS##_p allocate$() {					\
        return (CLASS##_p)fastiva.allocate(getRawContext$());					\
    }																		\
	FASTIVA_IMPL_INSTANCE_CLASS_EX(CLASS, SUPER)							\



#define FASTIVA_IMPL_INSTANCE_CLASS_EX(CLASS, SUPER)						\
	public: static CLASS##_p ptr_cast$(fastiva_Instance_p ptr) {			\
		if (ptr !=NULL && ptr->clazz != FASTIVA_RAW_CLASS_PTR(CLASS)) \
			fastiva.checkInstanceOf(ptr, getRawContext$());					\
		return (CLASS##_p)ptr;												\
	}																		\
	public: static CLASS##_p dbg_cast$(fastiva_Instance_p ptr) {				\
		FASTIVA_DEBUG_POINTER_CAST(ptr, getRawContext$());					\
		return (CLASS##_p)ptr;													\
	}																		\
	public: static CLASS##_p isInstance$(fastiva_Instance_p ptr) {				\
		if (fastiva.isInstanceOf(ptr, getRawContext$())) {					\
			return (CLASS##_p)ptr;												\
		}																	\
		return (CLASS##_p)FASTIVA_NULL;										\
	}																		\
	public: static CLASS##_p isInstance$(CLASS##_p ptr) {						\
		return ptr;															\
	}																		\
	public: static CLASS##_p Null$() {										\
		return (CLASS##_p)FASTIVA_NULL;										\
	}																		\

/////////////////////////////////////////////////////////////////////////////

#define FASTIVA_DECL_POINTER_CLASS(TYPE, CLASS, SUPER)						\
	FASTIVA_DECL_##TYPE##_CLASS(CLASS, SUPER)								\
	public: CLASS##_p as__##CLASS##$() { return this; }						\


#define FASTIVA_DECL_IMPLEMENTATION_CLASS(TYPE, CLASS, SUPER)				\
	FASTIVA_CLASS CLASS##_I$ : public SUPER::VTABLE$ {						\
		friend class CLASS;													\
		public:																\
		typedef class SUPER##_I$ SUPER$;									\
		typedef class CLASS##_I$ THIS$;										\
		typedef class CLASS##_C$ STATIC$;									\
		typedef class CLASS* THIS_p$;										\
		typedef struct CLASS##_G$ G$;										\
		static void scanInstance$(CLASS##_p self,							\
			FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner);			\
		FASTIVA_DECL_SCAN_JPROXY(CLASS)										\


// @zee 2011.0629 TODO operator new() 없에을 수 있는지 검토 : 상위에 하나만 사용
#define FASTIVA_DECL_STATIC_TYPE(TYPE, CLASS, SUPER)						\
	FASTIVA_BEGIN_META_CLASS_OF_##TYPE(CLASS)								\
	friend class CLASS;														\
	public: typedef class CLASS##_C$ STATIC$;								\
	public: typedef class SUPER##_I$ SUPER_I$;								\
	FASTIVA_RUNTIME_CRITICAL(private):										\
	public: void* operator new (size_t, void* ptr) { return ptr; }			\
	public: static void initStatic$();										\
	public: static CLASS##_C$* getRawStatic$() {							\
		return (CLASS##_C$*)FASTIVA_RAW_CLASS_PTR(CLASS);					\
	}																		\
	public: static CLASS##_C$* importClass$() {								\
		CLASS##_C$* pClass = getRawStatic$();								\
		if (pClass->status != CLASS_INITIALIZED) {							\
			fastiva.initClass_$$(pClass);									\
		}																	\
		return pClass;														\
	}																		\


#define FASTIVA_BEGIN_META_CLASS_OF_INTERFACE(CLASS)						\
	class CLASS##_C$ : public fastiva_InterfaceClass {			

#define FASTIVA_BEGIN_META_CLASS_OF_ENUM(CLASS)								\
	class CLASS##_C$ : public fastiva_Class {								\
		public: static void initVTable$(CLASS##_C$*);						\

#define FASTIVA_BEGIN_META_CLASS_OF_INSTANCE(CLASS)							\
	class CLASS##_C$ : public fastiva_Class {								\
		public: static void initVTable$(CLASS##_C$*);						\

#define FASTIVA_BEGIN_META_CLASS_OF_JAVAPROXY(CLASS)						\
	class CLASS##_C$ : public fastiva_BytecodeClass {

#define FASTIVA_BEGIN_META_CLASS_OF_GENERIC(CLASS)							\
	class CLASS##_C$ : public fastiva_Class {								\
		public: static void initVTable$(CLASS##_C$*);						\

#define FASTIVA_BEGIN_META_CLASS_OF_ABSTRACT(CLASS)							\
	class CLASS##_C$ : public fastiva_Class {								\
		public: static void initVTable$(CLASS##_C$*);						\

/////////////////////////////////////////////////////////////////////////////

#define JPP_INIT_INSTANCE_CLASS(CLASS)									\
	fastiva.linkClass_$$((void*)CLASS##_G$::g_vtable$, FASTIVA_RAW_CLASS_PTR(CLASS));\


#define JPP_INIT_INTERFACE_CLASS(CLASS)									\
	fastiva.linkClass_$$(0, FASTIVA_RAW_CLASS_PTR(CLASS));					\


/////////////////////////////////////////////////////////////////////////////
// interface를 상속한 interface는 getInterfaceTable$() 함수는 없이
// INTERFACE_TABLE 만을 가진다.

#define FASTIVA_EMPTY_VTABLE			0

#define FASTIVA_INTERFACE_TABLE_FIELD_NAME(IFC)								\
	ivt_##IFC##$

#define FASTIVA_INTERFACE_TABLE_NAME(IFC, CLASS)							\
	FASTIVA_MERGE_TOKEN(CLASS, _I$)::FASTIVA_INTERFACE_TABLE_FIELD_NAME(IFC)


#ifdef ANDROID
#define JPP_BEGIN_INHERITED_INTERFACE_LIST(CLASS)							\
	void* CLASS##_G$::aIVT$[] = {

#define JPP_INHERITED_INTERFACE_ENTRY(IFC, CLASS)							\
	FASTIVA_RAW_CLASS_PTR(IFC),	NULL,									

#define JPP_END_INHERITED_INTERFACE_LIST(CLASS)								\
	NULL };



#define JPP_BEGIN_IMPLEMENTED_INTERFACE_LIST(CLASS, cntOwnImpl)				\
	void* CLASS##_G$::aIVT$[] = {

#define JPP_IMPLEMENTED_EMPTY_INTERFACE_ENTRY(IFC, CLASS)					\
	(void*)FASTIVA_RAW_INTERFACE_CONTEXT_PTR(IFC),							\
	(void*)-1,

#define JPP_IMPLEMENTED_INTERFACE_ENTRY(IFC, CLASS)							\
	FASTIVA_RAW_CLASS_PTR(IFC),	\
	(void*)FASTIVA_INTERFACE_TABLE_NAME(IFC, CLASS),

#define JPP_END_IMPLEMENTED_INTERFACE_LIST(CLASS)							\
	NULL };

#else

#define JPP_BEGIN_INHERITED_INTERFACE_LIST(CLASS)							\
	const fastiva_ClassContext* const CLASS##_G$::aIVT$[] = {

#define JPP_INHERITED_INTERFACE_ENTRY(IFC, CLASS)							\
		FASTIVA_RAW_INTERFACE_CONTEXT_PTR(IFC),

#define JPP_END_INHERITED_INTERFACE_LIST(CLASS)								\
		0 };



#define JPP_BEGIN_IMPLEMENTED_INTERFACE_LIST(CLASS, cntOwnImpl)				\
	const void* CLASS##_G$::aIVT$[] = {										\

#define JPP_IMPLEMENTED_EMPTY_INTERFACE_ENTRY(IFC, CLASS)					\
	(void*)FASTIVA_RAW_INTERFACE_CONTEXT_PTR(IFC),							\
	(void*)-1,

#define JPP_IMPLEMENTED_INTERFACE_ENTRY(IFC, CLASS)							\
		(void*)FASTIVA_RAW_INTERFACE_CONTEXT_PTR(IFC),						\
		(void*)FASTIVA_INTERFACE_TABLE_NAME(IFC, CLASS),

#define JPP_END_IMPLEMENTED_INTERFACE_LIST(CLASS)							\
		0 };

#endif
/////////////////////////////////////////////////////////////////////////////

#if !FASTIVA_SUPPORTS_GENERIC_CAST
	#define FASTIVA_DEBUG_POINTER_CAST(PTR, CONTEXT)						\
		fastiva.checkInstanceOf(PTR, CONTEXT)
#elif defined(_DEBUG) 
	#define FASTIVA_DEBUG_POINTER_CAST(PTR, CONTEXT)						\
		fastiva.checkInstanceOf_dbg(PTR, CONTEXT)
#else
	#define FASTIVA_DEBUG_POINTER_CAST(PTR, CONTEXT)						\
		// ignore
#endif

#if FASTIVA_SUPPORT_JNI_STUB
	#define FASTIVA_DECL_SCAN_JPROXY(CLASS)										\
	static void scanJavaProxyFields$(CLASS##_p self, void* pEnv0, FASTIVA_JPROXY_SCAN_METHOD method);
#else
	#define FASTIVA_DECL_SCAN_JPROXY(CLASS)										\
		// ignore
#endif





#define FASTIVA_DECL_NON_AMBIGUOUS_CAST_OPERATOR(IFC, CLASS)				\
	operator IFC##_p () { return (IFC##_p)(CLASS##_p)((void**)(void*)this)[0]; }		


#define FASTIVA_IMPL_SUPER_TYPE_OF_ARRAY_OBJECT()							\
	struct fm::CheckArray$*												\
	get_base_pointer_$$(class fastiva_ArrayHeader*, void** pRef) {			\
		return (fm::CheckArray$*)(void*)(pRef[0] = this);				\
	}



#define FASTIVA_DECL_STATIC_FIELDS_EX(CLASS)								\


#ifdef _DEBUG
	#define FASTIVA_DEF_EOVF_OF_VIRTUAL_T(VIRTUAL_T)						\
		FOX_NAKED void VIRTUAL_T::eovf() {									\
			_asm xor ecx, ecx												\
			_asm xor ecx, ecx												\
		}																	
#else 
	#define FASTIVA_DEF_EOVF_OF_VIRTUAL_T(VIRTUAL_T)						\
		void VIRTUAL_T::eovf() {											\
			fastiva.eovf_$$();												\
		}																	
#endif



#define FASTIVA_IMPL_CLASS_IMPORT_FUNCTION(CLASS, SUPER)					\
	// nothing

#define FASTIVA_IMPL_SYSTEM_CLASS_IMPORT_FUNCTION(CLASS)					\
	FASTIVA_IMPL_CLASS_IMPORT_FUNCTION(CLASS, SUPER)


#if 1	
	/* 2007.08 newInstance는 JNI를 통해 구현한다. 대부분의 경우,
	   <init>함수는 method-table의 최상단에 위치하므로 Perfomance answpsms djqtek.
   */
	#define FASTIVA_DECL_NEW_INSTANCE_METHOD()
		// IGNORE
	#define FASTIVA_IMPL_NEW_INSTANCE_FUNCTION(CLASS, isPublic)
		// IGNORE

#else
	#define FASTIVA_DECL_NEW_INSTANCE_METHOD()								\
		private: java_lang_Object_p newInstance$(void* packageInfo)

	#define FASTIVA_IMPL_NEW_INSTANCE_FUNCTION(CLASS, isPublic)				\
		java_lang_Object_p CLASS##_C$::newInstance$(void* packageInfo) {	\
			if (!isPublic) {												\
				fastiva.checkInternalClass(packageInfo,					\
					FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS));					\
			}																\
			return FASTIVA_NEW(CLASS)();									\
		}
#endif

#define FASTIVA_UNKNOWN_CLASS_ACCESS(className, member, args, value_t)		\
	(fastiva.throwNoClassDefFoundError(className), (value_t)0)

#define FASTIVA_UNKNOWN_FIELD_ACCESS(CLASS, member, args, value_t)			\
	(fastiva.throwNoSuchFieldError(FASTIVA_RAW_CLASS_PTR(CLASS), member), (value_t)0)

#define FASTIVA_UNKNOWN_METHOD_CALL(CLASS, member, args, value_t)			\
	(fastiva.throwNoSuchMethodError(FASTIVA_RAW_CLASS_PTR(CLASS), member), (value_t)0)

#define FASTIVA_UNKNOWN_FIELD(...)  // IGNORE

#define FASTIVA_UNKNOWN_METHOD(...)  // IGNORE


#ifdef __INTERFACA_INHRITANCE__
1) C++ Multi-inherit를 사용한다.
	Java와 동일하게 Interface-Ptr와 Instance-Ptr가 동일한 값을 가지게 하면,
	Interface-함수 호출에 대한 overload가 매우 심해진다.
	Interface-ptr 변환이 단 한 번만 이루어져도 C++ inherit를 사용하는 부담은
	상쇄된다. 이로 인한 Instance의 크기 증가와 thunk 생성의 문제는 있으나
	performance 에 좀 더 초점을 맞춘다. 
	C++ Interface가 확실하게 빠르다!!!!!

2) Interface-Table
	단, Interface-Table은 기존의 virtual-table과는 달리, Fasiva에서 새로이
	생성한다. iinterface_thunk() template 을 사용하면 FOX dynamic-loading및
	interface-table 양식의 Compiler 종속 문제를 해결할 수 있다.

#endif

#endif // __FASTIVA_CLASS_DEF_H__
