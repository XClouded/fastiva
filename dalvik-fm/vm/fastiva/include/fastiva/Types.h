
#ifndef __FASTIVA_TYPES_H__
#define __FASTIVA_TYPES_H__

#include <fastiva/Config.h>

struct fastiva_InstanceContextInfo;
struct fastiva_InterfaceContextInfo;

struct ClassObject;

#if (FASTIVA_BUILD_TARGET == FASTIVA_BUILD_TARGET_RUNTIME_LIBCORE) 
	#define FASTIVA_RUNTIME_EXPORT	FOX_EXPORT_API
#else
	#define FASTIVA_RUNTIME_EXPORT	FOX_IMPORT_API
#endif

#define FASTIVA_CLASS	class FOX_NO_VTABLE

#define fastiva_Throwable_I$	java_lang_Object_I$
#define fastiva_ClassLoader_I$	java_lang_Object_I$ 
#define fastiva_Thread_I$		java_lang_Object_I$
#define fastiva_MetaClass_I$	java_lang_Object_I$

#define fastiva_Throwable_G$	java_lang_Object_G$
#define fastiva_ClassLoader_G$	java_lang_Object_G$ 

#define FASTIVA_TYPDEF_SYSTEM_STRUCT(T)										\
	struct T;																\
	typedef const T*	T##_cp;												\
	typedef T*	T##_p;														\

#define JPP_TYPEDEF_PRELOADED_POINTER(CLASS)								\
	class CLASS;										\
	typedef CLASS		CLASS##_T$;											\
	typedef CLASS##_T$*		CLASS##_p;										\

#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
#define FASTIVA_BEGIN_DECL_CLASS_CONTEXT_GLOBALS(CLASS, SUPER, Type)		\
	struct FASTIVA_RUNTIME_EXPORT CLASS##_G$ : public SUPER##_G$ {					\
		public: enum { inheritDepth$ = SUPER##_G$::inheritDepth$ + 1 };		\
		static class CLASS##_C$* g_class$[1];
#else
#define FASTIVA_BEGIN_DECL_CLASS_CONTEXT_GLOBALS(CLASS, SUPER, Type)		\
	struct FASTIVA_RUNTIME_EXPORT CLASS##_G$ : public SUPER##_G$ {									\
		public: enum { inheritDepth$ = SUPER##_G$::inheritDepth$ + 1 };		\
		static struct fastiva_RawClass g_class$[1];
#endif

#define FASTIVA_IVTABLE_ID(CLASS)										\
	FASTIVA_RAW_CLASS_PTR(CLASS)->ifc.itableId$

#define FASTIVA_BEGIN_DECL_INSTANCE_POINTER(CLASS, SUPER)					\
	JPP_TYPEDEF_PRELOADED_POINTER(CLASS)									\
	FASTIVA_BEGIN_DECL_CLASS_CONTEXT_GLOBALS(CLASS, SUPER, Instance)		\
		public: static class CLASS##_EXVT$ g_vtable$[1];									


#define FASTIVA_BEGIN_DECL_INTERFACE_POINTER(CLASS, SUPER)					\
	JPP_TYPEDEF_PRELOADED_POINTER(CLASS)									\
	FASTIVA_BEGIN_DECL_CLASS_CONTEXT_GLOBALS(CLASS, SUPER, Interface)		\


#define FASTIVA_END_DECL_POINTER(CLASS, CLASS_NAME, IFC_TABLE)				\
	FASTIVA_DECL_##IFC_TABLE##_TABLE()	};									\
	FASTIVA_DECL_CLASS_DESCRIPTOR(CLASS, CLASS_NAME)

#define FASTIVA_DECL_CLASS_DESCRIPTOR(CLASS, CLASS_NAME) \
	static const char FASTIVA_SIG_##CLASS##_aaaaaaap[] = "[[[[[[[L" CLASS_NAME ";"; \
	static const char * FASTIVA_SIG_##CLASS##_p = FASTIVA_SIG_##CLASS##_aaaaaaap + 7 - 0; \
	static const char * FASTIVA_SIG_##CLASS##_ap = FASTIVA_SIG_##CLASS##_aaaaaaap + 7 - 1; \
	static const char * FASTIVA_SIG_##CLASS##_aap = FASTIVA_SIG_##CLASS##_aaaaaaap + 7 - 2; \
	static const char * FASTIVA_SIG_##CLASS##_aaap = FASTIVA_SIG_##CLASS##_aaaaaaap + 7 - 3; \
	static const char * FASTIVA_SIG_##CLASS##_aaaap = FASTIVA_SIG_##CLASS##_aaaaaaap + 7 - 4; \
	static const char * FASTIVA_SIG_##CLASS##_aaaaap = FASTIVA_SIG_##CLASS##_aaaaaaap + 7 - 5; \
	static const char * FASTIVA_SIG_##CLASS##_aaaaaap = FASTIVA_SIG_##CLASS##_aaaaaaap + 7 - 6; \


/*
#define FASTIVA_SIG_java_lang_Short_4D "[[[[Ljava/lang/Short;"
#define FASTIVA_PARAM_TYPE_java_lang_Short_p 0
*/

#ifdef ANDROID
#define FASTIVA_DECL_EXTENDED_INTERFACE_TABLE()								\
	static void* aIVT$[];

#define FASTIVA_DECL_IMPLEMENTED_INTERFACE_TABLE()							\
	static void* aIVT$[];
#else
#define FASTIVA_DECL_EXTENDED_INTERFACE_TABLE()								\
	static fastiva_ClassContext* const aIVT$[];

#define FASTIVA_DECL_IMPLEMENTED_INTERFACE_TABLE()							\
	static void* aIVT$[];
#endif


#define FASTIVA_DECL_NO_INTERFACE_TABLE()									\
	// IGNORE : just inherits super class


#define FASTIVA_DECL_CUSTOM_POINTER(CLASS, CLASS_NAME)									\
	JPP_TYPEDEF_PRELOADED_POINTER(CLASS)									\
	static const char * FASTIVA_SIG_##CLASS##_p = ("[[[[L" CLASS_NAME ";" + 4);		\
	FASTIVA_DECL_CUSTOM_ARRAY(CLASS)


#define FASTIVA_DECL_UNKNOWN_POINTER(CLASS, CLASS_NAME)	\
	class CLASS : public java_lang_Object { int unused; };	\
	typedef CLASS		CLASS##_T$;	\
	typedef CLASS##_T$*		CLASS##_p;	\
	typedef java_lang_Object_G$	CLASS##_G$;	\
	FASTIVA_DECL_OBJECT_ARRAY(CLASS) \
	FASTIVA_DECL_CLASS_DESCRIPTOR(CLASS, CLASS_NAME)


#define FASTIVA_DECL_ARRAY_EX(ARRAY_T, CLASS, isArraySuperType)			\
	typedef ARRAY_T<CLASS, CLASS##_p, 1, isArraySuperType> CLASS##_A;		\
	typedef CLASS##_A* CLASS##_ap;		\
	typedef CLASS##_A* CLASS##_A_p;		\
	typedef ARRAY_T<CLASS, CLASS##_ap, 2, isArraySuperType> CLASS##_AA;		\
	typedef CLASS##_AA* CLASS##_aap;		\
	typedef CLASS##_AA* CLASS##_AA_p;		\
	typedef ARRAY_T<CLASS, CLASS##_aap, 3, isArraySuperType> CLASS##_AAA;		\
	typedef CLASS##_AAA* CLASS##_aaap;		\
	typedef CLASS##_AAA* CLASS##_AAA_p;		\
	typedef ARRAY_T<CLASS, CLASS##_aaap, 4, isArraySuperType> CLASS##_AAAA;	\
	typedef CLASS##_AAAA* CLASS##_aaaap;		\
	typedef CLASS##_AAAA* CLASS##_AAAA_p;		\
	typedef ARRAY_T<CLASS, CLASS##_aaaap, 5, isArraySuperType> CLASS##_AAAAA;		\
	typedef CLASS##_AAAAA* CLASS##_aaaaap;		\
	typedef CLASS##_AAAAA* CLASS##_AAAAA_p;		\
	typedef ARRAY_T<CLASS, CLASS##_aaaaap, 6, isArraySuperType> CLASS##_AAAAAA;		\
	typedef CLASS##_AAAAAA* CLASS##_aaaaaap;		\
	typedef CLASS##_AAAAAA* CLASS##_AAAAAA_p;		\
	typedef ARRAY_T<CLASS, CLASS##_aaaaaap, 7, isArraySuperType> CLASS##_AAAAAAA;	\
	typedef CLASS##_AAAAAAA* CLASS##_aaaaaaap;		\
	typedef CLASS##_AAAAAAA* CLASS##_AAAAAAA_p;		\

#define FASTIVA_DECL_OBJECT_ARRAY(CLASS)		\
	FASTIVA_DECL_ARRAY_EX(fastiva_PointerArray, CLASS, false)


#define FASTIVA_CHECK_NULL(pObj, CLASS_p)	(pObj == (CLASS_p)FASTIVA_NULL)

#define FASTIVA_CHECK_POINTER(ptr)											\
	if (ptr == FASTIVA_NULL) fastiva_throwNullPointerException()

// ZZZ
#define JPP_META_INTERFACE(IFC)				fastiva_Interface // IFC##_p

#define JPP_META_INTERFACE_p	OBSOLETE // fastiva_Interface_p

#define FASTIVA_PROXY_STATIC_REF(CLASS)

#define FASTIVA_STATIC_REF(CLASS)											\
	FOX_STATIC_PTR<CLASS##_C$, (CLASS##_G$::type$ & ACC_STATIC$) != 0> pStatic$##CLASS

#ifdef FASTIVA_USE_PREIMPORT_CLASSES
	#define FASTIVA_FAST_STATIC_REF(CLASS)	\
		FOX_STATIC_PTR<CLASS##_C$, false> pStatic$##CLASS
#else
	#define FASTIVA_FAST_STATIC_REF(CLASS)	\
		FASTIVA_STATIC_REF(CLASS)
#endif

#define FASTIVA_RAW_STATIC_REF(CLASS)											\
	java_lang_Class_p pRawStatic$##CLASS = FASTIVA_RAW_CLASS_PTR(CLASS)

#define FASTIVA_UNKNOWN_STATIC_REF(CLASS, CLASSNAME)	\
	FOX_UNKNOWN_STATIC_PTR pStatic$##CLASS(CLASSNAME)

#define FASTIVA_UNKNOWN_RAW_STATIC_REF(CLASS, CLASSNAME)	\
	FOX_UNKNOWN_STATIC_PTR pRawStatic$##CLASS(CLASSNAME)

#define JPublic		&& deprecated && //	public:
#define	JPackage	&& deprecated && public:
#define	JProtected	&& deprecated && public:
#define JPrivate	&& deprecated && private:

#if (FASTIVA_BUILD_TARGET >= FASTIVA_BUILD_TARGET_RUNTIME_LIBCORE)
#	define FASTIVA_MODULE_NAME(name)	fastiva_libcore_##name
#else
#	define FASTIVA_MODULE_NAME(name)	fastiva_component_##name
#endif

#if 0
#define JPP_REF_NAME__undefined$ 0
#define JPP_REF_NAME__init$$ 1
#define JPP_REF_NAME__initStatic$$ 2
#define JPP_REF_NAME__main_$ 3
#define JPP_REF_NAME__cntPredefinedID 4
#endif

#ifdef ANDROID
enum dalivk_ClassFlags {
    CLASS_ISFINALIZABLE$$        = (1<<31), // class/ancestor overrides finalize()
    CLASS_ISCLASS$$              = (1<<28), // class is *the* class Class

    CLASS_ISREFERENCE$$          = (1<<27), // class is a soft/weak/phantom ref
                                          // only ISREFERENCE is set --> soft
    CLASS_ISWEAKREFERENCE$$      = (1<<26), // class is a weak reference
    CLASS_ISFINALIZERREFERENCE$$ = (1<<25), // class is a finalizer reference
    CLASS_ISPHANTOMREFERENCE$$   = (1<<24), // class is a phantom reference

	CLASS_ISFASTIVACLASS$		 = (1<<22), // Fastiva Extension !!!		

    CLASS_ISOPTIMIZED$$          = (1<<17), // class may contain opt instrs
    CLASS_ISPREVERIFIED$$        = (1<<16), // class has been pre-verified
};
#endif

enum {
	ACC_DEFAULT$		=   0,

	ACC_PUBLIC$			=	0x0001,	// FMC

	ACC_PRIVATE$		=	0x0002,	// FMC

	ACC_PROTECTED$		=	0x0004,	// FMc : ACC_EXTERNAL$(Downloaded Class)
	ACC_EXTERNAL$		=   0x0004,	// === EX-CLASS-TYPE

	ACC_STATIC$			=	0x0008,	// FMC

	ACC_FINAL$			=	0x0010,	// FMC

	ACC_SYNCHRONIZED$	=	0x0020,	// -Mc : ACC_PHANTOM_REF$(c)

	ACC_VOLATILE$		=	0x0040,	// FMc : ACC_BRIDGE$(M) | ACC_SOFT_REF$(c)
	ACC_BRIDGE$         =   0X0040, // JDK 5.0 
#ifdef ANDROID
	ACC_PHANTOM_REF$	=	CLASS_ISREFERENCE$$ | CLASS_ISPHANTOMREFERENCE$$,
	ACC_SOFT_REF$		=	CLASS_ISREFERENCE$$,
	ACC_WEAK_REF$		=	CLASS_ISREFERENCE$$ | CLASS_ISWEAKREFERENCE$$,
	ACC_FINALIZER_REF$	=   CLASS_ISREFERENCE$$ | CLASS_ISFINALIZERREFERENCE$$,
	ACC_FINALIZE$		=	CLASS_ISFINALIZABLE$$,
	ACC_META_CLASS$		=   CLASS_ISCLASS$$,
#else
	ACC_PHANTOM_REF$	=	0x0020,	// === EX-CLASS-TYPE
	ACC_SOFT_REF$		=	0x0040,	// === EX-CLASS-TYPE
	ACC_WEAK_REF$		=	0x0060,	// === EX-CLASS-TYPE
	ACC_FINALIZE$		=	0x0080,	// === EX-CLASS-TYPE
#endif

	ACC_TRANSIENT$		=	0x0080,	// FMc : ACC_VARARGS$(M) | ACC_FINALIZE$(c)
	ACC_VARARGS$		=	0x0080, // JDK 5.0 

	ACC_NATIVE$			=	0x0100,	// -Mc : ACC_PRELOAD$(c)
	ACC_PRELOAD$		=   0x0100,	// === EX-CLASS-TYPE

	ACC_INTERFACE$		=	0x0200,	// f-C : interface ref or member method of interface
	ACC_OVERRIDE_RET$	=	0x0200,	// == EX-METHOD-TYPE

	ACC_ABSTRACT$		=	0x0400,	// -MC

	ACC_STRICTFP$		=	0x0800,	// fmC : ACC_MEMBER := !ACC_STATIC
	//ACC_MEMBER$			=	0x0800,	// === EX-FIELD/METHOD-type (ACC_MIRANDA for dalvik)

	ACC_SYNTHETIC$		=   0x1000, // -MC

	ACC_VIRTUAL$		=   0x2000, // -mC : ACC_ANNOTATION$(c)
	ACC_ANNOTATION$		=   0x2000, // JDK 5.0 
	ACC_ENUM$			=   0x4000, // FmC JDK 5.0 // and for enum-field marking flag.
	ACC_BYTECODES$		=   0x4000, // === EX-METHOD/CLASS-TYPE


	ACC_JAVA_PROXY$	    =  0x7FFFFFFF,
};

#define ACC_DECLARED$  (ACC_STATIC$ | ACC_MEMBER$)  && OBSOLETE &&
#define ACC_INHERITED$ 0x80000000 && OBSOLETE &&
#define ACC_INHERITED_PUBLIC$ (ACC_INHERITED$ | ACC_PUBLIC$) && OBSOLETE &&

typedef enum {
	ACC_EXPORT_FLGAS$		=	ACC_PUBLIC$ | ACC_PROTECTED$,
	ACC_CONSTRUCTOR_FLAGS$	=	ACC_PUBLIC$ | ACC_PROTECTED$ | ACC_PRIVATE$ | ACC_STRICTFP$, 
							// can't be a synchronized, native
	ACC_CLASS_TYPE_FLAGS$	=	ACC_PUBLIC$ | ACC_FINAL$ | ACC_INTERFACE$ | ACC_ABSTRACT$
							  | ACC_FINAL$ | ACC_ENUM$ | ACC_ANNOTATION$,
    ACC_FIELD_TYPE_FLAGS$   =   ACC_PUBLIC$ | ACC_PROTECTED$ | ACC_PRIVATE$ | ACC_STATIC$
							  | ACC_FINAL$ | ACC_VOLATILE$ | ACC_TRANSIENT$,
    ACC_METHOD_TYPE_FLAGS$  =   ACC_PUBLIC$ | ACC_PROTECTED$ | ACC_PRIVATE$ | ACC_STATIC$
							  | ACC_FINAL$ | ACC_NATIVE$ | ACC_SYNCHRONIZED$ | ACC_ABSTRACT$
							  | ACC_STRICTFP$,
} ACC_FLAGS$;


//=========================================================================//
#define java_lang_Object_PARAM_T$		java_lang_Object_p
//#define java_lang_Class_PARAM_T$		FASTIVA_PARAM_T(java_lang_Class)
//#define java_lang_String_PARAM_T$		FASTIVA_PARAM_T(java_lang_String)
//#define java_lang_Throwable_PARAM_T$	FASTIVA_PARAM_T(java_lang_Throwable)
//#define java_lang_Thread_PARAM_T$		FASTIVA_PARAM_T(java_lang_Thread)


//struct fastiva_Instance_G$;
//struct fastiva_Interface_G$;
//struct fastiva_Class_G$;


JPP_TYPEDEF_PRELOADED_POINTER(fastiva_Instance);

#define fastiva_Throwable		java_lang_Object

JPP_TYPEDEF_PRELOADED_POINTER(fastiva_ClassLoader);
JPP_TYPEDEF_PRELOADED_POINTER(fastiva_BytecodeProxy);
JPP_TYPEDEF_PRELOADED_POINTER(fastiva_Thread);
JPP_TYPEDEF_PRELOADED_POINTER(fastiva_PrimitiveClass)
JPP_TYPEDEF_PRELOADED_POINTER(fastiva_MetaClass)
JPP_TYPEDEF_PRELOADED_POINTER(fastiva_Class)

JPP_TYPEDEF_PRELOADED_POINTER(java_lang_Enum);
JPP_TYPEDEF_PRELOADED_POINTER(java_lang_Class);
JPP_TYPEDEF_PRELOADED_POINTER(java_lang_ClassLoader);
JPP_TYPEDEF_PRELOADED_POINTER(java_lang_String);
JPP_TYPEDEF_PRELOADED_POINTER(java_lang_Throwable);
JPP_TYPEDEF_PRELOADED_POINTER(java_lang_Thread);


JPP_TYPEDEF_PRELOADED_POINTER(java_lang_Object);
JPP_TYPEDEF_PRELOADED_POINTER(java_lang_Cloneable);
JPP_TYPEDEF_PRELOADED_POINTER(java_io_Serializable);

typedef void* void_p;

class fastiva_Module;
struct JNI_HashEntry;
struct JNI_RawContext;
struct JNI_FindField;
struct JNI_FindMethod;
struct fastiva_PackageInfo;
struct JNI_HashEntry;
struct fastiva_Scanner;
struct fastiva_JniCallState;
struct fastiva_AnnotationItem;
//struct fox_MonitorContext;
//struct fox_NativeTaskContext;

class java_lang_Cloneable;
class java_io_Serializable;
class fastiva_Class;
class fastiva_SynchronizedLink;
class fastiva_ArrayHeader;
class fastiva_Class;
class fastiva_PrimitiveClass;
class fastiva_Instance;
class fastiva_Interface;
//class fm::WeakRef_R$;
class fastiva_Synchronize;

struct fastiva_JniCallInfo;
struct fastiva_PackageInfo;
#ifdef ANDROID
	struct Thread;
	#define fastiva_Task Thread
#else
	struct fastiva_Task;
#endif
struct fastiva_ImplementInfo;
struct JNI_HashEntry;

struct fastiva_InstanceContextInfo;
struct fastiva_InterfaceContextInfo;

//struct fm::RAW_CONSTRUCTOR_PARA;

struct fastiva_NativeMethodInfo;

#ifdef ANDROID
#define fastiva_ClassContext		ClassObject
#define fastiva_InterfaceContext	ClassObject
#define fastiva_InstanceContext		ClassObject
#else
FASTIVA_TYPDEF_SYSTEM_STRUCT(fastiva_ClassContext)
FASTIVA_TYPDEF_SYSTEM_STRUCT(fastiva_InterfaceContext)
FASTIVA_TYPDEF_SYSTEM_STRUCT(fastiva_InstanceContext)
#endif

struct fastiva_VirtualContext;

struct fastiva_IVTable;
struct fastiva_AbstractTable;
struct fastiva_PrimitiveContext;
struct fastiva_ClassMoniker;
//struct fastiva_InstanceField;
//struct fm::InterfaceField;
//struct fm::ArrayField;
struct fastiva_ExceptionContext;
//struct fastiva_ArrayInfo;

struct fastiva_ConstantUcs16String;
struct fastiva_ConstantAsciiString;
struct fastiva_ConstantString;
struct fastiva_ProxyInfo;
//struct fastiva_AbstractMethodInfo;
//struct fm::AbstractMethod;

struct fastiva_Field;
struct fastiva_Method;
//struct fastiva_Param;
struct fastiva_JniInfo;
struct fastiva_Package;
struct fastiva_Runtime;

class fastiva_Module;

class fastiva_Rewinder;


class SkPaint;
class SkCanvas;
class SkMatrix;
class SkPath;
class SkShader;
class SkPathMeasure;
class SkTypeface;
class SkXfermode;
class SkPathMeasurePair;

#if FASTIVA_SUPPORTS_JAVASCRIPT
	enum PropertyID {};
	typedef class fastiva_lang_Generic*			fastiva_lang_Generic_p;
	typedef class com_wise_jscript_JsObject*	com_wise_jscript_JsObject_p;
	struct JsPrimitive;
	struct JsBinOperation;//typedef java_lang_String_p PropertyID;
	typedef JsBinOperation* JsBinOperation_p;//typedef java_lang_String_p PropertyID;
	typedef const fastiva_ClassContext* RawContext_p;

	class com_wise_jscript_JsPrimitive;
	class com_wise_jscript_JsGlobal;
	class JsLocalFrame;

	struct JsVariant;
	struct JsBinOperation;
	struct JsReflectionTable;

	class JsLocalFrame;
	class JsValueSet;
	class JsSlotMap;
	struct JavaCodeAttr;

	typedef JsVariant JsValueEx;

	enum ASSIGN_RESULT {
		UNKNOWN_PROPERTY,
		UNREMOVABLE_PROPERTY,
		UNWRITABLE_PROPERTY,
		ASSIGN_SUCCESS,
	};

#endif

struct fastiva_Primitives {
	enum {
		jbool, unicod, jfloat, jdouble,
		jbyte, jshort, jint, jlonglong, 
		jcustom,
		jvoid,
		cntType,
		void_p = jvoid
	};
};

template <class T, bool initOnAccess> class FOX_STATIC_PTR {
private: 
	T* m_ptr;												
public: 
	FOX_STATIC_PTR() {
		if (!initOnAccess) {
			m_ptr = (T*)T::getRawStatic$();
		}
		else {
			m_ptr = ADDR_ZERO; 
		}
	}						
	
	T* operator->() {												
		if (initOnAccess && m_ptr == ADDR_ZERO) {
			this->m_ptr = (T*)T::importClass$();
			KASSERT(this->m_ptr == (T*)T::getRawStatic$());
		}
		return m_ptr;												
	}
};

template <class T> class FOX_RAW_STATIC_PTR {
private: 
	T* m_ptr;
public: 
	FOX_RAW_STATIC_PTR() { 
		m_ptr = (T*)T::getRawStatic$();
		KASSERT(this->m_ptr != ADDR_ZERO);
	}

	operator java_lang_Class_p () {
		return m_ptr;												
	}

	java_lang_Class_p operator->() {
		return m_ptr;												
	}
};


void FOX_NO_RETURN fastiva_throwNoClassDefFoundErr(const char* className);

class FOX_UNKNOWN_STATIC_PTR {
	const char* m_name;
public: 
	FOX_UNKNOWN_STATIC_PTR(const char* name) { 
		this->m_name = name;
	}

	operator java_lang_Class_p () {
		fastiva_throwNoClassDefFoundErr(m_name);	
	}

	java_lang_Class_p operator->() {
		fastiva_throwNoClassDefFoundErr(m_name);	
	}
};


union fastiva_Convert64 {
	u4 arg[2];
	s8 ll;
	double dd;
};

union fastiva_Convert32 {
	u4 arg;
	float ff;
};


/*
#define FASTIVA_CLASS_ID_START		0xF0000000 // = end of protoIdx
#define FASTIVA_DECL_PRIMITIVE_CLASS(type, jtype) \
	struct type##_G$ { \
		enum { g_class$ = FASTIVA_PROTO_START | (fastiva_Primitives::jtype << 4) }; \
	};

FASTIVA_DECL_PRIMITIVE_CLASS(Byte, jbyte)
FASTIVA_DECL_PRIMITIVE_CLASS(Bool, jbool)
FASTIVA_DECL_PRIMITIVE_CLASS(Short, jshort)
FASTIVA_DECL_PRIMITIVE_CLASS(Unicod, unicod)
FASTIVA_DECL_PRIMITIVE_CLASS(Int, jint)
FASTIVA_DECL_PRIMITIVE_CLASS(Longlong, jlonglong)
FASTIVA_DECL_PRIMITIVE_CLASS(Float, jfloat)
FASTIVA_DECL_PRIMITIVE_CLASS(Double, jdouble)
FASTIVA_DECL_PRIMITIVE_CLASS(Void, jvoid)
*/

typedef void (*FASTIVA_SCAN_METHOD)(fastiva_Instance_p, fastiva_Scanner* scanner);
typedef void (*FASTIVA_JPROXY_SCAN_METHOD)(void* pEnv, fastiva_BytecodeProxy_p* pProxy);

#define FASTIVA_MAX_ARRAY_DIMENSION 7

#define FASTIVA_JNI_ARRAY_DIMENSION_BITS(dimension)		((dimension) << 28)

#define FASTIVA_PACKAGE_INFO_NAME(PACKAGE)										\
	PACKAGE##_Package$

#define JPP_DECL_PACKAGE_INFO(PACKAGE)											\
	extern const fastiva_PackageInfo FASTIVA_PACKAGE_INFO_NAME(PACKAGE);

#endif // __FASTIVA_TYPES_H__
