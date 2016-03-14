#ifndef __FASTIVA_JNI_DEF_H__
#define __FASTIVA_JNI_DEF_H__

#include <fastiva/Runtime.h>
#include <fastiva/class_def.h>
#include <fastiva/ClassContext.h>
#ifdef __GNUC__
#include <alloca.h>
#endif

extern void fastiva_debug_break(const char*, bool forceSleep);
#ifdef _DEBUG
	#define FASTIVA_ASSERT(v) if (!(v)) { fastiva_debug_break(#v, true); }; 
#else
	#define FASTIVA_ASSERT(v) assert(v)
#endif 

struct fastiva_JniCallInfo {
	Thread* thread;
	const u2* extra_currPC;
	void* jniEnv;
	int wasInFNI;
	void* localRefCookie;
	void* (*init_jni_stack)(Thread*, jmp_buf* buf);
	void* old_sp;
	jmp_buf jmpBuf;
};


#define JPP_JNI_RETURNS(TYPE)	int

#define JPP_JNI_ADD_LOCAL_REF(obj)				fastiva.addLocalReference(jni$->thread, obj)

#define JPP_JNI_METHOD_INDEX(CLASS, ret_t, slot, cnt, args) \
	CLASS##_jni_method_table_index$::FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot, cnt, args)

#define JPP_BEGIN_JNI_CALL(CLASS, ret_t, slot, cnt, args)								\
	typedef ret_t (*JNI_METHOD_T$)(void*, void* FASTIVA_PARAM_SEPERATOR_##cnt FASTIVA_PARAM_TYPES_##cnt args); \
	int jni_method_index$ = CLASS##_jni_method_table_index$::FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot, cnt, args);	\
	fastiva_JniCallInfo* jni$ = (fastiva_JniCallInfo*)alloca(sizeof(fastiva_JniCallInfo));				\
	JNI_METHOD_T$ jni_method$ = (JNI_METHOD_T$)fastiva.beginJniCall(  CLASS##_C$::getRawStatic$(), jni_method_index$, jni$);\
	void* pJniEnv$ = jni$->jniEnv;					\
	void* jObj; 

#define JPP_BEGIN_FAST_JNI_CALL(CLASS, ret_t, slot, cnt, args)								\
	typedef ret_t (*JNI_METHOD_T$)(void*, void* FASTIVA_PARAM_SEPERATOR_##cnt FASTIVA_PARAM_TYPES_##cnt args); \
	int jni_method_index$ = CLASS##_jni_method_table_index$::FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot, cnt, args);	\
	fastiva_JniCallInfo* jni$ = (fastiva_JniCallInfo*)alloca(sizeof(fastiva_JniCallInfo));				\
	JNI_METHOD_T$ jni_method$ = (JNI_METHOD_T$)fastiva.beginFastJniCall(  CLASS##_C$::getRawStatic$(), jni_method_index$, jni$);\
	void* pJniEnv$ = jni$->jniEnv;					\
	void* jObj; 


#define JPP_PUSH_LOCAL_REF() \
	if (jni$->init_jni_stack != NULL) { \
		jni$->old_sp = jni$->init_jni_stack(jni$->thread, &jni$->jmpBuf);	\
		if (FASTIVA_FAST_JNI != 0) { jObj = self; } \
	} else { jObj = self; } \
	if (FASTIVA_FAST_JNI == 0 && jni$->init_jni_stack != NULL) 

/*
Internal Native Call. Internal Native 함수는 vtable 에 등록되며, Fastiva 와 ABI 가 동일하다.
아래는 vtable을 거치지 않고 직접 호출된 경우를 처리하기 위한 것이다.
*/
#define JPP_BEGIN_INTERNAL_STATIC_CALL(CLASS, ret_t, slot, cnt, args) \
	typedef ret_t (*JNI_METHOD_T$) args; \
	int jni_method_index$ = CLASS##_jni_method_table_index$::FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot, cnt, args);	\
	JNI_METHOD_T$ method$ = (JNI_METHOD_T$)CLASS##_G$::g_vtable$->FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot##_S$, cnt, args);//[jni_method_index$];

#define JPP_BEGIN_INTERNAL_INSTANCE_CALL(CLASS, ret_t, slot, cnt, args) \
	typedef ret_t (*JNI_METHOD_T$)(void* FASTIVA_PARAM_SEPERATOR_##cnt FASTIVA_PARAM_TYPES_##cnt args); \
	int jni_method_index$ = CLASS##_jni_method_table_index$::FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot, cnt, args);	\
	JNI_METHOD_T$ method$ = (JNI_METHOD_T$)CLASS##_G$::g_vtable$->FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot, cnt, args);//[jni_method_index$];

#define JPP_END_INTERNAL_NATIVE_CALL() \
		// IGNORE


#define JPP_END_JNI_CALL_OBJ_RESULT(res)												\
	fastiva.endJniCall(res, jni$);

#define JPP_END_JNI_CALL_NO_RESULT()												\
	fastiva.endJniCall(0, jni$);

#define JPP_END_FAST_JNI_CALL_OBJ_RESULT(res)												\
	fastiva.endFastJniCall(res, jni$);

#define JPP_END_FAST_JNI_CALL_NO_RESULT()												\
	fastiva.endFastJniCall(0, jni$);


typedef struct {
    char *name;
    char *signature;
    void *fnPtr;
} JNINativeMethod_ex;


#ifdef _DEBUG
	#define FASTIVA_CHECK_JNI_RESULT(res, CLASS)							\
		//CLASS##_T$::ptr_cast$((java_lang_Object_p)res)
#else
	#define FASTIVA_CHECK_JNI_RESULT(res, CLASS)							\
		// DO NOTHING
#endif

#ifndef EMBEDDED_RUNTIME
	#define FASTIVA_REF_COUNT_BASE	0x80008000
#else
	#define FASTIVA_REF_COUNT_BASE	0
#endif


#define FASTIVA_DECL_NULL_JNI_INFO(CLASS)									\
	static const int CLASS##_JNI_INFO$ = 0;

#define FASTIVA_DECLARED_METHOD_COUNT(CLASS)			\
	(sizeof(aMethodInfo_##CLASS##$) / sizeof(fastiva_MethodInfo))	

#define FASTIVA_DECLARED_FIELD_COUNT(CLASS)			\
	(sizeof(aFieldInfo_##CLASS##$) / sizeof(fastiva_FieldInfo))

#ifdef ANDROID
#define FASTIVA_DECL_JNI_INFO2(CLASS)								\
	static const fastiva_ClassContext* CLASS##_JNI_INFO$[] = {		
#else
#define FASTIVA_DECL_JNI_INFO2(CLASS)								\
	static const void* CLASS##_JNI_INFO$[] = {						\
		(void*)FASTIVA_CLASS_SIG(CLASS),							\
		(void*)(FASTIVA_DECLARED_FIELD_COUNT(CLASS)					\
			 +  (FASTIVA_DECLARED_METHOD_COUNT(CLASS) << 16)		\
			 + FASTIVA_REF_COUNT_BASE),								\
		(void*)aFieldInfo_##CLASS##$,				\
		(void*)aMethodInfo_##CLASS##$,				

#endif

void* fastiva_jniProlog(
	java_lang_Class_p pClass
);

void fastiva_jniEpilog(
	void* pEnv
);

void fastiva_lockGlobalRef(
	fastiva_Instance_p pObj
);

void fastiva_releaseGlobalRef(
	fastiva_Instance_p pObj
);




#if FASTIVA_SUPPORTS_JAVASCRIPT
	#include <fastiva/lang/Generic.h>
	#include <fastiva/lang/Generic.inl>
#else
	typedef java_lang_Object fastiva_Generic;
	typedef java_lang_Object_p fastiva_Generic_p;
#endif


#if FASTIVA_SUPPORTS_JAVASCRIPT
struct fastiva_Generic_I$ : public java_lang_Class_I$ {
	static void FOX_FASTCALL(JPP_SCAN_TYPE_A)(fastiva_lang_Generic_p pGeneric) {
		return;
	}
	static void FOX_FASTCALL(JPP_SCAN_TYPE_B)(fastiva_lang_Generic_p pGeneric) {
		return;
	}
	static void FOX_FASTCALL(JPP_SCAN_TYPE_C)(fastiva_lang_Generic_p pGeneric) {
		return;
	}
	static void FOX_FASTCALL(JPP_SCAN_TYPE_D)(fastiva_lang_Generic_p pGeneric) {
		return;
	}
};
#endif

#if FASTIVA_SUPPORTS_JAVASCRIPT
class fastiva_lang_Generic_A : public fastiva_ArrayHeader_A //fastiva_PointerArray<fastiva_lang_Generic, fastiva_lang_Generic*, fastiva_ArrayHeader_A>
{
public:
	typedef fastiva_lang_Generic	BASE_T;
	typedef fastiva_ArrayHeader_A	HEADER;
	typedef fastiva_lang_Generic*	ITEM_T;
	typedef fastiva_lang_Generic_A	fastiva_PointerArray;

	static fastiva_PointerArray* Null$() {
		return (fastiva_PointerArray*)FASTIVA_NULL;
	}

	ITEM_T get$(int idx) const {
		this->checkBound(idx);
		ITEM_T item = m_aItem[idx];
		return item;
	}

	void set$(int idx, ITEM_T value) {
		// instance/interface array 는 그 상위 class의 array로 casting 될 수 있다.
		// item을 검사하여야 한다.
		this->checkBound(idx);
		int len = m_length;
		fm::setGenericArrayItem(this, m_aItem + idx, value);
		KASSERT(len == m_length);
	}

	static fastiva_PointerArray* isInstance$(fastiva_Instance_p pObj) {
		if (pObj == FASTIVA_NULL) {
			return (fastiva_PointerArray*)FASTIVA_NULL;
		}
		void* pArray = fastiva.isArrayInstanceOf(
			pObj, BASE_T::getRawContext$(), HEADER::dimension$);
		KASSERT(pArray == FASTIVA_NULL || pArray == pObj);
		return (fastiva_PointerArray*)pArray;
	}

	static fastiva_PointerArray* ptr_cast$(fastiva_Instance_p pObj) {
		fm::checkArrayInstanceOf(
			pObj, BASE_T::getRawContext$(), HEADER::dimension$);
		return (fastiva_PointerArray*)pObj;
	}

	static fastiva_PointerArray* create$(int d1, int d2 = fastiva_NO_ARRAY_ITEM$, 
								int d3 = fastiva_NO_ARRAY_ITEM$, 
								int d4 = fastiva_NO_ARRAY_ITEM$) {
		if (dimension$ == 1) {
			return (fastiva_PointerArray*)
				fm::allocatePointerArray(BASE_T::getRawContext$(), d1);
		}

		int aLength[dimension$];
		aLength[0] = d1;
		aLength[1] = d2;
		if (dimension$ > 2) {
			aLength[2] = d3;
		}
		else {
			KASSERT(d3 == fastiva_NO_ARRAY_ITEM$);
		}
		if (dimension$ > 3) {
			aLength[3] = d4;
		}
		else {
			KASSERT(d4 == fastiva_NO_ARRAY_ITEM$);
		}

		return (fastiva_PointerArray*)fastiva.allocateMultiArray(
			BASE_T::getRawContext$(), dimension$, aLength);
	}

private:
	union {
		ITEM_T m_aItem[64];
		java_lang_Object_p m_aRawItem[64];
	};
};


typedef fastiva_lang_Generic_A* fastiva_lang_Generic_ap;
typedef fastiva_PointerArray<fastiva_lang_Generic, fastiva_lang_Generic_ap, fastiva_ArrayHeader_AA> fastiva_lang_Generic_AA;
typedef fastiva_lang_Generic_AA* fastiva_lang_Generic_aap;
typedef fastiva_PointerArray<fastiva_lang_Generic, fastiva_lang_Generic_aap, fastiva_ArrayHeader_AAA> fastiva_lang_Generic_AAA;
typedef fastiva_lang_Generic_AAA* fastiva_lang_Generic_aaap;
typedef fastiva_PointerArray<fastiva_lang_Generic, fastiva_lang_Generic_aaap, fastiva_ArrayHeader_AAAA> fastiva_lang_Generic_AAAA;
typedef fastiva_lang_Generic_AAAA* fastiva_lang_Generic_aaaap;

#endif

/*
#define JPP_ARG_LIST__jbool$  1
#define JPP_ARG_LIST__unicod$ 2
#define JPP_ARG_LIST__jfloat$ 3
#define JPP_ARG_LIST__jdouble$ 4
#define JPP_ARG_LIST__jbyte$ 5
#define JPP_ARG_LIST__jshort$ 6
#define JPP_ARG_LIST__jint$ 7
#define JPP_ARG_LIST__jlonglong$ 8
#define JPP_ARG_LIST__void$ 9
#define JPP_ARG_LIST__jbool_p$ JPP_ARG_LIST__jbool$
#define JPP_ARG_LIST__bool_p$ JPP_ARG_LIST__jbool_p$
#define JPP_ARG_LIST__unicod_p$ JPP_ARG_LIST__unicod$
#define JPP_ARG_LIST__jfloat_p$ JPP_ARG_LIST__jfloat$
#define JPP_ARG_LIST__float_p$ JPP_ARG_LIST__jfloat_p$
#define JPP_ARG_LIST__jdouble_p$ JPP_ARG_LIST__jdouble$
#define JPP_ARG_LIST__double_p$ JPP_ARG_LIST__jdouble_p$
#define JPP_ARG_LIST__jbyte_p$ JPP_ARG_LIST__jbyte$
#define JPP_ARG_LIST__byte_p$ JPP_ARG_LIST__jbyte_p$
#define JPP_ARG_LIST__jshort_p$ JPP_ARG_LIST__jshort$
#define JPP_ARG_LIST__short_p$ JPP_ARG_LIST__jshort_p$
#define JPP_ARG_LIST__jint_p$ JPP_ARG_LIST__jint$
#define JPP_ARG_LIST__int_p$ JPP_ARG_LIST__jint_p$
#define JPP_ARG_LIST__jlonglong_p$ JPP_ARG_LIST__jlonglong$
#define JPP_ARG_LIST__longlong_p$ JPP_ARG_LIST__jlonglong_p$
#define JPP_ARG_LIST__void_p$ JPP_ARG_LIST__void$
#define JPP_ARG_LIST__Bool_A_p$ (JPP_ARG_LIST__jbool$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Bool_AA_p$ (JPP_ARG_LIST__jbool$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Bool_AAA_p$ (JPP_ARG_LIST__jbool$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Bool_AAAA_p$ (JPP_ARG_LIST__jbool$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__Unicod_A_p$ (JPP_ARG_LIST__unicod$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Unicod_AA_p$ (JPP_ARG_LIST__unicod$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Unicod_AAA_p$ (JPP_ARG_LIST__unicod$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Unicod_AAAA_p$ (JPP_ARG_LIST__unicod$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__Float_A_p$ (JPP_ARG_LIST__jfloat$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Float_AA_p$ (JPP_ARG_LIST__jfloat$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Float_AAA_p$ (JPP_ARG_LIST__jfloat$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Float_AAAA_p$ (JPP_ARG_LIST__jfloat$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__Double_A_p$ (JPP_ARG_LIST__jdouble$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Double_AA_p$ (JPP_ARG_LIST__jdouble$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Double_AAA_p$ (JPP_ARG_LIST__jdouble$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Double_AAAA_p$ (JPP_ARG_LIST__jdouble$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__Byte_A_p$ (JPP_ARG_LIST__jbyte$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Byte_AA_p$ (JPP_ARG_LIST__jbyte$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Byte_AAA_p$ (JPP_ARG_LIST__jbyte$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Byte_AAAA_p$ (JPP_ARG_LIST__jbyte$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__Short_A_p$ (JPP_ARG_LIST__jshort$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Short_AA_p$ (JPP_ARG_LIST__jshort$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Short_AAA_p$ (JPP_ARG_LIST__jshort$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Short_AAAA_p$ (JPP_ARG_LIST__jshort$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__Int_A_p$ (JPP_ARG_LIST__jint$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Int_AA_p$ (JPP_ARG_LIST__jint$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Int_AAA_p$ (JPP_ARG_LIST__jint$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Int_AAAA_p$ (JPP_ARG_LIST__jint$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__Longlong_A_p$ (JPP_ARG_LIST__jlonglong$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(1))
#define JPP_ARG_LIST__Longlong_AA_p$ (JPP_ARG_LIST__jlonglong$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(2))
#define JPP_ARG_LIST__Longlong_AAA_p$ (JPP_ARG_LIST__jlonglong$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(3))
#define JPP_ARG_LIST__Longlong_AAAA_p$ (JPP_ARG_LIST__jlonglong$ + FASTIVA_JNI_ARRAY_DIMENSION_BITS(4))
#define JPP_ARG_LIST__cntPredefinedID (JPP_ARG_LIST__void$ + 1)
*/





#endif // __FASTIVA_JNI_DEF_H__


/**====================== end ========================**/
