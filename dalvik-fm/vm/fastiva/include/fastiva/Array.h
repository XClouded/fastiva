#ifndef __FASTIVA_ARRAY_H__
#define __FASTIVA_ARRAY_H__


//#include <java/lang/Cloneable.ptrh>
//#include <java/io/Serializable.h>

enum {
	fastiva_NO_ARRAY_ITEM$ = 0x80008000
};

#ifdef _DEBUG
	#define UNSAFE_ARRAY_CAST$(ARRAY_p, pObj)								\
		((ARRAY_p)0)->ptr_cast$(pObj)
#else
	#define UNSAFE_ARRAY_CAST$(ARRAY_p, pObj)								\
		((ARRAY_p)(fastiva_Instance_p)(pObj))
#endif


#define ARRAY_GET$(array, idx)		array->get_ex$(idx, array##_length$)
#define ARRAY_SET$(array, idx, v)	array->set_ex$(idx, array##_length$, v)


//=========================================================================//
// fastiva_ArrayHeader
//=========================================================================//

class fastiva_ArrayHeader : public java_lang_Object_T$
{
FASTIVA_RUNTIME_CRITICAL(protected):
	const jint m_length;
#if FASTIVA_SUPPORTS_DYNAMIC_ARRAY

#if !FASTIVA_SUPPORT_JNI_STUB
	void* m_javaRef$;
#endif

	void* getBuffer_unsafe$() const {
		return *(void**)(this + 1);
	}
#else
#ifdef ANDROID
	jint just_for_8byte_align_padding;
#endif
	void* getBuffer_unsafe$() const {
		return (void*)(this + 1);
	}
#endif

protected:
	fastiva_ArrayHeader(int length = 0) : m_length(length) {}

public:
	java_lang_Object_p getInstance$() FASTIVA_PURE_API { return this; }

	int length() const {
		KASSERT(m_length >= 0);
		return m_length;
	}

	java_lang_Cloneable_p as__java_lang_Cloneable() const {
		return (java_lang_Cloneable_p)this;
	}

	java_io_Serializable_p as__java_io_Serializable() const {
		return (java_io_Serializable_p)this;
	}

	void* operator new (size_t, void* ptr) { return ptr; }

	void checkBound(int idx) const {
		this->checkBound_ex$(idx, m_length);
	}

	void checkBound_ex$(int idx, int bound) const {
	#ifndef FASTIVA_NO_ARRAY_BOUND_CHECK
		KASSERT(bound >= 0);
		if ((unsigned int)idx >= (unsigned int)bound) {
			fastiva.throwArrayIndexOutOfBoundsException();
		}
	#endif
	}

	void checkBound(int idx, int len) const {
		KASSERT(m_length >= 0);
		if ((int)(idx | len) < 0
		||  (unsigned int)(idx + len) > (unsigned int)m_length) {
			fastiva.throwArrayIndexOutOfBoundsException();
		}
	}

	static void* isExactInstanceOf(fastiva_Instance_p pObj, java_lang_Class_p pClass) FASTIVA_PURE_API {
		if (pObj != FASTIVA_NULL && pObj->getClass$() == (void*)pClass) {
			return pObj;
		}
		return (void*)FASTIVA_NULL;
	}

	static void* checkExactInstanceOf(fastiva_Instance_p pObj, java_lang_Class_p pClass) FASTIVA_PURE_API {
		if (pObj == FASTIVA_NULL || pObj->getClass$() == (void*)pClass) {
			return pObj;
		}
		fastiva.throwClassCastException(pObj, pClass);
	}

	typedef class fastiva_ArrayHeader_A COMPOSITE_T$;
	typedef class java_lang_Object OBJECT_ARRAY_HEADER_T;

};

#if 0
// @todo array-assign ������ �ٲ����.
// @todo fastiva_NO_ARRAY_ITEM$ �� ���� �߰��Ǿ���.
class fastiva_ArrayHeader_A : public fastiva_ArrayHeader {
public:
	enum { dimension$ = 1 }; 
	typedef class fastiva_ArrayHeader    COMPONENT_T$;
	typedef class fastiva_ArrayHeader_AA COMPOSITE_T$;
	class OBJECT_ARRAY_HEADER_T {};
};

class fastiva_ArrayHeader_AA : public fastiva_ArrayHeader_A {
public:
	enum { dimension$ = 2 }; 
	typedef class fastiva_ArrayHeader_A   COMPONENT_T$;
	typedef class fastiva_ArrayHeader_AAA COMPOSITE_T$;
	class OBJECT_ARRAY_HEADER_T
		: public fastiva_ArrayHeader_A::OBJECT_ARRAY_HEADER_T {};

};



class fastiva_ArrayHeader_AAA : public fastiva_ArrayHeader_AA {
public:
	enum { dimension$ = 3 }; 
	typedef class fastiva_ArrayHeader_AA   COMPONENT_T$;
	typedef class fastiva_ArrayHeader_AAAA COMPOSITE_T$;
	class OBJECT_ARRAY_HEADER_T
		: public fastiva_ArrayHeader_AA::OBJECT_ARRAY_HEADER_T {};

};


class fastiva_ArrayHeader_AAAA : public fastiva_ArrayHeader_AAA {
public:
	enum { dimension$ = 4 }; 
	typedef class fastiva_ArrayHeader_AAA   COMPONENT_T$;
	typedef class fastiva_ArrayHeader_AAAAA COMPOSITE_T$;
	class OBJECT_ARRAY_HEADER_T
		: public fastiva_ArrayHeader_AAA::OBJECT_ARRAY_HEADER_T {};
	
};

class fastiva_ArrayHeader_AAAAA : public fastiva_ArrayHeader_AAAA {
public:
	enum { dimension$ = 5 }; 
	typedef class fastiva_ArrayHeader_AAAA   COMPONENT_T$;
	typedef class fastiva_ArrayHeader_AAAAAA COMPOSITE_T$;
	class OBJECT_ARRAY_HEADER_T
		: public fastiva_ArrayHeader_AAAA::OBJECT_ARRAY_HEADER_T {};
	
};

class fastiva_ArrayHeader_AAAAAA : public fastiva_ArrayHeader_AAAAA {
public:
	enum { dimension$ = 6 }; 
	typedef class fastiva_ArrayHeader_AAAAA   COMPONENT_T$;
	typedef class fastiva_ArrayHeader_AAAAAAA COMPOSITE_T$;
	class OBJECT_ARRAY_HEADER_T
		: public fastiva_ArrayHeader_AAAAA::OBJECT_ARRAY_HEADER_T {};
	
};

class fastiva_ArrayHeader_AAAAAAA : public fastiva_ArrayHeader_AAAAAA {
public:
	enum { dimension$ = 7 }; 
	typedef class fastiva_ArrayHeader_AAAAAA   COMPONENT_T$;
	typedef class fastiva_ArrayHeader_AAAAAAAA COMPOSITE_T$;
	class OBJECT_ARRAY_HEADER_T
		: public fastiva_ArrayHeader_AAAAAA::OBJECT_ARRAY_HEADER_T {};
	
};

/*
template <class BASE_T, class ITEM_T, class HEADER> 
class fm::Array : public HEADER {
public:
	typedef ITEM_T		ITEM_T;
};
*/
#endif

template <int T_ID, class ITEM_T, int dimension> 
class fastiva_PrimitiveArray : public fastiva_ArrayHeader {
public:
	//zzz // NDK_TODO : ������ ������ �����ϱ� ���� �ϴ� �Ʒ��� ���� ������. Ȯ�� �� ���۾� �ʿ�
	enum { dimension$ = dimension }; 

	class Buffer {
	public:
		Buffer(fastiva_PrimitiveArray* pArray) {
			m_pArray = pArray;
			m_pItem = getBufferPtr() + 0;
		}
		Buffer(fastiva_PrimitiveArray* pArray, int idx) {
			m_pArray = pArray;
			m_pArray->checkBound(idx);
			m_pItem = getBufferPtr() + idx;
		}
		Buffer(fastiva_PrimitiveArray* pArray, int idx, int len) {
			m_pArray = pArray;
			m_pArray->checkBound(idx, len);
			m_pItem = getBufferPtr() + idx;
		}
		operator ITEM_T* () const {
			return m_pItem;
		}
		ITEM_T& operator [] (int idx) const {
			return m_pItem[idx];
		}
		~Buffer() {
			//deleteBuffer$((void*)this);
		}
	protected:
		ITEM_T* getBufferPtr() const {
			return m_pArray->m_aItem;
		}
		fastiva_PrimitiveArray* m_pArray;
		ITEM_T* m_pItem;
	};

	ITEM_T* getBuffer_unsafe$() const {
		return (ITEM_T*)m_aItem;
	}

	public: static fastiva_PrimitiveArray* Null$() FASTIVA_PURE_API {
		return (fastiva_PrimitiveArray*)FASTIVA_NULL;
	}

	static uint getItemTypeID() FASTIVA_PURE_API {
		return T_ID;
	}


	ITEM_T get$(int idx) const {
		this->checkBound(idx);
		return m_aItem[idx];
	}

	ITEM_T get_ex$(int idx, int bound) const {
		this->checkBound_ex$(idx, bound);
		return m_aItem[idx];
	}

	ITEM_T get_unsafe$(int idx) const {
		return m_aItem[idx];
	}

	void set$(int idx, ITEM_T value) {
#ifdef FASTIVA_GC_DEBUG
		fastiva.ensureArray(this, sizeof(value));
#endif
		this->checkBound(idx);
		set_unsafe$(idx, value);
	}

	void set_ex$(int idx, int bound, ITEM_T value) {
#ifdef FASTIVA_GC_DEBUG
		fastiva.ensureArray(this, sizeof(value));
#endif
		this->checkBound_ex$(idx, bound);
		set_unsafe$(idx, value);
	}

	void set_unsafe$(int idx, ITEM_T value) {
		if (dimension == 1) {
			m_aItem[idx] = value;
		}
		else {
			fastiva.setArrayItem(this, getRawStatic$()->getContext$(), 
				m_aItem + idx, (fastiva_Instance_p)(int)value);
		}
	}

	static fastiva_Class_p getRawStatic$() FASTIVA_PURE_API {
		return importClass$();
	}

	static fastiva_PrimitiveClass_p importClass$() FASTIVA_PURE_API {
		return (fastiva_PrimitiveClass_p)fastiva.getPrimitiveClass(T_ID, dimension$);
		//return (fastiva_PrimitiveClass_p)fastiva.primitiveArrayClasses[T_ID][dimension$-1];
	}

	static fastiva_PrimitiveArray* isInstance$(fastiva_Instance_p pObj) FASTIVA_PURE_API {
		return (fastiva_PrimitiveArray*)isExactInstanceOf(pObj, importClass$());
	}

	static fastiva_PrimitiveArray* ptr_cast$(fastiva_Instance_p pObj) FASTIVA_PURE_API {
		return (fastiva_PrimitiveArray*)checkExactInstanceOf(pObj, importClass$());
	}

	static fastiva_PrimitiveArray* dbg_cast$(fastiva_Instance_p pObj) FASTIVA_PURE_API {
#if defined(_DEBUG) || !FASTIVA_SUPPORTS_GENERIC_CAST
		return (fastiva_PrimitiveArray*)checkExactInstanceOf(pObj, importClass$());
#else
		return (fastiva_PrimitiveArray*)pObj;
#endif
	}

	static fastiva_PrimitiveArray* create$(java_lang_Class_p pArrayClass, int length) {
		FASTIVA_DBREAK();
	}

	static fastiva_PrimitiveArray* create$(int d1, int d2 = fastiva_NO_ARRAY_ITEM$, 
								int d3 = fastiva_NO_ARRAY_ITEM$, 
								int d4 = fastiva_NO_ARRAY_ITEM$) {
		if (dimension$ == 1) {
			KASSERT(T_ID < fastiva_Primitives::jvoid);
			return (fastiva_PrimitiveArray*)
				fastiva.allocatePrimitiveArray(importClass$(), d1);
		}

		int aLength[4];
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

		return (fastiva_PrimitiveArray*)fastiva.allocateMultiArray(
			importClass$(), aLength);
	}

	static int getAllocSize(int length) {
		return (int)&((fastiva_PrimitiveArray*)0)->m_aItem[length];
	} 


FASTIVA_RUNTIME_CRITICAL(protected):
#if FASTIVA_SUPPORTS_DYNAMIC_ARRAY
	//typedef	ITEM_T ITEM_ARRAY_T[64];
	//ITEM_ARRAY_T& m_aItem;
	ITEM_T* m_aItem;
#else
	ITEM_T m_aItem[64];
#endif
};


template <class BASE_T, class ITEM_T, int dimension, bool isArraySuperType> 
class fastiva_CustomArray : public fastiva_PrimitiveArray<fastiva_Primitives::jcustom, ITEM_T, dimension> 
{
private:
	typedef fastiva_PrimitiveArray<fastiva_Primitives::jcustom, ITEM_T, dimension> SUPER_T$;

public:
	void set$(int idx, ITEM_T value) {
#ifdef FASTIVA_GC_DEBUG
		fastiva.ensureArray(this, sizeof(value));
#endif
		this->checkBound(idx);
		if (dimension == 1) {
			fastiva.setCustomArrayItem(this, ((SUPER_T$*)this)->m_aItem + idx, (BASE_T*)value);
		}
		else {
			set_unsafe$(idx, value);
		}
	}

	static fastiva_CustomArray* create$(int d1, int d2 = fastiva_NO_ARRAY_ITEM$, 
								int d3 = fastiva_NO_ARRAY_ITEM$, 
								int d4 = fastiva_NO_ARRAY_ITEM$) {
		return (fastiva_CustomArray*)SUPER_T$::create$(d1, d2, d3, d4);
	}
};


template <class BASE_T, class ITEM_T, int dimension, bool isArraySuperType> 
class fastiva_PointerArray : public fastiva_ArrayHeader { // fm::Array<BASE_T, ITEM_T, HEADER> {
public:
	enum { dimension$ = dimension }; 
	
	typedef BASE_T		BASE_T$;

	static fastiva_PointerArray* Null$() {
		return (fastiva_PointerArray*)FASTIVA_NULL;
	}

	ITEM_T get$(int idx) const {
		this->checkBound(idx);
		java_lang_Object_p item = m_aRawItem[idx];
		//KASSERT(sizeof(ITEM_T) == sizeof(void*));
		return *(ITEM_T*)&item;
	}

	ITEM_T get_ex$(int idx, int bound) const {
		this->checkBound_ex$(idx, bound);
		java_lang_Object_p item = m_aRawItem[idx];
		//KASSERT(sizeof(ITEM_T) == sizeof(void*));
		return *(ITEM_T*)&item;
	}

	void set$(int idx, ITEM_T value) {
		this->set_ex$(idx, m_length, value);
	}

	void set_ex$(int idx, int bound, ITEM_T value) {

		// instance/interface array �� �� ���� class�� array�� casting �� �� �ִ�.
		// item�� �˻��Ͽ��� �Ѵ�.
#ifdef FASTIVA_GC_DEBUG
		fastiva.ensureArray(this, sizeof(value));
#endif
		this->checkBound_ex$(idx, bound);
		if (isArraySuperType) {
			fastiva.setAbstractArrayItem(this, BASE_T::getRawContext$(),
				m_aRawItem + idx, value->getInstance$());
		}
		else {
			fastiva.setArrayItem(this, BASE_T::getRawContext$(), 
				m_aRawItem + idx, value->getInstance$());
		}
	}

	static fastiva_PointerArray* isInstance$(fastiva_Instance_p pObj) FASTIVA_PURE_API {
		if (pObj == FASTIVA_NULL) {
			return (fastiva_PointerArray*)FASTIVA_NULL;
		}
		void* pArray = fastiva.isArrayInstanceOf(
			pObj, BASE_T$::getRawContext$(), dimension$);
		KASSERT(pArray == FASTIVA_NULL || pArray == pObj);
		return (fastiva_PointerArray*)pArray;
	}

	static fastiva_PointerArray* ptr_cast$(fastiva_Instance_p pObj) FASTIVA_PURE_API {
		fastiva.checkArrayInstanceOf(
			pObj, BASE_T$::getRawContext$(), dimension$);
		return (fastiva_PointerArray*)pObj;
	}

	static fastiva_PointerArray* dbg_cast$(fastiva_Instance_p pObj) FASTIVA_PURE_API {
#if defined(_DEBUG) || !FASTIVA_SUPPORTS_GENERIC_CAST
		fastiva.checkArrayInstanceOf(
			pObj, BASE_T$::getRawContext$(), dimension$);
#endif
		return (fastiva_PointerArray*)pObj;
	}

	static java_lang_Class_p getRawStatic$() FASTIVA_PURE_API {
		return importClass$();
	}

	static java_lang_Class_p importClass$() FASTIVA_PURE_API {
		return fastiva.getArrayClass(BASE_T$::getRawContext$(), dimension$);
	}

	static fastiva_PointerArray* create$(int d1, int d2 = fastiva_NO_ARRAY_ITEM$, 
								int d3 = fastiva_NO_ARRAY_ITEM$, 
								int d4 = fastiva_NO_ARRAY_ITEM$) {
		if (dimension$ == 1) {
			return (fastiva_PointerArray*)
				fastiva.allocatePointerArray(BASE_T$::getRawContext$(), d1);
		}

		int aLength[4];
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

		return (fastiva_PointerArray*)fastiva.allocateMultiArrayEx(
			BASE_T$::getRawContext$(), dimension$, aLength);
	}

FASTIVA_RUNTIME_CRITICAL(protected):
#if FASTIVA_SUPPORTS_DYNAMIC_ARRAY
	//typedef	java_lang_Object_p RAW_ITEM_ARRAY_T[64];
	//union {
	//	RAW_ITEM_ARRAY_T* volatile m_aRawItem;
	//};
	java_lang_Object_p* m_aRawItem;
#else
	union {
		ITEM_T m_aItem[64];
		java_lang_Object_p m_aRawItem[64];
	};
#endif
};


FASTIVA_DECL_ARRAY_EX(fastiva_PointerArray, java_lang_Object, true)
FASTIVA_DECL_ARRAY_EX(fastiva_PointerArray, java_lang_Cloneable, true)
FASTIVA_DECL_ARRAY_EX(fastiva_PointerArray, java_io_Serializable, true)


#define FASTIVA_DECL_PRIMITIVEARRAY(jtype, CType)		\
typedef fastiva_PrimitiveArray<fastiva_Primitives::jtype, jtype, 1> CType##_A; \
typedef CType##_A* CType##_ap; \
typedef fastiva_PrimitiveArray<fastiva_Primitives::jtype, CType##_ap, 2> CType##_AA; \
typedef CType##_AA* CType##_aap; \
typedef fastiva_PrimitiveArray<fastiva_Primitives::jtype, CType##_aap, 3> CType##_AAA; \
typedef CType##_AAA* CType##_aaap; \
typedef fastiva_PrimitiveArray<fastiva_Primitives::jtype, CType##_aaap, 4> CType##_AAAA; \
typedef CType##_AAAA* CType##_aaaap; \
typedef fastiva_PrimitiveArray<fastiva_Primitives::jtype, CType##_aaaap, 5> CType##_AAAAA; \
typedef CType##_AAAAA* CType##_aaaaap; \
typedef fastiva_PrimitiveArray<fastiva_Primitives::jtype, CType##_aaaaap, 6> CType##_AAAAAA; \
typedef CType##_AAAAAA* CType##_aaaaaap; \
typedef fastiva_PrimitiveArray<fastiva_Primitives::jtype, CType##_aaaaaap, 7> CType##_AAAAAAA; \
typedef CType##_AAAAAAA* CType##_aaaaaaap; \
static const char * FASTIVA_SIG_##CType##_ap = FASTIVA_SIG_##jtype##_p - 1; \
static const char * FASTIVA_SIG_##CType##_aap = FASTIVA_SIG_##jtype##_p - 2; \
static const char * FASTIVA_SIG_##CType##_aaap = FASTIVA_SIG_##jtype##_p - 3; \
static const char * FASTIVA_SIG_##CType##_aaaap = FASTIVA_SIG_##jtype##_p - 4; \
static const char * FASTIVA_SIG_##CType##_aaaaap = FASTIVA_SIG_##jtype##_p - 5; \
static const char * FASTIVA_SIG_##CType##_aaaaaap = FASTIVA_SIG_##jtype##_p - 6; \
static const char * FASTIVA_SIG_##CType##_aaaaaaap = FASTIVA_SIG_##jtype##_p - 7; \


FASTIVA_DECL_PRIMITIVEARRAY(jbyte, Byte)
FASTIVA_DECL_PRIMITIVEARRAY(jbool, Bool)
FASTIVA_DECL_PRIMITIVEARRAY(unicod, Unicod)
FASTIVA_DECL_PRIMITIVEARRAY(jshort, Short)
FASTIVA_DECL_PRIMITIVEARRAY(jint, Int)
FASTIVA_DECL_PRIMITIVEARRAY(jlonglong, Longlong)
FASTIVA_DECL_PRIMITIVEARRAY(jfloat, Float)
FASTIVA_DECL_PRIMITIVEARRAY(jdouble, Double)

/*
typedef Byte_A* Byte_A_p;
typedef Byte_AA* Byte_AA_p;
typedef Byte_AAA* Byte_AAA_p;
typedef Byte_AAAA* Byte_AAAA_p;

typedef Bool_A* Bool_A_p;
typedef Bool_AA* Bool_AA_p;
typedef Bool_AAA* Bool_AAA_p;
typedef Bool_AAAA* Bool_AAAA_p;

typedef Unicod_A* Unicod_A_p;
typedef Unicod_AA* Unicod_AA_p;
typedef Unicod_AAA* Unicod_AAA_p;
typedef Unicod_AAAA* Unicod_AAAA_p;

typedef Short_A* Short_A_p;
typedef Short_AA* Short_AA_p;
typedef Short_AAA* Short_AAA_p;
typedef Short_AAAA* Short_AAAA_p;

typedef Int_A* Int_A_p;
typedef Int_AA* Int_AA_p;
typedef Int_AAA* Int_AAA_p;
typedef Int_AAAA* Int_AAAA_p;

typedef Longlong_A* Longlong_A_p;
typedef Longlong_AA* Longlong_AA_p;
typedef Longlong_AAA* Longlong_AAA_p;
typedef Longlong_AAAA* Longlong_AAAA_p;

typedef Float_A* Float_A_p;
typedef Float_AA* Float_AA_p;
typedef Float_AAA* Float_AAA_p;
typedef Float_AAAA* Float_AAAA_p;

typedef Double_A* Double_A_p;
typedef Double_AA* Double_AA_p;
typedef Double_AAA* Double_AAA_p;
typedef Double_AAAA* Double_AAAA_p;
*/



/*
���� 1)
array-casting�� �̷������ �������� �ش� header-file�� include���� �ʾƵ� �ȴ�.
�̸� ���� ���ʿ��� header�� include �ϴ� �ʿ伺�� �ּ�ȭ�Ͽ���.
���� 2) 
super-class�� ����� �Լ��� overloading(not override) �� ���, super-class��
�ش� �Լ��� hidden�ȴ�. ����, super-class�� ����� �Լ���
overloading�ϴ� ��쿣, �ش� �Լ��� ������ �Լ��� ���� Class���� ������ �־��
�Ѵ�. operator() �����ڴ� �̿� �ش����� �ʴ´�.
���� 3) 
fm::array_casting_cache�� ����ϴ� ����.
ARRAY_PTR$() macro ������ array ���ڸ� �� �� �̻� ����ϴ� ���, side-effect��
�߻��Ѵ�. (side-effect ��. Object_A::create$(1), Object_AA[i++]).
get_base_pointer_$$()���� ��ȯ�� pointer�� target�� BASE_T�� �ٲٴ� ��������
�ּҰ��� ������ �߻��� �� �ִµ�, �̸� �ذ��� ����� ����.
�ش� src-ARRAY�� BASE* �κ��� ������ �� �ִ� ��� pointer-type�� casting��
����ϴ� ������ wrapper-class�� ���� ������ ������ �ִ�.
1) operator() �����ڸ� ����ϴ� ���,
	hidden-overriding ������ ���� �� ������, check_array_$$ ����
	���� type�� parameter�� ����ϱ� ������ ambiguous casting ������ ���� �� ����.
2) fm::ValidArrayCasting$* cast_array_$$() �� ����ϴ� ���,
	hidden-overriding ������ ����, ��� interface �� ������ ��� super-type��
	���� �Լ��� ������ �־�� �ϰ�, �̸� ���� inline�Լ��� ����ϸ�,
	inheritance ������ �ذ��� �� ������, 
	fm::convertArrayBasePtr(java_lang_Object*, void*) ��
	fm::convertArrayBasePtr(void*, fastiva_ArrayHeader*) �� 
	ambiguous ������ �ذ��� �� ����.
���� 4)
	SUPER_TYPE_OF_ARRAY_OBJECT �� 
	getNonAmbiguousPtr$(fm::InvalidArrayCasting$*) �Լ���,
	(Equal-Dimensional) PrimitiveArray�� casting�� �ÿ� ȣ��ȴ�. 
		T* getNonAmbiguousPtr$(void*) { return this; }						
���� 5) 
	operator[]�� ���ġ �ʴ� ����.
	1) []������ ���� R-Value �� L-Value�� ���� Compiler�� �Ǵ��� ��Ȯġ �ʴ�.
	ITEM_REF_T$�� ����Ͽ� set$()�� ����ϰ��� �� ��, ������ �ȴ�.
	2) return type�� array �� ���, parameter passing ����� �޶����� ������ �ִ�.
*/
#endif
