#include <precompiled_libcore.h>
#include <Fastiva.h>

#include <java/lang/Boolean.inl>
#include <java/lang/Byte.inl>
#include <java/lang/Character.inl>
#include <java/lang/Short.inl>
#include <java/lang/Integer.inl>
#include <java/lang/Long.inl>
#include <java/lang/Float.inl>
#include <java/lang/Double.inl>
#ifndef FASTIVA_CLDC
#include <java/lang/Number.inl>
#endif
#include <java/lang/Class.inl>
#include <java/lang/ClassLoader.inl>

#include <kernel/kernel.h>
#include <kernel/sys.h>
//#include <kernel/HeapMark.h>
#include <kernel/Runnable.h>
#include <pal/fox_sys_package.h>
#include <string.h>
#include <stdlib.h>
#include <fastiva_malloc.h>

const int JNI_FIELD_USED	= 0x8000;
const int JNI_METHOD_USED	= 0x4000;

extern "C" void fastiva_throwNoSuchMethodError();

/*
static int __comparePackageName(const fastiva_ClassContext* pContext, const char* sig) {
	const fastiva_Package* pPackage = pContext->m_pPackage;
	if (pPackage == ADDR_ZERO || pPackage->m_pName[0] == 0) {
		return 0;
	}
	const char* packageName = pPackage->m_pName;
	int len = 1;
	while (*packageName != 0) {
		if (*packageName ++ != *sig ++) {
			return -1;
		}
		len ++;
	}
	if (*sig == '/') {
		return len;
	}
	else {
		return -1;
	}
}
*/

static ushort __registerRefName(ushort id, const char** aRefName) {
	int idx = id - JPP_REF_NAME_ID::cntPredefinedID;
	if (idx < 0) {
		return id;
	}

	const char* pszName = aRefName[idx];
	if (pszName >= (void*)0x10000) {
		int nid = kernelData.g_utfHashTable.addRefName(pszName);
		pszName = (char*)nid;
		aRefName[idx] = pszName;
	}
	return (ushort)pszName;
}


static ushort __registerContext(ushort id0, const fastiva_ClassContext** aContext) {
	int id = id0 & (FASTIVA_JNI_ARRAY_DIMENSION_BITS(1) - 1);
	int idxContext = id - fastiva_Primitives::cntType;
	if (idxContext < 0) {
		return id0;
	}

	int dimension = id0 - id;
	const fastiva_ClassContext* pContext = aContext[idxContext];
	//*
	if (pContext < (void*)0x10000) {
		return (short)((int)pContext + dimension);
	}
	//*/

	if (pContext->isRawContext()) {
		pContext = fm::validateContext(pContext);
	}
	FASTIVA_DBREAK();
	//2012.0607 DK
	//(id0 == (pContext->m_id));
	aContext[idxContext] = pContext;//(fastiva_ClassContext*)id0;
	return id0 + dimension;
}

int fastiva_ClassContext::getFieldCount() const {
	int cntField = this->m_pJNI->m_cntField;
	if (cntField < 0) {
		*(short*)&this->m_pJNI->m_cntField = cntField &= 0x7FFF;
#if 1
		fastiva_Field* pField = (fastiva_Field*)m_pJNI->m_aField;
		const char** aRefName = (const char**)this->getPackage()->m_pModule->m_aRefName;
		const fastiva_ClassContext** aContext = this->getPackage()->m_pModule->m_ppImportedContext;
		
		for (int i = cntField; i-- > 0; pField++) {
			pField->m_name = __registerRefName(pField->m_name, aRefName);
			pField->m_type = __registerContext(pField->m_type, aContext);
		}
#endif
	}
	return cntField;
}

int fastiva_ClassContext::getMethodCount() const {
	int cntMethod = this->m_pJNI->m_cntMethod;
	if (cntMethod < 0) {
		FASTIVA_DBREAK(); // Not implemented;

		*(short*)&this->m_pJNI->m_cntMethod = cntMethod &= 0x7FFF;
		fastiva_Method* pMethod = (fastiva_Method*)m_pJNI->m_aMethod;
		const fastiva_ModuleInfo* pModule = this->getPackage()->m_pModule;
		const char** aRefName = (const char**)pModule->m_aRefName;
		const JNI_HashEntry* aArgList = pModule->m_aArgList;
		const fastiva_ClassContext** aContext = pModule->m_ppImportedContext;
		//int cntRefName = pModule->m_cntRefName;
		
		for (int i = cntMethod; i-- > 0; pMethod++) {
			pMethod->m_name = __registerRefName(pMethod->m_name, aRefName);
			pMethod->m_retType = __registerContext(pMethod->m_retType, aContext);

			int cntArg = pMethod->m_cntArg;
			int idx;
			switch (cntArg) {
				case 0:
					continue;
				case 1:
					pMethod->m_args = __registerContext(pMethod->m_args, aContext);
					continue;
				default:
					idx = pMethod->m_args - JPP_EXTERNAL_CLASS_ID_START;
					break;
			}
			
			KASSERT (idx >= 0);
			JNI_HashEntry* pArgList = (JNI_HashEntry*)&aArgList[idx];
			ushort* pArgType = (ushort*)(pArgList->m_pszToken);
			if (pArgType <= (void*)0xFFFF) {
				pMethod->m_args = (ushort)pArgType;
			}
			else {
				uint hashCode = pArgList->m_hashCode;
				pArgType ++;
				while (cntArg -- > 0) {
					int argID = *pArgType;
					*pArgType++ = __registerContext(argID, aContext);
				}
				ushort id = kernelData.g_utfHashTable.addArguments((ushort*)(pArgList->m_pszToken), hashCode);
				pArgList->m_pszToken = (char*)id;
				pMethod->m_args = id;
			}
		}
	}
	return cntMethod;
}

inline static int __compareClassName(const fastiva_ClassContext* pContext, const char* sig) {
	const char* className = pContext->m_pBaseName;
	int len = 1;
	while (*className != 0) {
		if (*className ++ != *sig ++) {
			return -1;
		}
		len ++;
	}
	if (*sig == ';' || *sig == 0) {
		return len;
	}
	else {
		return -1;
	}
}


inline static int __compareParamSig(const char* ref, const char* sig) {
	/*
	const char *ref1;
	const char *ref2;

	if (strlen(ref) > strlen(sig)) {
		ref1 = ref;
		ref2 = sig;
	}
	else {
		ref1 = sig;
		ref2 = ref;
	}

	while (*ref1 != ')') {
		if (*ref1 ++ != *ref2 ++) {
			return -1;
		}
	}
	return 0;
	*/
	while (*ref != ')' || *sig != ')') {
		if (*ref++ != *sig++) {
			return -1;
		}
	}
	return 0;
}

static jbool __checkValue(const fastiva_Field* pField, fastiva_Instance_p pValue) {
	if (pValue == FASTIVA_NULL) {
		return true;
	}
	//const fastiva_ClassContext* pContext = pField->getContext();
	ushort type = pField->m_type;//pValue = fm::getRealInstance(pValue);
	int dim = type / FASTIVA_JNI_ARRAY_DIMENSION_BITS(1);
	type = type & (FASTIVA_JNI_ARRAY_DIMENSION_BITS(1) - 1);

	fastiva_ClassContext* pContext = *JNI_RawContext::getContextSlot(type);
	
	if (dim > 0) {
		return fastiva.isArrayInstanceOf(pValue, pContext, dim) != FASTIVA_NULL;
	}
	if (!pContext->isInstance()) {
		return pValue->getInterfaceTable$(pContext->toInterfaceContext()) != FASTIVA_NULL;
	}
	else {
		return fastiva.isInstanceOf(pValue, pContext) != NULL;
	}
}

/*
java_lang_Class_p fastiva_Field::getClass() const {
	return fastiva.getArrayClass(getContext(), getDimension());
}
*/

/*
const fastiva_ClassContext* fastiva_Field::getContext() const {
	const fastiva_ClassContext* pContext = this->m_pType;

	if ((int)pContext < 0x10) {
		pContext = fm::getPrimitiveContext((fastiva_ClassMoniker*)pContext);
	}
	if (pContext->isRawContext()) {
		pContext = fm::loadMoniker((fastiva_ClassMoniker*)pContext);
	}
	return pContext;
}
*/

/*
void fastiva_Field::setObjectValue(java_lang_Object_p pOwner, fastiva_Instance_p pValue0) const {
	//*ppField = pValue;

}

java_lang_Object_p fastiva_Field::getObjectValue(java_lang_Object_p pOwner) const {
}
*/

inline static void __setPrimitiveValue(int field_type, void* pSlot, jlonglong value) {
	switch ((field_type) & 3) {
	case 0: // jbool, jbyte
		*(jbyte*)pSlot = (jbyte)value;
		break;
	case 1: // unicod, jshort
		*(jshort*)pSlot = (jshort)value;
		break;
	case 2: // int, float
		*(jint*)pSlot = (jint)value;
		break;
	case 3: // long, double:
		*(jlonglong*)pSlot = (jlonglong)value;
		break;
	default:
		FASTIVA_DBREAK();
	}
}

void fastiva_Field::setValue(
	java_lang_Object_p pOwner, 
	jlonglong* pValue
) const {
	//KASSERT(pOwner == FASTIVA_NULL || !fm::isInterface(pOwner));

	//if (this->isConstant()) {
	//	return;//fastiva_throwIllegalArgumentException();
	//}

	void* pSlot = this->getSlot_$$(pOwner);

	if (!this->isPrimitive()) {
		#ifdef KERNEL_DEBUG
			int siz = this->isStatic() ? 0xFFFF
				: fm::getInstanceContext(pOwner)->m_sizInstance;
			if (!(pSlot <= (char*)pOwner + siz - 4)) {
				FASTIVA_DBREAK();
			}
		#endif
		java_lang_Object_p pObj = *(java_lang_Object_p*)pValue;//fm::getRealInstance(*(java_lang_Object_p*)pValue);
		KASSERT(__checkValue(this, pObj));
		//bool isInterface = getDimension() == 0 && !m_pType->isInstance();
		//if (isInterface) {
		//	pObj = (java_lang_Object_p)fm::checkImplemented(pObj, m_pType->toInterfaceContext());
		//}
		if (this->isStatic()) {
			((java_lang_Class_p)pOwner)->setField_$$(pObj, pSlot);
		}
		else {
			pOwner->setField_$$(pObj, pSlot);
		}
		return;
	}

	#ifdef KERNEL_DEBUG
		int siz = this->isStatic() ? 0xFFFF
			: fm::getInstanceContext(pOwner)->m_sizInstance;
		if (!(pSlot <= (char*)pOwner + siz - fm::getPrimitiveArrayItemSize(this->m_type))) {
			FASTIVA_DBREAK();
		}
	#endif
	__setPrimitiveValue((int)this->m_type, pSlot, *pValue);
}

jlonglong fastiva_Field::getValue(
	java_lang_Object_p pOwner
) const {
	//KASSERT(pOwner == FASTIVA_NULL || !fm::isInterface(pOwner));
	void* pSlot = this->getSlot_$$(pOwner);

	if (!this->isPrimitive()) {
		#ifdef KERNEL_DEBUG
			int siz = this->isStatic() ? 0xFFFF
				: fm::getInstanceContext(pOwner)->m_sizInstance;
			if (!(pSlot <= (char*)pOwner + siz - 4)) {
				FASTIVA_DBREAK();
			}
		#endif
		java_lang_Object_p pValue = *(java_lang_Object_p*)pSlot;
		KASSERT(__checkValue(this, pValue));
		return (jlonglong)pValue;
	}

	int field_type = (int)m_type;
	jlonglong v = 0;
	#ifdef KERNEL_DEBUG
		if (!this->isStatic()) {
			int siz = fm::getInstanceContext(pOwner)->m_sizInstance;
			if (pSlot < (char*)pOwner 
				||  pSlot > (char*)pOwner + siz - fm::getPrimitiveArrayItemSize(field_type)) {
				FASTIVA_DBREAK();
			}
		}
	#endif

	switch (field_type) {
	case 0: // jbool
		*(jint*)&v = *(jbool*)pSlot;
		break;
	case 1: // unicod
		*(jint*)&v = *(unicod*)pSlot;
		break;
	
	case 2: // float
	case 6: // int
		*(jint*)&v = *(jint*)pSlot;
		break;
	
	case 3: // double:
	case 7: // long:
		*(jlonglong*)&v = *(jlonglong*)pSlot;

	case 4: // jbyte
		*(jint*)&v = *(jbyte*)pSlot;
		break;

	case 5: // jshort
		*(jint*)&v = *(jshort*)pSlot;
		break;
	default:
		FASTIVA_DBREAK();
	}
	return v;
}

int JNI_FindField::matches(const fastiva_Field* pField) {
	if (*(int*)&m_name != *(jint*)pField) {
		return 0;
	}

	if ((pField->m_accessFlags & this->m_accessFlags) == 0) {
		return -1;
	}

	return +1;
}

const fastiva_Field* fastiva_ClassContext::getDeclaredField(
	JNI_FindField* pFind
) const {
	const fastiva_JniInfo* pJNI = this->m_pJNI;
	if (pJNI == ADDR_ZERO) {
		return ADDR_ZERO;
	}

	const fastiva_Field* pField = pJNI->m_aField;
	for (int i = this->getFieldCount(); i-- > 0; pField++) {
		int res = pFind->matches(pField);
		if (res < 0) {
			break;
		}
		if (res > 0) {
			fm::__registerJNIFieldClass(this);
			return pField;
		}
	}

	return ADDR_ZERO;
}

int JNI_FindMethod::matches(const fastiva_Method* pMethod) {
	if (*(int*)&m_name != *(jint*)pMethod) { //  | ACC_PUBLIC$ | ACC_PROTECTED$ | ACC_PRIVATE$
		return 0;
	}

	if ((this->m_accessFlags & pMethod->m_accessFlags) == 0) {
		return -1;
	}

	if (this->m_retType != pMethod->m_retType) {
		return -1;
	}

	return +1;
}

const fastiva_Method* fastiva_ClassContext::getDeclaredMethod(
	JNI_FindMethod* pFind
) const {
	const fastiva_JniInfo* pJNI = this->m_pJNI;
	if (pJNI == ADDR_ZERO) {
		return ADDR_ZERO;
	}

	/**
	 Overriding 된 함수를 포함한 모든 Member 함수를 반환한다.
	*/
	jint searchKey = *(jint*)pFind;
	const fastiva_Method* pMethod = pJNI->m_aMethod;
	for (int i = this->getMethodCount(); i-- > 0; pMethod++) {
		int res = pFind->matches(pMethod);
		if (res < 0) {
			break;
		}

		if (res > 0) {
			fm::__registerJNIMethodClass(this);
			return pMethod;
		}
	}
	return ADDR_ZERO;
}




/*
const fastiva_Field* fastiva_InstanceContext::getDeclaredField(
	const char* fieldName,
	const char* sig
) const {
	const fastiva_Field* pField = this->m_aField;
	int count = this->m_cntField;

	for (int i = 0 ; i < count; i++, pField++) {
		if (strcmp(fieldName, pField->m_name) == 0) {
			const fastiva_ClassContext* pFieldContext = pField->getContext();
			int lenPackage = __comparePackageName(pFieldContext, sig);
			if (lenPackage >= 0
			&&  __compareClassName(pFieldContext, sig+lenPackage) >= 0) {
				return pField;
			}
		}
	}
	return ADDR_ZERO;
}
*/



const fastiva_Field* fastiva_InterfaceContext::getField(
	JNI_FindField* pFind
) const {
	const fastiva_Field* pField;
	KASSERT(!this->isRawContext());
	pField = this->getDeclaredField(pFind);
	if (pField != ADDR_ZERO) {
		return pField;
	}

	int cntImpl = this->getOwnInterfaceCount();
	const fastiva_ClassContext** ppIFC = this->m_ppIFC;
	for (int i = cntImpl; --i >= 0; ppIFC++) {
		// 반드시 interface 가 implemented 된 순서에 따라 검색한다.
		// 먼저 implemented 된 interface 의 상위 class 를 다름에 implemented 된
		// interface 보다 먼저 조사한다. (중복된 interface 상속은 어쩔 수 없다.)
		pField = ppIFC[0]->getField(pFind);
		if (pField != ADDR_ZERO) {
			return pField;
		}
	}
	return ADDR_ZERO;
}

const fastiva_Field* fastiva_InstanceContext::getField(
	JNI_FindField* pFind
)  const {
	KASSERT(!this->isRawContext());
	const fastiva_InstanceContext* pContext = this;

	bool searchStatic = (pFind->m_accessFlags & ACC_STATIC$) != 0;
	const fastiva_Field* pField;
	while (true) {
		if ((pField = pContext->getDeclaredField(pFind)) != ADDR_ZERO) {
			return pField;
		}
		const fastiva_InstanceContext* pSuperContext = pContext->getSuperInstanceContext();
		if (pSuperContext == ADDR_ZERO) {
			break;
		}
		/** 
			Java name 검색시, super-class 검색 전에 반드시 interface 에 선언된 static-field를 먼저 검색한다.
			검색 순서는 source 상에서 implements 한 순서와 동일하여야 한다.
		*/
		if (searchStatic) {
			int cntIVT = pContext->getOwnInterfaceCount();
			const fastiva_ImplementInfo* pImpl = pContext->m_aImplemented;
			for (int i = cntIVT; --i >= 0; pImpl++) {
				if ((pField = pImpl->m_pInterfaceContext->getField(pFind)) != ADDR_ZERO) {
					return pField;
				}
			}
		}
		pContext = pSuperContext;
	};

	return ADDR_ZERO;
}

const fastiva_Field* fastiva_ClassContext::getField(
	JNI_FindField* pFind
) const {
	KASSERT(pFind->m_accessFlags != 0);
	KASSERT(!this->isRawContext());
	if (!this->isInstance()) {
		if ((pFind->m_accessFlags & ACC_STATIC$) == 0) {
			return ADDR_ZERO;
		}

		return this->toInterfaceContext()->getField(pFind);
	}
	const fastiva_Field* pField;
	const fastiva_InstanceContext* pContext = this->toInstanceContext();

	pField = pContext->getField(pFind);
	return pField;

	return ADDR_ZERO;
}


const fastiva_Method* fastiva_InterfaceContext::getMethod(
	JNI_FindMethod* pFind
) const {
	const fastiva_Method* pMethod;
	KASSERT(!this->isRawContext());
	pMethod = this->getDeclaredMethod(pFind);
	if (pMethod != ADDR_ZERO) {
		return pMethod;
	}
    // 검색 순서는 source 상에서 implements 한 순서와 동일하여야 한다.

	int cntImpl = this->getOwnInterfaceCount();
	const fastiva_ClassContext** pImpl = this->m_ppIFC;
	const fastiva_ClassContext* pIFC;
	for (int i = cntImpl; --i >= 0; pImpl++) {
		// 반드시 interface 가 implemented 된 순서에 따라 검색한다.
		// 먼저 implemented 된 interface 의 상위 class 를 다름에 implemented 된
		// interface 보다 먼저 조사한다. (중복된 interface 상속은 어쩔 수 없다.)
		const fastiva_ClassContext* pIFC = *pImpl;
		pMethod = pIFC->getMethod(pFind);
		if (pMethod != ADDR_ZERO) {
			return pMethod;
		}
	}
	return ADDR_ZERO;
}

const fastiva_Method* fastiva_InstanceContext::getMethod(
	JNI_FindMethod* pFind
) const {
	KASSERT(!this->isRawContext());
	const fastiva_Method* pMethod;
	const fastiva_ClassContext* pContext = this;
	while ((pMethod = pContext->getDeclaredMethod(pFind)) == ADDR_ZERO) {
		pContext = pContext->getSuperClassContext();
		if (pContext == ADDR_ZERO) {
			return ADDR_ZERO;
		}
        // getField와 달리 super-class가 interface보다 먼저 검색된다.
        // Interface 상속시 abstract-method가 반드시 생성되므로,
        // interface에 대한 검색을 할 필요가 없다.
        // 단, Interpreter 구현시 이를 보장하여야 한다.
	};
	return pMethod;
}

const fastiva_Method* fastiva_ClassContext::getMethod(
	JNI_FindMethod* pFind
) const {
	KASSERT(pFind->m_accessFlags != 0);
	KASSERT(!this->isRawContext());
	const fastiva_Method* pMethod;
	if (!this->isInstance()) {
		if ((pFind->m_accessFlags & ACC_STATIC$) == 0) {
			return ADDR_ZERO;
		}
		pMethod = this->toInterfaceContext()->getMethod(pFind);
		if (pMethod != ADDR_ZERO) {
			return pMethod;
		}
		return pMethod;
	}
	// INSTANCE-Method를 먼저 검색한 후, interface의 Method를 검색한다.
	const fastiva_InstanceContext* pContext = this->toInstanceContext();
	pMethod = pContext->getMethod(pFind);
    return pMethod;
}

/*
void fastiva_Field::getPrimitiveValue(
	java_lang_Object_p pOwner, 
	const fastiva_ClassMoniker* pType, 
	void* pValue
) const {
	KASSERT(pOwner->getInstance$() == pOwner);
	if (!this->isPrimitive()
	||   this->m_pContext != (const void*)pType) {
		fastiva_throwIllegalArgumentException();
	}

	void* pSlot = __getSlotOf(pOwner);
	switch ((int)pType / 2) {
	case 0: // jbool, jbyte
		*(jbyte*)pValue = *(jbyte*)pSlot;
		break;
	case 1: // unicod, jshort
		*(jshort*)pValue = *(jshort*)pSlot;
		break;
	case 2: // int, float
		*(jint*)pValue = *(jint*)pSlot;
		break;
	case 3: // long, double:
		*(jlonglong*)pValue = *(jlonglong*)pSlot;
		break;
	}
}
*/

static inline java_lang_Object_p __checkOwner(
	const fastiva_Field* pField,
	java_lang_Object_p pOwner,
	java_lang_Class_p pOwnerClass,
	int flags
) {
	//KASSERT(pOwner == FASTIVA_NULL || !fm::isInterface(pOwner));
	
	if ((pField->m_accessFlags & flags) != 0) {
		fastiva_throwIllegalArgumentException();
	}
	if (pField->isStatic()) {
		if (pField->getDimension() == 0 && !pOwnerClass->isLinked$()) {
			// Reflection에 의해 
			// method나 field-id를 얻는 과정에서는 class가 initialize되지 않는다.
			// method 호출시나, field-value access 시 class를 initialize 해야 한다.
			fm::linkClass((fastiva_Class_p)pOwnerClass);
		}
		if (pOwner != FASTIVA_NULL && pOwner != pOwnerClass) {
			fastiva_throwIllegalArgumentException();
		}
	}
	else {
		//여기서 null인가를 체크하지 안흥면 IllegalArgumentException이 발생.
		if (pOwner == FASTIVA_NULL) {
			fastiva_throwNullPointerException();
		}
		if (!pOwnerClass->isInstance(pOwner)) {
			fastiva_throwIllegalArgumentException();
		}
	}
	return pOwner;
}

const int aConversionMap[] = {
	// value : target;
	99, // boolean : none
	fastiva_Primitives::jshort, // byte : short, int, long, float, double
	fastiva_Primitives::jint, // unicod : int, long, float, double
	fastiva_Primitives::jint, // short : int, long, float, double
	fastiva_Primitives::jfloat, // int : float, long, double
	fastiva_Primitives::jdouble, // float : double
	fastiva_Primitives::jfloat, // long : float, double
	fastiva_Primitives::jdouble, // float : double
};

inline static void __checkConversion(int target_type, int value_type) {
	
	KASSERT(target_type <= fastiva_Primitives::cntType);
	KASSERT(value_type <= fastiva_Primitives::cntType);

	FASTIVA_DBREAK();
	// primitives 의 순서가 바뀌었으나, aConversionMap 이 변경되지 않았다.
	if (target_type == value_type) {
		return;
	}

	if (target_type < aConversionMap[value_type]) {
		fastiva_throwIllegalArgumentException();
	}
/*
	switch (target_type) {
	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jbool):
		// boolen 은 type 변경 불가.
		goto error;

	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jbyte):
	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(unicod):
	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jshort):
	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jint):
		if (target_type <= (value_type | 0x1)) {
			// jbyte -> jbyte
			// unicod -> jshort 
			// jshort -> jshort
			// jint -> jfloat;
			// jfloat -> jfloat;
			goto error;
		}
		break;
				
	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jlonglong):
		if (target_type <= value_type + 1) {
			// jfloat -> jlonglong;
			goto error;
		}
		break;

	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jfloat):
		if (value_type == FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jdouble)) {
			goto error;
		}
		break;

	case FASTIVA_PRIMITIVE_TYPE_INDEX_OF(jdouble):
		break;

	default:
		KASSERT("Invalid Moniker");
	}
	return;

error:
	fastiva_throwIllegalArgumentException();
*/
}


java_lang_Object_p fastiva_Field::getReflectionValue(
	java_lang_Object_p pOwner,
	java_lang_Class_p pOwnerClass
) const {
	pOwner = __checkOwner(this, pOwner, pOwnerClass, 0);

	void* pSlot = this->getSlot_$$(pOwner);

	if (!this->isPrimitive()) {
		java_lang_Object_p pValue = *(java_lang_Object_p*)pSlot;
		if (!__checkValue(this, pValue)) {
			fastiva_throwIllegalArgumentException();
		}
		return pValue;
	}

	int field_type = (int)m_type;
	return createReflectionValue(field_type, (jlonglong*)pSlot);
}

void* fastiva_Field::getSlot_$$(
	java_lang_Object_p pOwner
) const {
	KASSERT(pOwner != ADDR_ZERO && pOwner != FASTIVA_NULL);
#ifdef _DEBUG 
	// owner를 validation하는 부분이 없다. 조심할 것.
	if (this->isStatic()) {
		KASSERT(pOwner->getClass$() == (java_lang_Class_p)java_lang_Class_C$::getRawStatic$());
	}
	else {
		KASSERT(pOwner->getClass$() != (java_lang_Class_p)java_lang_Class_C$::getRawStatic$());
	}
#endif
	return (char*)pOwner + this->m_offset;
}

java_lang_Object_p fastiva_Field::createReflectionValue(
	int field_type,
	jlonglong* pSlot
) {
	switch (field_type) {
	case fastiva_Primitives::jbool:
		return FASTIVA_NEW(java_lang_Boolean)(*(jbool*)pSlot);
		break;;

	case fastiva_Primitives::jbyte:
		return FASTIVA_NEW(java_lang_Byte)(*(jbyte*)pSlot);;
		break;

	case fastiva_Primitives::unicod:
		return FASTIVA_NEW(java_lang_Character)(*(unicod*)pSlot);;
		break;

	case fastiva_Primitives::jshort:
		return FASTIVA_NEW(java_lang_Short)(*(jshort*)pSlot);;
		break;

	case fastiva_Primitives::jint:
		return FASTIVA_NEW(java_lang_Integer)(*(jint*)pSlot);;
		break;
	
	case fastiva_Primitives::jlonglong:
		return FASTIVA_NEW(java_lang_Long)(*(jlonglong*)pSlot);;
		break;

	case fastiva_Primitives::jfloat:
		return FASTIVA_NEW(java_lang_Float)(*(jfloat*)pSlot);;
		break;

	case fastiva_Primitives::jdouble:
		return FASTIVA_NEW(java_lang_Double)(*(jdouble*)pSlot);;
		break;
	default:
		fastiva_throwIllegalArgumentException();
	}
	return ADDR_ZERO; // 여기는 오지 않는다. 컴파일러 오류 때문에 삽입
}

int fastiva_Field::getPrimitiveType(
	java_lang_Class_p pClass
) {
	int id = fm::getPrimitiveTypeID((fastiva_Class_p)pClass);
	if (id >= 0) {
		return id;
	}
	id = fm::getPrimitiveReflectionType((fastiva_Class_p)pClass);
	if (id < 0) {
		fastiva_throwIllegalArgumentException();
	}
	return id;
}

jlonglong fastiva_Field::getPrimitiveValue(
	int field_type, 
	java_lang_Object_p pValue
) {

	int value_type = getPrimitiveType(pValue->getClass());
	__checkConversion(field_type, value_type);

	jlonglong value;
	switch (field_type) {
		// field-type 에 맞게 value를 변경.
	case fastiva_Primitives::jfloat:
		*(float*)&value = ((java_lang_Float_p)pValue)->floatValue();
		break;
	case fastiva_Primitives::jdouble:
		*(double*)&value = ((java_lang_Double_p)pValue)->doubleValue();
		break;
	default:
		*(jlonglong*)&value = ((java_lang_Long_p)pValue)->longValue();
	}
	return value;
}


void fastiva_Field::setReflectionValue(
	java_lang_Object_p pOwner, 
	java_lang_Class_p pOwnerClass,
	java_lang_Object_p pValue
) const {
	pOwner = __checkOwner(this, pOwner, pOwnerClass, ACC_FINAL$);

	if (!this->isPrimitive()) {
		void* pSlot = this->getSlot_$$(pOwner);
		if (!__checkValue(this, pValue)) {
			fastiva_throwIllegalArgumentException();
		}
		pOwner->setField_$$(pValue, pSlot);
		return;
	}
	///KASSERT(!this->isConstant());
	int field_type = (int)this->m_type;

	jlonglong value = getPrimitiveValue(field_type, pValue);
	__setPrimitiveValue(field_type, this->getSlot_$$(pOwner), value);
}

jlonglong fastiva_Field::getPrimitiveValue(
	java_lang_Object_p pOwner, 
	java_lang_Class_p pOwnerClass,
	const fastiva_ClassMoniker* pMoniker
) const {
	if (!this->isPrimitive()) {
		fastiva_throwIllegalArgumentException();
	}

	pOwner = __checkOwner(this, pOwner, pOwnerClass, 0);

	int field_type = (int)m_type;
	int value_type = (int)pMoniker;

	__checkConversion(value_type, field_type);
	//Static인 경우 pOwner가 FASTIVA_NULL일 수 있다.
	return getValue(this->isStatic() ? pOwnerClass : pOwner);
}

void fastiva_Field::setPrimitiveValue(
	java_lang_Object_p pOwner, 
	java_lang_Class_p pOwnerClass,
	const fastiva_ClassMoniker* pMoniker,
	jlonglong value
) const {
	if (!this->isPrimitive()) {
		fastiva_throwIllegalArgumentException();
	}

	pOwner = __checkOwner(this, pOwner, pOwnerClass, ACC_FINAL$);

	///KASSERT(!this->isConstant());
	int field_type = (int)this->m_type;
	int value_type = (int)pMoniker;

	if (field_type != value_type) {
		__checkConversion(field_type, value_type);

		// field-type 에 맞게 value를 변경.
		switch (field_type) {
		case fastiva_Primitives::jfloat:
			switch (value_type) {
			case fastiva_Primitives::jdouble:
				KASSERT("CONVERSION CHECK WRONG");
			default:
				*(float*)&value = (float)value;
				break;
			}
			break;

		case fastiva_Primitives::jdouble:
			switch (value_type) {
			case fastiva_Primitives::jfloat:
				*(double*)&value = *(float*)&value;
				break;
			default:
				*(double*)&value = (double)value;
				break;
			}
			break;

		default:
			switch (value_type) {
			case fastiva_Primitives::jfloat:
			case fastiva_Primitives::jdouble:
				KASSERT("CONVERSION CHECK WRONG");
			default:
				break;
			}
		}
	}

	#ifdef KERNEL_DEBUG
		int siz = this->isStatic() ? 0xFFFF
			: fm::getInstanceContext(pOwner)->m_sizInstance;
		if (!(this->getSlot_$$(pOwner) <= (char*)pOwner + siz - fm::getPrimitiveArrayItemSize(field_type))) {
			FASTIVA_DBREAK();
		}
	#endif
	__setPrimitiveValue(field_type, this->getSlot_$$(pOwner), value);
	return;

}


uint total_jni_t = 0;

void* fastiva_jniProlog(java_lang_Class_p pClass) {
	fastiva_Task* pTask = fastiva_getCurrentTask();
// v3.2006 #importlib 를 사용하면, registerNatves() 를 호출할 필요가 없다(?)
//	if ((pClass->m_marked & HM_NATIVE_REGISTERED) == 0) {
//		fm::registerNatives(pClass);
//	}
	return pTask;//getVM_ENV();
}


void* fm::getVM_ENV() {
	return fox_task_currentTask();
	//return getJNIEnv();
}

void fastiva_jniEpilog(void* pENV) {
	fastiva_Task* pCurrTask = (fastiva_Task*)pENV;
	KASSERT(pENV == fastiva_getCurrentTask());
	java_lang_Throwable_p pThrowable = pCurrTask->m_pJniException;
	if (pThrowable != ADDR_ZERO) {
		pCurrTask->m_pJniException = ADDR_ZERO;
		fastiva.throwException(pThrowable);
	}
}

void* fastiva_Method::getMethodAddr(
	java_lang_Object_p pObj, 
	java_lang_Class_p pClass
) const {
	void *pfnMethod;
	if ((this->m_accessFlags & (ACC_FINAL$ | ACC_STATIC$)) != 0) {
		pClass = this->getDeclaredContext()->m_pClass;
	}

	if (pClass != ADDR_ZERO) {
		// NonVirtual-call 또는 Staic-call 이다.
		// VTable$의 후반부에 final-method와 static-method의 VTable이 들어있다.
		if (!pClass->isLinked$()) {
			fm::linkClass(pClass);
		}
		pfnMethod = *(void**)((char*)pClass->obj.vtable$ + this->m_offset);
		if (pfnMethod == fastiva_throwAbstractMethodError) {
			goto find_method_of_instance;
		}
	}
	else {
find_method_of_instance:
		if ((this->getDeclaredContext()->m_paccessFlags & ACC_INTERFACE$) != 0) {
			// get-interface-method-addr;
			const int* pIVT = pObj->getInterfaceTable$(this->getInterfaceContext());
			pfnMethod = *(void**)((char*)pIVT + this->m_offset);
		}
		else {
			// get-virtual-method-addr;
			pfnMethod = *(void**)(*((int*)pObj) + this->m_offset);
		}
	}
	return pfnMethod;
}

struct ParamConvertor {
	const fastiva_Method* pMethod;
	const int is64bitArg;
	int cntParam;
	jlonglong paramFlags;
	int* pParam;
	int* pArg;

	ParamConvertor(jlonglong paramFlags, int is64bit, const fastiva_Method* pMethod)
		: is64bitArg(is64bit), cntParam((char)paramFlags) {
			this->paramFlags = paramFlags;
			this->pMethod = pMethod;
	}

	void extractStackParam();
	static jlonglong FOX_FASTCALL(call_non_inline_wrapper)(ParamConvertor* self, void* pObj, void *pfnMethod, int* pArg);
};

int cntInvoke = 0;
#ifndef __GNUC__
#if defined(_WIN32)
	#pragma runtime_checks( "", off )
#endif

static jlonglong FOX_FASTCALL(call_method)(ParamConvertor* pc, void* pObj, void *pfnMethod, int* pArg) {
	jint* aParam = (jint*)alloca((26*2+1)*sizeof(int));
	pc->pParam = aParam;
	pc->pArg = pArg;
	int ecx_v = (jint)pObj;

	pc->extractStackParam(); 

#ifdef _ARM_
	FASTIVA_DBREAK();
	return 0;
#else
	_asm {
		mov ecx, [ecx_v];
		/*
		lea eax, aParam
		//mov eax, aParam
		mov [eax + 26*2*4], esp;
		mov esp, eax
		*/
	}

#if defined(_DEBUG) && defined(_WIN32)
	_asm {
		mov eax, aParam
		cmp eax, esp;
		je  good_sp;  // 마지막 alloca() 된 pointer는 sp와 일치한다.
		int 3;
good_sp:
	}

	jlonglong res;
	_asm call [pfnMethod];
	_asm mov DWORD PTR [res], eax;
	_asm mov DWORD PTR [res+4], edx;
#else
	jlonglong res = ((jlonglong(_fastcall*)())pfnMethod)();
#endif
	//_asm {
	//	lea ecx, aParam
	//	//mov ecx, aParam
	//	mov esp, [ecx + 26*2*4];
	//}
	return res;
#endif
}

#if defined(_WIN32)
	#pragma runtime_checks( "", restore )
#endif

#else
static jlonglong FOX_FASTCALL(call_fm::method)(ParamConvertor pc, void* pObj, void *pfnMethod, int* pArg) {

	jint* aParam = (jint*)alloca((26*2+1)*sizeof(int));
	this->pParam = aParam;
	this->pArg = pArg;
	int ecx_v = (jint)pObj;
	if (ecx_v == 0) {
		ecx_v = extractStackParam();
	}
	int edx_v = extractStackParam();

	while (cntParam > 0) {
		*pParam ++ = extractStackParam(); 
	}

#ifdef __GNUC__
	//"수정이 필요하다 !!!!"
	__asm__ __volatile__ (
		"movl %0, %%edx			\n\t"
		"movl %1, %%eax			\n\t"
		//"movl %2, %%ecx 			\n\t"
		//"movl %%esp, 26*2*4(%%ecx)	\n\t"
		//"movl %%ecx, %%esp			\n\t"
		:
		: "m"(edx_v), "m"(ecx_v)//, "m"(aParam)
	);
#else
	_asm {
		mov edx, [edx_v];
		mov ecx, [ecx_v];
		//lea eax, aParam
		//mov eax, aParam
		//mov [eax + 26*2*4], esp;
		//mov esp, eax
	}

	_asm {
		mov eax, aParam
		cmp eax, esp;
		je  good_sp;  // 마지막 alloca() 된 pointer는 sp와 일치한다.
		int 3;
good_sp:
	}

#endif

#if defined(_DEBUG) && defined(_WIN32)
	jlonglong res;
	_asm call [pfnMethod];
	_asm mov DWORD PTR [res], eax;
	_asm mov DWORD PTR [res+4], edx;
#else
#ifdef __GNUC__
	//gcc에서 아래와 같이 콜하면 eax에 pfnMethod를 넣어놓고 call한다.
	//따라서 parameter가 이상한 값이 들어가게 된다. 따라서 assemble로 코딩한다.
	jlonglong res = 0;
	int res1 = 0;
	int res2 = 0;
	__asm__ __volatile__ (
		"call *%0 \n\t"
		"movl %%eax, %1\n\t"
		"movl %%edx, %2 \n\t"
		: "=m"(pfnMethod), "=m"(res1), "=m"(res2)
	);
	res = (jlonglong)(((jlonglong)res2 << 32) | res1);
#else
	jlonglong res = ((jlonglong(_fastcall*)())pfnMethod)();
#endif
#endif
#ifdef __GNUC__
	//__asm__ __volatile__ (
	//	"movl %0, %%ecx				\n\t"
	//	"movl 26*2*4(%%ecx), %%esp	\n\t"
	//	:
	//	: "m"(aParam)
	//);
#else
	//_asm {
	//	//lea ecx, aParam
	//	mov ecx, aParam
	//	mov esp, [ecx + 26*2*4];
	//}
#endif
	return res;
}
#endif


#ifdef _ARM_
jlonglong ParamConvertor::call_non_inline_wrapper(ParamConvertor* self, void* pObj, void *pfnMethod, int* pArg) {
	FASTIVA_DBREAK();
	return 0;
}
#else
FOX_NAKED jlonglong ParamConvertor::call_non_inline_wrapper(ParamConvertor* self, void* pObj, void *pfnMethod, int* pArg) {
	_asm jmp call_method;
}
#endif

void ParamConvertor::extractStackParam() {
	if (cntParam > 1) {
		int a = 3;
	}

	for (; cntParam -- > 0; ) {
		uint flag = (uint)(paramFlags >> 32);
		paramFlags <<= 2;

		if (flag >= 0x80000000) { 
			// DOUBLE OR LONG (register에 저장되지 않는다.)
			// 주의 ARM7에서는 (?)
			*pParam++ = *pArg++;
			*pParam++ = *pArg++;
			continue;
		}
		
		if ((int)flag >= 0x40000000) { // FLOAT
			// Float 도 (register에 저장되지 않는다.)
			// 주의 ARM7에서는 (?)
			*(float*)pParam++ = (float)*(double*)pArg;
			pArg += 2;
			continue;
		}
		
		int v = *pArg ++;
		if (is64bitArg) {
			*pArg ++;
		}

#if 0 //ndef FASTIVA_NO_USE_INTERFACE_SLOT
		if ((flag & 0x40000000) != 0) { // INTERFACE
			java_lang_Class_ap aArgType = pMethod->getParamTypes();
			int idxParam = aArgType->length() - cntParam - 1;
			java_lang_Class_p pArgType = aArgType->get$(idxParam);
			const fastiva_ClassContext* pArgContext;
			pArgContext = fm::getInstanceContext(pArgType)->toInterfaceContext();
			if (fm::isInterface((fastiva_Instance_p)v)) {
				fm::Interface_p pIFC = (fm::Interface_p)v;
				const fastiva_ClassContext* pParamContext = 
					((fastiva_IVTable*)pIFC->m_pIVT)->m_pContext;
				if (pArgContext == pParamContext
				||  pArgContext->isAssignableFrom(pParamContext)) {
					goto pop_param;
				}
				v = (int)pIFC->getInstance$();
			}
			v = (int)fm::checkImplemented(((fastiva_Instance_p)v), pArgContext);
		}
#endif
pop_param:
		*pParam ++ = v;
		//return v;
	}
}


jlonglong fastiva_Method::invoke(
	java_lang_Object_p pObj, 
	java_lang_Class_p pClass, 
	va_list args
) const {
	void *pfnMethod = getMethodAddr(pObj, pClass);

/*
	if ((unsigned __int64) this->m_paramFlags <= 0xFF) {
		_asm mov ecx, pObj;
		_asm or ecx, ecx;
		_asm jne call_method;
		_asm mov ecx, pClass
call_method:
		_asm mov esp, [args];
		_asm sub esp, 4;
		_asm mov eax, [esp-12];
		_asm mov [esp], eax
		_asm jmp  [pfnMethod];
	}
*/

	//FASTIVA_DBREAK();
	/*
	ParamConvertor params(this->m_paramFlags, false, this);
	/*/
	ParamConvertor params(0, false, this);
	//*/
	return ParamConvertor::call_non_inline_wrapper(&params, pObj, pfnMethod, (int*)(void*)args);
}

jlonglong fastiva_Method::invoke(
	java_lang_Object_p pObj, 
	java_lang_Class_p pClass, 
	jlonglong* pArg64
) const {
	void *pfnMethod = getMethodAddr(pObj, pClass);
	int* pArg = (jint*)pArg64;

//	FASTIVA_DBREAK();
	/*
	ParamConvertor params(this->m_paramFlags, true, this);
	/*/
	ParamConvertor params(this->m_cntArg, true, this);
	//*/
	return ParamConvertor::call_non_inline_wrapper(&params, pObj, pfnMethod, pArg);
}




java_lang_Class_ap fastiva_Method::getExceptionTypes() const {
	java_lang_Class_ap aTypes = java_lang_Class_A::create$(0);
	return aTypes;
}

java_lang_Class_p fastiva_Method::getReturnType() const {
#ifndef FASTIVA_NO_USE_INTERFACE_SLOT
	int id = FASTIVA_JNI_TYPE_ID_PTR$(java_lang_String);
	
	JNI_FindClass fc;
	return fc.loadRawClass(this->m_retType);


	//java_lang_Class_ap pArray = getParamTypes();
	// pArray의 실제 길이는 pArray-length()보다 길다.
	// ArrayBoundException이 발생하지 않도록 getPointerBffer()를 이용한다.
	//fastiva_Instance_p pItem = ((fastiva_Instance_p*)fm::getUnsafeBuffer(pArray))[pArray->length()];
	//return (java_lang_Class_p)pItem;
#else
	char *pdes = strchr(m_sig, ')');
	if (pdes == ADDR_ZERO) {
		return (java_lang_Class_p)FASTIVA_NULL;
	}
	++pdes;
	int len = strlen(pdes);
	return fm::findClass(FASTIVA_SIGNATURE_LOADER, pdes, len);
#endif
}

java_lang_Class_ap fastiva_Method::getParamTypes() const {
#ifndef FASTIVA_NO_PROXY_THUNK
#ifndef FASTIVA_NO_USE_INTERFACE_SLOT
	if (this->m_aParam != ADDR_ZERO) {
		return (java_lang_Class_ap)this->m_aParam;
	}
#endif
#endif
	//FASTIVA_DBREAK();

	JNI_FindClass fc(NULL);

	int cc = this->m_cntArg;
	java_lang_Class_ap aTypes = java_lang_Class_A::create$(cc);
	if (cc == 1) {
		java_lang_Class_p pClass = fc.loadRawClass(this->m_args);
		aTypes->set$(0, pClass);
	}
	else if (cc != 0) {
		if (cc == 3) {
			int a = 3;
		}

		int count = 0;
		JNI_ArgIterator iter;
		iter.init(this->m_args);
	
		while (count < cc) {
			uint typeId = iter.nextID();
			java_lang_Class_p pClass = fc.loadRawClass(typeId);
			aTypes->set$(count, pClass);
			count ++;
		}
	}

#if 0
	// 가장 마지막 ITEM은 RETURN-TYPE을 저장한다.
	java_lang_Class_ap aTypes = java_lang_Class_A::create$(getParamSize() + 1);


	int count = 0;
	int extended_size = 0;
	java_lang_Class_p pClass;
	const char* pSig = this->m_sig;
	char k;

	while ((k = *pSig) != 0) {
		switch (k) {
			case '(':
				break;

			case ')':
				pSig ++;
				continue;
				//aTypes->set$(count, fm::findClass(FASTIVA_SIGNATURE_LOADER, pSig, strlen(pSig)));
				//goto PARSE_COMPLETED;

			case 'D':
			case 'J': 
				extended_size ++;
			case 'V':
			case 'B':
			case 'C':
			case 'S':
			case 'I':
			case 'Z':
			case 'F': {
				pClass = fm::getPrimitiveClassBySig(k);
				aTypes->set$(count++, pClass);
				break;
			}
			
			case 'L': {
				const char *pStart = pSig;
				pSig = strchr(pSig, ';');
				aTypes->set$(count++, fm::findClass(FASTIVA_SIGNATURE_LOADER, pStart, (pSig - pStart) + 1));
				break;
			}

			case '[': {
				const char *pStart = pSig++;
				int dimension = 1;
				while ((k = *pSig) == '[') {
					dimension ++;
					pSig ++;
				}
				if (k == 'L') {
					pSig = strchr(pSig, ';');
					pClass = fm::findClass(FASTIVA_SIGNATURE_LOADER, pStart, pSig - pStart + 1);
				}
				else {
					pClass = fastiva.getArrayClass(fm::getPrimitiveContextBySig(k), dimension);
				}
				aTypes->set$(count++, pClass);
				break;
			}

			default: {
				KASSERT("signature error" == 0);
			}
		}
		pSig++;
	}
PARSE_COMPLETED:
	KASSERT(count + extended_size == getParamSize() + 1);
	// return-type을 제외한 길이로 aTypes의 길이를 변경한다.
	fm::setArrayLength_$$(aTypes, count - 1);
#ifndef FASTIVA_NO_PROXY_THUNK
#ifndef FASTIVA_NO_USE_INTERFACE_SLOT
	fastiva_lockGlobalRef(aTypes);
	*(void**)&this->m_aParam = aTypes;
#endif
#endif
	return aTypes;
#endif
	return aTypes;
}


static jlonglong FOX_FASTCALL(resolve_jni_method_addr)(void* unused, const char* mname[]) {
	char full_name[1204];
	full_name[0] = '_';
	char* fcc = full_name + 1;
	const char* cc;
	for (cc = mname[0]; *cc != 0; cc++) {
		*fcc ++ = *cc;
	}
	*fcc ++ = '_';
	for (cc = mname[1]; *cc != 0; cc++) {
		*fcc ++ = *cc;
	}
	/*
	*fcc ++ = '@';
	int sizStack = 0;
	for (cc++ ; *cc != 0; cc++) {
		char digit = *cc;
		*fcc ++ = digit;
		sizStack = (sizStack * 10) + (digit - '0');
	}
	*/
	*fcc = 0;
	mname[1] = 0;
#if 1 //def FASTIVA_CLDC
	int res = (int)fastiva_throwNoSuchMethodError;
#else
	int res = (int)java_lang_ClassLoader_C$::getRawStatic$()->findNative(
		(java_lang_ClassLoader_p)FASTIVA_NULL, fastiva.createAsciiString(full_name));
	if (res == 0) {
#ifdef _DEBUG
		int res = (int)java_lang_ClassLoader_C$::getRawStatic$()->findNative(
			(java_lang_ClassLoader_p)FASTIVA_NULL, fastiva.createAsciiString(full_name));
#endif
		res = (int)fastiva_throwNoSuchMethodError;
	}
#endif
	mname[0] = (const char*)res;
	return *(jlonglong*)(void*)(mname);
}

FOX_NO_RETURN void FOX_FASTCALL(__throwException)(fastiva_Task* pCurrTask) {
	KASSERT(pCurrTask == fastiva_getCurrentTask());
	fastiva.throwException(pCurrTask->m_pJniException);
}
	
#ifndef _ARM_
FOX_NAKED 
#endif
jlonglong fastiva_Runtime::invokeJNI(
	void* pObj,
	void* pMethodName
) {
#ifdef _ARM_
	FASTIVA_DBREAK();
	return 0;
#else
	_asm {
		mov eax, [esp];

		mov  [esp], ecx; // return address를 제거한다.
		push eax;

		mov eax, [edx];
		mov ecx, [edx+4];

		or ecx, ecx
		je	call_jni;

		call resolve_jni_method_addr;
call_jni:
		mov ecx, fs:[18h];
		mov ecx, [ecx + 14h];
		pop edx;
		push ecx;
		mov  [ecx].m_invokeJNI_retAddr, edx;
		call eax;

		mov ecx, fs:[18h];
		mov ecx, [ecx + 14h];
		cmp [ecx].m_pJniException, 0
		jne __throwException
		
		mov ecx, [ecx].m_invokeJNI_retAddr
		jmp  ecx;
		/*
		call eax;	// stack 이 모두 제거된 상태이다.
		mov ecx, fs:[18h];
		mov ecx, [ecx + 14h];
		mov ecx, [ecx].m_pJniException
		or  ecx, ecx
		je  no_exception
		jmp __throwException;
no_exception:
		ret;		// return 할 주소값이 없어진 상태이다.
		*/
	}
#endif
}
