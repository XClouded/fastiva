#ifndef __FASTIVA_CLASS_CONTEXT_H__
#define __FASTIVA_CLASS_CONTEXT_H__


#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
    typedef fastiva_Class_p* FASTIVA_CLASS_SLOT;
#else 
    typedef fastiva_Class_p FASTIVA_CLASS_SLOT;
#endif

#define FASTIVA_DECL_PRIMITIVE_TYPE_INFOS()										\
	const char* m_pBaseName;													\
	FASTIVA_CLASS_SLOT m_pClass0;													\
	jshort m_accessFlags;														\
	ushort m_inheritDepth; 														\
	const fastiva_Package* m_pPackage;											\

	// 반드시 m_pBaseName으로 시작한다. (TokenSlot 처리를 위해서)
	// m_accessFlags: RawContext인 경우 음수값을 갖는다.
	// m_inheritDepth: Class 상속 깊이와 primitiveTypeID 저장을 위해서도 쓰인다.


#define FASTIVA_DECL_COMMON_CLASS_INFOS()										\
	FASTIVA_DECL_PRIMITIVE_TYPE_INFOS()											\
	const struct fastiva_JniInfo* m_pJNI;										\
	ushort m_cntOwnIFC;															\
	ushort m_cntVirtualMethod;													\
	void (*initVTable$)(void*);												\
	void (*initStatic$)();														\
	ushort m_sizStatic;															\
	ushort m_annotationsId;														\
	const char* m_genericSig;



#define FASTIVA_DECL_INSTANCE_CLASS_INFOS()										\
	const fastiva_ImplementInfo* m_aImplemented;								\
	ushort m_sizInstance;														\
	ushort m_reserved__;														\
	const void* m_pfnInit;														\
	fastiva_InstanceContext_cp m_pSuperContext;						

//  m_pSuperContext = InterfaceContextInfo.m_ppIFC[0] 와 같은 위치에 있어야 한다(?)


#ifdef EMBEDDED_RUNTIME 
	#define FASTIVA_CONTEXT_ACC_FLAG(CLASS)	(CLASS##_G$::type$ | CLASS_ISFASTIVACLASS$)
#else
	#define FASTIVA_CONTEXT_ACC_FLAG(CLASS)	(CLASS##_G$::type$ | ACC_EXTERNAL$ | CLASS_ISFASTIVACLASS$)
#endif

#define FASTIVA_GET_CLASS_ANNOTATION(CLASS) \
	(CLASS##_G$::annotations$ < 0 ? NULL : &fastiva_AnnotationCache$[CLASS##_G$::annotations$])

#define FASTIVA_INIT_COMMON_CONTEXT(PACKAGE, CLASS, STR_NAME, GEN_SIG, CVM)	\
		STR_NAME,	\
		(FASTIVA_CLASS_SLOT)(void*)&CLASS##_G$::g_class$, \
		(ushort)FASTIVA_CONTEXT_ACC_FLAG(CLASS),	\
		(ushort)CLASS##_G$::inheritDepth$,	\
		(const fastiva_Package*)(void*)&PACKAGE##_Package$,	\
		(const fastiva_JniInfo*)(void*)CLASS##_JNI_INFO$,	\
		(CLASS##_G$::cntOwnIFC), (ushort)CVM,	\
		(void(*)(void*))(void*)CLASS##_C$::initVTable$,	\
		(void(*)())(void*)CLASS##_C$::initStatic$,	\
		(ushort)sizeof(CLASS##_C$),	\
		FASTIVA_GET_CLASS_ANNOTATION(CLASS), GEN_SIG, 

#ifdef ANDROID
	#define SUPER_CLASS_OR_CONTEXT(CLASS)	&CLASS::SUPER_G$::g_class$
#else
	#define SUPER_CLASS_OR_CONTEXT(CLASS)	&CLASS::SUPER_G$::g_context$
#endif

#define JPP_INIT_INSTANCE_CONTEXT(PACKAGE, NAME, GEN_SIG)						\
	FASTIVA_INIT_INSTANCE_CONTEXT_$$(PACKAGE, PACKAGE##_##NAME, #NAME,			\
		0, 	SUPER_CLASS_OR_CONTEXT(PACKAGE##_##NAME), GEN_SIG)

#define JPP_INIT_ENUM_CONTEXT(PACKAGE, NAME, STR_NAME, GEN_SIG)					\
	static const void* NAME##_values$(PACKAGE##_##NAME##_C$* pClass) {			\
		return pClass->values_();												\
	}																			\
	FASTIVA_INIT_INSTANCE_CONTEXT_$$(PACKAGE, PACKAGE##_##NAME, STR_NAME,		\
		NAME##_values$, SUPER_CLASS_OR_CONTEXT(PACKAGE##_##NAME), GEN_SIG)

#define JPP_INIT_INSTANCE_CONTEXT_EX(PACKAGE, NAME, init$, STR_NAME, GEN_SIG)	\
	FASTIVA_INIT_INSTANCE_CONTEXT_$$(PACKAGE, PACKAGE##_##NAME,	STR_NAME,		\
		init$, SUPER_CLASS_OR_CONTEXT(PACKAGE##_##NAME), GEN_SIG)


#define JPP_INIT_ROOT_OBJECT_CONTEXT(PACKAGE, NAME, init$, STR_NAME)			\
	FASTIVA_INIT_INSTANCE_CONTEXT_$$(PACKAGE, PACKAGE##_##NAME,	STR_NAME,		\
		init$, 0, 0)

#define JPP_INIT_INTERFACE_CONTEXT(PACKAGE, NAME, GEN_SIG)						\
	JPP_INIT_INTERFACE_CONTEXT_EX(PACKAGE, NAME, #NAME, GEN_SIG)

#define JPP_INIT_INTERFACE_CONTEXT_EX(PACKAGE, NAME, STR_NAME, GEN_SIG)			\
	FASTIVA_INIT_INTERFACE_CONTEXT_$$(PACKAGE, PACKAGE##_##NAME, STR_NAME, GEN_SIG)




#ifdef ANDROID
#define FASTIVA_INIT_INSTANCE_CONTEXT_$$(PACKAGE, CLASS, STR_NAME, init$, super_context, GEN_SIG)	\
	FASTIVA_ALLOCATE_FUNC(CLASS)											\
	CLASS##_EXVT$ CLASS##_G$::g_vtable$[1];									\
	extern const fastiva_PackageInfo PACKAGE##_Package$;					\
	FASTIVA_INIT_RAW_CLASS(PACKAGE, CLASS, STR_NAME, init$, super_context, GEN_SIG)	
#else
#define FASTIVA_INIT_INSTANCE_CONTEXT_$$(PACKAGE, CLASS, STR_NAME, init$, super_context, GEN_SIG)	\
	FASTIVA_ALLOCATE_FUNC(CLASS)											\
	CLASS##_EXVT$ CLASS##_G$::g_vtable$[1];									\
	extern const fastiva_PackageInfo PACKAGE##_Package$;					\
	const fastiva_InstanceContextInfo CLASS##_G$::g_context$ = {			\
		FASTIVA_INIT_COMMON_CONTEXT(PACKAGE, CLASS, STR_NAME, GEN_SIG,		\
		sizeof(CLASS##_I$) / sizeof(int))									\
		(fastiva_ImplementInfo*)(void*)CLASS##_G$::aIVT$, 					\
		(ushort)sizeof(CLASS), 0, (void*)init$,	\
		(fastiva_InstanceContext*)(void*)super_context,						\
	}; \
	FASTIVA_INIT_RAW_CLASS(PACKAGE, CLASS, STR_NAME, init$, super_context, GEN_SIG)	
#endif


#ifdef ANDROID
#define FASTIVA_INIT_INTERFACE_CONTEXT_$$(PACKAGE, CLASS, STR_NAME, GEN_SIG)	\
	extern const fastiva_PackageInfo PACKAGE##_Package$;					\
	FASTIVA_INIT_RAW_CLASS(PACKAGE, CLASS, STR_NAME, init$, &java_lang_Object_G$::g_class$, GEN_SIG)	
#else
#define FASTIVA_INIT_INTERFACE_CONTEXT_$$(PACKAGE, CLASS, STR_NAME, GEN_SIG)	\
	extern const fastiva_PackageInfo PACKAGE##_Package$;					\
	const fastiva_InterfaceContextInfo CLASS##_G$::g_context$ = { 			\
		FASTIVA_INIT_COMMON_CONTEXT(PACKAGE, CLASS, STR_NAME, GEN_SIG,		\
		sizeof(CLASS##_I$) / sizeof(int))								\
		FASTIVA_IVTABLE_ID(CLASS),											\
		(const fastiva_ClassContext**)CLASS##_G$::aIVT$ }; \
	FASTIVA_INIT_RAW_CLASS(PACKAGE, CLASS, STR_NAME, init$, &java_lang_Object_G$::g_class$, GEN_SIG)	
#endif
/*
		STR_NAME,	\
		(FASTIVA_CLASS_SLOT)(void*)&CLASS##_G$::g_class$, \
		(ushort)FASTIVA_CONTEXT_ACC_FLAG(CLASS),	\
		(ushort)CLASS##_G$::inheritDepth$,	\
		(const fastiva_Package*)(void*)&PACKAGE##_Package$,	\
		(const fastiva_JniInfo*)(void*)CLASS##_JNI_INFO$,	\
		(CLASS##_G$::cntOwnIFC), (ushort)CVM,	\
		(void(*)(void*))(void*)CLASS##_C$::initVTable$,	\
		(void(*)())(void*)CLASS##_C$::initStatic$,	\
		(ushort)sizeof(CLASS##_C$),	\
		FASTIVA_GET_CLASS_ANNOTATION(CLASS), GEN_SIG, 
*/


#ifndef FASTIVA_USE_PREIMPORT_CLASSES
	#define JPP_PREIMPORTED_CLASS_LIST_NAME_WITH_COMMA(CLASS)	// IGNORE
	#define	JPP_EMPTY_PREIMPORTED_CLASSES_IN_INSTANCE(CLASS)	// IGNORE
	#define	JPP_BEGIN_PREIMPORTED_CLASSES_IN_INSTANCE(CLASS)	// IGNORE
	#define	JPP_PREIMPORTED_CLASS_IN_INSTANCE(CLASS)			// IGNORE
	#define	JPP_END_PREIMPORTED_CLASSES_IN_INSTANCE(CLASS)		// IGNORE
	#define	JPP_PREIMPORT_CLASS(CLASS)							// IGNORE
#else
	#define JPP_PREIMPORTED_CLASS_LIST_NAME_WITH_COMMA(CLASS)	\
		CLASS##_precomportedClasses$,
	#define	JPP_EMPTY_PREIMPORTED_CLASSES_IN_INSTANCE(CLASS)	\
		static fastiva_Class_p* const CLASS##_precomportedClasses$ = 0;
	#define	JPP_BEGIN_PREIMPORTED_CLASSES_IN_INSTANCE(CLASS)	\
		const static fastiva_Class_p CLASS##_precomportedClasses$[] = {
	#define	JPP_PREIMPORTED_CLASS_IN_INSTANCE(CLASS)	\
			FASTIVA_RAW_CLASS_PTR(CLASS),
	#define	JPP_END_PREIMPORTED_CLASSES_IN_INSTANCE(CLASS)	\
			0 };
	#define	JPP_PREIMPORT_CLASS(CLASS)	\
		fastiva.preimportClass(CLASS##_C$::getRawStatic$());
#endif



#define FASTIVA_COMMENT_ZERO(v)	0

#define FASTIVA_VTABLE_COUNT(CLASS)	(sizeof(CLASS##_I$) / sizeof(int))
//(const int**)CLASS##_G$::aIVT$, 

#ifndef ANDROID 
#define FASTIVA_INIT_RAW_CLASS(PACKAGE, CLASS, STR_NAME, init$, super_context, GEN_SIG)	\
	FASTIVA_GLOBAL CLASS##_C$ CLASS##_G$::g_class$ __attribute__((aligned (8))) __attribute__ ((section ("FASTIVA_CLASSES")));
#else
#define FASTIVA_INIT_RAW_CLASS(PACKAGE, CLASS, STR_NAME, init$, super_class, GEN_SIG)	\
	FASTIVA_GLOBAL fastiva_RawClass CLASS##_G$::g_class$[] __attribute__((aligned (8))) __attribute__ ((section ("FASTIVA_CLASSES"))) = { { \
	FASTIVA_COMMENT(int* vtable)							(int*)(void*)&java_lang_Class_G$::g_vtable$, \
	FASTIVA_COMMENT(fastiva_Class_p    clazz)				(fastiva_Class_p)(void*)&java_lang_Class_G$::g_class$, \
	0, \
	0, \
	FASTIVA_COMMENT(const int*		obj.vtable$)			{ { (int*)(void*)CLASS##_G$::g_vtable$, \
	FASTIVA_COMMENT(const int**     obj.itables$)			  NULL} }, \
	FASTIVA_COMMENT(const fastiva_ClassContext*)			(void(*)())(void*)CLASS##_C$::initStatic$, \
    FASTIVA_COMMENT(const char*		descriptor)				FASTIVA_SIG_##CLASS##_p, \
	FASTIVA_COMMENT(char*           descriptorAlloc)		{(void(*)(void*))(void*)CLASS##_C$::initVTable$}, \
    FASTIVA_COMMENT(u4              accessFlags)			(u4)(FASTIVA_CONTEXT_ACC_FLAG(CLASS) | CLASS_ISOPTIMIZED$$ | CLASS_ISPREVERIFIED$$), \
    FASTIVA_COMMENT(u4				serialNumber)			0, \
    FASTIVA_COMMENT(DvmDex*         pDvmDex)				NULL, \
    FASTIVA_COMMENT(ClassStatus     status)					CLASS_NOTREADY, \
    FASTIVA_COMMENT(ClassObject*    verifyErrorClass)		NULL, \
    FASTIVA_COMMENT(u4              initThreadId)			0, \
	FASTIVA_COMMENT(size_t          objectSize)				CLASS##_G$::isInterface$ ? 0 : sizeof(CLASS), \
    FASTIVA_COMMENT(ClassObject*    elementClass)			NULL, \
    FASTIVA_COMMENT(int             arrayDim)				0, \
    FASTIVA_COMMENT(PrimitiveType   primitiveType)			PRIM_NOT, \
    FASTIVA_COMMENT(ClassObject*    super)					(ClassObject*)(void*)super_class, \
    FASTIVA_COMMENT(Object*         classLoader)			NULL, \
	FASTIVA_COMMENT(InitiatingLoaderList initiatingLoaderList) { 0, 0 }, \
	FASTIVA_COMMENT(int             interfaceCount)			CLASS##_G$::cntOwnIFC, \
	FASTIVA_COMMENT(ClassObject**   interfaces				(ClassObject**)(void*)CLASS##_G$::aOwnIFC$) \
	FASTIVA_COMMENT(int             directMethodCount)		FASTIVA_DECLARED_METHOD_COUNT(CLASS), \
    FASTIVA_COMMENT(Method*         directMethods--)		NULL, \
    FASTIVA_COMMENT(int             virtualMethodCount--)	0, \
	FASTIVA_COMMENT(Method*         virtualMethods--)		(Method*)(void*)aMethodInfo_##CLASS##$, \
    FASTIVA_COMMENT(int             vtableCount)			FASTIVA_VTABLE_COUNT(CLASS), \
    FASTIVA_COMMENT(Method**        vtable--)				NULL, \
    FASTIVA_COMMENT(int             iftableCount--)			0, \
    FASTIVA_COMMENT(InterfaceEntry* iftable)				(InterfaceEntry*)(void*)CLASS##_G$::aIVT$, \
	FASTIVA_COMMENT(int             ifviPoolCount)			{ FASTIVA_GET_CLASS_ANNOTATION(CLASS) }, \
	FASTIVA_COMMENT(int*            ifviPool)				{ CLASS##_JNI_INFO$ }, \
    FASTIVA_COMMENT(int             ifieldCount)			0, \
    FASTIVA_COMMENT(int             ifieldRefCount)			FASTIVA_COMMENT_ZERO(CLASS##_G$::--cntRefIField), \
	FASTIVA_COMMENT(InstField*      ifields)				(InstField*)(void*)FASTIVA_MERGE_3TOKEN(aFieldInfo_, CLASS, $), \
    FASTIVA_COMMENT(u4 refOffsets--)							0, \
	FASTIVA_COMMENT(const char*     sourceFile)				{ GEN_SIG }, \
    FASTIVA_COMMENT(int             sfieldCount)			FASTIVA_DECLARED_FIELD_COUNT(CLASS), \
	FASTIVA_COMMENT(StaticField*    sfields)				{ (StaticField*)0 }, \
	JPP_PREIMPORTED_CLASS_LIST_NAME_WITH_COMMA(CLASS) \
	FASTIVA_COMMENT(char    filler)							{ 0 } \
	} };
#endif


#undef FASTIVA_METHOD_DALVIK_OVERRIDE
#ifdef ANDROID
#define FASTIVA_METHOD_DALVIK_OVERRIDE FASTIVA_METHOD_OVERRIDE // override super package-private method (in another package)
#else
#define FASTIVA_METHOD_DALVIK_OVERRIDE FASTIVA_METHOD_VIRTUAL // can not override super package-private method (in another package)
#endif



#define FASTIVA_RAW_CLASS_PTR(CLASS)								\
	((fastiva_Class_p)(void*)&FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_class$)

#ifdef ANDROID
#define FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS)		FASTIVA_RAW_CLASS_PTR(CLASS)

#define FASTIVA_RAW_INTERFACE_CONTEXT_PTR(CLASS)	FASTIVA_RAW_CLASS_PTR(CLASS)
#else
#define FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS)								\
	((fastiva_ClassContext*)(void*)&FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_context$)						\

#define FASTIVA_RAW_INTERFACE_CONTEXT_PTR(CLASS)							\
	((fastiva_InterfaceContext*)(void*)&FASTIVA_MERGE_TOKEN(CLASS, _G$)::g_context$)					\

#endif




#define FASTIVA_DECL_INTERFACE_MONIKER(IFC, SLOT, INSTANCE)					\
//	static const int IFC##__##INSTANCE##_IVT$ = (int)INSTANCE::SLOT##_IVT$;

#if FASTIVA_NULL_ADDR == 0 
	#define FASTIVA_ALLOCATE_FUNC(CLASS)
#else
	#define FASTIVA_ALLOCATE_FUNC(CLASS)									\
		CLASS##_p CLASS::allocate$() {										\
			CLASS##_p pObj = (CLASS##_p)fastiva.allocate(getRawContext$());\
			return (CLASS##_p)(new (pObj)CLASS##_T$());						\
	}																		
#endif



#define JPP_DEFAULT_INIT_THUNK_NAME(CLASS)	CLASS##_defInit$

#define JPP_IMPL_DEFAULT_INIT_THUNK(CLASS)										\
	static const void* JPP_DEFAULT_INIT_THUNK_NAME(CLASS)(CLASS##_p pObj) {		\
		return pObj->init$();													\
	}



struct fastiva_RawClass;

typedef unsigned int FASTIVA_TOKEN_T;

/**
 InstanceContext와 InterfaceContext 크기가 달라 Slot을 사용.
 */
#ifndef ANDROID

struct fastiva_ImplementInfo {
#ifdef ANDROID
	fastiva_Class_p clazz;
#else
	const fastiva_InterfaceContext* m_pInterfaceContext;
#endif
	const int* m_pIVTable;
};



struct fastiva_ClassInfo	{
	FASTIVA_DECL_COMMON_CLASS_INFOS()
};


struct fastiva_InterfaceContextInfo {
	FASTIVA_DECL_COMMON_CLASS_INFOS()
	int ifc.itableId$;
	// Reflection 지원을 위해, 소스 상에서 implements 한 것과 동일한 순서와 값을 가지는
	// Interface-array를 지원한다.
	const fastiva_ClassContext** m_ppIFC;
};


struct fastiva_InstanceContextInfo {
	FASTIVA_DECL_COMMON_CLASS_INFOS()
	FASTIVA_DECL_INSTANCE_CLASS_INFOS()
};




#endif

#ifdef ANDROID

struct fastiva_FieldInfo {
	// Same field definitions with Dalvik InstField;
    ClassObject*    clazz;          
    const char*     name;
    const char*     signature;      
    u4              accessFlags;
	const char*		genericSig;
    void*           annotationsId;
	u8				byteOffset8;
};
#else
/**
 Constant Field라 하더라도 Class는 해당 value에 대한 static-field를 가지도록 한다.
 @todo JNI_core를 이에 맞게 수정할 것.
 */
struct fastiva_FieldInfo { // exported only
	FASTIVA_TOKEN_T m_name;
	unsigned short m_accessFlags; //  VM 연동을 위하여 m_accessFlags 반드시 필요. (interface 여부도 표시).
	unsigned short m_offset; // constant Context or field offset;
	FASTIVA_TOKEN_T m_type;
#ifdef ANDROID
	unsigned int m_idAnnotations;
#else
	const char* m_genericSig;
#endif
};
#endif

#ifndef ANDROID
struct fastiva_MethodInfo { 
	FASTIVA_TOKEN_T m_name;
	unsigned short m_accessFlags; 
	unsigned short m_offset; // offset from vtable or func-addr;
	FASTIVA_TOKEN_T m_retType;
	FASTIVA_TOKEN_T m_args;
	unsigned char m_cntArg;
	unsigned char m_argSize; // the stack size of paramters / size of stack-slot
	const char* m_genericSig;
	const fastiva_ClassContext* m_internal;
	void* m_jniBridge;
#ifdef ANDROID
	unsigned int m_idAnnotations;
#endif
#if defined(_DEBUG) && defined(_WIN32)
	const char* m_dbg_methodName;
#endif
};
#else
#define fastiva_MethodInfo	Method

#define FASTIVA_ARG_LIST_ID(dexProto)		((u4)(dexProto)->dexFile)

#define FASTIVA_RET_TTYPE_ID(dexProto)		((u4)(dexProto)->protoIdx)
//#define FASTIVA_ANNOTATION_ID(dexProto)	((u4)(dexProto)->dexFile & 0xFFFF)
#define FASTIVA_IS_FASTIVA_PROTO(dexProto)	((u4)(dexProto)->protoIdx >= FASTIVA_CLASS_ID_START)
//#define FASTIVA_IS_VALID_ANNOTATION_ID(id)	((s2)(id) != -1)
#endif

#ifndef ANDROID
struct fastiva_JniInfo { // exported only
	friend class ClassFileParser;
	friend struct fastiva_ClassContext;

public:
#if ANDROID
	const char* m_signature;
#endif
	short m_cntField;
	short m_cntMethod;

	const fastiva_Field* m_aField;
	const fastiva_Method* m_aMethod;
#if FASTIVA_SUPPORTS_REFLECTION
	const fastiva_ClassContext* m_pEnclosingClass;
	const fastiva_ClassContext* m_pDeclaredClasses[1];
#endif
};

#endif

#endif // __FASTIVA_CLASS_CONTEXT_H__
