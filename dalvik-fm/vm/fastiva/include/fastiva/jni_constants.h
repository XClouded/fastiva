#ifndef __FASTIVA_JNI_CONSTANTS_H__
#define __FASTIVA_JNI_CONSTANTS_H__

#include <fastiva/Types.h>

#define FASTIVA_GET_LIBCORE_PACKAGE_ID(PACKAGE)		&& OBSOLETE && (JPP_PACKAGE_ID::PACKAGE##$ << 16)
#define FASTIVA_GET_PACKAGE_ID(PACKAGE)				&& OBSOLETE && ((JPP_PACKAGE_ID::PACKAGE##$ << 16) + 0x8000)

#define JPP_EXTERNAL_PACKAGE_ID_START		JPP_PACKAGE_ID::LIBCORE_PACKAGE_ID_COUNT

#define JPP_EXTERNAL_REF_NAME_ID_START		JPP_REF_NAME_ID::LIBCORE_REF_NAME_ID_COUNT

#define JPP_LIBCORE_ARGLIST_ID_START		0x8000 // = SHORT_MIN (	16bit)

//#define FASTIVA_IS_VALID_ARGLIST_ID(id)		(id < FASTIVA_CLASS_ID_START)

#define JPP_EXTERNAL_ARGLIST_ID_START		&& OBSOLETE && JPP_ARGLIST_ID::LIBCORE_ARGLIST_ID_COUNT

#ifdef ANDROID
#define fastiva_ClassContext ClassObject
#endif

namespace JPP_REF_NAME_ID { 
	enum {
		undefined,
		init$$,
		initStatic$$,
		main_$,
		cntPredefinedID,
		appIdStart = 0x10000,
	};
};


//#define FASTIVA_CLASS_SIG(CLASS)	(FASTIVA_SIG_##CLASS##_4D + 4)


#ifdef EMBEDDED_RUNTIME
	#define JPP_RESERVE_IDS(cntSystem, cntModule)	reserved = 0x7FFFFFFF - cntSystem
#else
	#define JPP_RESERVE_IDS(cntSystem, cntModule)	reserved = cntModule
#endif


#define FASTIVA_MAX_PACKAGE_CNT_PER_MODULE (1 << 11)
#define FASTIVA_CLASS_ID_START		0x10000
//#define FASTIVA_CLASS_ID_BITS	12
#define FASTIVA_PACKAGE_ID_SHIFT 16
//#define FASTIVA_MAX_CLASS_CNT_PER_PACKAGE  (1 << FASTIVA_CLASS_ID_BITS)

#define JPP_BEGIN_LIBCORE_PACKAGE_ID_LIST()										\
//	namespace JPP_PACKAGE_ID {													
//		enum {	

#define JPP_BEGIN_EXTERNAL_PACKAGE_ID_LIST()									\
//	namespace JPP_PACKAGE_ID {													
//		enum {	$unused1 = FASTIVA_MAX_PACKAGE_CNT_PER_MODULE - 1, 

#define JPP_END_LIBCORE_PACKAGE_ID_LIST()										\
//		LIBCORE_PACKAGE_ID_COUNT, };};

#define JPP_END_EXTERNAL_PACKAGE_ID_LIST()										\
//		};};

#define JPP_DECL_PACKAGE_ID_SLOT(PACKAGE)								\
//	PACKAGE##$,


#define JPP_BEGIN_CLASS_ID_LIST()										\
	// namespace JPP_CLASS_ID {	enum {									

#define JPP_BEGIN_PACKAGE_CLASS_ID(PACKAGE)									\
		//PACKAGE##_$$ = ((JPP_PACKAGE_ID::PACKAGE##$ << FASTIVA_PACKAGE_ID_SHIFT) | FASTIVA_CLASS_ID_START) -1,

#define JPP_DECL_CLASS_ID(CLASS, CLASS_NAME)	\
		// FASTIVA_DECL_CLASS_DESCRIPTOR(CLASS, CLASS_NAME)

#define JPP_END_CLASS_ID_LIST()	 // }; };


#define JPP_BEGIN_LIBCORE_IVTABLE_ID_LIST()								\
	namespace JPP_IVTABLE_ID {											\
		enum { $unused = 0,

#define JPP_BEGIN_EMPTY_INTERFACE_IVTABLE_ID_LIST()						\
	namespace JPP_IVTABLE_ID {											\
		enum { 

#define JPP_BEGIN_EXTERNAL_IVTABLE_ID_LIST()							\
	namespace JPP_IVTABLE_ID {											\
		enum { $unused1 = JPP_LIBCORE_IVTABLE_ID_COUNT - 1,

#define JPP_BEGIN_LIBCORE_CLASS_ID_LIST()								\
	$$$ obsolete &&& namespace JPP_CLASS_ID {											\
		enum { $unused0 = JPP_CLASS_ID::cntPredefinedClassID - 1,

#define JPP_BEGIN_EXTERNAL_CLASS_ID_LIST()								\
	$$$ obsolete &&& namespace JPP_CLASS_ID {											\
		enum { $unused1 = JPP_EXTERNAL_CLASS_ID_START - 1,

#define JPP_END_EMPTY_INTERFACE_IVTABLE_ID_LIST()						\
		}; };

#define JPP_END_LIBCORE_IVTABLE_ID_LIST()									\
		JPP_LIBCORE_IVTABLE_ID_COUNT, }; };									\

#define JPP_END_EXTERNAL_IVTABLE_ID_LIST()									\
		}; };									

#define JPP_END_LIBCORE_CLASS_ID_LIST()										\
		LIBCORE_CLASS_ID_COUNT, }; };										\

#define JPP_END_EXTERNAL_CLASS_ID_LIST()									\
		$$$ obsolete &&& }; };																\



#define JPP_DECL_IVTABLE_ID(CLASS)										\
	CLASS##_vt$,

#define JPP_DECL_EMPTY_INTERFACE_IVTABLE_ID(CLASS)						\
	CLASS##_vt$ = -1,


#define JPP_DECL_ARRAY_CLASS_ID(CLASS_p, CLASS)							\
	&&& OBSOLETE &&& CLASS_p##$, CLASS##_p$ = CLASS_p##$,




/*
#define JPP_DECL_LIBCORE_CLASS_ID(PACKAGE, CLASS)										\
	static const int PACKAGE##_##CLASS##_p$ = (int)FASTIVA_RAW_CLASS_CONTEXT_PTR(PACKAGE##_##CLASS);	\
	static const int PACKAGE##_##CLASS##_A_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(1);	\
	static const int PACKAGE##_##CLASS##_AA_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(2);	\
	static const int PACKAGE##_##CLASS##_AAA_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(3);	\
	static const int PACKAGE##_##CLASS##_AAAA_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(4);	\
//	PACKAGE##_##CLASS##_p$ = FASTIVA_GET_LIBCORE_PACKAGE_ID(PACKAGE) + PACKAGE##$package::CLASS,	\


#define JPP_DECL_CLASS_ID(PACKAGE, CLASS)										\
	static const int PACKAGE##_##CLASS##_p$ = (int)FASTIVA_RAW_CLASS_CONTEXT_PTR(PACKAGE##_##CLASS);	\
	static const int PACKAGE##_##CLASS##_A_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(1);	\
	static const int PACKAGE##_##CLASS##_AA_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(2);	\
	static const int PACKAGE##_##CLASS##_AAA_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(3);	\
	static const int PACKAGE##_##CLASS##_AAAA_p$ = PACKAGE##_##CLASS##_p$ | FASTIVA_JNI_ARRAY_DIMENSION_BITS(4);	\
*/


#define JPP_BEGIN_LIBCORE_REF_NAME_ID_LIST()								\
	namespace JPP_REF_NAME_ID {												\
		enum {	$unused0 = JPP_REF_NAME_ID::cntPredefinedID - 1, 

#define JPP_BEGIN_EXTERNAL_REF_NAME_ID_LIST()										\
	namespace JPP_REF_NAME_ID {												\
		enum {	$unused1 = JPP_EXTERNAL_REF_NAME_ID_START - 1, 

#define JPP_END_LIBCORE_REF_NAME_ID_LIST()									\
		};};

#define JPP_END_EXTERNAL_REF_NAME_ID_LIST()									\
		};};


#define JPP_DECL_REF_NAME_ID_SLOT(NAME)										\
		NAME##$,

#define JPP_DECL_REF_NAME_ID_SLOT2(NAME, NAME2)								\
		NAME##$, NAME2##$ = NAME##$,

#define JPP_DECL_REF_NAME_ALIAS(NAME, ALIAS)								\
		ALIAS##$ = NAME##$,


//#define JPP_FIRST_APPLICATION_ARGLIST_ID			0x40000000
#ifdef ANDROID
#define JPP_BEGIN_LIBCORE_ARGLIST_ID_LIST()						\
	namespace fastiva_libcore_ArgListPool {										

#define JPP_BEGIN_EXTERNAL_ARGLIST_ID_LIST()						\
	namespace fastiva_component_ArgListPool {

#define JPP_DECL_ARGLIST_ID_SLOT(cnt, ARGS)							\
		extern const char* FASTIVA_ARGLIST_NAME_##cnt ARGS[cnt+1];

#define JPP_END_LIBCORE_ARGLIST_ID_LIST()							\
		};

#define JPP_END_EXTERNAL_ARGLIST_ID_LIST()							\
		};															


#else

#define JPP_BEGIN_LIBCORE_ARGLIST_ID_LIST()						\
	namespace JPP_ARGLIST_ID {										\
		enum {	

#define JPP_BEGIN_EXTERNAL_ARGLIST_ID_LIST()						\
	namespace JPP_ARGLIST_ID {										\
		enum {	$unused1 = JPP_EXTERNAL_ARGLIST_ID_START - 1, 


#define JPP_DECL_ARGLIST_ID_SLOT(cnt, ARGS)						\
		FASTIVA_MERGE_TOKEN(FASTIVA_ARGLIST_NAME_##cnt ARGS, $),	


#define JPP_END_LIBCORE_ARGLIST_ID_LIST()							\
		LIBCORE_ARGLIST_ID_COUNT, }; };

#define JPP_END_EXTERNAL_ARGLIST_ID_LIST()							\
		}; };															

#define JPP_INIT_ARGLIST_ID_SLOT(index, cntArg, name)							\
	static const FASTIVA_TOKEN_T g_aArgumentList_##index##$[] = { cntArg,
	// argument-id array 비교시 길이가 서로 다른 array 의 비교를 위해
	// cntArg를 가장 처음에 넣는다. (참고)문자열은 null-terminator 이용.

#define JPP_END_ARGLIST()		};

#endif

#define FASTIVA_JNI_BRIDGE_RESULT_void			//
#define FASTIVA_JNI_BRIDGE_RESULT_jint			pResult->i = 
#define FASTIVA_JNI_BRIDGE_RESULT_jlonglong		pResult->j = 
#define FASTIVA_JNI_BRIDGE_RESULT_float			pResult->f = 
#define FASTIVA_JNI_BRIDGE_RESULT_double		pResult->d = 

union JValue;
struct Method;
struct Thread;

#define JPP_IMPL_ANDROID_BRIDGE(fn)			void fastiva_jni_bridge_##fn (const u4* args, JValue* pResult, const Method* method, struct Thread* self) 
#define JPP_DECL_ANDROID_BRIDGE(fn)			JPP_IMPL_ANDROID_BRIDGE(fn);
#define FASTIVA_JNI_BRIDGE_START(res, args)	res (*func) args; *(const void**)&func = method->fastivaMethod;
#define FASTIVA_JNI_BRIDGE_END(res, args)	FASTIVA_JNI_BRIDGE_RESULT_##res (*func) args;




/*
#define FASTIVA_JNI_ARRAY_DIMENSION_OF_PTR(type, CLASS)						\
		CLASS::dimension$		
//(((type)0)->dimension$)

#define FASTIVA_JNI_ARRAY_DIMENSION_OF_VAL(VTYPE)				0
#define FASTIVA_JNI_ARRAY_DIMENSION_OF_void						0
//#define FASTIVA_JNI_ARRAY_DIMENSION_OF_NATIVE(type)					0

#define FASTIVA_JNI_CLASS_NAME_ID(CLASS)									\
		JPP_CLASS_ID::FASTIVA_MERGE_TOKEN(CLASS, _p$)

#define FASTIVA_JNI_CLASS_NAME_ID_OF_(CLASS)								\
		FASTIVA_JNI_CLASS_NAME_ID(CLASS)

#define FASTIVA_JNI_CLASS_NAME_ID_OF_VAL(VTYPE)								\
		JPP_CLASS_ID::FASTIVA_MERGE_TOKEN(VTYPE, $)

//#define FASTIVA_JNI_CLASS_NAME_ID_OF_NATIVE(VTYPE)							\
//		JPP_CLASS_ID::FASTIVA_MERGE_TOKEN(void, $)

#define FASTIVA_JNI_CLASS_NAME_ID_OF_void									\
		JPP_CLASS_ID::void$

#define FASTIVA_JNI_CLASS_NAME_ID_OF_PTR(type, CLASS)						\
		CLASS::BASE_T::type_id//FASTIVA_JNI_CLASS_NAME_ID(CLASS)

#if 0
		FASTIVA_JNI_CLASS_NAME_ID(CLASS)
#endif
*/

#define FASTIVA_JNI_BASE_CLASS_ID(CLASS)									\
	(FASTIVA_SIG_##CLASS##_p)

#if 1
#define FASTIVA_JNI_TYPE_ID_PTR$(CLASS)										\
		(FASTIVA_JNI_BASE_CLASS_ID(CLASS))

#define FASTIVA_JNI_TYPE_ID_JOBJ$(CLASS)									\
		(FASTIVA_JNI_BASE_CLASS_ID(CLASS))

#define FASTIVA_JNI_TYPE_ID_ARRAY$(dim, CLASS)								\
		(FASTIVA_JNI_BASE_CLASS_ID(CLASS)-dim)

#define FASTIVA_JNI_TYPE_ID_GENERIC$(CLASS)									\
		(FASTIVA_JNI_BASE_CLASS_ID(CLASS))

#define FASTIVA_JNI_TYPE_ID_void											\
		(FASTIVA_JNI_BASE_CLASS_ID(void))

#define FASTIVA_JNI_TYPE_ID_VAL$(CLASS)										\
		(FASTIVA_JNI_BASE_CLASS_ID(CLASS))

#else
#define FASTIVA_JNI_TYPE_ID_PTR$(CLASS)										\
		((int)FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS))

#define FASTIVA_JNI_TYPE_ID_JOBJ$(CLASS)										\
		((int)FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS))

#define FASTIVA_JNI_TYPE_ID_ARRAY$(dim, CLASS)										\
		((int)FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS)) && not impl &&

#define FASTIVA_JNI_TYPE_ID_GENERIC$(CLASS)									\
		((int)FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS))

#define FASTIVA_JNI_TYPE_ID_void											\
		(JPP_CLASS_ID::void$ << 3)

#define FASTIVA_JNI_TYPE_ID_VAL$(VTYPE)										\
		(JPP_CLASS_ID::FASTIVA_MERGE_TOKEN(VTYPE, $) << 3)

#endif

#define FASTIVA_JNI_FIELD_NAME_ID(NAME$)										\
	JPP_REF_NAME_ID::NAME$
	
//FASTIVA_MERGE_3TOKEN(JPP_REF_NAME__, NAME, $)


#define FASTIVA_JNI_INTERFACE_FLAG(field)		&& obsolete &&
		//(sizeof(field) > sizeof(void*) ? ACC_INTERFACE$ : 0)


//#define FASTIVA_JNI_METHOD_NAME_ID(NAME)	
//	JPP_REF_NAME_ID::FASTIVA_MERGE_TOKEN(NAME, $)
	
//FASTIVA_MERGE_3TOKEN(JPP_REF_NAME__, NAME, $)



#define FASTIVA_JNI_ARG_LIST_ID(cnt, ARGS)									\
	&& OBSOLETE && JPP_ARGLIST_ID::FASTIVA_MERGE_TOKEN(FASTIVA_ARGLIST_NAME_##cnt ARGS, $)

#define FASTIVA_PRIMITIVE_CLASS_ID(id)	((-1 << FASTIVA_PACKAGE_ID_SHIFT) | fastiva_Primitives::id)

namespace JPP_CLASS_ID_OBSOLETE { 
	enum {
		jbool$ = FASTIVA_PRIMITIVE_CLASS_ID(jbool), 
		unicod$ = FASTIVA_PRIMITIVE_CLASS_ID(unicod), 
		jfloat$ = FASTIVA_PRIMITIVE_CLASS_ID(jfloat), 
		jdouble$ = FASTIVA_PRIMITIVE_CLASS_ID(jdouble),
		jbyte$ = FASTIVA_PRIMITIVE_CLASS_ID(jbyte), 
		jshort$ = FASTIVA_PRIMITIVE_CLASS_ID(jshort), 
		jint$ = FASTIVA_PRIMITIVE_CLASS_ID(jint), 
		jlonglong$ = FASTIVA_PRIMITIVE_CLASS_ID(jlonglong), 
		jcustom$ = FASTIVA_PRIMITIVE_CLASS_ID(jcustom),
		void$ = FASTIVA_PRIMITIVE_CLASS_ID(jvoid),

		jbool_p$ = jbool$, bool_p$ = jbool_p$, 
		unicod_p$ = unicod$,
		jfloat_p$ = jfloat$, float_p$ = jfloat_p$,
		jdouble_p$ = jdouble$, double_p$ = jdouble_p$,
		jbyte_p$ = jbyte$, byte_p$ = jbyte_p$,
		jshort_p$ = jshort$, short_p$ = jshort_p$,
		jint_p$ = jint$, int_p$ = jint_p$,
		jlonglong_p$ = jlonglong$, longlong_p$ = jlonglong_p$,
		void_p$ = void$, void_p_p$ = void$,

		Bool_p$ = jbool_p$, 
		Unicod_p$ = unicod$,
		Float_p$ = jfloat_p$,
		Double_p$ = jdouble_p$,
		Byte_p$ = jbyte_p$,
		Short_p$ = jshort_p$,
		Int_p$ = jint_p$,
		Longlong_p$ = jlonglong_p$,
#if 0
		Bool_ap$,
		Unicod_ap$,
		Float_ap$,
		Double_ap$,
		Byte_ap$,
		Short_ap$,
		Int_ap$,
		Longlong_ap$,
		unused_1a,
		unused_1b,

		Bool_aap$,
		Unicod_aap$,
		Float_aap$,
		Double_aap$,
		Byte_aap$,
		Short_aap$,
		Int_aap$,
		Longlong_aap$,
		unused_2a,
		unused_2b,

		Bool_aaap$,
		Unicod_aaap$,
		Float_aaap$,
		Double_aaap$,
		Byte_aaap$,
		Short_aaap$,
		Int_aaap$,
		Longlong_aaap$,
		unused_3a,
		unused_3b,

		Bool_aaaap$,
		Unicod_aaaap$,
		Float_aaaap$,
		Double_aaaap$,
		Byte_aaaap$,
		Short_aaaap$,
		Int_aaaap$,
		Longlong_aaaap$,
		unused_4a,
		unused_4b,

		Bool_A_p$ = 		Bool_ap$,           
		Bool_AA_p$ =        Bool_aap$,          
		Bool_AAA_p$ =       Bool_aaap$,         
		Bool_AAAA_p$ =      Bool_aaaap$,        
		Unicod_A_p$ =       Unicod_ap$,         
		Unicod_AA_p$ =      Unicod_aap$,        
		Unicod_AAA_p$ =     Unicod_aaap$,       
		Unicod_AAAA_p$ =    Unicod_aaaap$,      
		Float_A_p$ =        Float_ap$,          
		Float_AA_p$ =       Float_aap$,         
		Float_AAA_p$ =      Float_aaap$,        
		Float_AAAA_p$ =     Float_aaaap$,       
		Double_A_p$ =       Double_ap$,         
		Double_AA_p$ =      Double_aap$,        
		Double_AAA_p$ =     Double_aaap$,       
		Double_AAAA_p$ =    Double_aaaap$,      
		Byte_A_p$ =         Byte_ap$,           
		Byte_AA_p$ =        Byte_aap$,          
		Byte_AAA_p$ =       Byte_aaap$,         
		Byte_AAAA_p$ =      Byte_aaaap$,        
		Short_A_p$ =        Short_ap$,          
		Short_AA_p$ =       Short_aap$,         
		Short_AAA_p$ =      Short_aaap$,        
		Short_AAAA_p$ =     Short_aaaap$,       
		Int_A_p$ =          Int_ap$,            
		Int_AA_p$ =         Int_aap$,           
		Int_AAA_p$ =        Int_aaap$,          
		Int_AAAA_p$ =       Int_aaaap$,         
		Longlong_A_p$ =     Longlong_ap$,       
		Longlong_AA_p$ =    Longlong_aap$,      
		Longlong_AAA_p$ =   Longlong_aaap$,     
		Longlong_AAAA_p$ =  Longlong_aaaap$,    

		cntPredefinedClassID,
#endif
	};

/*
	static bool is64bit(int type) {
		return type == jlonglong$ || type == jdouble$;
	}

	static int sizeOf(int type) {
		if (type <= void$) {
			if (type == void$) {
				return 0;
			}
			return 1 << (type & 3);
		}
		return sizeof(void*);
	}
*/
};


#endif // __FASTIVA_JNI_CONSTANTS_H__
