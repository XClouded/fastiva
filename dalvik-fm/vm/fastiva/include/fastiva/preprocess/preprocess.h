#ifndef __FASTIVA_PREPROCESS_H__
#define __FASTIVA_PREPROCESS_H__


#include "util.h"
#include "primitive_sig.h"
#include "params.h"

#if 1 // def EMBEDDED_RUNTIME
	#define FASTIVA_PRIVATE						public
#else
	#define FASTIVA_PRIVATE						private
#endif

#define FASTIVA_PARAM_T(type)			type//JPP_MECPP_PARAM_T$ type//##

#define FASTIVA_PREPROCESS_CLASS_C$					\
		FASTIVA_MERGE_TOKEN(FASTIVA_PREPROCESS_CLASS, _C$)

#define FASTIVA_ACC_public						public:
#define FASTIVA_ACC_protected					public:
#define FASTIVA_ACC_private						FASTIVA_PRIVATE:
#define FASTIVA_ACC_package						public: // field������ ���.
#define FASTIVA_ACC__(package)					public:	// method������ ���.
#define FASTIVA_ACC__native(acc)				FASTIVA_ACC_##acc // native-parameter �Լ�

#define ARRAY_VARIANT_SUFFIX_1		_ap
#define ARRAY_VARIANT_SUFFIX_2		_aap
#define ARRAY_VARIANT_SUFFIX_3		_aaap
#define ARRAY_VARIANT_SUFFIX_4		_aaaap
#define ARRAY_VARIANT_SUFFIX_5		_aaaaap
#define ARRAY_VARIANT_SUFFIX_6		_aaaaaap
#define ARRAY_VARIANT_SUFFIX_7		_aaaaaaap

#define ARRAY_TYPE_SUFFIX_1		_A
#define ARRAY_TYPE_SUFFIX_2		_AA
#define ARRAY_TYPE_SUFFIX_3		_AAA
#define ARRAY_TYPE_SUFFIX_4		_AAAA
#define ARRAY_TYPE_SUFFIX_5		_AAAAA
#define ARRAY_TYPE_SUFFIX_6		_AAAAAA
#define ARRAY_TYPE_SUFFIX_7		_AAAAAAA

#define FASTIVA_RETURN_TYPE_PTR$(CLASS)				CLASS##_p
#define FASTIVA_RETURN_TYPE_ARRAY$(dim, CLASS)		FASTIVA_MERGE_TOKEN(CLASS, ARRAY_VARIANT_SUFFIX_##dim)
#define FASTIVA_RETURN_TYPE_void					void
#define FASTIVA_RETURN_TYPE_VAL$(ret_t)				ret_t
#define FASTIVA_RETURN_TYPE_JOBJ$(CLASS)			CLASS##_p

#define FASTIVA_RETURN_TYPE_CLASS_PTR$(CLASS)		CLASS
#define FASTIVA_RETURN_TYPE_CLASS_ARRAY$(dim, CLASS)		FASTIVA_MERGE_TOKEN(CLASS, ARRAY_TYPE_SUFFIX_##dim)
#define FASTIVA_RETURN_TYPE_CLASS_void				&& ERROR &&
#define FASTIVA_RETURN_TYPE_CLASS_VAL$(ret_t)		&& ERROR &&
#define FASTIVA_RETURN_TYPE_CLASS_JOBJ$(CLASS)		CLASS

#define JPP_RETURNS(type)							FASTIVA_RETURN_TYPE_##type

#define JPP_PRIMITIVE_CLASS(type)					fastiva.getClass(fastiva_Primitives::type, 0)
//#define JPP_PRIMITIVE_ARRAY_CLASS(type, dimension)	fm::getPrimitiveArrayClass(type)
//#define JPP_OBJECT_ARRAY_CLASS(type, dimension)		fm::getObjectArrayClass(type)

#define FASTIVA_RETURN_INSTRUCUTION_PTR$(CLASS)			return
#define FASTIVA_RETURN_INSTRUCUTION_ARRAY$(dim, CLASS)			return
#define FASTIVA_RETURN_INSTRUCUTION_void				// nothing
#define FASTIVA_RETURN_INSTRUCUTION_VAL$(ret_t)			return
#define FASTIVA_RETURN_INSTRUCUTION_JOBJ$(ret_t)		return
//#define FASTIVA_RETURN_INSTRUCUTION_NATIVE(ret_t)		return

#define FASTIVA_INSTANCE_PARAM(T, _self, cnt, args)							\
	_self FASTIVA_PARAM_SEPERATOR_##cnt FASTIVA_PARAM_##T##_##cnt args

#define FASTIVA_VIRTUAL_METHOD_NAME_SUFFIX_public				// nothin
#define FASTIVA_VIRTUAL_METHOD_NAME_SUFFIX_protected			// nothin
#define FASTIVA_VIRTUAL_METHOD_NAME_SUFFIX_private				// nothin
#define FASTIVA_VIRTUAL_METHOD_NAME_SUFFIX_(package)			_##package##$

#define FASTIVA_VIRTUAL_METHOD_NAME(fn, acc)								\
	FASTIVA_MERGE_TOKEN(fn, FASTIVA_VIRTUAL_METHOD_NAME_SUFFIX_##acc)

// proxy ���� ����. inline �Լ� ������ �ܺ� package�� private �Լ��� 
// ȣ��� �� �����Ƿ�, private �� package �Լ��� thunk�� �����Ѵ�.
#define FASTIVA_THUNK_FILTER_public(t)		t
#define FASTIVA_THUNK_FILTER_protected(t)	t
#define FASTIVA_THUNK_FILTER_private(t)		t
#define FASTIVA_THUNK_FILTER_(t)			FASTIVA_PACKAGE_THUNK_FILTER
#define FASTIVA_PACKAGE_THUNK_FILTER(t)		t		
//#define FASTIVA_THUNK_FILTER_inline(acc)	FASTIVA_NOTHING
//#define FASTIVA_THUNK_FILTER_native(acc)	FASTIVA_THUNK_FILTER_##acc
#ifndef JPP_NATIVE_API_NAME
#	define JPP_NATIVE_API_NAME(PACKAGE, CLASS, METHOD)	fastiva_vm_##CLASS##__##METHOD
#endif

#define JPP_THROW_ABSTRACT_METHOD_ERROR()	fastiva.throwAbstractMethodError();


#define FASTIVA_METHOD_SLOT_NAME_EX(ret_t, slot, cnt, args) \
		FASTIVA_MERGE_TOKEN( \
			FASTIVA_MERGE_TOKEN( \
				FASTIVA_MERGE_TOKEN(slot, __), \
				FASTIVA_ARGLIST_NAME_##cnt args), \
			FASTIVA_MERGE_TOKEN($, ret_t))

#define FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args) \
		FASTIVA_METHOD_SLOT_NAME_EX(FASTIVA_RETURN_TYPE_##ret_t, slot, cnt, args) 



/*
 * �ش� �Լ��� ������ ����� class. C++�� name scope ������ �ذ��ϱ� ���� ���  
 */
#define FASTIVA_METHOD_SLOT_OWNER(CLASS, ret_t, slot, cnt, args) \
	FASTIVA_MERGE_TOKEN(CLASS, _I$)::FASTIVA_METHOD_SLOT_OWNER_TYPE(ret_t, slot, cnt, args)

#define FASTIVA_METHOD_SLOT_OWNER_TYPE(ret_t, slot, cnt, args) \
	FASTIVA_MERGE_TOKEN(FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args), _OWNER_T$) 

#define FASTIVA_DECL_METHOD_SLOT_OWNER_TYPE(CLASS, ret_t, slot, cnt, args) \
	typedef FASTIVA_MERGE_TOKEN(CLASS, _I$) FASTIVA_METHOD_SLOT_OWNER_TYPE(ret_t, slot, cnt, args)

#define	FASTIVA_DECL_VIRTUAL_SLOT(CLASS, acc, ret_t, slot, cnt, args, offset) \
	void* FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args); \
	FASTIVA_DECL_METHOD_SLOT_OWNER_TYPE(CLASS, ret_t, slot, cnt, args);			

#define	FASTIVA_DECL_OVERRIDE_SLOT(CLASS, acc, ret_t, slot, cnt, args, offset) \
	&& MUST NOT BE HERE &&	

#define	FASTIVA_DECL_NON_VIRTUAL_SLOT(CLASS, ret_t, slot, cnt, args) \
	void* FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args); \
	FASTIVA_DECL_METHOD_SLOT_OWNER_TYPE(CLASS, ret_t, slot, cnt, args);

#define	FASTIVA_DECL_STATIC_SLOT(CLASS, ret_t, slot, cnt, args) \
	void* FASTIVA_METHOD_SLOT_NAME(ret_t, slot##_S$, cnt, args); \
	FASTIVA_DECL_METHOD_SLOT_OWNER_TYPE(CLASS, ret_t, slot, cnt, args);



#define	FASTIVA_IMPL_NON_VIRTUAL_THUNK_obsolete(ret_t, fn, cnt, args)				\
	private: virtual FASTIVA_RETURN_TYPE_##ret_t							\
	FASTIVA_THISCALL(FASTIVA_METHOD_SLOT_NAME(ret_t, fn, cnt, args))		\
		(FASTIVA_PARAM_PAIRS_##cnt args) {									\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t								\
				this->fn(FASTIVA_PARAM_NAMES_##cnt args);					\
		}

// ARM �迭 Calling-convention�� ���߱� ���� STATIC-class�� this�� �������� �Ѵ�.
#define	FASTIVA_IMPL_STATIC_VIRTUAL_THUNK_obsolete(ret_t, fn, cnt, args)				\
	private: virtual FASTIVA_RETURN_TYPE_##ret_t							\
	FASTIVA_THISCALL(FASTIVA_METHOD_SLOT_NAME(ret_t, fn, cnt, args))		\
		(FASTIVA_PARAM_PAIRS_##cnt args) {								\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t								\
				((STATIC$*)(void*)this)->fn(FASTIVA_PARAM_NAMES_##cnt args);\
		}



#define FASTIVA_ABSTRACT_METHOD_SIG_ABSTRACT$(acc)	&& obsolete && = 0
#define FASTIVA_ABSTRACT_METHOD_SIG_FINAL$(acc)		&& obsolete && // nothing	
#define FASTIVA_ABSTRACT_METHOD_SIG_(acc)			&& obsolete && // nothing	



#if 0 // ====================================================================
	package-method
    1) �ܺ� package������ �ش� �Լ� ȣ���� �Ұ����ؾ� �Ѵ�. 
		(���� �Լ��� �ܺ��Լ� ���� �޶�� �Ѵ�.)
    2) ���� pakcage���� Non-Virtual-Invoke�� �����ؾ� �Ѵ�.
      ��, call �ϴ� �Լ� �� ��ü�� virtual �Լ��̾�� �Ѵ�.
	  call �ϴ� �Լ��� inline-thunk�� ��쿣 Virtual-Invoke�� �Ǿ������.
    3) package �Լ���� exported �Լ����� ���� ���, overloadind �� ����
      virtual slot�� ������ �ٲ�� ������ �ذ��Ͽ��� �Ѵ�.

	public: virtual FASTIVA_RETURN_TYPE_##ret_t									
		FASTIVA_THISCALL(FASTIVA_VIRTUAL_METHOD_NAME(fn,acc)) 					
		(FASTIVA_PARAM_PAIRS_##cnt args) {									
			FASTIVA_RETURN_INSTRUCUTION_##ret_t this->THIS$::fn(			
					FASTIVA_PARAM_NAMES##_##cnt args);						
	}																		

#endif



//#define FASTIVA_INTERNAL_METHOD_NAME_SUFFIX_package(package)	package

// super-class�� ��ӵ� final �Լ��� interface ��Ӱ������� export�Ǵ� ���.
#define FASTIVA_METHOD_EXPORT_FINAL$(acc, cnt, ret, slot, fn, args, super_fn)	\
	&& OBSOLETE &&															\
	public: virtual FASTIVA_RETURN_TYPE_##ret								\
		FASTIVA_THISCALL(fn) (FASTIVA_PARAM_PAIRS_##cnt args) {				\
		FASTIVA_RETURN_INSTRUCUTION_##ret									\
			this->SUPER$::super_fn(FASTIVA_PARAM_NAMES##_##cnt args);		\
	}

#define FASTIVA_METHOD_INTERNAL_TO_EXPORT_THUNK(acc, cnt, ret_t, slot, fn, args)	\
	private: FASTIVA_RETURN_TYPE_##ret_t									\
	FASTIVA_THISCALL(fn##_##FASTIVA_PREPROCESS_PACKAGE##$) 					\
		(FASTIVA_PARAM_PAIRS_##cnt args) {									\
			FASTIVA_RETURN_INSTRUCUTION_##ret_t								\
				this->fn(FASTIVA_PARAM_NAMES##_##cnt args);					\
	}



// super-class�� ��ӵ� internal �Լ��� interface ��Ӱ������� export�Ǵ� ���.
// package-virtual�� ��쿡�� naming�� �޸��ϹǷ�, super-method�� final�� �ƴϴ�.
#define FASTIVA_METHOD_EXPORT_INTERNAL$(acc, cnt, ret_t, slot, fn, args)			\
	&& OBSOLETE && \
	FASTIVA_METHOD_EXPORT_FINAL$(acc, cnt, ret_t, slot, fn, args,					\
		fn##_##FASTIVA_PREPROCESS_PACKAGE##$)								\
	FASTIVA_METHOD_INTERNAL_TO_EXPORT_THUNK(acc, cnt, ret_t, slot, fn, args)


#define FASTIVA_INTERFACE_IMPL_SELF(cnt)									\
	java_lang_Object_p self FASTIVA_PARAM_SEPERATOR_##cnt

/*
 * interface �Լ��� �����Ѵ�. C++�� ���� interface �Լ��� instance�� �Ǳ�����
 * �Լ��� redirection�Ѵ�. �� ��, C++�� name scope ������ �ذ��ϱ� ���Ͽ�
 * �ش� �Լ��� ������ ����� class �� FASTIVA_METHOD_SLOT_OWNER �� ���� ��´�.
 */


#define FASTIVA_VIRTUAL_THUNK(CLASS, ret_t, slot, fn, cnt, args)					\
	inline FASTIVA_RETURN_TYPE_##ret_t CLASS::fn(						\
		FASTIVA_PARAM_PAIRS_##cnt args) {									\
		int offset = FASTIVA_VIRTUAL_SLOT_OFFSET(CLASS,ret_t,slot,cnt,args);	\
		int pfn = this->getInstanceVTable$()[offset];						\
		typedef FASTIVA_RETURN_TYPE_##ret_t (*FUNC_T) (		\
			FASTIVA_INSTANCE_PARAM(TYPES, void*, cnt, args));				\
		FASTIVA_RETURN_INSTRUCUTION_##ret_t	((FUNC_T)pfn)					\
			(FASTIVA_INSTANCE_PARAM(NAMES, this, cnt, args));				\
    }																		\

#define FASTIVA_INTERFACE_THUNK(CLASS, ret_t, slot, fn, cnt, args)				\
	inline FASTIVA_RETURN_TYPE_##ret_t CLASS::fn(							\
		FASTIVA_PARAM_PAIRS_##cnt args) {									\
		int ivt_id = FASTIVA_IVTABLE_ID(CLASS);								\
		int offset = FASTIVA_INTERFACE_SLOT_OFFSET(CLASS,ret_t,slot,cnt,args);\
		int index = this->getIVTable$(ivt_id)[offset];				\
		typedef FASTIVA_RETURN_TYPE_##ret_t (*FUNC_T) (		\
			FASTIVA_INSTANCE_PARAM(TYPES, void*, cnt, args));				\
		FUNC_T fn = (FUNC_T)this->getInstanceVTable$()[index];				\
		FASTIVA_RETURN_INSTRUCUTION_##ret_t	(fn)							\
			(FASTIVA_INSTANCE_PARAM(NAMES, getInstance$(), cnt, args));		\
    }																		\



#define FASTIVA_FINAL_THUNK(CLASS, ret_t, slot, fn, cnt, args) \
	inline FASTIVA_RETURN_TYPE_##ret_t CLASS::fn( \
		FASTIVA_PARAM_PAIRS_##cnt args) { \
		FASTIVA_RETURN_INSTRUCUTION_##ret_t	\
			FASTIVA_MERGE_TOKEN(CLASS, _I$)::fn \
			(FASTIVA_INSTANCE_PARAM(NAMES, this, cnt, args)); \
    }


#define FASTIVA_DECL_INSTANCE_METHOD(CLASS, ret_t, fn, cnt, args)					\
	static FASTIVA_RETURN_TYPE_##ret_t	fn	(FASTIVA_INSTANCE_PARAM(PAIRS, CLASS* self, cnt, args))

#define FASTIVA_DECL_STATIC_METHOD(CLASS, ret_t, fn, cnt, args)					\
	static FASTIVA_RETURN_TYPE_##ret_t	fn##$ args;


#define FASTIVA_INIT_SCAN_OFFSET_TABLE(CLASS)								\
	const unsigned short CLASS::g_aScanOffset$[]


#define JPP_IMPL_SCAN_INSTANCE(CLASS)										\
	inline void FASTIVA_MERGE_TOKEN(CLASS, _I$)::scanInstance$(FASTIVA_MERGE_TOKEN(CLASS, _p) self, FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner) {

#define JPP_IMPL_SCAN_JPROXY(CLASS)										\
	inline void FASTIVA_MERGE_TOKEN(CLASS, _I$)::scanJavaProxyFields$(FASTIVA_MERGE_TOKEN(CLASS, _p) self, void* pEnv0, FASTIVA_JPROXY_SCAN_METHOD method) {

#define JPP_IMPL_SCAN_INSTANCE_ARMCC(CLASS)								\
	void FASTIVA_MERGE_TOKEN(CLASS, _I$)::scanInstance$(FASTIVA_MERGE_TOKEN(CLASS, _p) self, FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner) { \
		SUPER$::scanInstance$(self, method, scanner);							\
	}

#define JPP_IMPL_SCAN_JPROXY_ARMCC(CLASS)								\
	void FASTIVA_MERGE_TOKEN(CLASS, _I$)::scanJavaProxyFields$(FASTIVA_MERGE_TOKEN(CLASS, _p) self, void* pEnv0, FASTIVA_JPROXY_SCAN_METHOD method) { \
		SUPER$::scanJavaProxyFields$(self, pEnv0, method);					\
	}


#define FASTIVA_IMPL_SCAN_FIELD_VAL$(type)			FASIVA_PRIMITIVE_FIELD_SCAN
#define FASTIVA_IMPL_SCAN_FIELD_PTR$(CLASS)			FASIVA_INSTANCE_FIELD_SCAN
#define FASTIVA_IMPL_SCAN_FIELD_ARRAY$(dim, CLASS)		FASIVA_ARRAY_FIELD_SCAN
#define FASTIVA_IMPL_SCAN_FIELD_GENERIC$(CLASS)		FASIVA_GENERIC_FIELD_SCAN
#define FASTIVA_IMPL_SCAN_FIELD_JOBJ$(type)			FASIVA_PRIMITIVE_FIELD_SCAN

#define FASIVA_PRIMITIVE_FIELD_SCAN(name)			// do nothing

#define FASIVA_INSTANCE_FIELD_SCAN(name)									\
		method((java_lang_Object_p)self->name, scanner);

#define FASIVA_ARRAY_FIELD_SCAN(name)									\
		method(self->name->getInstance$(), scanner);

#define FASIVA_GENERIC_FIELD_SCAN(name)										\
		if (((int)(void*)sekf->name) & 3 == 0) method(this->name);



// ����

/**
  ���� class���� ���� clsss�� ����� field�� ������ �̸��� ������ ����
  ó���ϱ� ���� �ݵ�� declrared-class�� �ʿ��ϴ�.
*/
//#define	FASTIVA_FIELD_SCAN_OFFSET_NATIVE(t)		FASIVA_PRIMITIVE_FIELD_SCAN_OFFSET



#define	FASTIVA_FIELD_SCAN_OFFSET_VAL$(t)		FASIVA_PRIMITIVE_FIELD_SCAN_OFFSET
#define	FASTIVA_FIELD_SCAN_OFFSET_JOBJ$(t)		FASIVA_PRIMITIVE_FIELD_SCAN_OFFSET
#define	FASTIVA_FIELD_SCAN_OFFSET_GENERIC$(CLASS)	FASIVA_GENERIC_FIELD_SCAN_OFFSET
#define	FASTIVA_FIELD_SCAN_OFFSET_ARRAY$(dim, CLASS)	FASIVA_POINTER_FIELD_SCAN_OFFSET
#define	FASTIVA_FIELD_SCAN_OFFSET_PTR$(CLASS)		FASIVA_POINTER_FIELD_SCAN_OFFSET
#define	FASTIVA_STATIC_FIELD_SCAN_OFFSET_VAL$(cls, field)		$$ maybe obsolete && 
#define	FASTIVA_STATIC_FIELD_SCAN_OFFSET_JOBJ$(cls, field)		$$ maybe obsolete && 

#define	FASIVA_PRIMITIVE_FIELD_SCAN_OFFSET(declrared_class, field)		
	// IGNORE

#define	FASIVA_POINTER_FIELD_SCAN_OFFSET(declrared_class, field)			\
	((int)&((declrared_class*)0)->field),

#define	FASIVA_GENERIC_FIELD_SCAN_OFFSET(declrared_class, field)			\
	((int)&((declrared_class*)0)->field) | 1,

	// IGNORE

//#define	FASTIVA_STATIC_FIELD_SCAN_OFFSET_NATIVE(cls, field)		
	// IGNORE

#define	FASTIVA_STATIC_FIELD_SCAN_OFFSET(cls, field, type)					\
	((int)&((declrared_class##_C$*)0)->field),


#ifdef ANDROID
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jbool		jint
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jbyte		jint
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jshort		jint
#define FASTIVA_PRIMITIVE_FIELD_TYPE_unicod		jint
#else
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jbool		jbool
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jbyte		jbyte
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jshort		jshort
#define FASTIVA_PRIMITIVE_FIELD_TYPE_unicod		unicod
#endif

#define FASTIVA_PRIMITIVE_FIELD_TYPE_jint		jint
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jlonglong	jlonglong
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jfloat		jfloat
#define FASTIVA_PRIMITIVE_FIELD_TYPE_jdouble	jdouble


#define FASTIVA_FIELD_TYPE_PTR$(CLASS)			CLASS##_p

#define FASTIVA_FIELD_TYPE_ARRAY$(dim, CLASS)	FASTIVA_MERGE_TOKEN(CLASS, ARRAY_VARIANT_SUFFIX_##dim) //fastiva_Interface_P<CLASS>

#define FASTIVA_FIELD_TYPE_VAL$(type)			FASTIVA_PRIMITIVE_FIELD_TYPE_##type

#define FASTIVA_FIELD_TYPE_JOBJ$(CLASS)			CLASS##_p

#define FASTIVA_FIELD_TYPE_GENERIC$(CLASS)		CLASS##_p



#define FASTIVA_FIELD_ACCESS_TYPE_PTR$(CLASS)			CLASS##_p

#define FASTIVA_FIELD_ACCESS_TYPE_ARRAY$(dim, CLASS)	FASTIVA_MERGE_TOKEN(CLASS, ARRAY_VARIANT_SUFFIX_##dim) //fastiva_Interface_P<CLASS>

#define FASTIVA_FIELD_ACCESS_TYPE_VAL$(type)			type

#define FASTIVA_FIELD_ACCESS_TYPE_JOBJ$(CLASS)			CLASS##_p

#define FASTIVA_FIELD_ACCESS_TYPE_GENERIC$(CLASS)		CLASS##_p

//#define FASTIVA_DECL_FIELD_NATIVE(type)	FASTIVA_DECL_PRIMITIVE_FIELD

#define FASTIVA_DECL_FIELD_ARRAY$(dim, type)		FASTIVA_DECL_POINTER_FIELD
#define FASTIVA_DECL_FIELD_PTR$(type)		FASTIVA_DECL_POINTER_FIELD
#define FASTIVA_DECL_FIELD_GENERIC$(type)	FASTIVA_DECL_GENERIC_FIELD
#define FASTIVA_DECL_FIELD_VAL$(type)	    FASTIVA_DECL_PRIMITIVE_FIELD
#define FASTIVA_DECL_FIELD_JOBJ$(type)		FASTIVA_DECL_JOBJ_FIELD

#ifdef ANDROID
#define FASTIVA_DECL_STATIC_FIELD(acc, type, name)		\
	public:	Field name##_fieldInfo_$$;					\
	union {												\
		type name;										\
		JValue name##_slot$;							\
	};
#else
#define FASTIVA_DECL_STATIC_FIELD(acc, type, name)		\
	FASTIVA_ACC_##acc type name;											
#endif


/**
 2012.11.24
 �ʵ���� ���ǵ� macro�� �浹�ϴ� ��쿡 ����Ͽ� '##'�� ������� �ʴ´�.
 */
#ifdef FASTIVA_GC_DEBUG
#define FASTIVA_CHECK_FIELD(ptr, pField, type) \
	fastiva.checkFieldAccess(ptr, pField, sizeof(type))
#else
#define FASTIVA_CHECK_FIELD(ptr, end_offset, type) // ignore
#endif

#define FASTIVA_DECL_CONSTANT_FIELD(acc, accJni, type, name, getter, setter, const_v)	\
	FASTIVA_ACC_##acc void setter(type v) {									\
		FASTIVA_CHECK_FIELD(this, &name, type);								\
		if ((accJni & ACC_VOLATILE$)) {										\
			this->setVolatileField_$$(v, (type*)(void*)&name);							\
		} 																	\
		else { 																\
			*(type*)(void*)&name = v;														\
		}																	\
	}																		\
	FASTIVA_ACC_##acc static type getter() {							\
		return const_v;													\
	}																		\

#define FASTIVA_DECL_PRIMITIVE_FIELD(acc, accJni, type, name, getter, setter)	\
	FASTIVA_ACC_##acc void setter(type v) {									\
		FASTIVA_CHECK_FIELD(this, &name, type);								\
		if ((accJni & ACC_VOLATILE$)) {										\
			this->setVolatileField_$$(v, (type*)(void*)&name);							\
		} 																	\
		else { 																\
			*(type*)(void*)&name = v;														\
		}																	\
	}																		\
	FASTIVA_ACC_##acc type getter() {										\
		FASTIVA_CHECK_FIELD(this, &name, type);								\
		if ((accJni & ACC_VOLATILE$)) {										\
			return (type)this->getVolatileField_$$((type*)(void*)&name);	\
		}																	\
		else {																\
			return *(type*)(void*)&name;													\
		}																	\
	}																		\


#define FASTIVA_DECL_POINTER_FIELD(acc, accJni, type, name, getter, setter)	\
	FASTIVA_ACC_##acc void setter(type v) {									\
		fastiva_Instance_p* pField = (fastiva_Instance_p*)(void*)&name;		\
		if ((accJni & ACC_VOLATILE$)) {										\
			this->setVolatileField_$$(*(fastiva_Instance_p*)(void*)&v, pField);		\
		}																	\
		else {																\
			this->setField_$$(*(fastiva_Instance_p*)(void*)&v, pField);				\
		}																	\
	}																		\
	FASTIVA_ACC_##acc type getter() {										\
		FASTIVA_CHECK_FIELD(this, &name, type);								\
		if ((accJni & ACC_VOLATILE$)) {										\
			return (type)this->getVolatileField_$$((fastiva_Instance_p*)(void*)&name);  	\
		}																	\
		else {																\
			return *(type*)(void*)&name;													\
		}																	\
	}																		\


#define FASTIVA_DECL_JOBJ_FIELD(acc, accJni, type, name, getter, setter)	\
	FASTIVA_ACC_##acc void setter(type v) {									\
		void* pField = (void*)&name;										\
		this->setField_$$((fastiva_BytecodeProxy_p)*(void**)&v, pField);	\
	}																		\
	FASTIVA_ACC_##acc type getter() {										\
		return *(type*)(void*)&name;										\
	}																		\


#define FASTIVA_DECL_GENERIC_FIELD(acc, accJni, type, name, getter, setter)	\
	FASTIVA_ACC_##acc void setter(type v) {									\
		void* pField = (void*)&name;										\
		this->setField_$$((fastiva_lang_Generic_p)*(void**)&v, pField);		\
	}																		\
	FASTIVA_ACC_##acc type getter() {										\
		return *(type*)(void*)&name;										\
	}																		\


/*=========================================================================*/

#define FASTIVA_JNI_ACC_public			ACC_PUBLIC$
#define FASTIVA_JNI_ACC_protected		ACC_PROTECTED$
#define FASTIVA_JNI_ACC_private			ACC_PRIVATE$
#define FASTIVA_JNI_ACC__(package)		0
#define FASTIVA_JNI_ACC_package			0

#define FASTIVA_JNI_FLAG_JNI$(acc, ...)											(acc)
#define FASTIVA_JNI_GENERIC_SIG_JNI$(acc, sig, ...)								(char*)sig
#define FASTIVA_JNI_SFIELD_INDEX_JNI$(acc, sig, index, ...)						index
#define FASTIVA_JNI_BRIDGE_JNI$(acc, sig, bridge, ...)							fastiva_jni_bridge_##bridge
#define FASTIVA_JNI_UTF_NAME_JNI$(acc, sig, b, a, name, ...)					name
#define FASTIVA_JNI_ANNOTATIONS_JNI$(acc, sig, b, annotations, ...)				(annotations < 0 ? NULL : &fastiva_AnnotationCache$[annotations])
#define FASTIVA_JNI_SHORT_SIG_JNI$(acc, sig, b, a, name, shortSig, ...)			shortSig
#define FASTIVA_JNI_ARG_INFO_JNI$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)	jniArgInfo
#define FASTIVA_JNI_VIRTUAL_PROXY_THUNK_JNI$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)	FASTIVA_VIRTUAL_PROXY_THUNK
#define FASTIVA_JNI_FINAL_PROXY_THUNK_JNI$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)		FASTIVA_FINAL_PROXY_THUNK
#define FASTIVA_JNI_STATIC_PROXY_THUNK_JNI$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)	FASTIVA_STATIC_PROXY_THUNK
#define FASTIVA_JNI_STATIC_THUNK_JNI$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)			FASTIVA_STATIC_THUNK
#define FASTIVA_JNI_PARAM_ANNOTATIONS_JNI$(acc, sig, b, a, name, shortSig, jniArgInfo, pa)		(pa < 0 ? NULL : &fastiva_ParameterAnnotationCache$[pa])

#define FASTIVA_JNI_FLAG_INL$(acc, ...)											(acc)
#define FASTIVA_JNI_GENERIC_SIG_INL$(acc, sig, ...)								(char*)sig
#define FASTIVA_JNI_SFIELD_INDEX_INL$(acc, sig, index, ...)						index
#define FASTIVA_JNI_BRIDGE_INL$(acc, sig, bridge, ...)							fastiva_jni_bridge_##bridge
#define FASTIVA_JNI_UTF_NAME_INL$(acc, sig, b, a, name, ...)					name
#define FASTIVA_JNI_ANNOTATIONS_INL$(acc, sig, b, annotations, ...)				(annotations < 0 ? NULL : &fastiva_AnnotationCache$[annotations])
#define FASTIVA_JNI_SHORT_SIG_INL$(acc, sig, b, a, name, shortSig, ...)			shortSig
#define FASTIVA_JNI_ARG_INFO_INL$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)	jniArgInfo
#define FASTIVA_JNI_VIRTUAL_PROXY_THUNK_INL$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)	FASTIVA_INLINE_PROXY_THUNK
#define FASTIVA_JNI_FINAL_PROXY_THUNK_INL$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)		FASTIVA_INLINE_PROXY_THUNK
#define FASTIVA_JNI_STATIC_PROXY_THUNK_INL$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)		FASTIVA_STATIC_THUNK
#define FASTIVA_JNI_STATIC_THUNK_INL$(acc, sig, b, a, name, shortSig, jniArgInfo, ...)			FASTIVA_STATIC_THUNK
#define FASTIVA_JNI_PARAM_ANNOTATIONS_INL$(acc, sig, b, a, name, shortSig, jniArgInfo, pa)		(pa < 0 ? NULL : &fastiva_ParameterAnnotationCache$[pa])


#define FASTIVA_JNI_FLAG_VIRTUAL			(ACC_VIRTUAL$)
#define FASTIVA_JNI_FLAG_ABSTRACT			(ACC_VIRTUAL$ | ACC_ABSTRACT$)
#define FASTIVA_JNI_FLAG_OVERRIDE			(ACC_VIRTUAL$)
#define FASTIVA_JNI_FLAG_SYNTHETIC			(ACC_SYNTHETIC$)
#define FASTIVA_JNI_FLAG_NON_VIRTUAL		(ACC_FINAL$)
#define FASTIVA_JNI_FLAG_STATIC				(ACC_STATIC$)


#define FASTIVA_METHOD_SELF_TYPE(CLASS, ret_t, slot, cnt, args) \
	FASTIVA_MERGE_TOKEN(CLASS, _I$)::FASTIVA_METHOD_SLOT_OWNER_TYPE(ret_t, slot, cnt, args)::THIS_p$

#define	FASTIVA_METHOD_ADDR_NON_VIRTUAL(CLASS, ret_t, slot, fn, cnt, args) \
	((FASTIVA_RETURN_TYPE_##ret_t (*)(FASTIVA_INSTANCE_PARAM(TYPES, \
		FASTIVA_METHOD_SELF_TYPE(CLASS, ret_t, slot, cnt, args), cnt, args))) \
			FASTIVA_MERGE_TOKEN(CLASS, _I$)::FASTIVA_METHOD_SLOT_OWNER_TYPE(ret_t, slot, cnt, args)::fn)

#define	FASTIVA_METHOD_ADDR_STATIC(CLASS, ret_t, slot, fn, cnt, args)	\
	((FASTIVA_RETURN_TYPE_##ret_t (*) FASTIVA_STATIC_PARAM_TYPES(cnt, args)) \
		CLASS::fn)

#define	FASTIVA_METHOD_ADDR_OVERRIDE(CLASS, ret_t, slot, fn, cnt, args) \
	FASTIVA_METHOD_ADDR_NON_VIRTUAL(CLASS, ret_t, slot, fn, cnt, args)

#define	FASTIVA_METHOD_ADDR_ABSTRACT(CLASS, ret_t, slot, fn, cnt, args) \
	fastiva.throwAbstractMethodError

#define	FASTIVA_METHOD_ADDR_VIRTUAL(CLASS, ret_t, slot, fn, cnt, args) \
	FASTIVA_METHOD_ADDR_NON_VIRTUAL(CLASS, ret_t, slot, fn, cnt, args)





#define FASTIVA_PRIMITIVE_CONTEXT_PTR(type)									\
	(fastiva_ClassContext*)(void*)fastiva_Primitives::type::idMoniker$


// @todo 2011.0622 VAL -> VAL$ �۾� �̷��. 
#define FASTIVA_JNI_FIELD_CONTEXT_OF_VAL(VTYPE)								\
	FASTIVA_PRIMITIVE_CONTEXT_PTR(VTYPE) && OBSOLETE &&
#define FASTIVA_JNI_FIELD_CONTEXT_OF_IFC(VTYPE)								\
	FASTIVA_RAW_CLASS_CONTEXT_PTR(VTYPE) && OBSOLETE &&
#define FASTIVA_JNI_FIELD_CONTEXT_OF_PTR(VTYPE)								\
	FASTIVA_RAW_CLASS_CONTEXT_PTR(VTYPE) && OBSOLETE &&
#define FASTIVA_JNI_FIELD_CONTEXT_OF_VAL_A(VTYPE)							\
	(fastiva_ClassContext*)(void*)VTYPE::BASE_T$::idMoniker$ && OBSOLETE &&
#define FASTIVA_JNI_FIELD_CONTEXT_OF_PTR_A(VTYPE)							\
	FASTIVA_RAW_CLASS_CONTEXT_PTR(VTYPE::BASE_T$) && OBSOLETE &&







//=========================================================================//

#define FASTIVA_DECL_GET_NATIVE_METHOD_ADDR_FUNC(CLASS)						\
		const void* CLASS::getNativeMethodAddress(int idx) {				\
			return FASTIVA_REVERSE_MERGE(FASTIVA_REVERSE_MERGE(				\
					$, FASTIVA_PREPROCESS_CLASS), aMethodInfo_)[idx].m_pDeclared;	\
		}

#define FASTIVA_JNI_METHOD_ADDR(acc, ret_t, fn, cnt, args)					\
	    && obsolete &&														\
		((ret_t (FOX_JNI_CALL*)(											\
			void*, FASTIVA_FIRST_PARAM_##cnt(FASTIVA, java_lang_Object_p)	\
			FASTIVA_PARAM_TYPES_##cnt args))								\
			THIS$::getNativeMethodAddress(									\
				(int)&((THIS$::IMPLEMENTATION$::NATIVE_SLOT$*)0)			\
				->FASTIVA_NATIVE_SLOT_NAME_##acc(fn, cnt, args)))				\

			
#define FASTIVA_JNI_METHOD_SLOT(acc, ret, fn, cnt, args)					\
		jbyte FASTIVA_NATIVE_SLOT_NAME_##acc(fn, cnt, args);

#define FASITVA_JNI_NATIVE_METHOD_SLOT(acc, ret, fn, cnt, args)				\
		jbyte FASTIVA_NATIVE_SLOT_NAME_##acc(fn, cnt, args);

/*
#define FASTIVA_JNI_NATIVE_FILTER_public(statement)			FASTIVA_NOTHING
	
#define FASTIVA_JNI_NATIVE_FILTER_protected(statement)		FASTIVA_NOTHING

#define FASTIVA_JNI_NATIVE_FILTER_(package)					FASTIVA_NOTHING

#define FASTIVA_JNI_NATIVE_FILTER_private(statement)		FASTIVA_NOTHING

#define FASTIVA_JNI_NATIVE_FILTER_inline(acc)	FASTIVA_JNI_NATIVE_FILTER_##acc

#define FASTIVA_JNI_FILTERED_NATIVE(statement)	statement

#define FASTIVA_JNI_NATIVE_FILTER_native(acc)	FASTIVA_JNI_FILTERED_NATIVE

#define FASTIVA_JNI_METHOD_SLOT_FILTER(args)	FASTIVA_JNI_METHOD_SLOT args
*/



#ifndef FASTIVA_NO_USE_INTERFACE_SLOT
	#define FASTIVA_METHOD_PARAM_INFO()		, 0
#else
	#define FASTIVA_METHOD_PARAM_INFO()		// IGNORE
#endif

#ifndef FASTIVA_NO_INTERPRETER
	#define FASTIVA_BYTE_CODE_INFO()		, 0
#else
	#define FASTIVA_BYTE_CODE_INFO()		// IGNORE
#endif

/*
#define FASTIVA_RETURN_TYPE_SIG_REF(ret_t)	FASTIVA_SIG_##ret_t

#define FASTIVA_RETURN_TYPE_SIG_VAL(ret_t)	FASTIVA_SIG_##ret_t

#define FASTIVA_RETURN_TYPE_SIG_void	"V"
*/

/*
#define FASTIVA_VIRTUAL_SLOT_OVERLOAD_OFFSET(acc, fn, cnt, args)			\
	FASTIVA_MERGE_TOKEN(FASTIVA_METHOD_SLOT_NAME(ret_t, fn, cnt, args), _offset)
*/

#define FASTIVA_VIRTUAL_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args) \
	CLASS::VTABLE$::FASTIVA_METHOD_SLOT_NAME(ret_t, OFFSET_##slot, cnt, args)

#define FASTIVA_INTERFACE_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args)	\
	FASTIVA_VIRTUAL_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args)

#define FASTIVA_INTERFACE_SLOT_OFFSET_obsolete(CLASS, ret_t, slot, cnt, args)	\
	(((int)&((CLASS::VTABLE$*)0x1000)-> \
		FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args) - 0x1000) / sizeof(void*))

#define FASTIVA_OVERRIDE_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args) \
	FASTIVA_VIRTUAL_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args)

#define FASTIVA_ABSTRACT_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args)	\
	FASTIVA_VIRTUAL_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args)

/*
#define FASTIVA_SYNTHETIC_SLOT_OFFSET(acc, CLASS, fn, cnt, args)				\
	FASTIVA_VIRTUAL_SLOT_OFFSET(acc, CLASS, fn, cnt, args)
*/

#define FASTIVA_NON_VIRTUAL_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args) \
	(FASTIVA_OFFSETOF(FASTIVA_MERGE_TOKEN(CLASS, _EXVT$), \
		FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args)) / sizeof(void*))


#define FASTIVA_STATIC_SLOT_OFFSET(CLASS, ret_t, slot, cnt, args) \
	(FASTIVA_OFFSETOF(FASTIVA_MERGE_TOKEN(CLASS, _EXVT$), \
		FASTIVA_METHOD_SLOT_NAME(ret_t, slot##_S$, cnt, args)) / sizeof(void*))

#define FASTIVA_STATIC_SLOT_OFFSET_obsolete(CLASS, ret_t, slot, cnt, args) \
	(FASTIVA_OFFSETOF(FASTIVA_MERGE_TOKEN(CLASS, _C$), \
		FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args)) / sizeof(void*))




#define FASTIVA_JNI_CONTEXT_OF(acc, fn, cnt, args)							\
		FASTIVA_RAW_CLASS_CONTEXT_PTR(FASTIVA_PREPROCESS_CLASS)





/*
#define FASTIVA_JNI_INTERFACE_METHOD_INFO(acc, ret, fn, cnt, args, type, acc_ex)\

#define FASTIVA_JNI_VIRTUAL_METHOD_INFO(acc, ret, fn, cnt, args, type, acc_ex)\

#define FASTIVA_JNI_FINAL_METHOD_INFO(acc, ret, fn, cnt, args, type, acc_ex)\


#define FASTIVA_JNI_STATIC_METHOD_INFO(acc, ret, fn, cnt, args, type, acc_ex)\

#define FASTIVA_JNI_CONSTRUCTOR_INFO(acc, cnt, args) 						\

#define FASTIVA_JNI_METHOD_INFO_1(args)		FASTIVA_JNI_METHOD_INFO args
*/









#define FASTIVA_FAST_STATIC(acc, ret_t)	&& OBSOLETED && FASTIVA_ACC_##acc static ret_t __fastcall



#define JPP_VIRTUAL(acc, ret_t)		FASTIVA_ACC_##acc FASTIVA_VIRTUAL FASTIVA_RETURN_TYPE_##ret_t


#define JPP_OVERRIDE(acc, ret_t)	FASTIVA_ACC_##acc FASTIVA_VIRTUAL FASTIVA_RETURN_TYPE_##ret_t


#define JPP_INTERFACE_SLOT(acc, ret_t)	FASTIVA_ACC_##acc FASTIVA_RETURN_TYPE_##ret_t


#define JPP_FINAL(acc, ret_t)	FASTIVA_ACC_##acc FASTIVA_RETURN_TYPE_##ret_t

#define JPP_STATIC(acc, ret_t)	FASTIVA_ACC_##acc FASTIVA_RETURN_TYPE_##ret_t

#define JNI_ARG_TYPE_PTR$(CLASS)				jobject
#define JNI_ARG_TYPE_JOBJ$(CLASS)				jobject
#define JNI_ARG_TYPE_ARRAY$(dim, CLASS)				jobject
#define JNI_ARG_TYPE_VAL$(ret_t)				ret_t

#define JNI_PRIMITIVE_RET_TYPE_jbool			Boolean
#define JNI_PRIMITIVE_RET_TYPE_jbyte			Byte
#define JNI_PRIMITIVE_RET_TYPE_jchar			Char
#define JNI_PRIMITIVE_RET_TYPE_jshort			Short
#define JNI_PRIMITIVE_RET_TYPE_jint				Int
#define JNI_PRIMITIVE_RET_TYPE_jlong			Long
#define JNI_PRIMITIVE_RET_TYPE_jfloat			Float
#define JNI_PRIMITIVE_RET_TYPE_jdouble			Double


#ifndef FASTIVA_NO_SUPPORTS_JAVA_PROXY
	#define JNI_CALL_METHOD_PTR$(CLASS)				fastiva_jni_getEnv()->CallObjectMethod
	#define JNI_CALL_METHOD_JOBJ$(CLASS)			fastiva_jni_getEnv()->CallObjectMethod
	#define JNI_CALL_METHOD_ARRAY$(dim, CLASS)				fastiva_jni_getEnv()->CallObjectMethod
	#define JNI_CALL_METHOD_void					fastiva_jni_getEnv()->CallVoidMethod
	#define JNI_CALL_METHOD_VAL$(ret_t)				(ret_t)fastiva_jni_getEnv()->FASTIVA_MERGE_3TOKEN(Call, JNI_PRIMITIVE_RET_TYPE_##ret_t, Method)

	#define JNI_CALL_STATIC_METHOD_PTR$(CLASS)				fastiva_jni_getEnv()->CallStaticObjectMethod
	#define JNI_CALL_STATIC_METHOD_JOBJ$(CLASS)				fastiva_jni_getEnv()->CallStaticObjectMethod
	#define JNI_CALL_STATIC_METHOD_ARRAY$(dim, CLASS)				fastiva_jni_getEnv()->CallStaticObjectMethod
	#define JNI_CALL_STATIC_METHOD_void						fastiva_jni_getEnv()->CallStaticVoidMethod
#define JNI_CALL_STATIC_METHOD_VAL$(ret_t)				(ret_t)fastiva_jni_getEnv()->FASTIVA_MERGE_3TOKEN(CallStatic, JNI_PRIMITIVE_RET_TYPE_##ret_t, Method)



#define JNI_GET_FIELD_PTR$(CLASS)				(CLASS##_p)fastiva_jni_getEnv()->GetObjectField
	#define JNI_GET_FIELD_JOBJ$(CLASS)				(CLASS##_p)fastiva_jni_getEnv()->GetObjectField
	#define JNI_GET_FIELD_ARRAY$(dim, CLASS)				(CLASS##_p)fastiva_jni_getEnv()->GetObjectField
	#define JNI_GET_FIELD_VAL$(ret_t)				(ret_t)fastiva_jni_getEnv()->FASTIVA_MERGE_3TOKEN(Get, JNI_PRIMITIVE_RET_TYPE_##ret_t, Field)

	#define JNI_GET_STATIC_FIELD_PTR$(CLASS)				(CLASS##_p)fastiva_jni_getEnv()->GetStaticObjectField
	#define JNI_GET_STATIC_FIELD_JOBJ$(CLASS)				(CLASS##_p)fastiva_jni_getEnv()->GetStaticObjectField
	#define JNI_GET_STATIC_FIELD_ARRAY$(dim, CLASS)				(CLASS##_p)fastiva_jni_getEnv()->GetStaticObjectField
	#define JNI_GET_STATIC_FIELD_VAL$(ret_t)				(ret_t)fastiva_jni_getEnv()->FASTIVA_MERGE_3TOKEN(GetStatic, JNI_PRIMITIVE_RET_TYPE_##ret_t, Field)

	#define JNI_SET_FIELD_PTR$(CLASS)				fastiva_jni_getEnv()->SetObjectField
	#define JNI_SET_FIELD_JOBJ$(CLASS)				fastiva_jni_getEnv()->SetObjectField
	#define JNI_SET_FIELD_ARRAY$(dim, CLASS)				fastiva_jni_getEnv()->SetObjectField
	#define JNI_SET_FIELD_VAL$(type)				fastiva_jni_getEnv()->FASTIVA_MERGE_3TOKEN(Set, JNI_PRIMITIVE_RET_TYPE_##type, Field)

	#define JNI_SET_STATIC_FIELD_PTR$(CLASS)				fastiva_jni_getEnv()->SetStaticObjectField
	#define JNI_SET_STATIC_FIELD_JOBJ$(CLASS)				fastiva_jni_getEnv()->SetStaticObjectField
	#define JNI_SET_STATIC_FIELD_ARRAY$(dim, CLASS)				fastiva_jni_getEnv()->SetStaticObjectField
	#define JNI_SET_STATIC_FIELD_VAL$(type)					fastiva_jni_getEnv()->FASTIVA_MERGE_3TOKEN(SetStatic, JNI_PRIMITIVE_RET_TYPE_##type, Field)

	#define JPP_JAVA_JNI_METHOD_ID(slot, cnt, args)								\
		FASTIVA_MERGE_TOKEN(m_jniMethodID_, FASTIVA_METHOD_SLOT_NAME(ret_t, slot, cnt, args))

   #define FASTIVA_DECL_METHOD(CLASS, ret_t, fn, cnt, args)				\
		FASTIVA_RETURN_TYPE_##ret_t	fn (FASTIVA_PARAM_PAIRS_##cnt args)

	#define FASTIVA_CALL_JAVA_METHOD(CLASS, IFC, ret_t, fn, cnt, args)			\
		FASTIVA_RETURN_TYPE_##ret_t	CLASS::fn (FASTIVA_PARAM_PAIRS_##cnt args)	{ \
			FASTIVA_RETURN_INSTRUCUTION_##ret_t									\
				JNI_CALL_METHOD_##ret_t((jobject)this, (jmethodID)FASTIVA_MERGE_TOKEN(m_jniMethodID_, fn) FASTIVA_PARAM_SEPERATOR_##cnt FASTIVA_PARAM_NAMES_##cnt args);	\
		}																		\

	#define FASTIVA_CALL_JAVA_STATIC_METHOD(CLASS, IFC, ret_t, fn, cnt, args)					\
		FASTIVA_RETURN_TYPE_##ret_t	CLASS##_C$::fn (FASTIVA_PARAM_PAIRS_##cnt args)	{ \
			FASTIVA_RETURN_INSTRUCUTION_##ret_t									\
				JNI_CALL_STATIC_METHOD_##ret_t((jclass)g_javaClass, (jmethodID)FASTIVA_MERGE_TOKEN(m_jniMethodID_, fn) FASTIVA_PARAM_SEPERATOR_##cnt FASTIVA_PARAM_NAMES_##cnt args);	\
		}																		\

	#define FASTIVA_CALL_JAVA_CONSTRUCTOR_METHOD(CLASS, IFC, ret_t, fn, cnt, args)					\
		FASTIVA_RETURN_TYPE_##ret_t	CLASS::fn (FASTIVA_PARAM_PAIRS_##cnt args)	{ \
			jobject obj = fastiva_jni_getEnv()->NewObject((jclass)g_javaClass, (jmethodID)FASTIVA_MERGE_TOKEN(m_jniMethodID_, fn) FASTIVA_PARAM_SEPERATOR_##cnt FASTIVA_PARAM_NAMES_##cnt args);	\
			return obj;														\
		}																		\

#endif

//#define JPP_GENERIC_API(acc, ret_t)	FASTIVA_ACC_##acc ret_t __fastcall 

// ==============================================================
// FOR GENENRAL 
// ==============================================================



#endif // __FASTIVA_PREPROCESS_H__




/**====================== end ========================**/
