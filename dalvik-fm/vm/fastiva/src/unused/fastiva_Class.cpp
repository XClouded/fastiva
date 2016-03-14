#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <kernel/Runnable.h>
#include <kernel/HeapMark.h>


#if FASTIVA_SUPPORTS_JAVASCRIPT
#include <com/wise/jscript/JsScript.h>
#endif

#include <fox/Heap.h>
#include <memory.h>

//typedef fm::Kernel KERNEL;
void* set_current_thread_access_level(const fastiva_ClassContext* pContext);
extern "C" jlonglong FOX_FASTCALL(currentTimeMillis_MD)();
int g_cacheTypeCheckHit = 0;

/* v3
#ifdef FASTIVA_SHORT_INTERFACE
#ifndef FASTIVA_NO_USE_INTERFACE_SLOT
void* fm::Interface_T$::castInterface$(void* slot) {
	int slot_offset = (char*)slot - (char*)this;
	int cast_offset = *(int*)((char*)this->m_pIVT + slot_offset);
	fm::Interface_p pIFC = (fm::Interface_p)((char*)this->getInstance$() + cast_offset);
	KASSERT(getInstance$() == pIFC->getInstance$());
	return pIFC;
}
#endif
#endif
*/
//const fastiva_ClassContext& fastiva_Instance::g_context$ = *(const fastiva_ClassContext*)0;

void* fastiva_Instance_G$::aIVT$[] = {
	0 
};





#define JNI_SIG_jbool	"Z"
#define JNI_SIG_jbyte		"B"
#define JNI_SIG_unicod		"C"
#define JNI_SIG_jshort		"S"
#define JNI_SIG_jint		"I"
#define JNI_SIG_jfloat		"F"
#define JNI_SIG_jlonglong	"J"
#define JNI_SIG_jdouble		"D"
#define JNI_SIG_jvoid		"V"
#define JNI_SIG_jcustom		"?"


#define FASTIVA_INIT_PRIMITIVE_CONTEXT(T) {								\
	JNI_SIG_##T,														\
	(fastiva_Class_p)&s_aPrimitiveClass[fastiva_Primitives::T][0],		\
	0, fastiva_Primitives::T, ADDR_ZERO, 								\
}

/*
	static const fastiva_PrimitiveInfo g_aPrimitiveContext[fastiva_Primitives::cntType];
	static jbyte g_aPrimitiveClass[fastiva_Primitives::cntType][sizeof(fastiva_Class_T$)];
	static jbyte g_aPrimitiveArrayClass[fastiva_Primitives::cntType-1][sizeof(fastiva_Class_T$)];
*/

jbyte s_aPrimitiveClass[fastiva_Primitives::cntType][FASTIVA_MAX_ARRAY_DIMENSION+1][sizeof(fastiva_PrimitiveClass_T$)];
fastiva_PrimitiveClass_p* fastiva_Runtime::primitiveClasses;

const fastiva_ClassInfo s_aPrimitiveContext[fastiva_Primitives::cntType] = {
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jbool),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(unicod),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jfloat),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jdouble),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jbyte),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jshort),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jint),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jlonglong),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jcustom),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jvoid),
};

const fastiva_ClassContext* fastiva_Runtime::primitiveContexts[] = {
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 0),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 1),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 2),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 3),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 4),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 5),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 6),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 7),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 8),
	(const fastiva_ClassContext*)(s_aPrimitiveContext + 9),
};

void* fastiva_BytecodeProxy::getRawStatic$() {
	FASTIVA_DBREAK();
	return 0;
}

void* fastiva_BytecodeProxy::importClass$() {
	FASTIVA_DBREAK();
	return 0;
}

fastiva_BytecodeProxy fastiva_BytecodeProxy::g_debugInstance;

int fm::getPrimitiveTypeID(fastiva_Class_p pClass) {
	if (pClass >= (void*)&s_aPrimitiveClass[0]
	&&  pClass < (void*)&s_aPrimitiveClass[fastiva_Primitives::cntType]) {
		if (fm::getArrayDimension(pClass) == 0) {
			return ((int)pClass - (int)s_aPrimitiveClass) / sizeof(s_aPrimitiveClass[0]);
		}
	};
	return -1;
}

int fm::getPrimitiveReflectionType(fastiva_Class_p pClass) {
	if (pClass == (java_lang_Class_p)java_lang_Boolean_C$::getRawStatic$()) {
		return fastiva_Primitives::jbool;
	}
	else
	if (pClass == (java_lang_Class_p)java_lang_Byte_C$::getRawStatic$()) {
		return fastiva_Primitives::jbyte;
	}
	else
	if (pClass == (java_lang_Class_p)java_lang_Short_C$::getRawStatic$()) {
		return fastiva_Primitives::jshort;
	}
	else
	if (pClass == (java_lang_Class_p)java_lang_Character_C$::getRawStatic$()) {
		return fastiva_Primitives::unicod;
	}
	else
	if (pClass == (java_lang_Class_p)java_lang_Integer_C$::getRawStatic$()) {
		return fastiva_Primitives::jint;
	}
	else
	if (pClass == (java_lang_Class_p)java_lang_Long_C$::getRawStatic$()) {
		return fastiva_Primitives::jlonglong;
	}
	else 
	if (pClass == (java_lang_Class_p)java_lang_Float_C$::getRawStatic$()) {
		return fastiva_Primitives::jfloat;
	}
	else
	if (pClass == (java_lang_Class_p)java_lang_Double_C$::getRawStatic$()) {
		return fastiva_Primitives::jdouble;
	}
	return -1;
}


void fm::initPrimitiveClasses() {
	fastiva_Runtime::primitiveClasses = (fastiva_PrimitiveClass_p*)sys_heap_virtualAlloc(0, 65536*4);
	memset((void*)s_aPrimitiveClass, 0, sizeof(s_aPrimitiveClass));
	{
		initArrayVTable();
		for (int typeID = 0; typeID < fastiva_Primitives::cntType; typeID++) {
			fastiva_PrimitiveClass_p pClass = (fastiva_PrimitiveClass_p)&s_aPrimitiveClass[typeID][0];
			*(int*)pClass = (int)java_lang_Class_G$::g_vtable$; // <- vtable 초기화.
			pClass->m_pContext$ = fastiva.primitiveContexts[typeID];
			pClass->m_pClass$ = java_lang_Class_C$::getRawStatic$();
			fastiva.primitiveClasses[typeID] = pClass;

			for (int dimension = 1; dimension <= FASTIVA_MAX_ARRAY_DIMENSION; dimension ++) {
				fastiva_PrimitiveClass_p pArrayClass = pClass + 1;
				memcpy(pArrayClass, pClass, sizeof(fastiva_Class_T$));
				if (dimension == 1) {
					pArrayClass->obj.vtable$ = &kernelData.primitiveArrayVTable;
				}
				else {
					pArrayClass->obj.vtable$ = &kernelData.pointerArrayVTable;
				}
				//pArrayClass->m_pIVTables = kernelData.arrayIVTables;
				fm::setArrayDimension(pArrayClass, dimension); // ar

				(pArrayClass)->m_pComponentType$ = pClass;
				pClass->m_pCompositeType$ = pArrayClass;
				pClass = pArrayClass;

				fastiva.primitiveClasses[typeID + dimension * fastiva_Primitives::cntType] = pArrayClass;
			}
		}
		{
			fastiva.primitiveClasses[fastiva_Primitives::jcustom]->obj.vtable$ = &kernelData.proxyArrayVTable;
		}
	}
}


const fastiva_ClassContext* fm::getPrimitiveContext(
	uint primitiveTypeID
) {
	KASSERT(primitiveTypeID < fastiva_Primitives::cntType);
	return (fastiva_ClassContext*)(fastiva.primitiveContexts[primitiveTypeID]);
}





int fm::getPrimitiveArrayItemSize(
	uint primitiveTypeID
) {
	uint idx = (uint)primitiveTypeID;
	if (idx == fastiva_Primitives::jcustom) {
		return sizeof(void*);
	}
	KASSERT(idx < fastiva_Primitives::jvoid); // void 제외
	return 1 << (idx & 3);
}


int fm::getPrimitiveArrayItemSize(
	const fastiva_ClassContext* pContext
) {
	KASSERT(pContext->isPrimitive());
	KASSERT((ushort)pContext->m_inheritDepth <= fastiva_Primitives::jvoid);
	return fm::getPrimitiveArrayItemSize(pContext->m_inheritDepth);
}


const int* fastiva_Instance::getInterfaceTable$(
	const fastiva_ClassContext* pInterfaceContext
) {
	const fastiva_InstanceContext* srcContext = fm::getInstanceContext(this);
	const fastiva_ImplementInfo* pImpl = pInterfaceContext->isAssignableFrom(srcContext);
	// null-pointer excpetion이 발생할 수 있다.
	return (int*)pImpl->m_pIVTable;
}

jbool fastiva_ClassContext::isInstance(const fastiva_Instance_p pObj) const {
	const fastiva_InstanceContext* srcContext = fm::getInstanceContext(pObj);
	if (this->isInstance()) {
		if ((void*)srcContext == (void*)this) {
			return true;
		}
		return this->toInstanceContext()->isAssignableFrom(srcContext);
	}
	else {
		return this->toInterfaceContext()->isAssignableFrom(srcContext) != ADDR_ZERO;
	}
}

// Array가 아닌 경우에만 허용된다.
jbool fastiva_ClassContext::isAssignableFrom(java_lang_Class_p srcClass) const {
	const fastiva_ClassContext* srcContext = srcClass->m_pContext$;
	if (fm::getArrayDimension(srcClass) != 0) {
		return false;
	}
	if (srcContext == this) {
		return true;
	}
	return isAssignableFrom(srcContext);
}

jbool fastiva_InstanceContext::isAssignableFrom(const fastiva_InstanceContext* pContext) const {
	KASSERT(this->isInstance());
	// 속도를 위해서 PContext의 적합성 검사는 하지 않는다.
	// if (pContext->isInterface() || pContext->isPrimitive()) {
	//	return false;
	// }
	KASSERT ((void*)pContext != (void*)this);
	int depth = pContext->getInheritDepth() - this->getInheritDepth();

	while (depth -- > 0) {
		pContext = pContext->m_pSuperContext;
	}
	return (pContext == this);
}

jbool fastiva_InterfaceContext::isAssignableFrom(const fastiva_ClassContext* pContext) const {
	KASSERT(pContext != this);

	const fastiva_ClassContext** pImpl = pContext->m_ppIFC;
	const fastiva_ClassContext* pIFC;
	for (; (pIFC = *pImpl) != NULL; pImpl++) {
/*
		int depth = pIFC->getInheritDepth() - this->getInheritDepth();
		while (depth -- > 0) {
			pIFC = pIFC->m_ppIFC[0];
		}
*/
		if (pIFC == this) {
			return true;
		}
		if (this->isAssignableFrom(pIFC)) {
			return true;
		}
	}
	return false;
}

const fastiva_ImplementInfo* fastiva_InterfaceContext::isAssignableFrom(const fastiva_InstanceContext* pContext) const {
	const fastiva_ImplementInfo* pImpl = pContext->m_aImplemented;
	const fastiva_InterfaceContext* pIFC;
	for (; (pIFC = pImpl->m_pInterfaceContext) != NULL; pImpl++) {
		if (pIFC == this) {
			return pImpl;
		}
	}
	return ADDR_ZERO;
}

/*
jbool fastiva_InterfaceContext::isAssignableFrom(const fastiva_ClassContext* pContext0) const {
	if (fm::isPrimitive(pContext0)) {
		return false;
	}
	else if (!pContext0->isInterface()) {
		return pContext0->toInstanceContext()->getInterfaceTable(this) != ADDR_ZERO;
	}
	else {
		return isAssignableFrom(pContext0->toInterfaceContext());
	}
}
*/


void* fastiva_Runtime::isInstanceOf(
    fastiva_Instance_p ptr,
	const fastiva_ClassContext* pContext0
) {
	CHECK_STACK_FAST();
	// @1. null 인가 검사.
	if (ptr == FASTIVA_NULL) {
		return ptr;
	}
	KASSERT(ptr != ADDR_ZERO);
	fastiva_Instance_p pObj = ptr->getInstance$();
	java_lang_Class_p pObjClass = pObj->getClass$();
	const fastiva_InstanceContext* clsContext = pContext0->toInstanceContext();
	const fastiva_ClassContext* objContext = pObjClass->m_pContext$;
	const bool isArray = pObjClass->m_pComponentType$ != 0;

	if (isArray) {
		if (objContext == FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Object)
		||  objContext == FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Cloneable)
		||  objContext == FASTIVA_RAW_CLASS_CONTEXT_PTR(java_io_Serializable)) {
			return ptr;
		}
		return ADDR_ZERO;
	}

	if ((void*)clsContext == (void*)objContext) {
		return ptr;
	}

	if (isArray || !clsContext->m_pClass->isLinked$()) {
		// UNLINKED Class 이다. 즉 한 번도 생성된 적이 없다.
		// 내부의 Context 정보가 불완전하므로 return; 
		return ADDR_ZERO;
	}

	if (clsContext->isAssignableFrom(objContext->toInstanceContext())) {
		return ptr;
	}
	else {
		return ADDR_ZERO;
	}
}

void fastiva_Runtime::checkInstanceOf(
	fastiva_Instance_p ptr,
	const fastiva_ClassContext* pContext0
) {
	if (ptr != ADDR_ZERO && !isInstanceOf(ptr, pContext0)) {
		fastiva.throwClassCastException(ptr, pContext0->m_pClass);
	}
}

void fastiva_Runtime::checkInstanceOf_dbg(
	fastiva_Instance_p ptr,
	const fastiva_ClassContext* pContext0
) {
	if (ptr != ADDR_ZERO && !isInstanceOf(ptr, pContext0)) {
		char name[512];
		int len = fm::getClassNameUTF8(name, sizeof(name), pContext0);
		fox_printf("!!! Critical type cast error : %s", name);
		FASTIVA_DBREAK();
		fastiva.throwClassCastException(ptr, pContext0->m_pClass);
	}
}

void fastiva_Runtime::checkImplemented(
	fastiva_Instance_p ptr,
	const fastiva_ClassContext* pIfcContext
) {
	if (ptr != FASTIVA_NULL) {
		const fastiva_InstanceContext* srcContext = fm::getInstanceContext(ptr);
		const fastiva_ImplementInfo* pImpl = pIfcContext->isAssignableFrom(srcContext);
		if (pImpl == ADDR_ZERO) {
			fastiva.throwClassCastException(ptr, pIfcContext->m_pClass);
			//__debugbreak();
		}
	}
}

static void* array_interface_table[] = { 0 };

const void** fastiva_Runtime::isImplemented(
	fastiva_Instance_p ptr,
	const fastiva_ClassContext* pIfcContext
) {
	if (ptr == FASTIVA_NULL) {
		return ADDR_ZERO;
	}
	java_lang_Class_p pClass = ptr->getClass$();
	if (pClass->m_pComponentType$ != ADDR_ZERO) {
		if ((void*)pIfcContext == (void*)java_lang_Cloneable::getRawContext$
		||  (void*)pIfcContext == (void*)java_io_Serializable::getRawContext$) {
			return (const void**)array_interface_table;
		}
		else {
			return 0; 
		}
	}
	const fastiva_InstanceContext* srcContext = fm::getInstanceContext(pClass);
	const fastiva_ImplementInfo* pImpl = pIfcContext->isAssignableFrom(srcContext);
	if (pImpl == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	return (const void**)pImpl->m_pIVTable;
}


/*
void* fm::isImplemented(
    fastiva_Instance_p ptr,
	const fastiva_ClassMoniker* pMoniker
) {
	const fastiva_ClassContext* pContext;
	pContext = fm::findLoadedMoniker(pMoniker);
	if (pContext == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	return isImplemented(ptr, pContext);
}
*/
/*
void fm::checkImplemented(
	fastiva_Instance_p ptr,
	int ifc_key
) {
	if (ptr->getInterfaceTable$(ifc_key) == 0) {
		fastiva.throwClassCastException();
	}
}
*/

#if 0
void* fm::isImplemented(
    fastiva_Instance_p ptr,
	const fastiva_ClassContext* pContext0
) {
	CHECK_STACK_FAST();
	const fastiva_ClassContext* clsContext = pContext0->toInterfaceContext();
	// @1. null 인가 검사.
	KASSERT(ptr != FASTIVA_NULL);
	KASSERT(ptr != ADDR_ZERO);
	fastiva_Instance_p pObj = ptr->getInstance$();
	KASSERT(!fm::isInterface(pObj));

	const fastiva_Class_p pObjClass = pObj->getClass();
	const fastiva_InstanceContext* objContext = pObjClass->m_pContext$->toInstanceContext();

#ifndef FASTIVA_CLDC	
	if (fm::getArrayDimension(pObjClass) > 0) {
		if ((void *)clsContext == (void *)java_io_Serializable_T$::getRawContext$()) {
			return (java_io_Serializable_p)(java_lang_Object_ap)pObj;
		}
		if ((void *)clsContext == (void *)java_lang_Cloneable_T$::getRawContext$()) {
			return (java_lang_Cloneable_p)(java_lang_Object_ap)pObj;
		}
		return ADDR_ZERO;
	}
#endif

	// 주의) array-class는 fastiva-class가 아니다.
	// 반드시 array-class 여부를 먼저 확인한 후에 fastiva_Class_T$로 casting한다.
	const fastiva_IVTable* pIVT;
	pIVT = (const fastiva_IVTable*)((fastiva_Class_p)pObjClass)->m_pInterface_C$;
	if (pIVT->m_pContext == clsContext) {
#ifdef _DEBUG
		g_cacheTypeCheckHit ++;
#endif
		return (char*)pObj + pIVT->m_offset;
	}
/*
#ifdef ENABLE_TYPE_CHECK_CACHE
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	if (pCurrTask->m_pCachedInterfaceClass == pObjClass
	&&  pCurrTask->m_pCachedInterfaceType == clsContext) {
#ifdef _DEBUG
		g_cacheTypeCheckHit ++;
#endif
		return (char*)ptr + pCurrTask->m_cachedInterfaceOffset;
	}
#endif
*/
	if (!clsContext->m_pClass->isLinked$()) {
		// UNLINKED Class 이다. 즉 한 번도 생성된 적이 없다.
		// 내부의 Context 정보가 불완전하므로 return; 
		return ADDR_ZERO;
	}

	pIVT = objContext->getInterfaceTable(clsContext);
	if (pIVT != ADDR_ZERO) {
		((fastiva_Class_p)pObjClass)->m_pInterface_C$ = pIVT;
/*
#ifdef ENABLE_TYPE_CHECK_CACHE
		pCurrTask->m_pCachedInterfaceClass = pObjClass;
		pCurrTask->m_pCachedInterfaceType = clsContext;
		pCurrTask->m_cachedInterfaceOffset = pIVT->m_offset;
#endif
*/
		return (char*)pObj + pIVT->m_offset;
	}
	return ADDR_ZERO;
}
#endif
#if 0
void* fm::checkImplemented(
	fastiva_Instance_p ptr,
	const fastiva_ClassMoniker* pMoniker
) {
	const fastiva_ClassContext* pContext;
	pContext = fm::findLoadedMoniker(pMoniker);
	if (pContext == ADDR_ZERO) {
		fastiva.throwClassCastException(ptr, pContext->m_pClass$);
	}
	return checkImplemented(ptr, pContext);
}

#endif




/*
const fastiva_IVTable* fastiva_Instance_T$::getInterfaceTable$(
	const fastiva_ClassMoniker* pMoniker
) {
	const fastiva_ClassContext* pContext;
	pContext = fm::findLoadedMoniker(pMoniker);
	if (pContext == ADDR_ZERO) {
		fastiva.throwClassCastException();
	}

	KASSERT(!pContext->isInstance());
	return getInterfaceTable(pContext);
}

const fastiva_IVTable* fastiva_Instance_T$::getInterfaceTable(
	const fastiva_ClassContext* pContext0
) {
	const fastiva_ClassContext* pIFC = pContext0->toInterfaceContext();
	if (this == FASTIVA_NULL) {
		fastiva_throwNullPointerException();
	}
	KASSERT(this != ADDR_ZERO);

	const fastiva_InstanceContext* pContext = this->getInstanceContext();
	const fastiva_IVTable* pIVT = pContext->getInterfaceTable(pIFC);
	if (pIVT == ADDR_ZERO) {
		fastiva.throwClassCastException();
	}
	return pIVT;
}
*/


//*/




void* fastiva_Runtime::isArrayInstanceOf(
	fastiva_Instance_p pObj,
	const fastiva_ClassContext* clsContext, 
	int dimension
) {
	CHECK_STACK_FAST();
	jbool succeed;

	KASSERT(pObj != FASTIVA_NULL);
	KASSERT(pObj != ADDR_ZERO);
	KASSERT(dimension > 0 && dimension <= FASTIVA_MAX_ARRAY_DIMENSION);
	//KASSERT(fm::isPureInstance(this));

	java_lang_Class_p pObjClass = pObj->getClass$();
	int srcDepth = fm::getArrayDimension(pObjClass);
	if (srcDepth < dimension) {
		return ADDR_ZERO;
	}

	fastiva_ArrayHeader* pArray = (fastiva_ArrayHeader*)pObj;
	const fastiva_ClassContext* objContext = pObjClass->m_pContext$;
	jbool isPrimitive = objContext->isPrimitive() | clsContext->isPrimitive();

	if (clsContext == java_lang_Object::getRawContext$()) {
		if (dimension <= srcDepth - (isPrimitive ? 1 : 0)) {
			return pObj;
		}
		else {
			return ADDR_ZERO;
		}
	}

	if (srcDepth != dimension) {
		return ADDR_ZERO;
	}

	if (clsContext == objContext) {
		return pObj;
	}

	if (isPrimitive) {
		return ADDR_ZERO;
	}

#ifdef ENABLE_TYPE_CHECK_CACHE
	fastiva_Task* pCurrTask = fastiva_getCurrentTask();
	if (pCurrTask->m_pCachedArrayClass == objContext
	&&  pCurrTask->m_pCachedArrayType == clsContext) {
#ifdef _DEBUG
		g_cacheTypeCheckHit ++;
#endif
		return pObj;
	}
#endif

	if (fm::isLoaded(clsContext->m_pClass)
	&&  clsContext->isAssignableFrom(objContext)) {
#ifdef ENABLE_TYPE_CHECK_CACHE
		pCurrTask->m_pCachedArrayClass = objContext;
		pCurrTask->m_pCachedArrayType = clsContext;
#endif
		return pObj;
	}
	else {
		return ADDR_ZERO;
	}
}



void fastiva_Runtime::checkArrayInstanceOf(
	fastiva_Instance_p pObj,
	const fastiva_ClassContext* clsContext, 
	int dimension
) {
	//KASSERT(this != ADDR_ZERO);
	if (pObj == FASTIVA_NULL) {
		return;
	}
	if (!isArrayInstanceOf(pObj, clsContext, dimension)) {
		fastiva.throwClassCastException(pObj, clsContext->m_pClass);
	}
}



/*
java_lang_Class_p fm::importMoniker(
	const fastiva_ClassMoniker* pMoniker
) {
	java_lang_Class_p pClass = (java_lang_Class_p)pMoniker->m_pStatic0;
	if (pClass != ADDR_ZERO) {
		return pClass;
	}
	const fastiva_ClassContext* pContext = fm::loadMoniker(pMoniker);
	return (java_lang_Class_p)
		(*(void**)&pMoniker->m_pStatic0 = (fastiva_Method*)fm::linkClass(pContext->m_pClass));
}
//*/
/*
NAKED int fm::getMethodAddr(const fastiva_Method* pStatic, int offset) {

	if (((int)pStatic & 1) != 0) {
		pStatic = pStatic & ~1;
	}
	return 
	__asm mov  eax, [ecx].m_pContext$
	__asm mov  eax, [eax+edx]
	__asm ret
}
*/


/*
__int64 fm::initInterfaceTrack(const fastiva_ClassContext* pIfcCtx) {
	if (pIfcCtx->m_ppIFC[0] != 0) {
		__int64 res = initInterfaceTrack(pIfcCtx->m_ppIFC[0]);
		pIfcCtx->m_pClass->obj.vtable$ = (void*)res;
		return res;
	}
	else {
		int track = pIfcCtx->m_crc32 & (FASTIVA_MAX_INTERFACE_TRACK - 1);
		int tslot = (int)pIfcCtx->m_pClass->obj.vtable$;
		if (tslot <= 0) {
			tslot = FastUtil__add((uint*)&kernelData.g_aInterfaceTrack[track], 1);
		}
		return (((__int64)track) << 32) + tslot;
	}
}
*/


const fastiva_ClassContext* fm::getPrimitiveContextBySig(unicod signature) {
	switch (signature) {
		case 'Z':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jbool);
		case 'B':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jbyte);
		case 'C':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(unicod);
		case 'S':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jshort);
		case 'I':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jint);
		case 'J':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jlonglong);
		case 'F':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jfloat);
		case 'D':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jdouble);
		case 'V':
			return FASTIVA_PRIMITIVE_CONTEXT_OF(jvoid);
	}
	return ADDR_ZERO;
}


java_lang_Class_p fm::getPrimitiveClassBySig(unicod signature) {
	switch (signature) {
		case 'Z':
			return FASTIVA_PRIMITIVE_CLASS_OF(jbool);
		case 'B':
			return FASTIVA_PRIMITIVE_CLASS_OF(jbyte);
		case 'C':
			return FASTIVA_PRIMITIVE_CLASS_OF(unicod);
		case 'S':
			return FASTIVA_PRIMITIVE_CLASS_OF(jshort);
		case 'I':
			return FASTIVA_PRIMITIVE_CLASS_OF(jint);
		case 'J':
			return FASTIVA_PRIMITIVE_CLASS_OF(jlonglong);
		case 'F':
			return FASTIVA_PRIMITIVE_CLASS_OF(jfloat);
		case 'D':
			return FASTIVA_PRIMITIVE_CLASS_OF(jdouble);
		case 'V':
			return FASTIVA_PRIMITIVE_CLASS_OF(jvoid);
	}
	return ADDR_ZERO;
}


// PACKAGE-INFO 내에서 pContext가 존재하는지 확인한다.
void fm::checkInternalClass(void* packageInfo, const fastiva_ClassContext* pContext) {
	if (packageInfo == ADDR_ZERO) {
		return;
	}
	else {
		fastiva_throwInstantiationException();
	}
}

int fm::getClassNameUTF8(char* name, int length, const fastiva_ClassContext* pContext) {
	const char* pszPackageName = pContext->getPackageName();
	char* dst = name;
	const char* src;
	for (src = pszPackageName; *src != 0;) {
		*dst ++ = *src ++;
	}
	*dst++ = '/';
	for (src = pContext->m_pBaseName; *src != 0;) {
		*dst ++ = *src ++;
	}
	*dst = 0;
	return dst - name;
}

// 다음 함수는 Package-Component(Fox) 마다 Copy되어야 한다.
//extern void* g_pPackageInfo = ADDR_ZERO;

void __fastcall JSCRIPT_INIT(void* self) {
    return;
}

java_lang_Object_p fm::newInstance(java_lang_Class_p pClass, const fastiva_ClassContext* pCallerContext) {

	CHECK_STACK_FAST();

	const fastiva_ClassContext * pContext = pClass->m_pContext$;

	KASSERT(pContext->isInstance());
	KASSERT(fm::getArrayDimension(pClass) == 0);

	typedef void (FOX_FASTCALL(*DEF_INIT)) (void*);
	DEF_INIT pfnInit = (DEF_INIT)pContext->toInstanceContext()->m_pfnInit;
	if (pfnInit != 0) {
		java_lang_Object_p pObj = fastiva.allocate(pContext);
		pfnInit(pObj);
		return pObj;
	}
	else {
#if FASTIVA_SUPPORTS_JAVASCRIPT
		if (com_wise_jscript_JsScript::getRawContext$()->isAssignableFrom(pContext)) {
			pfnInit = JSCRIPT_INIT;
			pfnInit(pObj);
			return pObj;
		}
#endif
		fastiva_throwInstantiationException();
	}

#if 0
	const fastiva_ClassContext * pContext = pClass->m_pContext$;
	JNI_FindMethod fm;
	fm.initDefaultConstructor();
	if (pCallerContext == ADDR_ZERO) {
		pCallerContext = pContext;
	}

	int acc_flag = ACC_FINAL$;
	/*
	if (pContext->m_pPackage != pCallerContext->m_pPackage) {
		acc_flag = ACC_PUBLIC$ | ACC_PROTECTED$;
	}
	else {
		acc_flag = ACC_FINAL$;
	}
	*/
	fm.m_accessFlags = acc_flag;

	const fastiva_Method * pMethod = pContext->getDeclaredMethod(&fm);
	typedef void (FOX_FASTCALL(*DEF_INIT)) (void*);
	DEF_INIT pfnInit;

	if (pMethod == ADDR_ZERO) {
#if FASTIVA_SUPPORTS_JAVASCRIPT
		if (com_wise_jscript_JsScript::getRawContext$()->isAssignableFrom(pContext)) {
			pfnInit = JSCRIPT_INIT;
		}
		else 
#endif
		{
			// array, interface, non-default-constructor class 들.
			fastiva_throwInstantiationException();
		}
	}
	else {
		pfnInit = (DEF_INIT)pMethod->getMethodAddr(ADDR_ZERO, pClass);
	}
	/*
	if ((pContext->m_flagDefInit & ACC_PUBLIC$) == 0) {
		if ((pContext->m_flagDefInit & ACC_PRIVATE$) != 0) 
		||  (pCurrPackage != pContext->m_pPackage)) {
			fastiva_throwIllegalAccessException();
		}
	}
	*/

	KASSERT(pContext->isInstance());
	KASSERT(fm::getArrayDimension(pClass) == 0);
	java_lang_Object_p pObj = fastiva.allocate(pContext);
	pfnInit(pObj);
	return pObj;
#endif

	// @1. 생성 가능여부 조사
	/*
		fastiva_Class_T$::newInstance$() 내에서 모두 걸러진다.
	*/

	// forName()에 의해 initStatic()은 이미 수행된 상태.
	// forName()에 의해 KASSERT(ACC_PUBLIC != 0); 

	// @ about-static initializing 
	// Array.getComponentType()에 의해 initialing되지 않은 Class에 대해
	// newInstance() 호출될 수 있으나, fm::Allocate$() 수행시
	// importClass() 호출된다. 2006.02.13 현재, pClass->newInstance$() 가 virtual 함수로
	// 선언되어 있고, 해당 vtable이 필요하기 때문에 임시로 importClass$()를 사용한다.



	// @3. 기본 생성자 FLAG 조사
	// 기본 생성자가  ACC_PROTECTED OR ACC_INTERNAL인 경우,
	// 동일 PACKAGE에 속하는지 검사.
	
	/*
	if ((pContext->m_flagDefInit & ACC_PUBLIC$) == 0) {
		if ((pContext->m_flagDefInit & ACC_PRIVATE$) != 0) 
		||  (pCurrPackage != pContext->m_pPackage)) {
			fastiva_throwIllegalAccessException();
		}
	}
	*/

	// @4. allocation
	// ACC_ABSTRACT 에 대한 조사는 fastiva.allocate()안에서 이루어짐.
//	java_lang_Object_p pNewObj;
//#if FASTIVA_NULL_ADDR == 0
//	pNewObj = fastiva.allocate(pContext);
//#else
//	pNewObj = pContext->allocate$();
//#endif
//	
//	// @5. instance initialization
//	FASTIVA_DEFAULT_CONSTRUCTOR_FN_TYPE defInit$;
//	_asm int 3;
//	/*v3
//	defInit$ = fm::getDefaultConstructor(pNewObj);
//	
//	//default constructor가 없으면 InstantiationException발생.
//	if (defInit$ == ADDR_ZERO) {
//		fastiva_throwInstantiationException();
//	}
//	//KASSERT(defInit$ != ADDR_ZERO);
//	*/
//	defInit$(pNewObj);
//	return pNewObj;
}

/**====================== end ========================**/

#if FASTIVA_SUPPORTS_JNI
char* __strcpy(char* dst, const char* src) {
	while (true) {
		*dst = *src;
		if (*dst == 0) {
			return dst;
		}
		dst ++;
		src ++;
	}
}

int getParamCount(const char *sig) {
	int type = 0;
	int curIdx = 0;

	while (sig) {
		switch (*sig) {
		case 'V':
		case 'B':
		case 'C':
		case 'S':
		case 'I':
		case 'Z':
		case 'F':
			curIdx++;
			sig++;
			break;
		case 'D':
		case 'J':
			curIdx +=2;
			sig++;
			break;

		case 'L':
			curIdx++;
			sig = strchr(sig, ';');
			sig++;
			break;

		case '[':
			curIdx++;
			sig++;
SIG_ARRAY:
			switch (*sig) {
			case 'L':
				sig = strchr(sig, ';');
				sig++;
				break;
			case '[':
				sig++;
				goto SIG_ARRAY;
				break;

			default:
				sig++;
				break;

			}
			break;

		case '(':
			sig++;
			continue;

		case ')':
			return curIdx;
		default:
			return 0;
		}
	}
	return curIdx;
}


void* fm::getNativeMethodAddr(
	const fastiva_InstanceContext* pContext,
	const fastiva_MethodInfo* pInfo
) {
	char method_name[1024] = "_Java_";
	char* buf = method_name + 6;
	
	const char *pPackageName = pContext->m_pPackage->m_pszName;
	while (*pPackageName != '\0') {
		if (*pPackageName == '.' || *pPackageName == '/') {
			*buf++ = '_';
		}
		else {
			*buf++ = *pPackageName;
		}

		pPackageName++;
	}
	
	*buf ++ = '_';
	buf = __strcpy(buf, pContext->m_pBaseName);
	*buf ++ = '_';
	const char *pMethodName = JNI_FindMethod::getMethodName(pInfo->m_name);
	while (*pMethodName != '\0') {
		if (*pMethodName == '_') {
			// Method Name에 '_'가 있으면 1을 붙이게 되어 있다.
			*buf++ = *pMethodName++;
			*buf++ = '1';
			continue;
		}
		*buf++ = *pMethodName++;
	}
	if (buf > method_name + 8) {
		if (buf[-2] == '0' && buf[-1] == '$') {
			buf -= 2;
		}
	}
	//buf = __strcpy(buf, pInfo->m_name);
	
	*buf++ = '@';

	int sizParam = (pInfo->m_argSize + 2) * 4;
	//buf = itoa(sizParam, buf, 10);
	//Linux에서는 itoa가 없다.
	sprintf(buf, "%d", sizParam);
	void* fn_addr = findNativeMethod_MD(method_name);
	if (fn_addr == ADDR_ZERO) {
		fox_debug_printf("native method %s not found\n", method_name);
		fn_addr = (void *)fastiva_throwUnsatisfiedLinkError;
	}
	return fn_addr;
}


jint fm::registerNatives(java_lang_Class_p pClass, 
	const JNINativeMethod_ex *pMethod,
	jint nMethods
) {
	const fastiva_ClassContext* pContext = pClass->m_pContext$;
	for (int i = nMethods; i-- > 0; pMethod++) {
		JNI_FindMethod fm;
		fm.init(pMethod->name, pMethod->signature);
		fm.m_accessFlags = ACC_NATIVE$;
		const fastiva_MethodInfo* pMI = pContext->getDeclaredMethod(&fm);
		if (pMI != ADDR_ZERO) {
			*(const void**)&pMI->m_code = pMethod->fnPtr;
		}
		else {
			fox_debug_printf("[WARNING] native slot not found %s, %s\n", pMethod->name, pMethod->signature);
			return -1;
		}
	}
	return 0;
}

void fm::registerNatives(java_lang_Class_p pClass) {
	const fastiva_InstanceContext* pContext = pClass->m_pContext$->toInstanceContext();
//	do {
		if ((pContext->m_accessFlags & ACC_NATIVE$) == 0) {
			const fastiva_JniInfo* pJNI = pContext->m_pJNI;
			pClass = pContext->m_pClass;
			if (pJNI != ADDR_ZERO && (pClass->m_mark$ & HM_NATIVE_REGISTERED) == 0) {
				const fastiva_MethodInfo* pMethod = pJNI->m_aMethod;
				for (int i = pJNI->m_cntMethod; --i >= 0; ) {
					if ((pMethod->m_accessFlags & ACC_NATIVE$) != 0) {
						*(void**)pMethod->m_code = fm::getNativeMethodAddr(pContext, pMethod);
					}
					pMethod ++;
				}
				pClass->m_mark$ |= HM_NATIVE_REGISTERED;
			}
		}
//		pContext = pContext->m_pSuperContext;
//	} while (pContext != ADDR_ZERO);
}


jint fm::unregisterNatives(java_lang_Class_p pClass) {
	const fastiva_InstanceContext* pContext = pClass->m_pContext$->toInstanceContext();
	const fastiva_JniInfo* pJNI = pContext->m_pJNI;
	pClass = pContext->m_pClass;
	if (pJNI != ADDR_ZERO && (pClass->m_mark$ & HM_NATIVE_REGISTERED) == 0) {
		const fastiva_MethodInfo* pMethod = pJNI->m_aMethod;
		for (int i = pJNI->m_cntMethod; --i >= 0; ) {
			if ((pMethod->m_accessFlags & ACC_NATIVE$) != 0) {
				*(void**)pMethod->m_code = (void *)fastiva_throwUnsatisfiedLinkError;
			}
			pMethod ++;
		}
		pClass->m_mark$ &= ~HM_NATIVE_REGISTERED;
	}
	return 0; // return -1 on fail
}

#else 
jint fm::registerNatives(java_lang_Class_p, JNINativeMethod_ex const *,int) { 
	FASTIVA_DBREAK();
	return 0; 
}
int fm::unregisterNatives(java_lang_Class_p) { 
	FASTIVA_DBREAK();
	return 0; 
}
#endif


