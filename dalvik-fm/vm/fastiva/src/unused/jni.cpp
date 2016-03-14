#include <kernel/Kernel.h>
#include <kernel/sys.h>


java_lang_Class_p fastiva_FieldInfo::getClass() {
	return KERNEL::getArrayClass(getContext(), getDimension());
}

const fastiva_ClassContext* fastiva_FieldInfo::getContext() {
	const fastiva_ClassContext* pContext;
	if (this->isConstant()) {
		pContext = (fastiva_ClassContext*)this->m_moniker;
	}
	else {
		pContext = this->m_pContext;
	}
	if (pContext < (void*)0x10) {
		pContext = KEENEL::getPrimitiveContext((fastiva_ClassMoniker*)pContext);
	}
	return pContext;
}

void* fastiva_FieldInfo::getAddress(java_lang_Object_p pOwner) {
	if (this->isConstant()) {
		return m_pValue;
	}
	else { 
		KASSERT(!this->isStatic() || pObj->getClass() == java_lang_Class::importClass$());
		return (char*)pObj + this->m_offset;
	}
}

void fastiva_FieldInfo::setObjectValue(java_lang_Object_p pOwner, fastiva_Unkown_p pValue0) {
	KASSERT(!this->isConstant());

	java_lang_Object_p pValue = pValue0->getInstance$();
	java_lang_Class_p pValueType = pValue->getClass();
	java_lang_Class_p pTargetType = this->getClass();
	if (!pTargetType->isAssignableFrom(pValueType->getClass())) {
		fastiva_throwIllegalArgumentException();
	}

	if (m_pContext->isIterface() && getDimension() == 0) {
		pValue = (java_lang_Object_p)((char*)pValue + m_pContext->getCastingOffsetFrom(pValue));
	}

	java_lang_Object_p* ppField = getAddress(pOwner);
	*ppField = pValue;
}

/*
void fastiva_FieldInfo::setPrimitiveValue(java_lang_Object_p pOwner, void* pValue) {
	KASSERT(!this->isConstant());
	KASSERT(!this->m_pContext < (const void*) 0x10);

	java_lang_Object_p pValue = ((fastiva_Unknown_p)pValue)->getInstance$();
	java_lang_Class_p pValueType = pSourceVal->getClass();
	java_lang_Class_p pTargetType = this->getClass();
	if (!pTargetType->isAssignableFrom(pValueType->getClass())) {
		fastiva_throwIllegalArgumentException();
	}

	if (m_pContext->isIterface() && getDimension() == 0) {
		pValue = (java_lang_Object_p)((char*)pValue + m_pContext->getCastingOffsetFrom(pValue));
	}
	java_lang_Object_p* ppField = getAddress(pOwner);
	*ppField = pValue;
}
*/

