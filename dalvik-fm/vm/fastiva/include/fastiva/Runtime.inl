#ifndef __FASTIVA_RUNTIME__INL__
#define __FASTIVA_RUNTIME__INL__

/*
inline const int* java_lang_Class::getInterfaceTable$(
	const fastiva_ClassContext* pIfcType
) const {
// 대훈형이 내자리서 손본 것 - jay
	return g_fastivaRuntime.java_lang_Class_getInterfaceTable$(this, pIfcType);
}

/*
inline java_lang_Class_p java_lang_Class::getClass$(
) const {
	return g_fastivaRuntime.java_lang_Class_getClass$);
}
*/

extern fastiva_Runtime g_fastivaRuntime;

inline void fastiva_throwArrayIndexOutOfBoundsException() {
	g_fastivaRuntime.fastiva_throwArrayIndexOutOfBoundsException();
}


inline java_lang_Object_p fastiva.allocate(
	const fastiva_ClassContext* pContext
) {
	return g_fastivaRuntime.allocate(pContext);
}

inline java_lang_Object_p fm::allocateEx(
	const fastiva_InstanceContext* pContext,
	int sizInstance
) {
	return g_fastivaRuntime.allocateEx(pContext, sizInstance);
}

inline fastiva_ArrayHeader* fm::allocateMultiArray(
	const fastiva_ClassContext* pContext,
	int dimension,
	const int* aLength 
) {
	return g_fastivaRuntime.allocateMultiArray(pContext, dimension, aLength);
}

inline fastiva_ArrayHeader* fm::allocateMultiPrimitiveArray(
	uint primitiveTypeID,//const fastiva_ClassMoniker*,
	int dimension,
	const int* aLength 
) {
	return g_fastivaRuntime.allocateMultiPrimitiveArray(primitiveTypeID, dimension, aLength);
}

inline fastiva_ArrayHeader* fm::allocatePrimitiveArray(
	uint primitiveTypeID,//const fastiva_ClassMoniker*,
	int  length 
) {
	return g_fastivaRuntime.allocatePrimitiveArray(primitiveTypeID, length);
}

inline fastiva_ArrayHeader* fm::allocatePointerArray(
	const fastiva_ClassContext* pContext,
	int  length 
) {
	return g_fastivaRuntime.allocatePointerArray(pContext, length);
}

inline fastiva_ArrayHeader* fm::allocateInitializedArray(
	uint primitiveTypeID,//const fastiva_ClassMoniker*,
	const void* data,
	int length
) {
	return g_fastivaRuntime.allocateInitializedArray(primitiveTypeID, data, length);
}




inline void fm::setInstanceField(
	fastiva_Instance_p pObj, 
	fastiva_Instance_p pValue, 
	void* pFieldPos
) {
	return g_fastivaRuntime.setInstanceField(pObj, pValue, pFieldPos);
}


inline void fm::setStaticField(
	fastiva_Class_p pClass, 
	fastiva_Instance_p pValue, 
	void* pFieldPos
) {
	return g_fastivaRuntime.setStaticField(pClass, pValue, pFieldPos);
}

inline void fm::setStaticField(
	fastiva_Class_p pClass, 
	fastiva_Interface_p pValue, 
	void* pFieldPos
) {
	return g_fastivaRuntime.setStaticFieldEx(pClass, pValue, pFieldPos);
}


inline void fm::setArrayItem(
	fastiva_ArrayHeader* pArray, 
	const fastiva_ClassContext* pBaseType,
	void* pItemSlot, 
	fastiva_Instance_p pItem
) {
	return g_fastivaRuntime.setArrayItem(pArray, pBaseType, pItemSlot, pItem);
}

inline void fm::setAbstractArrayItem(
	fastiva_ArrayHeader* pArray, 
	const fastiva_ClassContext* pBaseType,
	void* pItemSlot, 
	fastiva_Instance_p pItem
) {
	return g_fastivaRuntime.setAbstractArrayItem(pArray, pBaseType, pItemSlot, pItem);
}



inline jlonglong fastiva.invokeJNI(
	void* pObj,
	void* pMethodName
) {
	return g_fastivaRuntime.invokeJNI(pObj, pMethodName);
}


inline java_lang_String_p fastiva.createString(
	const char* str
) {
	return g_fastivaRuntime.createString(str);
}

inline java_lang_String_p fastiva.createStringA(
	const char* str, 
	int str_len
) {
	return g_fastivaRuntime.createStringA(str, str_len);
}

inline java_lang_String_p fastiva.createStringW(
	const unicod* ucs, 
	int ucs_len
) {
	return g_fastivaRuntime.createStringW(ucs, ucs_len);
}

inline java_lang_String_p fm::createUTFString(
	const char* str
) {
	return g_fastivaRuntime.createUTFString(str);
}

inline int fm::getUnicodeLengthOfUTF8(
	const char* utf8,
	int str_len
) {
	return g_fastivaRuntime.getUnicodeLengthOfUTF8(utf8, str_len);
}

inline java_lang_String_p fm::createUTFStringA(
	const char* str, 
	int str_len
) {
	return g_fastivaRuntime.createUTFStringA(str, str_len);
}

inline int fastiva.getUTFLength(
	const unicod* ucs, 
	int ucs_len
) {
	return g_fastivaRuntime.getUTFLength(ucs, ucs_len);
}

// return end_of_utf_string;
inline char* fastiva.getUTFChars(
	char* buff,
	const unicod* ucs, 
	int ucs_len
) {
	return g_fastivaRuntime.getUTFChars(buff, ucs, ucs_len);
}

inline java_lang_String_p fastiva.createAsciiString(
	const char* str
) {
	return g_fastivaRuntime.createAsciiString(str);
}

inline java_lang_String_p fm::createAsciiStringA(
	const char* str, 
	int str_len
) {
	return g_fastivaRuntime.createAsciiStringA(str, str_len);
}


inline void fm::memset(
	void* buff, 
	int filler, 
	int length
) {
	return g_fastivaRuntime.memset(buff, filler, length);
}

inline int fm::getTickCount() {
	return g_fastivaRuntime.getTickCount();
}

inline void fm::monitorEnter(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.monitorEnter(pObj);
}

inline void fm::monitorExit(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.monitorExit(pObj);
}

/*
void fm::wait(
	fastiva_Instance_p pObj,
	int period
) {
	return g_fastivaRuntime.wait);
}

void fm::notify(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.notify);
}

void fm::notifyAll(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.notifyAll);
}
*/

inline void fm::beginSynchronized(
	fastiva_Instance_p pObj,
	Synchronize* pSync
) {
	return g_fastivaRuntime.beginSynchronized(pObj, pSync);
}

inline void fm::endSynchronized(
	fastiva_Instance_p pObj,
	Synchronize* pSync
) {
	return g_fastivaRuntime.endSynchronized(pObj, pSync);
}

inline void fm::linkSynchronized(
	fastiva_SynchronizedLink* pLink
) {
	return g_fastivaRuntime.linkSynchronized(pLink);
}

inline void fm::unlinkSynchronized(
	fastiva_SynchronizedLink* pLink
) {
	return g_fastivaRuntime.unlinkSynchronized(pLink);
}


/*
int fm::getMethodAddr$(
	const fastiva_Method* pStatic, int offset
) {
	return g_fastivaRuntime.getMethodAddr$);
}

void fm::cleanStaticData) (
	java_lang_Class_p pClass, 
	int sizClass
) {
	return g_fastivaRuntime.allocate);
}
*/

//==========================================================//
//			EXCEPTION MANAGER 								//
//==========================================================//

inline FOX_NO_RETURN void fastiva.throwException(
	java_lang_Throwable_p ex
) {
	g_fastivaRuntime.throwException(ex);
}

inline int fm::pushExceptionHandler(
	ExceptionContext* pContext
) {
	return g_fastivaRuntime.pushExceptionHandler(pContext);
}

inline void fm::removeExceptionHandler(
	ExceptionContext* pContext
) {
	return g_fastivaRuntime.removeExceptionHandler(pContext);
}

inline void fm::rethrow(
	ExceptionContext* pContext, void* pException
) {
	return g_fastivaRuntime.rethrow(pContext, pException);
}


inline int fm::dispatchNativeException(void* nativeException) {
	return g_fastivaRuntime.dispatchNativeException(nativeException);
}

// do not call this function directly;
inline void fm::beginManagedSection_$$(
	struct fastiva_ManagedSection* pContext
) {
	return g_fastivaRuntime.beginManagedSection_$$(pContext);
}

// do not call this function directly;
inline void fm::endManagedSection_$$(
	struct fastiva_ManagedSection* pContext
) {
	return g_fastivaRuntime.endManagedSection_$$(pContext);
}

// do not call this function directly;
inline void fm::enterNativeSection_$$(
	struct fastiva_NativeSectionContext* pContext
) {
	return g_fastivaRuntime.enterNativeSection_$$(pContext);
}

// do not call this function directly;
inline void fm::leaveNativeSection_$$(
	struct fastiva_NativeSectionContext* pContext
) {
	return g_fastivaRuntime.leaveNativeSection_$$(pContext);
}


// ========================= Stack Checking ====================== //
/*
	void fm::checkStackTrace(
	int flag,
	const char* funcname
) {
	return g_fastivaRuntime.allocate);
}

jdouble fm::IEEEremainder(
	jdouble double0, 
	jdouble double2
) {
	return g_fastivaRuntime.allocate);
}


void fm::checkStack() {
	return g_fastivaRuntime.allocate);
}

void* fm::getVM_ENV() {
	return g_fastivaRuntime.allocate);
}

//java_lang_Object_p fm::getReferent) (
//	const fm::WeakRef_R$* pRef 
//) {
	return g_fastivaRuntime.allocate);
}
*/

inline void fm::checkInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
	return g_fastivaRuntime.checkInstanceOf(ptr, pContext);
}

inline void fm::checkImplemented(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
	g_fastivaRuntime.checkImplemented(ptr, pContext);
}

/*
void* fm::checkImplemented(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext*
) {
	return g_fastivaRuntime.allocate);
}
void* fm::checkImplemented(
	fastiva_Instance_p ptr, 
	const fastiva_ClassMoniker*
) {
	return g_fastivaRuntime.allocate);
}
*/
inline bool fastiva.isInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
	return g_fastivaRuntime.isInstanceOf(ptr, pContext);
}

/*
void* fm::isImplemented(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext*
) {
	return g_fastivaRuntime.allocate);
}
*/
inline const void** fm::isImplemented(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
	return g_fastivaRuntime.isImplemented(ptr, pContext);
}

inline void fm::checkArrayInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext, 
	int dimension
) {
	return g_fastivaRuntime.checkArrayInstanceOf(ptr, pContext, dimension);
}
inline void fm::checkPrimitiveArrayInstanceOf(
	fastiva_Instance_p ptr, 
	uint primitiveTypeID,//const fastiva_ClassMoniker*,
	int dimension
) {
	return g_fastivaRuntime.checkPrimitiveArrayInstanceOf(ptr, primitiveTypeID, dimension);
}

inline void* fastiva.isArrayInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext, 
	int dimension
) {
	return g_fastivaRuntime.isArrayInstanceOf(ptr, pContext, dimension);
}

inline void* fm::isPrimitiveArrayInstanceOf(
	fastiva_Instance_p ptr, 
	uint primitiveTypeID,//const fastiva_ClassMoniker*,
	int dimension
) {
	return g_fastivaRuntime.isPrimitiveArrayInstanceOf(ptr, primitiveTypeID, dimension);
}
/*
void fm::checkInternalClass(
	void* packageInfo,
	const fastiva_ClassContext* pContext
) {
	return g_fastivaRuntime.checkInternalClass);
}

java_lang_Object_p fm::newInstance(
	java_lang_Class_p pClass,
	const fastiva_ClassContext* pCallerContext
) {
	return g_fastivaRuntime.newInstance);
}
// void fm::setField_$$(fastiva_Instance_p pOwner, fastiva_Instance_p pValue, void* pField);
*/
/*
java_lang_Object_p fm::setField_$$) (
	java_lang_Object_p pNewValue, 
	fastiva_Instance_p pOwner
);

java_lang_Object_p fm::setStaticField_$$) (
	java_lang_Object_p pNewValue, 
	fastiva_Instance_p pOwner
);

java_lang_Object_p fm::setReferent_$$) (
	java_lang_Object_p pNewValue, 
	fastiva_Instance_p pOwner
);
//*/


inline void* fastiva_jniProlog(
	java_lang_Class_p pClass
) {
	return g_fastivaRuntime.jniProlog(pClass);
}

inline void fastiva_jniEpilog(
	void* pEnv
) {
	return g_fastivaRuntime.jniEpilog(pEnv);
}

inline void fastiva_lockGlobalRef(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.lockGlobalRef(pObj);
}

inline void fastiva_releaseGlobalRef(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.releaseGlobalRef(pObj);
}
/*
void fm::lockLocalRef(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.lockLocalRef);
}

void fm::releaseLocalRef(
	fastiva_Instance_p pObj
) {
	return g_fastivaRuntime.releaseLocalRef);
}
*/
inline int fm::checkImported(
	java_lang_Class_p pClass
) {
	return g_fastivaRuntime.checkImported(pClass);
}


//inline float fm::IEEEremainder(float v1, float v2) {
//	return g_fastivaRuntime.IEEERemainderF(v1, v2);
//}

inline double fm::IEEEremainder(double v1, double v2) {
	return g_fastivaRuntime.IEEEremainder(v1, v2);
}


inline void fm::linkClass_$$(void* pVTable, const fastiva_ClassContext* pContext) {
	return g_fastivaRuntime.linkClass_$$(pVTable, pContext);
}

inline void* fm::initClass_$$(java_lang_Class_p pClass) {
	return g_fastivaRuntime.initClass_$$(pClass);
}

#endif // __FASTIVA_RUNTIME__INL__
