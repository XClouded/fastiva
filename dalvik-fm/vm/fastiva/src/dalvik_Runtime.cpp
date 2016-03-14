#include <Dalvik.h>
#include "JniInternal.h"
#include <dalvik_Kernel.h>
#include <kernel/Module.h>
#include <string.h>
#include <math.h>                   // needed for fmod, fmodf

#include <java/lang/ClassLoader$SystemClassLoader.h>
#include <java/lang/AbstractMethodError.inl>
#include <java/lang/ref/FinalizerReference.inl>
#include <libcore/io/StructPollfd.inl>
#include <dalvik/annotation/AnnotationDefault.h>
#include <java/lang/ClassLoader.inl>
#include <java/lang/BootClassLoader.inl>
#include <java/lang/String.inl>
#include <java/lang/StringIndexOutOfBoundsException.inl>
#include <java/lang/reflect/Proxy.inl>
#include <java/lang/reflect/InvocationHandler.inl>
#include <dalvik/system/DexPathList.inl>
#include <dalvik/system/DexPathList$Element.inl>
#include <dalvik/annotation/Signature.inl>
#include <libcore/icu/LocaleData.h>
#include "alloc/HeapInternal.h"
#include "java/lang/ClassLoader$SystemClassLoader.inl"
#include "java/lang/annotation/Retention.inl"

extern bool fastiva_dvmCanAllocClass(ClassObject* clazz);

#ifdef _DEBUG
#define SHOW_JNI_TRACE 0
extern int g_showJniTrace;
static int native_get_int_call = 0;
static int native_get_int_call_break = 118;
#endif

extern void* fastiva_dvmAddLocalReference(Thread* self, Object* obj);
ClassObject* d2f_getRawStaticOfClass();


void fastiva_Instance::operator delete(void *) {
	FASTIVA_DBREAK() // CAN NOT BE HERE;
}

bool fm::isLinked$(
	fastiva_MetaClass* pClass
) {
	// No throws.
    return dvmIsClassInitialized((ClassObject*)(void*)pClass);
}


void fastiva_Runtime::linkClass_$$(
	void* pVTable, 
	fastiva_Class_p pClass
) {
	const InterfaceEntry* pImpl = pClass->iftable;
	for (fastiva_Class_p pIFC ; (pIFC = (fastiva_Class_p)pImpl->clazz) != ADDR_ZERO; pImpl++) {
		if (pIFC->status != CLASS_INITIALIZED) {
			initClass_$$(pIFC);
		}
	}

	if (!dvmIsInterfaceClass(pClass)) {
		fastiva_Class_p superC = (fastiva_Class_p)pClass->super;
		
		if (superC != NULL) {
			if (superC->status != CLASS_INITIALIZED) {
				initClass_$$(superC);
			}
			//const int cntII = sizeof(fastiva_Instance_I$) / sizeof(void*);
#ifdef _DEBUG
			void** pSuperVTable = (void**)superC->obj.vtable$;
			void** pSelfVTable = (void**)pVTable;
			for (int i = superC->vtableCount; --i >= 0; ) {
				assert(*pSelfVTable != 0);
				if (0) {
					*pSelfVTable = *pSuperVTable;
				}
				pSelfVTable++;
				pSuperVTable++;
			}
#endif
		}
	}
}


void fastiva_Runtime::initClass_$$(
	fastiva_Class_p pClass
) {
	assert(pClass->status != CLASS_INITIALIZED);
	if (!dvmInitClass(pClass)) {
		fastiva_popDalvikException();
	}
}


java_lang_String_p fastiva_Runtime::getImmortalString(
	fastiva_Module* pModule,
	int stringId
) {
	java_lang_String_p pStr = pModule->m_stringPool[stringId];
	if (pStr == NULL) {
		pStr = fm::createStringConstant((const unicod*)pModule->m_aConstString[stringId]);
		pModule->m_stringPool[stringId] = pStr;
	}
	return pStr;
}


//======================================================//
// Type Checking
//======================================================//

fastiva_Class_p fastiva_Runtime::getArrayClass(
	const fastiva_ClassContext* pContext,
	int dimension
) {
	fastiva_Class_p pClass = fm::getRawClass(pContext);
// #todo remove check code below!!
	//if (gDvm.classJavaLangObject == NULL) {
	//	return NULL;
	//}
	while (--dimension >= 0) {
		pClass = (fastiva_Class_p)dvmFindArrayClassForElement(pClass);
	}
	return pClass;
}

void fastiva_Runtime::checkInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
    if (ptr != NULL && !isInstanceOf(ptr, pContext)) {
		throwClassCastException(ptr, fm::getRawClass(pContext));
	}
}

void fastiva_Runtime::checkInstanceOf_dbg(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
    if (ptr != NULL && !isInstanceOf(ptr, pContext)) {
		throwClassCastException(ptr, fm::getRawClass(pContext));
	}
}

void fastiva_Runtime::checkImplemented(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
    if (ptr != NULL && !isInstanceOf(ptr, pContext)) {
		throwClassCastException(ptr, fm::getRawClass(pContext));
	}
}

void* fastiva_Runtime::isInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
	assert(ptr == NULL || ptr->clazz != NULL);
	if (ptr != NULL && dvmInstanceof(ptr->clazz, fm::getRawClass(pContext))) {
		return ptr;
	}
	return NULL;
}

const void** fastiva_Runtime::isImplemented(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext
) {
	return (const void**)isInstanceOf(ptr, pContext);
}

void fastiva_Runtime::checkArrayInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext, 
	int dimension
) {
	fastiva_Class_p arrayClass = getArrayClass(pContext, dimension);
    if (ptr != NULL && !dvmInstanceof((ClassObject*)ptr->clazz, arrayClass)) {
		throwClassCastException(ptr, arrayClass);
	}
}

void* fastiva_Runtime::isArrayInstanceOf(
	fastiva_Instance_p ptr, 
	const fastiva_ClassContext* pContext, 
	int dimension
) {
	fastiva_Class_p arrayClass = getArrayClass(pContext, dimension);
    if (dvmInstanceof(ptr->clazz, arrayClass)) {
		return ptr;
	}
	return NULL;
}

//======================================================//
// Allocation
//======================================================//


FOX_RESTRICT_API java_lang_Object_p fastiva_Runtime::allocate(
	const fastiva_ClassContext* pContext
) {
	dvmCheckSuspendPending(dvmThreadSelf());
	ClassObject* clazz = fm::getRawClass(pContext);
	Object* newObj;
    if (!fastiva_dvmCanAllocClass(clazz) || (!dvmIsClassInitialized(clazz) && !dvmInitClass(clazz))) {
		fastiva_popDalvikException();
    }
	else {
		newObj = dvmAllocObject(clazz, ALLOC_DONT_TRACK);
	}

	if (IS_CLASS_FLAG_SET(clazz, CLASS_ISFINALIZABLE)) {
        java_lang_ref_FinalizerReference_C$::getRawStatic$()->add_(newObj);
    }

	return (java_lang_Object_p)newObj;
}


void fm::initObject(
	fastiva_Class_p pClass,
	fastiva_Instance_p pRookie
) {
	// !! It must not throw any exceptions.
	int* pVTable = (int*)((fastiva_Class_p)pClass)->obj.vtable$;
	*(int**)pRookie = pVTable;
	*(const void**)&pRookie->m_pClass$ = (const void*)pClass;
#ifdef FASTIVA_REF
    if (IS_CLASS_FLAG_SET(pRookie->clazz, CLASS_ISREFERENCE)) {
		dvmLockHeap();
		fastiva_addReference((Object*)pRookie);
		dvmUnlockHeap();
	}
#endif
}



FOX_RESTRICT_API fastiva_ArrayHeader* fastiva_Runtime::allocatePrimitiveArray(
	fastiva_PrimitiveClass_p pClass,
	int  length 
) {
    ArrayObject* newObj = dvmAllocArrayByClass(pClass, length, ALLOC_DEFAULT);
	if (newObj == NULL) {
		fastiva_popDalvikException();
	}
	dvmReleaseTrackedAlloc((Object*) newObj, NULL);
	return (fastiva_ArrayHeader*)newObj;
}

/*
FOX_RESTRICT_API fastiva_ArrayHeader* fastiva_Runtime::allocateInitializedArray(
	fastiva_PrimitiveClass_p,
	const void* data,
	int length
) {
	FASTIVA_NOT_IMPL();
}
*/

FOX_RESTRICT_API fastiva_ArrayHeader* fastiva_Runtime::allocateMultiArray(
	fastiva_Class_p pArrayClass,
	const int* aLength 
) {
	int cnt = 0;
	int max_cnt = ((ClassObject*)pArrayClass)->arrayDim;
	while (aLength[cnt] > 0 && cnt < max_cnt) {
		cnt ++;
	}
	cnt --;
	ArrayObject* newObj = dvmAllocMultiArray(pArrayClass, cnt, aLength);
	if (newObj == NULL) {
		fastiva_popDalvikException();
	}
	dvmReleaseTrackedAlloc(newObj, dvmThreadSelf());
	return (fastiva_ArrayHeader*)newObj;
}


FOX_RESTRICT_API fastiva_ArrayHeader* fastiva_Runtime::allocatePointerArray(
	const fastiva_ClassContext* pContext,
	int  length 
) {
    ClassObject* arrayClass = dvmFindArrayClassForElement(fm::getRawClass(pContext));
    ArrayObject* newObj = dvmAllocArrayByClass(arrayClass, length, ALLOC_DEFAULT);
	if (newObj == NULL) {
		fastiva_popDalvikException();
	}
	dvmReleaseTrackedAlloc(newObj, dvmThreadSelf());
	return (fastiva_ArrayHeader*)newObj;
}

FOX_RESTRICT_API fastiva_ArrayHeader* fastiva_Runtime::allocateMultiArrayEx(
	const fastiva_ClassContext* pContext,
	int dimension,
	const int* aLength 
) {
	fastiva_Class_p arrayClass = getArrayClass(pContext, dimension);
	return allocateMultiArray(arrayClass, aLength);
}

//======================================================//
// Reference manipulation
//======================================================//
/*
void fastiva_Runtime::setJavaInstanceField(
	fastiva_Instance_p pObj, 
	fastiva_BytecodeProxy_p pValue, 
	void* pFieldPos
) {
	FASTIVA_NOT_IMPL();
}
*/

void fastiva_Runtime::setInstanceField(
	fastiva_Instance_p pObj, 
	fastiva_Instance_p pValue, 
	void* pFieldPos
) {
	assert(pObj == NULL || dvmIsValidObject((Object*)pObj));
	assert(pValue == NULL || dvmIsValidObject((Object*)pValue));
	dvmSetFieldObject((Object*)pObj, (char*)pFieldPos - (char*)pObj, (Object*)pValue);
}

void fastiva_Runtime::setInstanceVolatileField(
	fastiva_Instance_p pObj,
	fastiva_Instance_p pValue, 
	fastiva_Instance_p* pFieldPos
) {
	assert(dvmIsValidObject((Object*)pObj));
	assert(pValue == NULL || dvmIsValidObject((Object*)pValue));
	if (dvmIsClassObject((Object*)pObj)) {
		dvmSetStaticFieldObjectVolatile((StaticField*)((Field*)pFieldPos - 1), (Object*)pValue);
	}
	else {
		dvmSetFieldObjectVolatile((Object*)pObj, (char*)pFieldPos - (char*)pObj, (Object*)pValue);
	}
}

int fastiva_Runtime::getVolatileFieldInt(
	int* pFieldPos
) {
	// Cf. dvmGetFieldIntVolatile(NULL, (int)pFieldPos-(int)pObj);
    return android_atomic_acquire_load(pFieldPos);
}

jlonglong fastiva_Runtime::getVolatileFieldLong(
	jlonglong* pFieldPos
) {
	// Cf. dvmGetFieldLongVolatile(NULL, (int)pFieldPos-(int)pObj);
    s8 val = dvmQuasiAtomicRead64(pFieldPos);
    ANDROID_MEMBAR_FULL();
    return val;
}

void fastiva_Runtime::setVolatileFieldInt(
	int value,
	int* pFieldPos
) {
	// Cf. dvmSetFieldIntVolatile(NULL, (int)pFieldPos-(int)pObj, value);
    ANDROID_MEMBAR_STORE();
    *pFieldPos = value;
    ANDROID_MEMBAR_FULL();
}

void fastiva_Runtime::setVolatileFieldLong(
	jlonglong value,
	jlonglong* pFieldPos
) {
	//fastiva_debug_break("setVolatileFieldLong", true);
	// Cf. dvmSetFieldLongVolatile(NULL, (int)pFieldPos-(int)pObj, value);
    dvmQuasiAtomicSwap64Sync(value, pFieldPos);
}



struct malloc_chunk {
  size_t               prev_foot;  /* Size of previous chunk (if free).  */
  size_t               head;       /* Size and inuse bits. */
  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;
};

#define mem2chunk(mem)      ((malloc_chunk*)((char*)(mem) - TWO_SIZE_T_SIZES))
#define SIZE_T_SIZE         (sizeof(size_t))
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)

void* fastiva_getFieldOffset(u1* ptr, int offset) {
	malloc_chunk* chunk = mem2chunk(ptr);
	/** why not use (chunk-head - 8) but ((chunk-head - 8) ??
	 - if not defined FOOTERS chunk->prev_foot is a part of prev-chunk.
	*/

	if (ptr == NULL) {
		return (void*)offset;
	}

        u4 end = (u4)(offset & ~3) + 4;
	FASTIVA_ASSERT(offset >= 4 && ((chunk->head - 4) & ~3) >= end);
	ClassObject* clazz = ((Object*)ptr)->clazz;
	FASTIVA_ASSERT((clazz == NULL && offset == 4)
		|| (clazz->objectSize == 0 && ptr == (void*)d2f_getRawStaticOfClass()) 
		|| (clazz->objectSize == 0 && clazz->descriptor[0] == '[' && end < sizeof(ArrayObject)) 
		|| end <= clazz->objectSize);
	return ((void*) (((u1*)(ptr)) + (offset)));
}

void fastiva_Runtime::ensureArray(const void* _array, int itemSize) {
	Object* obj = (Object*)_array;
	FASTIVA_ASSERT(obj->clazz->descriptor[0] == '[');
	int s;
	switch(obj->clazz->descriptor[1]) {
	case 'Z': case 'B':
		s = 1;
		break;
	case 'C': case 'S':
		s = 2;
		break;
	case 'J': case 'D':
		s = 8;
		break;
	default:
		s = 4;
		break;
	}
	FASTIVA_ASSERT(s == itemSize);
	uint end = sizeof(Object) + s * ((ArrayObject*)obj)->length;
	malloc_chunk* chunk = mem2chunk(obj);
	FASTIVA_ASSERT(end <= ((chunk->head - 4) & ~3));
}


void fastiva_Runtime::checkFieldAccess(
	fastiva_Instance* ptr, 
	void* field, 
	int size
) {
#if FASTIVA_USE_TOKENIZED_FIELD_INFO
	malloc_chunk* chunk = mem2chunk(ptr);
	int offset;

	ClassObject* clazz = ((Object*)ptr)->clazz;
	if (clazz == java_lang_Class_C$::getRawStatic$()) {
		offset = (int)field - (int)((fastiva_Class_p)ptr)->sfields;
		FASTIVA_ASSERT((u4)(offset + size) <= sizeof(StaticField) * ((ClassObject*)ptr)->sfieldCount);
	}
	else {
		offset = (int)field - (int)ptr;
		FASTIVA_ASSERT((u4)(offset + size) <= clazz->objectSize);
	}
	FASTIVA_ASSERT(offset >= 4 && ((chunk->head - 4) & ~3) >= (u4)(offset + size));
#endif
}


void fastiva_Runtime::debugBreak(
	const char* msg,
	int forceSleep
) {
	fastiva_debug_break(msg, forceSleep != 0);
}

void fastiva_Runtime::pushCallStack(
	fastiva_CallStack* cs 
) {
	ALOGE("Fastiva call: [%d:%p] %s", dvmGetSysThreadId(), cs, cs->m_strMethod);
}

void fastiva_Runtime::popCallStack(
	fastiva_CallStack* cs 
) {
	ALOGE("Fastiva done: [%d:%p] %s", dvmGetSysThreadId(), cs, cs->m_strMethod);
}

static java_lang_Object_p bootClassLoader = NULL;
static java_lang_Object_p systemClassLoader = NULL;

bool fastiva_isSystemClass(fastiva_Class_p pClass) {
	if (bootClassLoader == NULL || systemClassLoader == NULL) {
		bootClassLoader = java_lang_Object_C$::getRawStatic$()->classLoader;
		if (bootClassLoader != NULL) {
			systemClassLoader = java_lang_ClassLoader_0SystemClassLoader_C$::getRawStatic$()->get__loader();
		}
	}
	
	java_lang_Object_p loader = pClass->classLoader;
	return loader == NULL || loader == bootClassLoader || loader == systemClassLoader;
}

void* fastiva_Runtime::pushCallerClass(
	fastiva_CallerClassStack* cs,
	fastiva_Class_p pClass
) {
	Thread* self = dvmThreadSelf();
#ifndef FASTIVA_USE_CPP_EXCEPTION
	cs->pushRewinder(self);
#else 
	cs->m_pTask = self;
#endif
	cs->m_pClass2 = self->m_pCallerClass;
	cs->m_pAppClass2 = self->m_pCallerAppClass;
	self->m_pCallerClass = pClass;
	if (pClass == NULL) {
		return 0;
	}

	if (!fastiva_isSystemClass(pClass)) {
#ifdef _DEBUG
		// ALOGD("### push caller %s", pClass->descriptor);
#endif
		self->m_pCallerAppClass = pClass;
	}


	return 0;
}


void fastiva_Runtime::popCallerClass(
	fastiva_CallerClassStack* cs 
) {
	Thread* self = cs->m_pTask;
	assert(self == dvmThreadSelf());
#ifdef _DEBUG
	if (self->m_pCallerAppClass != cs->m_pAppClass2) {
		// ALOGD("### restore caller %s", cs->m_pAppClass2 == NULL ? "NULL" : cs->m_pAppClass2->descriptor);
	}
#endif

	self->m_pCallerClass = cs->m_pClass2;
	self->m_pCallerAppClass = cs->m_pAppClass2;
}

void fastiva_Runtime::initFastivaApp(
	void* dexPathList
) {
	dalvik_system_DexPathList_p cp = (dalvik_system_DexPathList_p)dexPathList;
	java_lang_String_p s = cp->findLibrary_(fm::createUTFString("fandroid"));
	if (s != NULL) {
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
		java_lang_String_p fastiva_Dalvik_java_lang_Runtime_nativeLoad(java_lang_String_p fileNameObj, java_lang_ClassLoader_p classLoader);
		s = fastiva_Dalvik_java_lang_Runtime_nativeLoad(s, cp->get__definingContext());
#else 
		java_lang_String_p fastiva_Dalvik_java_lang_Runtime_nativeLoad(java_lang_String_p fileNameObj, java_lang_ClassLoader_p classLoader, java_lang_String_p ldLibraryPathObj);
		s = fastiva_Dalvik_java_lang_Runtime_nativeLoad(s, cp->get__definingContext(), NULL);
#endif
		if (s == NULL) {
			ALOGD("##################### FANDROID LOAD SUCCESS");

			/* DexFile À» Á¦°ÅÇÑ´Ù..
               cp->set__dexElements(dalvik_system_DexPathList_0Element_A::create$(0));
             */
		}
		else {
			ALOGD("##################### FANDROID LOAD FAIL");
		}
	}
    else {
		//ALOGD("##################### NO FANDROID FOUND");
    }
}

java_lang_Class_p fastiva_Runtime::getPrimitiveClass(int primitiveType, int arrayDimension) {
	int t = (int)primitiveType;
	if (t < 0 && t >= fastiva_Primitives::cntType) {
		FASTIVA_DBREAK();
	}
	return (java_lang_Class_p)Kernel::primitiveClasses[t + fastiva_Primitives::cntType * arrayDimension];
} 



/*
jlonglong fastiva_Runtime::readInterfaceField(
	void* pField
) {
	FASTIVA_NOT_IMPL();
}
*/

void fastiva_Runtime::setStaticField(
	fastiva_MetaClass_p pClass, 
	fastiva_Instance_p pValue, 
	void* pFieldPos
) {
	*(fastiva_Instance_p*)pFieldPos = pValue;
	assert(dvmIsValidObject(pClass));
	assert(pValue == NULL || dvmIsValidObject((Object*)pValue));

#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
    if (pValue != NULL) {
		dvmWriteBarrierField(pClass, pFieldPos);
	}
#endif
}


void fastiva_Runtime::setArrayItem(
	fastiva_ArrayHeader* pArray, 
	const fastiva_ClassContext* pBaseType,
	void* pItemSlot, 
	fastiva_Instance_p pItem
) {
	// No throws
    dvmSetObjectArrayElement((ArrayObject*)pArray, (void**)pItemSlot - (void**)pArray->getBuffer_unsafe$(), (Object*)pItem);
}

void fastiva_Runtime::setAbstractArrayItem(
	fastiva_ArrayHeader* pArray, 
	const fastiva_ClassContext* pBaseType,
	void* pItemSlot, 
	fastiva_Instance_p pItem
) {
	// No Throws
    dvmSetObjectArrayElement((ArrayObject*)pArray, (void**)pItemSlot - (void**)pArray->getBuffer_unsafe$(), (Object*)pItem);
}

/*
void fastiva_Runtime::setCustomArrayItem(
	fastiva_ArrayHeader* pArray, 
	void* pItemSlot, 
	fastiva_BytecodeProxy_p pItem
) {
	FASTIVA_NOT_IMPL();
}
*/

//======================================================//
// Synchronization
//======================================================//


void fastiva_Runtime::monitorEnter(
	fastiva_Instance_p pObj
) {
	// No throws
    dvmLockObject(dvmThreadSelf(), (Object*)pObj);
}

void fastiva_Runtime::monitorExit(
	fastiva_Instance_p pObj
) {
	// No throws
    dvmUnlockObject(dvmThreadSelf(), (Object*)pObj);
}


void fastiva_Runtime::beginSynchronized(
	fastiva_Instance_p pObj,
	fastiva_Synchronize* pSync
) {
	pSync->m_pTask = (fastiva_Task*)dvmThreadSelf();
	pSync->m_pObj = pObj;
#ifndef FASTIVA_USE_CPP_EXCEPTION
	pSync->pushRewinder(pSync->m_pTask);
#endif
	// No throws
    dvmLockObject((Thread*)pSync->m_pTask, (Object*)pObj);
}

void fastiva_Runtime::endSynchronized(
	fastiva_Instance_p pObj,
	fastiva_Synchronize* pSync
) {
	// No throws
    dvmUnlockObject((Thread*)pSync->m_pTask, (Object*)pObj);
}

/*
void fastiva_Runtime::linkSynchronized(
	fastiva_SynchronizedLink*
) {
	FASTIVA_NOT_IMPL();
}

void fastiva_Runtime::unlinkSynchronized(
	fastiva_SynchronizedLink*
) {
	FASTIVA_NOT_IMPL();
}
*/

//======================================================//
// Eception Handling
//======================================================//

/*
FOX_NO_RETURN void fastiva_Runtime::throwException(
	java_lang_Throwable_p ex
) {
	FASTIVA_NOT_IMPL();
}

void fastiva_Runtime::throwArrayIndexOutOfBoundsException(
) {
	FASTIVA_NOT_IMPL();
}

FOX_NO_RETURN void fastiva_Runtime::throwClassCastException(
	fastiva_Instance_p pObj, 
	java_lang_Class_p pClass
) {
	FASTIVA_NOT_IMPL();
}

int fastiva_Runtime::pushExceptionHandler(
	fastiva_ExceptionContext* pContext
) {
	FASTIVA_NOT_IMPL();
}

void fastiva_Runtime::removeExceptionHandler(
	fastiva_ExceptionContext* pContext
) {
	FASTIVA_NOT_IMPL();
}

void fastiva_Runtime::rethrow(
	fastiva_ExceptionContext* pContext, void* pException
) {
	FASTIVA_NOT_IMPL();
}
*/

//======================================================//
// Floating Point
//======================================================//

double fastiva_Runtime::IEEEremainder(
	double a, 
	double b
) {
	return fmod(a, b);
}

float fastiva_Runtime::IEEEremainderF(
	float a, 
	float b
) {
	return fmodf(a, b);
}

//======================================================//
// String manipulation
//======================================================//
/*
java_lang_String_p fastiva_Runtime::createString(
	const char* str
) {
	FASTIVA_NOT_IMPL();
}

java_lang_String_p fastiva_Runtime::createStringA(
	const char* str, 
	int str_len
) {
	FASTIVA_NOT_IMPL();
}
*/

java_lang_String_p fm::createStringW(
	const unicod* ucs, 
	int ucs_len
) {
	java_lang_String_p pStr = (java_lang_String_p)dvmCreateStringFromUnicode((jchar*)ucs, ucs_len);
	if (pStr == NULL) {
		fastiva_popDalvikException();
	}
	dvmReleaseTrackedAlloc((Object*)pStr, dvmThreadSelf());
	return pStr;
}

java_lang_String_p fm::createUTFString(
	const char* str
) {
	java_lang_String_p pStr = (java_lang_String_p)dvmCreateStringFromCstr(str);
	if (pStr == NULL) {
		fastiva_popDalvikException();
	}
	dvmReleaseTrackedAlloc((Object*)pStr, dvmThreadSelf());
	return pStr;
}

/*
int fastiva_Runtime::getUnicodeLengthOfUTF8(
	const char* utf8,
	int str_len
) {
	FASTIVA_NOT_IMPL();
}

java_lang_String_p fastiva_Runtime::createUTFStringA(
	const char* str, 
	int str_len
) {
	FASTIVA_NOT_IMPL();
}

int fastiva_Runtime::getUTFLength(
	const unicod* ucs, 
	int ucs_len
) {
	FASTIVA_NOT_IMPL();
}
*/

// return end_of_utf_string;
/*
char* fastiva_Runtime::getUTFChars(
	char* buff,
	const unicod* ucs, 
	int ucs_len
) {
	FASTIVA_NOT_IMPL();
}
*/

java_lang_String_p fm::createAsciiString(
	const char* str
) {
	java_lang_String_p pStr = (java_lang_String_p)dvmCreateStringFromCstrAndLength(str, dvmUtf8Len(str));
	if (pStr == NULL) {
		fastiva_popDalvikException();
	}
	dvmReleaseTrackedAlloc((Object*)pStr, dvmThreadSelf());
	return pStr;
}

/*
size_t dvmUtf8Len(const char* utf8Str, int str_len)
{
	if (str_len <= 0) {
		return 0;
	}
    size_t len = 0;
	const char* end = utf8Str + str_len;
    int ic;

    while (utf8str != end) {
		ic = *utf8Str++;
        len++;
        if ((ic & 0x80) != 0) {
            // two- or three-byte encoding 
            utf8Str++;
            if ((ic & 0x20) != 0) {
                // three-byte encoding 
                utf8Str++;
            }
        }
    }

    return len;
}

java_lang_String_p fastiva_Runtime::createAsciiStringA(
	const char* str, 
	int str_len
) {
	java_lang_String_p pStr = (java_lang_String_p)dvmCreateStringFromCstrAndLength(str, dvmUtf8Len(str));
	if (pStr == NULL) {
		fastiva_popDalvikException();
	}
	dvmReleaseTrackedAlloc((Object*)pStr, dvmThreadSelf());
	return pStr;
}
*/
/*
jlonglong fastiva_Runtime::invokeJNI(
	void* pObj,
	void* pMethodName
) {
	FASTIVA_NOT_IMPL();
}


int fastiva_Runtime::dispatchNativeException(
	void*
) {
	FASTIVA_NOT_IMPL();
}
*/
fastiva_ManagedSection::fastiva_ManagedSection(
	void* unused
) {
	Kernel::beginManagedSection_$$(this);
}

void Kernel::beginManagedSection_$$(struct fastiva_ManagedSection* pFrame) {
	fastiva_Task* pCurrTask = (fastiva_Task*)dvmThreadSelf();
	FASTIVA_ASSERT("Not implemented" == 0);
	/*
	 AttachCurrentThread() & set NativeStackBottom
	*/
}

fastiva_ManagedSection::~fastiva_ManagedSection(
) {
	Kernel::endManagedSection_$$(this);
}

void Kernel::endManagedSection_$$(struct fastiva_ManagedSection* pFrame) {
	fastiva_Task* pCurrTask = (fastiva_Task*)pFrame->m_pThread;
	FASTIVA_ASSERT("Not implemented" == 0);
	/*
	 restore NativeStackBottom
	*/
}




void fastiva_Runtime::throwAbstractMethodError(
) {
	THROW_EX_NEW$(java_lang_AbstractMethodError, ());
}

void fastiva_Runtime::throwStringIndexOutOfBoundsException(
	int count,
	int index
) {
	THROW_EX_NEW$(java_lang_StringIndexOutOfBoundsException, (count, index));
}


void fastiva_Runtime::throwNoSuchMethodError(
	fastiva_Class_p pClass, 
	const char* name
) {
    std::string msg;
    msg += pClass->descriptor;
    msg += ".";
    msg += name;
    dvmThrowNoSuchMethodError(msg.c_str());
	fastiva_popDalvikException();
	// noreturn ¸¸Á·À» À§ÇØ.
	throwAbstractMethodError();
}

void fastiva_Runtime::throwNoSuchFieldError(
	fastiva_Class_p pClass, 
	const char* name
) {
    std::string msg;
    msg += pClass->descriptor;
    msg += ".";
    msg += name;
    dvmThrowNoSuchFieldError(msg.c_str());
	fastiva_popDalvikException();
	// noreturn ¸¸Á·À» À§ÇØ.
	throwAbstractMethodError();
}

void fastiva_Runtime::throwNoClassDefFoundError(
	const char* name
) {
	dvmThrowNoClassDefFoundError(name);
	fastiva_popDalvikException();
	// noreturn ¸¸Á·À» À§ÇØ.
	throwAbstractMethodError();
}

void* fastiva_Runtime::addLocalReference(
	fastiva_Task* pCurrTask,
	java_lang_Object_p obj
) {
	return fastiva_dvmAddLocalReference((Thread*)pCurrTask, (Object*)obj);
}

#if	!FASTIVA_SCAN_INTO_JNI_STACK
static void* fastiva_lockCurrentNativeStack(Thread* self, jmp_buf* buf) {
	void* old_sp = fastiva_lockStack(self, buf);
	dvmChangeStatus(self, THREAD_NATIVE);
	return old_sp;
}
#endif


void* fastiva_Runtime::beginJniCall(
	fastiva_Class_p pClass, 
	int method_index, 
	fastiva_JniCallInfo* pInfo
) {
	Thread* self = dvmThreadSelf();

	FASTIVA_ASSERT(self->status == THREAD_RUNNING);

	Method* methodToCall = ((fastiva_Class_p)pClass)->virtualMethods + method_index;
	pInfo->jniEnv = self->jniEnv;
	pInfo->thread = self;
	if (methodToCall->insns == 0) {
		dvmResolveNativeMethod(NULL, (JValue*)-1, methodToCall, NULL);
		fastiva_popDalvikException();
	};
	//pInfo->wasInFNI = ((JNIEnvExt*)self->jniEnv)->inFNI;

#if SHOW_JNI_TRACE
		if (SHOW_JNI_TRACE > 1 || g_showJniTrace) {
			ALOGE("Jni call: [%d] %s, %s sp=%p", dvmGetSysThreadId(), methodToCall->name, methodToCall->clazz->descriptor, &pClass);
		}
#endif

#if FASTIVA_SCAN_INTO_JNI_STACK
	pInfo->init_jni_stack = NULL;
#else
    StackSaveArea* newSaveArea;
    u4* newFp;
    u4* fp = self->interpSave.curFrame;
    newFp = (u4*) SAVEAREA_FROM_FP(fp) - methodToCall->registersSize;
    newSaveArea = SAVEAREA_FROM_FP(newFp);

	newSaveArea->prevFrame = fp;
//        newSaveArea->savedPc = SAVEAREA_FROM_FP(fp)->xtra.currentPc;
#if defined(WITH_JIT) && defined(MTERP_STUB)
    newSaveArea->returnAddr = 0;
#endif
    newSaveArea->method = methodToCall;

    newSaveArea->xtra.localRefCookie = self->jniLocalRefTable.segmentState.all;

    self->interpSave.curFrame = newFp;

    if (self->interpBreak.ctl.subMode != 0) {
        dvmReportPreNativeInvoke(methodToCall, self, fp);
    }
	pInfo->init_jni_stack = fastiva_lockCurrentNativeStack;
#endif
	return (void*)methodToCall->insns;

}


void* fastiva_Runtime::beginFastJniCall(
	fastiva_Class_p pClass, 
	int method_index, 
	fastiva_JniCallInfo* pInfo
) {
	Thread* self = dvmThreadSelf();

	FASTIVA_ASSERT(self->status == THREAD_RUNNING);

	Method* methodToCall = ((fastiva_Class_p)pClass)->virtualMethods + method_index;
	pInfo->jniEnv = self->jniEnv;
	pInfo->thread = self;
	if (methodToCall->insns == 0) {
		dvmResolveNativeMethod(NULL, (JValue*)-1, methodToCall, NULL);
		fastiva_popDalvikException();
	};

#if SHOW_JNI_TRACE
		if (SHOW_JNI_TRACE > 1 || g_showJniTrace) {
			ALOGE("Jni call: [%d] %s, %s", dvmGetSysThreadId(), methodToCall->name, methodToCall->clazz->descriptor);
		}
#endif

	return (void*)methodToCall->insns;

}

void fastiva_Runtime::preimportClass(
	fastiva_Class_p clazz
) {
	if (clazz->status < CLASS_INITIALIZING && kernelData.vmInitialized) {
		Thread* self = dvmThreadSelf();
		d2f_initStatic(self, clazz);
		dvmClearException(self);
	}
}

void fastiva_initPreImportedClasses() {
	kernelData.vmInitialized = true;
#ifdef FASTIVA_USE_PREIMPORT_CLASSES
	Thread* self = dvmThreadSelf();

	HashTable *table = gDvm.loadedClasses;
    dvmHashTableLock(table);
    for (int i = 0; i < table->tableSize; ++i) {
        HashEntry *entry = &table->pEntries[i];
        if (entry->data != NULL && entry->data != HASH_TOMBSTONE) {
			fastiva_Class_p pClass = (fastiva_Class_p)entry->data;
			if (pClass->preimportedClasses != NULL) {
				const fastiva_Class_p* ppClass = pClass->preimportedClasses;
				fastiva_Class_p clazz;
				while ((clazz = *ppClass) != NULL) {
					if (pClass->status < CLASS_INITIALIZING) {
						d2f_initStatic(self, clazz);
						dvmClearException(self);
					}
					ppClass ++;
				}
			}
        }
	}
    dvmHashTableUnlock(table);
#endif
}

java_lang_Object_p fastiva_Runtime::endFastJniCall(
	void* res, 
	fastiva_JniCallInfo* pInfo
) {
	Thread* self = (Thread*)pInfo->thread;
	fastiva_popDalvikException(self);
	if (res != NULL) {
		// global ï¿½Ç´ï¿½ weakRef Ã³ï¿½ï¿½.
		res = dvmDecodeIndirectRef(self, (jobject)res);
	}
	return (java_lang_Object_p)res;
}

java_lang_Object_p fastiva_Runtime::endJniCall(
	void* res, 
	fastiva_JniCallInfo* pInfo
) {
	Thread* self = (Thread*)pInfo->thread;
	//bool isFNI = ((JNIEnvExt*)pInfo->jniEnv)->inFNI;
	//((JNIEnvExt*)pInfo->jniEnv)->inFNI = pInfo->wasInFNI;


#if	!FASTIVA_SCAN_INTO_JNI_STACK
	dvmChangeStatus(self, THREAD_RUNNING);
        assert(pInfo->old_sp == NULL);
	//fastiva_releaseStack(self, pInfo->old_sp);
	if (!FASTIVA_FAST_JNI && res != NULL && self->exception == NULL) {
		res = dvmDecodeIndirectRef(self, (jobject)res);
	}
// fastiva_dvmPopFrame(self); ï¿½ï¿½ï¿½ï¿½
    StackSaveArea* currSaveArea = SAVEAREA_FROM_FP(self->interpSave.curFrame);
	self->interpSave.curFrame = currSaveArea->prevFrame;
	self->jniLocalRefTable.segmentState.all = currSaveArea->xtra.localRefCookie;
#endif
	fastiva_popDalvikException(self);
	if (FASTIVA_FAST_JNI && res != NULL) {
		// for global weakRef
		res = dvmDecodeIndirectRef(self, (jobject)res);
	}
#if SHOW_JNI_TRACE
		if (SHOW_JNI_TRACE > 1 || g_showJniTrace) {
			ALOGE("Jni done: [%d] sp=%p", dvmGetSysThreadId(), &res);
		}
#endif

	return (java_lang_Object_p)res;
}

#ifdef _DEBUG
	static int cntDebugClass = 0;
	ClassObject* debug_clazz = NULL;
	static int breakAtCount = 6;
#endif

void d2f_initObject(Object* newObj, ClassObject* clazz) {


#ifdef _DEBUG
	if (debug_clazz == NULL) {
		debug_clazz = FASTIVA_RAW_CLASS_PTR(libcore_icu_LocaleData);
	}
	if (clazz == debug_clazz) {
		newObj->flags = ++cntDebugClass;
		assert(breakAtCount != cntDebugClass);
	}
#endif

	Thread* self = dvmThreadSelf();
	int* pVTable = (int*)((fastiva_Class_p)clazz)->obj.vtable$;
	FASTIVA_ASSERT(pVTable != NULL);
	*(int**)newObj = pVTable;
#ifdef _DEBUG
	if (clazz == gDvm.classJavaLangClass) {
		assert(pVTable == (void*)java_lang_Class_G$::g_vtable$);
	}
#endif

}

// for Debug Break;
void d2f_setPendingException(Thread* self, Object* exception, bool throwNow)
{
    FASTIVA_ASSERT(exception != NULL);
    self->exception = exception;
}

void fastiva_Runtime::setCustomArrayItem(fastiva_ArrayHeader*, void*, fastiva_BytecodeProxy*) {
	FASTIVA_ASSERT("setCustomArrayItem" == 0);
	abort();
}
void fastiva_Runtime::setJavaInstanceField(fastiva_Instance_p, fastiva_BytecodeProxy*, void*) {
	FASTIVA_ASSERT("setJavaInstanceField" == 0);
	abort();
}
fastiva_ArrayHeader* fastiva_Runtime::allocateInitializedArray(fastiva_PrimitiveClass*, void const*, int) {
	FASTIVA_ASSERT("allocateInitializedArray" == 0);
	abort();
	return 0;
}


int fastiva_Runtime::loadExternalModule(fastiva_Module* mi, java_lang_ClassLoader_p classLoader) {
#ifndef _DEBUG
	#define DBG_LOG(msg, ...) // ignore
#else
	#define DBG_LOG ALOGD
#endif

#ifdef _DEBUG	
	//assert(strstr(mi->m_szFileName, "GoogleContactsSyncAdapter") == 0);
#endif
	mi->m_pClassLoader = classLoader;
	classLoader->m_pModule = mi;
	kernelData.g_appModule = mi;
	Thread* self = dvmThreadSelf();
#if !FASTIVA_SCAN_INTO_JNI_STACK
	void* old_sp = self->m_pNativeStackPointer;
    ThreadStatus oldStatus = dvmChangeStatus(self, THREAD_RUNNING);
#endif
	DBG_LOG("##################### LOAD FANDROID 1");
	//mi->prepareRawClasses();
	DBG_LOG("##################### LOAD FANDROID 2");
	mi->initStringPool();
	DBG_LOG("##################### LOAD FANDROID 3");
	//g_pLibcoreModule->linkPreloadableClasses(true);
	mi->internStringPool();
	mi->initAllClasses();
	DBG_LOG("##################### LOAD FANDROID 5");
#if !FASTIVA_SCAN_INTO_JNI_STACK
	FASTIVA_SET_NATIVE_STACK_POINTER(self, old_sp);
    dvmChangeStatus(self, oldStatus);
#endif

	return 0;
}

static int invoke_test[5] = { 0, 0, 0, 0, 0 };
static int cntCall = 0;
int g_cntDebug = 2828;
extern "C" void check_invokeMethodNoRange(Method* m) {
	int jniArgInfo = (short)m->jniArgInfo;
	FASTIVA_ASSERT(jniArgInfo == 0 || jniArgInfo == 1 || jniArgInfo == 2);

	if (invoke_test[jniArgInfo] == 0) {
		invoke_test[jniArgInfo] = true;
		if (true || jniArgInfo == 2) {
			//fastiva_debug_break(true);
		}
	}
}


#ifdef FASTIVA_USE_CPP_EXCEPTION

static int cntBridgeCall = 0;
typedef void (*FASTIVA_JNI_BRIDGE_API)(const u4* argv, JValue* pResult, const Method* method, Thread* self);

extern "C" void fastiva_BridgeFunc(const u4* argv, JValue* pResult, const Method* method, Thread* self) {
	TRY$ {
#ifdef _DEBUG
		//ALOGE("fastiva_BridgeFunc %i %s.%s", dvmGetSysThreadId(), method->clazz->descriptor, method->name);
#endif
		void* func = (void*)method->nativeFunc;
		(*(FASTIVA_JNI_BRIDGE_API)func)(argv, pResult, method, self);
		return;
	}
	CATCH_ANY$ {
		self->exception = (Object*)catched_ex$;
	}
	fastiva_dvmCatchFastivaException(self);
}

extern "C" void fastiva_BridgeFunc2(const u4* argv, JValue* pResult, const Method* method, Thread* self) {
	TRY$ {
#ifdef _DEBUG
		//ALOGE("fastiva_BridgeFunc2 %i %s.%s", dvmGetSysThreadId(), method->clazz->descriptor, method->name);
#endif
		void* func = (void*)method->nativeFunc;
		(*(FASTIVA_JNI_BRIDGE_API)func)(argv, pResult, method, self);
		return;
	}
	CATCH_ANY$ {
		self->exception = (Object*)catched_ex$;
	}
	fastiva_dvmCatchFastivaException(self);
}

void fastiva_BridgeFunc_unused(const u4* argv, JValue* pResult, const Method* method, Thread* self) {

#if 0
    ffi_cif cif;
    const int kMaxArgs = 32;//method->argc+2;    /* +1 for env, maybe +1 for clazz */

#ifdef _WIN32 
    ffi_type** types = (ffi_type**)alloca(sizeof(ffi_type*)*kMaxArgs);
    void** values = (void**)alloca(sizeof(void*)*kMaxArgs);
#else
    ffi_type* types[kMaxArgs];
    void* values[kMaxArgs];
#endif
    ffi_type* retType;
    char sigByte;
    int srcArg, dstArg;

    //types[0] = &ffi_type_pointer;
    //values[0] = &pEnv;
	ClassObject* clazz = NULL;
	if ((method->accessFlags & ACC_STATIC$) != 0) {
		clazz = method->clazz;
	}

    types[0] = &ffi_type_pointer;
    if (clazz != NULL) {
        values[0] = &clazz;
        srcArg = 0;
    } else {
        values[0] = (void*) argv++;
        srcArg = 1;
    }
    dstArg = 1; // zee =2;

    /*
     * Scan the types out of the short signature.  Use them to fill out the
     * "types" array.  Store the start address of the argument in "values".
     */
	const char* shorty = method->shorty;
    retType = getFfiType(*shorty);
    while ((sigByte = *++shorty) != '\0') {
        types[dstArg] = getFfiType(sigByte);
        values[dstArg++] = (void*) argv++;
        if (sigByte == 'D' || sigByte == 'J')
            argv++;
    }


    /*
     * Prep the CIF (Call InterFace object).
     */
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, dstArg, retType, types) != FFI_OK) {
        ALOGE("ffi_prep_cif failed");
        dvmAbort();
    }
	void* func = (void*)method->insns;
    ffi_call(&cif, FFI_FN(func), pResult, values);
#endif
}
#endif

void d2f_executeJppMethod(Object* obj, const Method* method, JValue* pResult, va_list arg_list) {
	union {
		va_list argv;
		u4* args;
	};
	argv = arg_list;
	u4 save_obj = 0;
	bool isStatic = dvmIsStaticMethod(method);
	if (!isStatic) {
		args --;
		u4 save_obj = *args;
		*(args) = (int)obj;
	}
	TRY$ {
		(*method->nativeFunc)(args, pResult, method, NULL);
	}
	CATCH_ANY$ {
		dvmThreadSelf()->exception = (Object*)catched_ex$;
	}
	if (!isStatic) {
		*args = save_obj;
	}
}

void d2f_executeMethod(Thread* self, const Method* method, bool fromJni, JValue* pResult) {
	if (d2f_isFastivaMethod(method)) {
		FASTIVA_ASSERT(self->status == THREAD_RUNNING);
		TRY$ {
			u4* args = (u4*)self->interpSave.curFrame;
			(*method->nativeFunc)(args, pResult, method, NULL);
		}
		CATCH_ANY$ {
			self->exception = (Object*)catched_ex$;
		}
	}
	else if (dvmIsNativeMethod(method)) {
		FASTIVA_ASSERT(self->status == THREAD_RUNNING);
        TRACE_METHOD_ENTER(self, method);
        /*
         * Because we leave no space for local variables, "curFrame" points
         * directly at the method arguments.
         */
        (*method->nativeFunc)((u4*)self->interpSave.curFrame, pResult,
                              method, self);
        TRACE_METHOD_EXIT(self, method);
    } else {
		FASTIVA_ASSERT(self->status == THREAD_RUNNING);
		TRY$ {
			dvmInterpret(self, method, pResult);
		}
		CATCH_ANY$ {
			self->exception = (Object*)catched_ex$;
		}
    }
}

extern "C" 
#ifdef _WIN32
jlonglong __fastcall fastiva_dvmExecuteInterp(int si, int* args) {
	Object* obj = (Object*)args[0];
#else
jlonglong fastiva_dvmExecuteInterp(Object* obj, int si, va_list args) {
#endif
	Thread* self = dvmThreadSelf();
	FASTIVA_ASSERT(si < OVERRIDING_VIRTUAL_PROXY_COUNT);
	const Method* method = obj->clazz->vtable[si];
	JValue res;

	if (dvmIsNativeMethod(method)) {
		//void dvmCallMethodV(Thread* self, const Method* method, Object* obj,
		//					bool fromJni, JValue* pResult, va_list args);
		dvmCallMethodV(self, method, obj, false, &res, (va_list)args);
		fastiva_popDalvikException();
		return res.j;
	}
    
    //ClassObject* clazz;
    u4* ins;

	fastiva_dvmPushInterpFrame(self, method);
    //clazz = method->clazz;

    /* "ins" for new frame start at frame pointer plus locals */
    ins = ((u4*)self->interpSave.curFrame) +
           (method->registersSize - method->insSize);

    /* put "this" pointer into in0 if appropriate */
    FASTIVA_ASSERT(!dvmIsStaticMethod(method));
#ifdef WITH_EXTRA_OBJECT_VALIDATION
        FASTIVA_ASSERT(obj != NULL && dvmIsHeapAddress(obj));
#endif
#ifdef _WIN32
	memcpy(ins, (void*)args, method->insSize * sizeof(u4));
#else
    *ins++ = (u4)obj;
		// dvmPlatformInvokeHints() ï¿½ï¿½ï¿½ï¿½.
	if (method->jniArgInfo == 0) {
		int arg_stack_size = method->insSize - 1;
		if (arg_stack_size > 0) {
			memcpy(ins, *(void**)(void*)&args, arg_stack_size * sizeof(u4));
		}
	}
	else {
		const char* desc = &(method->shorty[1]); // [0] is the return type.
		int verifyCount = 0;
		verifyCount++;

		while (*desc != '\0') {
			switch (*(desc++)) {
				case 'D': case 'J': {
					u8 val = va_arg(args, u8);
					memcpy(ins, &val, 8);       // EABI prevents direct store
					ins += 2;
					verifyCount += 2;
					break;
				}
				case 'L': {     /* 'shorty' descr uses L for all refs, incl array */
					void* arg = va_arg(args, void*);
					FASTIVA_ASSERT(obj == NULL || dvmIsHeapAddress(obj));
					jobject argObj = reinterpret_cast<jobject>(arg);
					*ins++ = (u4) argObj;
					verifyCount++;
					break;
				}
				default: {
					/* Z B C S I -- all passed as 32-bit integers */
					*ins++ = va_arg(args, u4);
					verifyCount++;
					break;
				}
			}
		}
#ifndef NDEBUG
		if (verifyCount != method->insSize) {
			ALOGE("Got vfycount=%d insSize=%d for %s.%s", verifyCount,
            method->insSize, method->clazz->descriptor, method->name);
			FASTIVA_ASSERT(false);
			goto bail;
		}
#endif
	}
#endif

#if SHOW_JNI_TRACE
	//FASTIVA_ASSERT(strcmp(method->name, "enable") || 
	//	strcmp(method->clazz->descriptor, "Lcom/android/location/provider/LocationProviderBase$Service;"));
	if (SHOW_JNI_TRACE > 1 || g_showJniTrace) 
	ALOGE("JVM call: [%d] %s, %s", dvmGetSysThreadId(), method->name, method->clazz->descriptor);
#endif
	dvmInterpret(self, method, &res);

#ifndef NDEBUG
bail:
#endif
    fastiva_dvmPopFrame(self);

#if !defined(_ARM_)
	// @zee 2014.0607 OverideProxy¿¡¼­ È£ÃâµÊ.
	fastiva_popDalvikException();
#endif
	return res.j;
}

fastiva_Module* fastiva_getModule(const ClassObject* clazz) {
	java_lang_ClassLoader_p pCL = (java_lang_ClassLoader_p)clazz->classLoader;
	fastiva_Module *mi;
	while (pCL != NULL) {
		mi = pCL->m_pModule;
		if (mi != NULL) {
			return mi;
		}
		pCL = pCL->get__parent();
	}
	return g_pLibcoreModule;
}

ArrayObject* d2f_getClassAnnotations(const ClassObject* clazz) {
	const fastiva_ClassContext* pContext = clazz;//->m_pContext$;
	FASTIVA_ASSERT(pContext != NULL);
	fastiva_Module *mi = fastiva_getModule(clazz);
#ifdef _DEBUG
	//if (!strcmp(clazz->descriptor, "Llibcore/java/lang/reflect/AnnotationsTest$Type;")) {
	//	int a = 3;
	//}
#endif
	void* annos = mi->getAnnotations(pContext->m_annotationsInfo);
	return (ArrayObject*)annos;
}

ArrayObject* d2f_getMethodAnnotations(const Method* method) {
	FASTIVA_ASSERT(d2f_isFastivaMethod(method));
	//int id = FASTIVA_ANNOTATION_ID(&method->prototype);
	fastiva_Module *mi = fastiva_getModule(method->clazz);
	void* annos = mi->getAnnotations(method->annotations);
	return (ArrayObject*)annos;
}

ArrayObject* emptyAnnoArray();

ArrayObject* d2f_getParameterAnnotations(const Method* method) {
	FASTIVA_ASSERT(d2f_isFastivaMethod(method));
	//int id = FASTIVA_ANNOTATION_ID(&method->prototype);
	fastiva_Module *mi = fastiva_getModule(method->clazz);
	int* paramAnnotations = (int*)method->paramAnnotations;
	int cnt = paramAnnotations[0];
	if (cnt > 0xFFFF) {
		return (ArrayObject*)cnt;
	}
	java_lang_annotation_Annotation_aap aap = java_lang_annotation_Annotation_AA::create$(cnt);
	for (int i = 0; i < cnt; i++) {
		paramAnnotations++;
		int annoId = paramAnnotations[0];
		void* annos = annoId < 0 ? emptyAnnoArray() : mi->getAnnotations(mi->m_pAnnotationsInfo + annoId);
		aap->set$(i, (java_lang_annotation_Annotation_ap)annos);
	}
	return (ArrayObject*)aap;
}

ArrayObject* d2f_getFieldAnnotations(const Field* field) {
	fastiva_Module *mi = fastiva_getModule(field->clazz);

	void* annos = mi->getAnnotations((void*)field->annotationsId);
	return (ArrayObject*)annos;
}

static java_lang_annotation_Annotation_p findAnnotation(java_lang_annotation_Annotation_ap annotations, const ClassObject* annotationClazz) {
	if (annotations == NULL) {
		return NULL;
	}
	int len = annotations->length();
	for (int i = 0; i < len; i ++) {
		java_lang_annotation_Annotation_p anno = annotations->get$(i);
		if (((fastiva_Class_p)annotationClazz)->isAssignableFrom_(anno->clazz)) {
			return anno;
		}
	}
	return NULL;
}

Object* d2f_getClassAnnotation(const ClassObject* clazz, const ClassObject* annotationClazz) {
	if (annotationClazz == java_lang_annotation_Retention_C$::getRawStatic$()) {
		return fastiva_Module::getRuntimeRetainAnnotation();
	}
	java_lang_annotation_Annotation_ap annotations = (java_lang_annotation_Annotation_ap)d2f_getClassAnnotations(clazz);
	java_lang_annotation_Annotation_p annotation = findAnnotation(annotations, annotationClazz);
	return annotation;
}

Object* d2f_getMethodAnnotation(const Method* method, const ClassObject* annotationClazz) {
	java_lang_annotation_Annotation_ap annotations = (java_lang_annotation_Annotation_ap)d2f_getMethodAnnotations(method);
	java_lang_annotation_Annotation_p annotation = findAnnotation(annotations, annotationClazz);
	return annotation;
}

Object* d2f_getFieldAnnotation(const Field* field, const ClassObject* annotationClazz) {
#ifdef _DEBUG
	//assert(strstr(field->clazz->descriptor, "HttpHeaders") == NULL ||
//0 != strcmp(annotationClazz->descriptor, "Lcom/google/api/client/util/al;"));
#endif
	java_lang_annotation_Annotation_ap annotations = (java_lang_annotation_Annotation_ap)d2f_getFieldAnnotations(field);
	java_lang_annotation_Annotation_p annotation = findAnnotation(annotations, annotationClazz);
	return annotation;
}

Object* d2f_getDefaultAnnotationValue(const Method* pMethod, const ClassObject* clazz) {
	fastiva_AnnotationItem* anno = clazz->ifc.annoDefaults$;
	if (anno == NULL) {
#ifdef _DEBUG
             ALOGD("ZZZ No Def Anno Value %s.%s", clazz->descriptor, pMethod->name);
#endif
		return NULL;
	}
	if (anno->defVal.value == 0) {
		fastiva_Module::initDefaultAnnotationValues((fastiva_Class_p)clazz);
		assert(anno->defVal.value != 0);
	}
	int cntValue = anno->i;
	for (int i = 0; i < cntValue; i ++) {
		anno ++;
		if (!strcmp(pMethod->name, (char*)anno->member.name)) {
#ifdef _DEBUG
             ALOGD("ZZZ No Def Anno Value %s.%s", clazz->descriptor, pMethod->name);
#endif
			return anno->defVal.value;
		}
		else {
		}
	}
	return NULL;
}

union EnclosingInfo {
	ClassObject* enclosingClass;
	Method* enclosingMethod;
};

ClassObject* d2f_getEnclosingClass(const ClassObject* clazz) {
	ArrayObject* annos = d2f_getClassAnnotations(clazz);
	if (annos == NULL || annos->just_for_8byte_align_padding == 0) {
		return NULL;
	}
	EnclosingInfo info;
	info.enclosingMethod = (Method*)annos->just_for_8byte_align_padding;
	if (info.enclosingClass->clazz == java_lang_Class_C$::getRawStatic$()) {
		return info.enclosingClass;
	}
	return info.enclosingMethod->clazz;
}


ClassObject* d2f_getDeclaringClass(const ClassObject* clazz) {
	ArrayObject* annos = d2f_getClassAnnotations(clazz);
	if (annos == NULL || annos->just_for_8byte_align_padding == 0) {
		return NULL;
	}
	EnclosingInfo info;
	info.enclosingMethod = (Method*)annos->just_for_8byte_align_padding;
	if (info.enclosingClass->clazz == java_lang_Class_C$::getRawStatic$()) {
		return info.enclosingClass;
	}
	return NULL;
}


Object* d2f_getEnclosingMethod(const ClassObject* clazz) {
	ArrayObject* annos = d2f_getClassAnnotations(clazz);
	if (annos == NULL || annos->just_for_8byte_align_padding == 0) {
		return NULL;
	}
	EnclosingInfo info;
	info.enclosingMethod = (Method*)annos->just_for_8byte_align_padding;
	if (info.enclosingClass->clazz != java_lang_Class_C$::getRawStatic$()) {
        return dvmCreateReflectObjForMethod(info.enclosingMethod->clazz, info.enclosingMethod);
	}
	return NULL;
}

StringObject* d2f_getInnerClassName(const ClassObject* clazz) {
	// There are five kinds of classes (or interfaces):
	// a) Top level classes
	// b) Nested classes (static member classes)
	// c) Inner classes (non-static member classes)
	// d) Local classes (named classes declared within a method)
	// e) Anonymous classes


	// JVM Spec 4.8.6: A class must have an EnclosingMethod
	// attribute if and only if it is a local class or an
	// anonymous class.
	ClassObject* out_clazz = d2f_getEnclosingClass(clazz);
	if (out_clazz == NULL) {
		return NULL;
	}
	int out_len = strlen(out_clazz->descriptor);
	FASTIVA_ASSERT(clazz->descriptor[out_len-1] == '$');
	const char* name = clazz->descriptor + out_len;
	if (name[0] >= '0' && name[0] <= '9') {
		return NULL;
	}
	// @todo support utf8 someday;
	int len = strlen(name) - 1;
	Unicod_ap buf = Unicod_A::create$(len);
	for (int i = 0; i < len; i ++) {
		buf->set$(i, name[i]);
	}
	java_lang_String_p pStr = FASTIVA_NEW(java_lang_String)(0, len, buf);
	//StringObject* str = dvmCreateStringFromCstrAndLength(name, dvmUtf8Len(name) - 1);
	return pStr;
}

ArrayObject* d2f_getDeclaredClasses(const ClassObject* clazz) {
	const fastiva_ClassContext* pContext = clazz;//->m_pContext$;
	FASTIVA_ASSERT(pContext != NULL);

	//const fastiva_JniInfo* pJNI = pContext->m_pJNI;
	//if (pJNI == ADDR_ZERO || pJNI->m_pDeclaredClasses == ADDR_ZERO) {
	//	return NULL;
	//}

	const fastiva_ClassContext** ppContext = (const fastiva_ClassContext**)&pContext->m_pEnclosing_n_DeclaredClasses[1];
	int cntClass = 0;
	while (*ppContext != ADDR_ZERO) {
		ppContext ++;
		cntClass ++;
	}

	java_lang_Class_ap aClass = java_lang_Class_A::create$(cntClass);

	ppContext = (const fastiva_ClassContext**)&pContext->m_pEnclosing_n_DeclaredClasses[1];
	cntClass = 0;
	while ((pContext = *ppContext) != ADDR_ZERO) {
		//pContext = fm::validateContext(pContext);
		aClass->set$(cntClass ++, (java_lang_Class_p)pContext);//->getClass());
		ppContext ++;
	}
	
	return (ArrayObject*)aClass;
}

ArrayObject* d2f_getSignatureValue(ArrayObject* annotations) {
	fastiva_Class_p pSigClass = dalvik_annotation_Signature_C$::getRawStatic$();
	java_lang_annotation_Annotation_p annotation = findAnnotation((java_lang_annotation_Annotation_ap)annotations, pSigClass);
	if (annotation == NULL) {
		return NULL;
	}
	// Annotation Àº proxy¿¡ ÀÇÇØ »ý¼ºµÈ´Ù. value ´Â ±× Ã¹¹øÂ° field ÀÌ´Ù?
	ArrayObject* value = *(ArrayObject**)(annotation+1);
    assert(value->clazz == gDvm.classJavaLangObjectArray);
	return value;
}

ArrayObject* d2f_getClassSignatureAnnotation(const ClassObject* clazz) {
	const fastiva_ClassContext* pContext = clazz;//->m_pContext$;
	FASTIVA_ASSERT(pContext != NULL);
#if 0 // zz
	ArrayObject* annos = d2f_getClassAnnotations(pContext);
	return d2f_getSignatureValue(annos);
#else
	if (pContext->genericSig == NULL) {
		return NULL;
	}
	java_lang_Object_ap res = java_lang_Object_A::create$(1);
	res->set$(0, dvmLookupImmortalInternedString(fm::createAsciiString(pContext->genericSig)));
	return (ArrayObject*)res;
#endif
}

ArrayObject* d2f_getMethodSignatureAnnotation(const Method* pMethod) {
	FASTIVA_ASSERT(pMethod != NULL);
#if 0 // zz
	ArrayObject* annos = d2f_getMethodAnnotations(pMethod);
	return d2f_getSignatureValue(annos);
#else
	//zzz;
	if (pMethod->genericSig == NULL) {
		return NULL;
	}
	java_lang_Object_ap res = java_lang_Object_A::create$(1);
	res->set$(0, dvmLookupImmortalInternedString(fm::createAsciiString(pMethod->genericSig)));
	return (ArrayObject*)res;
#endif
}

ArrayObject* d2f_getFieldSignatureAnnotation(const Field* field) {
	FASTIVA_ASSERT(field != NULL);
#if 0
	ArrayObject* annos = d2f_getFieldAnnotations(field);
	return d2f_getSignatureValue(annos);
#else
	if (field->genericSig == NULL) {
		return NULL;
	}
	java_lang_Object_ap res = java_lang_Object_A::create$(1);
	res->set$(0, dvmLookupImmortalInternedString(fm::createAsciiString(field->genericSig)));
	return (ArrayObject*)res;
#endif
}

void d2f_registerInternalNativeMethods(DalvikNativeClass* pClass) {
    const DalvikNativeMethod* pMeth = pClass->methodInfo;
	ClassObject* clazz = dvmFindClassNoInit(pClass->classDescriptor, NULL);
	int* vtable = (int*)clazz->obj.vtable$;
    for (; pMeth->name != NULL; pMeth++) {
		if (pMeth->fnFastiva != NULL) {
			Method* method = dvmFindDirectMethodByDescriptor(clazz, pMeth->name, pMeth->signature);
			if (method == NULL) {
				method = dvmFindVirtualMethodByDescriptor(clazz, pMeth->name, pMeth->signature);
			}
			if (method == NULL) {
				ALOGE("%s->%s not found", clazz->descriptor, pMeth->name);
			}
			assert(method != NULL);
			vtable[method->methodIndex] = (int)pMeth->fnFastiva;
		}
    }
}

int d2f_computeInterpreterArgInfo(Method* meth) {
    int stackOffset = dvmIsStaticMethod(meth) ? 0 : 1;
	int padFlags = 0;

    /* Skip past the return type */
    const char* sig = meth->shorty + 1;

#if 0
	int padMask = 1 << stackOffset;
    while (true) {
        char sigByte = *(sig++);

        if (sigByte == '\0')
            break;

        if (sigByte == 'D' || sigByte == 'J') {
            if ((stackOffset & 1) != 0) {
                padFlags |= padMask;
                stackOffset++;
                padMask <<= 1;
            }
            stackOffset += 2;
            padMask <<= 2;
        } else {
            stackOffset++;
            padMask <<= 1;
        }
    }

    int jniHints = 0;

    if (stackOffset > DALVIK_JNI_COUNT_SHIFT) {
        /* too big for "fast" version */
        jniHints = DALVIK_JNI_NO_ARG_INFO;
    } else {
        assert((padFlags & (0xffffffff << DALVIK_JNI_COUNT_SHIFT)) == 0);
        stackOffset -= 2;           // r2/r3 holds first two items
        if (stackOffset < 0)
            stackOffset = 0;
        jniHints |= ((stackOffset+1) / 2) << DALVIK_JNI_COUNT_SHIFT;
        jniHints |= padFlags;
    }

    return jniHints;
#else 
    while (true) {
        char sigByte = *(sig++);

        if (sigByte == '\0')
            break;

        if (sigByte == 'D' || sigByte == 'J') {
            if ((stackOffset & 1) != 0) {
                padFlags |= 1 << (stackOffset / 2);
                stackOffset++;
            }
            stackOffset += 2;
        } else {
            stackOffset++;
        }
    }

    if ((u4)stackOffset >= sizeof(int) * 8 - 1) {
        return -1;
    } else {
        return padFlags;
    }
#endif
}

bool d2f_inFastivaFNI(Thread* self) {
	return false;//self->jniEnv != NULL && ((JNIEnvExt*)self->jniEnv)->inFNI;
}

#if 0 
void dvmHashTableLock(HashTable* pHashTable) {
#ifdef FASTIVA_SUSPEND_BY_SIGNAL
	dvmLockMutex(&gDvm.internLock);// &pHashTable->lock);
#else
	if (dvmTryLockMutex(&gDvm.internLock) != 0) {
		Thread* self = dvmThreadSelf();
		FASTIVA_SUSPEND_STACK_unsafe(self);		
	    ThreadStatus oldStatus = dvmChangeStatus(self, THREAD_VMWAIT);
		dvmLockMutex(&gDvm.internLock);// &pHashTable->lock);
	    dvmChangeStatus(self, oldStatus);
		FASTIVA_RESUME_STACK_unsafe(self);		
	}
#endif
}

void dvmHashTableUnlock(HashTable* pHashTable) {
    dvmUnlockMutex(&gDvm.internLock);// &pHashTable->lock);
}
#endif

#ifdef FASTIVA_PRELOAD_STATIC_INSTANCE
bool d2f_isFastivaClassObject(const Object* obj) {
	if (obj >= kernelData.g_classLow && obj <= kernelData.g_classHigh) {
		return obj->clazz == gDvm.classJavaLangClass;
	}
	return false;
}

int d2f_isFastivaClass(const ClassObject* clazz) {
	int res = clazz->accessFlags & CLASS_ISFASTIVACLASS$;
	assert(!res || (clazz->pDvmDex == NULL && clazz->arrayDim == 0));
	return res;
}

int d2f_isFastivaMethod(const Method* method) {
	return method->accessFlags & ACC_FASTIVA_METHOD;
}

#endif