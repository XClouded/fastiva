#include <Dalvik.h>
#include <dalvik_Kernel.h>
//#include "ffi.h"

#include <kernel/Module.h>
#include <string.h>

#include <java/lang/ref/FinalizerReference.h>
//#include <java/util/concurrent/atomic/AtomicReferenceFieldUpdater.h>

//static fastiva_Class* g_classClass;
#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
    typedef fastiva_Class_p FASTIVA_PRIMITIVE_CLASS_SLOT;
#else 
    typedef fastiva_Class FASTIVA_PRIMITIVE_CLASS_SLOT;
#endif

static FASTIVA_PRIMITIVE_CLASS_SLOT s_primitiveClasses[fastiva_Primitives::cntType * (FASTIVA_MAX_ARRAY_DIMENSION+1)];

extern void fastiva_dvmComputeRefOffsets(ClassObject* clazz);
extern int fastiva_dvmComputeJniArgInfo(const char* shorty);
extern void d2f_initIFTable(ClassObject* clazz);
extern bool fastiva_dvmPrecacheReferenceOffsets(ClassObject* clazz);


#define kalloc(item_type, cnt)	(item_type*)dvmLinearAlloc(NULL, sizeof(item_type) * cnt);//malloc2(sizeof(item_type) * cnt)
#define kcalloc(item_type, cnt)	(item_type*)dvmLinearAlloc(NULL, sizeof(item_type) * cnt)

inline  void DBREAK() {
#ifdef _WIN32
	_asm int 3;
#else
	FASTIVA_DBREAK();
#endif
}

#define ASSERT_EQ(a, b)		if ((a) != (b)) { DBREAK(); assert((a) == (b)); }
#define ASSERT_EQ_ARRAY(a, b, cnt)		assert(!memcmp(a, b, sizeof(*a)*cnt));


#define FASTIVA_INIT_PRIMITIVE_CONTEXT(T) {								\
	FASTIVA_SIG_##T,														\
	(FASTIVA_PRIMITIVE_CLASS_SLOT*)&s_primitiveClasses[fastiva_Primitives::T],		\
	0, fastiva_Primitives::T, ADDR_ZERO, 								\
	0, 0, 0, 0, (void (*)())0, 0, -1, 0													\
}

#if 0
fastiva_ClassInfo s_aPrimitiveContext[fastiva_Primitives::cntType] = {
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jbool),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(unicod),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jfloat),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jdouble),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jbyte),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jshort),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jint),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jlonglong),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jvoid),
	FASTIVA_INIT_PRIMITIVE_CONTEXT(jvoid),
};

const fastiva_ClassContext* Kernel::primitiveContexts[] = {
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
#endif

fastiva_PrimitiveClass_p* Kernel::primitiveClasses;

#if 0
static const char* getTokenString(uint nameID) {
	int idx = nameID;
	const char* name = 0;
	/*
	if (idx >= JPP_EXTERNAL_REF_NAME_ID_START) {
		nameID -= JPP_EXTERNAL_REF_NAME_ID_START;
		name = fastiva_component_ModuleInfo.m_aRefName[nameID].m_pszToken;
	}
	else 
	*/
	if (idx >= JPP_REF_NAME_ID::cntPredefinedID) {
		nameID -= JPP_REF_NAME_ID::cntPredefinedID;
		name = fastiva_libcore_ModuleInfo.m_aRefName[nameID].m_pszToken;
	}
	else {
		switch (idx) {
		case JPP_REF_NAME_ID::init$$:
			return "<init>";
		case JPP_REF_NAME_ID::initStatic$$:
			return "<cinit>";
		case JPP_REF_NAME_ID::main_$:
			return "main";
		}
	}
	return name;
}
#endif
#if 0
static ffi_type* getFfiType(char sigType)
{
    switch (sigType) {
    case 'V': return &ffi_type_void;
    case 'Z': return &ffi_type_uint8;
    case 'B': return &ffi_type_sint8;
    case 'C': return &ffi_type_uint16;
    case 'S': return &ffi_type_sint16;
    case 'I': return &ffi_type_sint32;
    case 'F': return &ffi_type_float;
    case 'J': return &ffi_type_sint64;
    case 'D': return &ffi_type_double;
    case '[':
    case 'L': return &ffi_type_pointer;
    default:
        ALOGE("bad ffitype 0x%02x", sigType);
        dvmAbort();
        return NULL;
    }
}
#endif
#if 0
static char getShortTypeSig(int typeId) {
	switch(typeId) {
		case fastiva_Primitives::jvoid:
			return 'V';
			break;
		case fastiva_Primitives::jfloat:
			return 'F';
			break;
		case fastiva_Primitives::jdouble:
			return 'D';
			break;
		case fastiva_Primitives::jlonglong:
			return 'J';
			break;
		case fastiva_Primitives::jbool:
			return 'Z';
			break;
		case fastiva_Primitives::jbyte:
			return 'B';
			break;
		case fastiva_Primitives::unicod:
			return 'C';
			break;
		case fastiva_Primitives::jshort:
			return 'S';
			break;
		case fastiva_Primitives::jint:
			return 'I';
			break;
		default:
			return 'L';
			break;
		}
}
#endif
/*
static const char* getShortMethodSig(const fastiva_MethodInfo* mi) {

	JNI_ArgIterator iter;
	int cntArg = iter.init(mi->m_args);
	char* sig = kalloc(char, cntArg + 2);
	if (sig == 0) {
		sig = kalloc(char, cntArg + 2);
	}
	char* shorty = sig;
	*sig ++ = getShortTypeSig(mi->m_retType);
	while (cntArg -- > 0) {
		*sig ++ = getShortTypeSig(iter.nextID());
	}
	*sig = 0;
	return shorty;
}
*/

#if 0 //def _DEBUG
static void initDalvikMethodInfo(Method* method, Method* mi) {

    /* access flags; low 16 bits are defined by spec (could be u2?) */
	// method->accessFlags = mi->m_accessFlags | ACC_NATIVE$ | ACC_FASTIVA_METHOD;

    /*
     * For concrete virtual methods, this is the offset of the method
     * in "vtable".
     *
     * For abstract methods in an interface class, this is the offset
     * of the method in "iftable[n]->methodIndexArray".
     */
	//method->methodIndex = mi->m_offset / sizeof(void*);

    /*
     * Method bounds; not needed for an abstract method.
     *
     * For a native method, we compute the size of the argument list, and
     * set "insSize" and "registerSize" equal to it.
     */
    // method->registersSize = mi->m_argSize;  /* ins + locals */
	//if ((mi->m_accessFlags & ACC_STATIC$) == 0) {
	//	method->registersSize ++;
	//}
    //method->outsSize = 0;
    //method->insSize = method->registersSize;

    /* method name, e.g. "<init>" or "eatLunch" */
    //method->name = getTokenString(mi->m_name);

    /*
     * Method prototype descriptor string (return and argument types).
     *
     * TODO: This currently must specify the DexFile as well as the proto_ids
     * index, because generated Proxy classes don't have a DexFile.  We can
     * remove the DexFile* and reduce the size of this struct if we generate
     * a DEX for proxies.
     */
    //method->prototype.dexFile = NULL;
    //method->prototype.method = mi;


    /* short-form method descriptor string */
    //method->shorty = getShortMethodSig(mi);

    /*
     * The remaining items are not used for abstract or native methods.
     * (JNI is currently hijacking "insns" as a function pointer, set
     * after the first call.  For internal-native this stays null.)
     */

    /* the actual code *//* instructions, in memory-mapped .dex */
	//if (method->clazz->obj.vtable$ == NULL) {
		//interface;
	//	method->fastivaMethod = 0;
	//}
	//else {
	//	method->fastivaMethod = (const u2*)method->clazz->obj.vtable$[mi->m_offset/sizeof(int)];
	//}

    /* JNI: cached argument and return-type hints */
	int argInfo = fastiva_dvmComputeJniArgInfo(method->shorty);
#ifdef _ARM_
	ASSERT_EQ(method->jniArgInfo, argInfo);
#else
	method->jniArgInfo = argInfo;
#endif
	int flags = (short)method->accessFlags | (d2f_computeInterpreterArgInfo(method) << 29) | ACC_FASTIVA_CPP_NO_FLOAT_ARGS | ACC_FASTIVA_CPP_NO_JNI_STACK_SHIFT;
	//if (method->jniArgInfo & ((1 << DALVIK_JNI_COUNT_SHIFT)-1) != 0) {
	//	method->accessFlags &= ~ACC_FASTIVA_CPP_NO_JNI_STACK_SHIFT;
	//}
	char c;
	for (const char* args = &method->shorty[1]; (c = *args) != 0; args++) {
		if (c == 'F') {
			flags &= ~(ACC_FASTIVA_CPP_NO_FLOAT_ARGS | ACC_FASTIVA_CPP_NO_JNI_STACK_SHIFT);
			break;
		}
		else if (c == 'J' || c == 'D') {
			flags &= ~ACC_FASTIVA_CPP_NO_JNI_STACK_SHIFT;
		}
		else if (c == 'L') {
			flags &= ~ACC_FASTIVA_CPP_NO_JNI_STACK_SHIFT;
		}
	}
	ASSERT_EQ(method->accessFlags, (flags | ACC_FASTIVA_METHOD));
    /*
     * JNI: native method ptr; could be actual function or a JNI bridge.  We
     * don't currently discriminate between DalvikBridgeFunc and
     * DalvikNativeFunc; the former takes an argument superset (i.e. two
     * extra args) which will be ignored.  If necessary we can use
     * insns==NULL to detect JNI bridge vs. internal native.
     */
	if (dvmIsInterfaceClass(method->clazz)) {
		ASSERT_EQ(method->nativeFunc, (DalvikBridgeFunc)dvmAbstractMethodStub);
	}
	else {
             //if (dvmIsAbstractMethod(method)) {
		//ASSERT_EQ(method->nativeFunc, (DalvikBridgeFunc)dvmAbstractMethodStub);
		//ASSERT_EQ(method->nativeFunc, fastiva_BridgeFunc);
	}

    /*
     * JNI: true if this static non-synchronized native method (that has no
     * reference arguments) needs a JNIEnv* and jclass/jobject. Libcore
     * uses this.
     */
    ASSERT_EQ(method->fastJni, (method->accessFlags & ACC_STATIC$) != 0);

    /*
     * JNI: true if this method has no reference arguments. This lets the JNI
     * bridge avoid scanning the shorty for direct pointers that need to be
     * converted to local references.
     *
     * TODO: replace this with a list of indexes of the reference arguments.
     */
    ASSERT_EQ(method->noRef, true); 

    /*
     * JNI: true if we should log entry and exit. This is the only way
     * developers can log the local references that are passed into their code.
     * Used for debugging JNI problems in third-party code.
     */
    ASSERT_EQ(method->shouldTrace, false);

    /*
     * Register map data, if available.  This will point into the DEX file
     * if the data was computed during pre-verification, or into the
     * linear alloc area if not.
     */
	// @zee fastivaMethod 로 사용.
    // method->registerMap = NULL; 

    /* set if method was called during method profiling */
    ASSERT_EQ(method->inProfile, false);
}
#endif

#if FASTIVA_USE_TOKENIZED_FIELD_INFO
static void initDalvikFieldInfo(InstField* field, const fastiva_FieldInfo* fi) {
    //const char*     name;
    //const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    //u4              accessFlags;

	field->accessFlags = fi->m_accessFlags;
	field->name = getTokenString(fi->m_name);
	field->signature = d2f_getTypeDescriptor(fi->m_type);
	FASTIVA_ASSERT((int)fi->m_idAnnotations == -1 || (fi->m_idAnnotations & ~0xFFFF) == 0);
	field->annotationsId = (u2)fi->m_idAnnotations;
	if ((fi->m_accessFlags & ACC_STATIC$) == 0) {
		field->byteOffset = fi->m_offset;
	}
}
#endif

#if 0
static int getOwnInterfaceCount(const fastiva_ClassContext* pContext) {
	return pContext->getOwnInterfaceCount();
}

static fastiva_Class_p* getInterfaces(const fastiva_ClassContext* pContext, int cntIFC) {
	if (cntIFC == 0) {
		return NULL;
	}
	fastiva_Class_p* aIFC = kalloc(fastiva_Class_p, cntIFC);

	if (!pContext->isInterface()) {
		const fastiva_InstanceContext* pInstContext = pContext->toInstanceContext();
		const fastiva_ImplementInfo* pImpl;
		int cnt = 0;
		for (pImpl = pInstContext->m_aImplemented; cnt < cntIFC; pImpl++) {
			aIFC[cnt ++] = pImpl->m_pInterfaceContext->getClass();
		}
	}
	else {
		const fastiva_ClassContext** ppIFC;
		const fastiva_ClassContext* pIfcContext = pContext->toInterfaceContext();
		int cnt = 0;
		for (ppIFC = pIfcContext->m_ppIFC; cnt < cntIFC; ppIFC++) {
			aIFC[cnt ++] = ppIFC[0]->getClass();
		}
	}
	return aIFC;
}
#endif

static int getImplementCount(fastiva_Class_p pClass) {
	InterfaceEntry* pImpl = pClass->iftable;
	int cnt = 0;

	if (dvmIsInterfaceClass(pClass)) {
		for (; pImpl->clazz != NULL; pImpl++, cnt++) {
		}
		return cnt;
	}

	InterfaceEntry* pSuperImpl = pClass->super->iftable;
	int cntSuperImpl = pClass->super->iftableCount;
	for (; pImpl->clazz != NULL; pImpl++, cnt++) {
		if (pImpl->methodIndexArray == NULL) {
			assert(pImpl->clazz->vtableCount >= 0);
			int i = cntSuperImpl;
			for (InterfaceEntry* baseImpl = pSuperImpl; --i >= 0; baseImpl ++) {
				if (pImpl->clazz == baseImpl->clazz) {
					pImpl->methodIndexArray = baseImpl->methodIndexArray;
					break;
				}
			}
			assert(i >= 0);
		}
		else {
			//assert(cnt < pClass->interfaceCount);
		}
	}
	//참고. maybe (cnt != pClass->interfaceCount + pClass->super->iftableCount);
	return cnt;
}

#if 0
static InterfaceEntry* getInterfaceEntries(const fastiva_ClassContext* pContext, int cntIFC) {
	if (cntIFC == 0) {
		return NULL;
	}
	InterfaceEntry* aIFC = kalloc(InterfaceEntry, cntIFC);

	if (pContext->isInterface()) {
		const fastiva_ClassContext** ppIFC;
		const fastiva_ClassContext* pIfcContext = pContext->toInterfaceContext();
		int cnt = 0;
		// @zee warning. for dalvik-compatibility it should have full hierarchical list
		//     same as instance..
		for (ppIFC = pIfcContext->m_ppIFC; cnt < cntIFC; ppIFC++) {
			aIFC[cnt++].clazz = ppIFC[0]->getClass();
		}
	}
	else {
		const fastiva_InstanceContext* pInstContext = pContext->toInstanceContext();
		const fastiva_ImplementInfo* pImpl;
		int cnt = 0;
		int idx = pContext->getOwnInterfaceCount() % cntIFC;
		for (pImpl = pInstContext->m_aImplemented; cnt < cntIFC; pImpl++, cnt++) {
			aIFC[idx].clazz = pImpl->m_pInterfaceContext->getClass();
			int* ivtable = (int*)pImpl->m_pIVTable;
			aIFC[idx].methodIndexArray = ivtable;
			idx = (idx + 1) % cntIFC; // locate ownInterface at bottom  
		}
	}
	return aIFC;
}
#endif

#define FOR_EACH(Type, pContext) {									\
	const fastiva_##Type##Info* p##Type =							\
	(const fastiva_##Type##Info*)pContext->m_pJNI->m_a##Type;	\
	int cnt##Type = pContext->m_pJNI->m_cnt##Type;					\
	for (int i = 0; i < cnt##Type; i++, p##Type++) {				\



static int getVirtualMethodCount(ClassObject* clazz) {
	int cnt = 0;
	Method* pMethod = clazz->virtualMethods;
	int cntMethod = clazz->directMethodCount + clazz->virtualMethodCount;
	for (int i = cntMethod; --i >= 0; pMethod ++) {
		if ((pMethod->accessFlags & ACC_VIRTUAL$) != 0) {
			cnt ++;
		}
	}
	return cnt;
}

#if 0
static Method* getAllMethods(const fastiva_ClassContext* pContext) {
	Method* methods = (Method*)(void*)pContext->m_pJNI->m_aMethod;
						//kalloc(Method, pContext->m_pJNI->m_cntMethod);
#ifdef _DEBUG
	Method* m = methods;
	int last_pri = 3;
	ClassObject* pClass = pContext->getClass();
	int cntMethod = pClass->directMethodCount + pClass->virtualMethodCount;
	for (int i = cntMethod; --i >= 0; m ++) {
		int pri;
		if ((m->accessFlags & (ACC_STATIC$)) != 0) {
			pri = 1;
		}
		else 
		if ((m->accessFlags & ACC_VIRTUAL$) == 0) {
			pri = 2;
		}
		else {
			pri = 3;
		}
		assert(pri <= last_pri);
		last_pri = pri;
		ASSERT_EQ(m->clazz, pClass);
		initDalvikMethodInfo(m, m);
	}
#endif
	return methods;
}

static int getVTableCount(const fastiva_ClassContext* pContext) {
	return pContext->m_cntVirtualMethod;
}
#endif

static Method** getVTable(ClassObject* pClass) {
	Method* declaredVirtualMethods = pClass->virtualMethods;
	int cnt = pClass->virtualMethodCount;

	Method** methods = kcalloc(Method*, pClass->vtableCount);
	// it can be (pClass->virtualMethodCount > pClass->vtableCount) 
	// when class contains ABSTRACT_SYTHETIC_METHOD's
	// see com/android/org/chromium/com/google/common/hash/AbstractHasher
	// --- no assert(cnt <= pClass->vtableCount);
	for (int i = 0; i < cnt; i ++) {
		Method* pMethod = declaredVirtualMethods ++;
		assert((pMethod->accessFlags & ACC_VIRTUAL$) != 0);
			//Method* m = findMethod(declaredVirtualMethods, pMethod->m_offset, cnt);
			//assert(m != NULL && pMethod->m_offset / sizeof(Method*) < cnt);
			assert(pMethod->methodIndex < pClass->vtableCount);
			methods[pMethod->methodIndex] = pMethod;
		//}
	}

	ClassObject* pSuperClass = pClass->super;
	if (dvmIsInterfaceClass(pClass) || pSuperClass == NULL) {
		return methods;
	}

	int* pVTable$ = (int*)pClass->obj.vtable$;
	const int* superVTable$ = pSuperClass->obj.vtable$;
	Method** superMethods = pSuperClass->vtable;
	assert(pSuperClass->vtableCount <= pClass->vtableCount);
	for (int i = pSuperClass->vtableCount; --i >= 0; ) {
		if (pVTable$[i] == 0) {
			pVTable$[i] = superVTable$[i]; 
		}
		if (methods[i] == NULL) {
			methods[i] = superMethods[i]; 
		}
	}

	Method* superDirectMethods = pSuperClass->directMethods;
	int superDirectCount = pSuperClass->directMethodCount;

	for (int i = pClass->vtableCount; --i >= 0/*pSuperClass->vtableCount*/; ) {
		// interface 상속을 통해 super final method 를 override 한 경우,
		// 바로 상위 super class 가 아닌 경우, 검색이 불가능하다.
		assert(methods[i] != NULL);
		if (methods[i] == NULL) {
			int func = pVTable$[i];
			int k = 0; 
			for (; k < superDirectCount; k++) {
				if ((int)superDirectMethods[k].fastivaMethod == func) {
					methods[i] = &superDirectMethods[k];
					break;
				}
			}
#ifdef _DEBUG
			if (k >= superDirectCount) {
				const char* m1;
				const char* m2;
				m1 = (i>0 && methods[i-1] != NULL) ? methods[i-1]->name : "m000";
				m2 = (i<pClass->vtableCount-1 && methods[i+1] != NULL) ? methods[i+1]->name : "m001";
				ALOGD("No super method found %s::%s,%s", pClass->descriptor, m1, m2);
			}
#endif
			assert(k < superDirectCount);
		}
	}

	return methods;
}

static int getInstanceFieldCount(const fastiva_ClassContext* pContext) {
	int cnt = 0;
	fastiva_FieldInfo* pField = (fastiva_FieldInfo*)pContext->ifields;
	for (int i = pContext->sfieldCount; --i >= 0; pField ++) {
		//const char* name = getTokenString(pField->m_name);
		if ((pField->accessFlags & ACC_STATIC$) != 0) {
			break;
		}
		cnt ++;
	}
	return cnt;
}



static int getReferenceFieldCount(const fastiva_ClassContext* pContext) {
	int cnt = 0;
	fastiva_FieldInfo* pField = (fastiva_FieldInfo*)pContext->ifields;
#ifdef _DEBUG
	bool noRefFound = false;
#endif

	for (int i = pContext->ifieldCount; --i >= 0; pField ++) {
#if FASTIVA_USE_TOKENIZED_FIELD_INFO
		if ((pField->m_accessFlags & ACC_STATIC$) != 0) {
			break;
		}
		if (pField->m_type >= fastiva_Primitives::cntType) {
			cnt ++;
		}
#else
		if ((pField->accessFlags & ACC_STATIC$) != 0) {
			break;
		}
		int ch = pField->signature[0];
		if (ch == '[' || ch == 'L') {
#ifdef _DEBUG
			assert(!noRefFound);
#endif
			cnt ++;
		}
#ifdef _DEBUG
		else {
			noRefFound = true;
		}
#endif
#endif
	}
	return cnt;
}

void initStaticFields(const fastiva_ClassContext* pContext, int cntInstanceField) {
#if FASTIVA_USE_TOKENIZED_FIELD_INFO

	ClassObject* c = pContext->getClass();
	StaticField* f = c->sfields;
	const fastiva_FieldInfo* pField = (fastiva_FieldInfo*)pContext->m_pJNI->m_aField + cntInstanceField;

	for (int i = pContext->m_pJNI->m_cntField - cntInstanceField; --i >= 0; ) {
		assert((pField->m_accessFlags & ACC_STATIC$) != 0);
		f->clazz = (fastiva_Class_p)c;
		initDalvikFieldInfo((InstField*)f, pField);
		f ++;
		pField ++;
	}
#endif
}

#if 0
static InstField* getInstanceFields(const fastiva_ClassContext* pContext) {
#if !FASTIVA_USE_TOKENIZED_FIELD_INFO
	return (InstField*)pContext->m_pJNI->m_aField;
#else
	int cnt = pContext->m_pJNI->m_cntField;
	if (cnt == 0) {
		return 0;
	}
	InstField* fields = kalloc(InstField, cnt);
	InstField* f = fields;

#ifdef _DEBUG
	int last_pri = 1000;
#endif

	FOR_EACH(Field, pContext) 
#ifdef _DEBUG
		// ensure dalivk compatibility:
                // in order of instance-ref, instance-primitive, static-ref, static-primitive 
		int pri = (((pField->m_accessFlags & ACC_STATIC$) == 0) ? 100: 50)
		        + ((pField->m_type >= fastiva_Primitives::cntType) ? 10 : 5);
		assert(pri <= last_pri);
		last_pri = pri;
		if ((pField->m_accessFlags & ACC_STATIC$) != 0) {
			break;
		}
#endif
		f->clazz = pContext->getClass();
		initDalvikFieldInfo(f, pField);
		f ++;
	}}
	return fields;
#endif
}
#endif
#ifdef _DEBUG
static int getDavlikClassFlags(const fastiva_ClassContext* pContext) {
/*
    CLASS_ISFINALIZABLE        = (1<<31), // class/ancestor overrides finalize()
    CLASS_ISARRAY              = (1<<30), // class is a "[*"
    CLASS_ISOBJECTARRAY        = (1<<29), // class is a "[L*" or "[[*"
    CLASS_ISCLASS              = (1<<28), // class is *the* class Class

    CLASS_ISREFERENCE          = (1<<27), // class is a soft/weak/phantom ref
                                          // only ISREFERENCE is set --> soft
    CLASS_ISWEAKREFERENCE      = (1<<26), // class is a weak reference
    CLASS_ISFINALIZERREFERENCE = (1<<25), // class is a finalizer reference
    CLASS_ISPHANTOMREFERENCE   = (1<<24), // class is a phantom reference

    CLASS_MULTIPLE_DEFS        = (1<<23), // DEX verifier: defs in multiple DEXs

    // unlike the others, these can be present in the optimized DEX file 
    CLASS_ISOPTIMIZED          = (1<<17), // class may contain opt instrs
    CLASS_ISPREVERIFIED        = (1<<16), // class has been pre-verified
	*/
/*
	const int REF_FLAGS = CLASS_ISREFERENCE | CLASS_ISPHANTOMREFERENCE | CLASS_ISWEAKREFERENCE;

	int flags = CLASS_ISOPTIMIZED | CLASS_ISPREVERIFIED;
	if (!dvmIsInterfaceClass(pContext) && pContext != java_lang_Object::getRawContext$()) {
		flags |= (pContext->super->accessFlags & REF_FLAGS);
	}
	if (pContext == java_lang_Class::getRawContext$()) {
		flags |= CLASS_ISCLASS;
	}
	if (pContext == java_lang_ref_FinalizerReference::getRawContext$()) {
		flags |= CLASS_ISREFERENCE | CLASS_ISFINALIZERREFERENCE;
	}

	int r_f = pContext->accessFlags & ACC_WEAK_REF$;
	switch (r_f) {
	case ACC_PHANTOM_REF$:
		flags |= CLASS_ISREFERENCE | CLASS_ISPHANTOMREFERENCE;
		break;
	case ACC_SOFT_REF$:
		flags |= CLASS_ISREFERENCE;
		break;
	case ACC_WEAK_REF$:
		flags |= CLASS_ISREFERENCE | CLASS_ISWEAKREFERENCE;
		break;
	}
*/
	u4 flags = pContext->accessFlags;
#ifdef _DEBUG
	const char* sig = pContext->descriptor;
    if (strcmp(sig, "Ljava/lang/ref/SoftReference;") == 0){
		assert((flags & CLASS_ISREFERENCE) != 0);
    } else if (strcmp(sig, "Ljava/lang/ref/WeakReference;") == 0) {
		assert((flags & CLASS_ISWEAKREFERENCE) != 0);
    } else if (strcmp(sig, "Ljava/lang/ref/FinalizerReference;") == 0) {
		assert((flags & CLASS_ISFINALIZERREFERENCE) != 0);
    }  else if (strcmp(sig, "Ljava/lang/ref/PhantomReference;") == 0) {
		assert((flags & CLASS_ISPHANTOMREFERENCE) != 0);
	}
#endif

	if ((pContext->accessFlags & ACC_FINALIZE$) != 0) {
		flags |= CLASS_ISFINALIZABLE;
	}
	return flags;
}
#endif

static const char* debug_class = "";//"Lcom/skb/btvmobiletab/popup/BTVPopup$BUTTON_TYPE;";

fastiva_Class_p 
fm::initRawClass(const ClassObject* pClass0) {//const fastiva_ClassContext*  pContext) {
        //ALOGD("loading %s", pClass0->descriptor);
	fastiva_Class_p pClass = (fastiva_Class_p)pClass0;
	const fastiva_ClassContext*  pContext = pClass;//->m_pContext$;
#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
	if (pClass == NULL) {// pClass->m_pContext$ != NULL) {
		dvmHashTableLock(gDvm.loadedClasses);
		if (pContext->getClass() != NULL) {
			if (pClass->m_pContext$ != NULL) {
				dvmHashTableUnlock(gDvm.loadedClasses);
				return pClass;
			}
		}
		else {
			pClass =(fastiva_Class_p)dvmMalloc(pContext->m_sizStatic, ALLOC_NON_MOVING);
			pContext->m_pClass0[0] = pClass;
			dvmReleaseTrackedAlloc((Object*) pClass, NULL);
		}
		dvmHashTableUnlock(gDvm.loadedClasses);
	}
	else 
#endif
	//if (pClass->m_pContext$ != NULL) {
	//	// m_pContext$ 는 init 종료 후 setting 된다.
	//	return pClass;
	//}

#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
	Thread* self = dvmThreadSelf();
    dvmLockObject(self, (Object*) pClass);
#endif
    if (pClass->status != CLASS_NOTREADY) {
#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
        dvmUnlockObject(self, (Object*) pClass);
#endif
		return (fastiva_Class_p)pClass;
	}
	pClass->status = CLASS_LOADED;

	if (kernelData.g_classLow > pClass) {
		kernelData.g_classLow = pClass;
	}
	else if (kernelData.g_classHigh < pClass) {
		kernelData.g_classHigh = pClass;
	}

	if (!dvmIsInterfaceClass(pClass)) {
		//ASSERT_EQ(pClass->ifc.itableId$, -1);
		if (pClass->super != NULL) {
			fm::initRawClass(pClass->super);
		}
		//if (pContext->toInstanceContext()->m_pSuperContext != NULL) {
		//	fm::initRawClass(pContext->toInstanceContext()->m_pSuperContext);
		//}
	}
	else {
		//ASSERT_EQ(pClass->ifc.itableId$, pClass->m_pContext$->toInterfaceContext()->ifc.itableId$);
	}



#ifdef _DEBUG
	assert(strcmp(pContext->descriptor, debug_class) != 0);
#endif

	ASSERT_EQ(pClass->clazz, (fastiva_Class_p)gDvm.classJavaLangClass);
	//ASSERT_EQ_ARRAY(pClass->descriptor, pContext->m_pJNI->m_signature, strlen(pContext->m_pJNI->m_signature) + 1);//	0x0017d0e8 "[Ljava/lang/Class;"	const char *
	//ASSERT_EQ(pClass->accessFlags, pClass->accessFlags);//pContext->m_accessFlags | getDavlikClassFlags(pContext));
#ifdef _DEBUG
	assert((int)(getDavlikClassFlags(pContext) & pClass->accessFlags) == (int)getDavlikClassFlags(pContext));
#endif
	//pClass->serialNumber; // ??	0x50000010	unsigned int
	//pClass->pDvmDex; // ??	0x00000000 {pDexFile=??? pHeader=??? pResStrings=??? ...}	DvmDex *
	pClass->status = CLASS_VERIFIED;//CLASS_INITIALIZED;
	//pClass->verifyErrorClass; // ??	0x00000000 {instanceData=0x0000000c descriptor=??? descriptorAlloc=??? ...}	ClassObject *
	//pClass->initThreadId; // ??	0x00000000	unsigned int
	if (dvmIsInterfaceClass(pContext)) {
		ASSERT_EQ(pClass->objectSize, 0);
	}
	else {
		assert(pClass->objectSize >= sizeof(Object));
		//ASSERT_EQ(pClass->objectSize, pContext->toInstanceContext()->m_sizInstance);//	0x00000000	unsigned int
	}
	//pClass->elementClass; // ??	0x017801f0 {instanceData=??? descriptor=??? descriptorAlloc=??? ...}	ClassObject *
	//pClass->arrayDim; // ??	0x00000001	int
	ASSERT_EQ(pClass->primitiveType, PRIM_NOT);
	if (dvmIsInterfaceClass(pContext)) {
		ASSERT_EQ(pClass->super, java_lang_Object_C$::getRawStatic$());
	}
	/*
	else if (pContext->toInstanceContext()->m_pSuperContext != NULL) {
		const fastiva_InstanceContext* superC = pContext->toInstanceContext()->m_pSuperContext;
		ASSERT_EQ(pClass->super, superC->getClass());
	}
	*/
	//pClass->classLoader; // ??	0x00000000	Object *
	//pClass->initiatingLoaderList.initiatingLoaders; // ??=0x00000000 
	//pClass->initiatingLoaderList.initiatingLoaderCount; //??=0x00000000;
	//ASSERT_EQ(pClass->interfaceCount, getOwnInterfaceCount(pContext));
	//ASSERT_EQ(pClass->interfaces, pClass->interfaces); // (ClassObject**)getInterfaces(pContext, pClass->interfaceCount), pClass->interfaceCount);

	//ASSERT_EQ(pClass->obj.vtable$, (int*)pContext->initVTable$(pClass));
	pClass->initVTable$(pClass);
	pClass->initVTable$ = NULL;// ==> pClass->descriptorAlloc = 0; // ?? pClass->descriptor;

	//ASSERT_EQ(pClass->virtualMethods, getAllMethods(pContext));

    /* static, private, and <init> methods */
	//ASSERT_EQ(pClass->directMethodCount, pContext->m_pJNI->m_cntMethod);
    /* virtual methods defined in this class; invoked through vtable */
	pClass->virtualMethodCount = getVirtualMethodCount(pClass);
	pClass->directMethodCount -= pClass->virtualMethodCount;
	pClass->directMethods = pClass->virtualMethods + pClass->virtualMethodCount;

    /*
     * Virtual method table (vtable), for use by "invoke-virtual".  The
     * vtable from the superclass is copied in, and virtual methods from
     * our class either replace those from the super or are appended.
     */
	//ASSERT_EQ(pClass->vtableCount, getVTableCount(pContext));
	pClass->vtable = getVTable(pClass);


    /*
     * Interface table (iftable), one entry per interface supported by
     * this class.  That means one entry for each interface we support
     * directly, indirectly via superclass, or indirectly via
     * superinterface.  This will be null if neither we nor our superclass
     * implement any interfaces.
     *
     * Why we need this: given "class Foo implements Face", declare
     * "Face faceObj = new Foo()".  Invoke faceObj.blah(), where "blah" is
     * part of the Face interface.  We can't easily use a single vtable.
     *
     * For every interface a concrete class implements, we create a list of
     * virtualMethod indices for the methods in the interface.
     */
	if (pClass->interfaceCount/*ownInterfaceCount*/ == 0) { // 
		if (pClass->super != NULL) {
			pClass->iftableCount = pClass->super->iftableCount; // all inherited interface count = (ImplementInfo count)
			assert(pClass->iftable == pClass->super->iftable || pClass->iftableCount == 0);
		}
	}
	else {
		pClass->iftableCount = getImplementCount((fastiva_Class_p)pClass); // all inherited interface count = (ImplementInfo count)
		assert(pClass->iftableCount >= pClass->interfaceCount);
	}

#if 1
	const InterfaceEntry* pImpl = pClass->iftable;
	for (int i = pClass->iftableCount; --i >= 0; pImpl++) {
		fm::initRawClass(pImpl->clazz);
	}
#else
	if (!pContext->isInterface()) {
		const fastiva_InstanceContext* pInstContext = pContext->toInstanceContext();
		const fastiva_ImplementInfo* pImpl;
		for (pImpl = pInstContext->m_aImplemented; pImpl->m_pInterfaceContext != 0; pImpl++) {
			fm::initRawClass(pImpl->m_pInterfaceContext);
		}
	}
	else {
		const fastiva_ClassContext** ppIFC;
		const fastiva_ClassContext* pIfcContext = pContext->toInterfaceContext();
		for (ppIFC = pIfcContext->m_ppIFC; *ppIFC != NULL; ppIFC++) {
			fm::initRawClass(*ppIFC);
		}
	}
#endif
	ASSERT_EQ(pClass->iftable, pClass->iftable); // getInterfaceEntries(pContext, pClass->iftableCount), pClass->iftableCount);

    /*
     * The interface vtable indices for iftable get stored here.  By placing
     * them all in a single pool for each class that implements interfaces,
     * we decrease the number of allocations.
     */
    // not-used pClass->ifviPoolCount;
    // not-used pClass->ifviPool;
	if (pContext == java_lang_ref_Reference::getRawContext$()) {
		//_asm int 3;
	}

	pClass->ifieldCount = getInstanceFieldCount(pContext);
	pClass->ifieldRefCount = getReferenceFieldCount(pContext);
	pClass->sfieldCount -= pClass->ifieldCount;
	//ASSERT_EQ(pClass->ifields, getInstanceFields(pContext));

	//set m_pContext$
	//ASSERT_EQ(pClass->m_pContext$, pContext);
	{ // initialize pClass->refOffsets
		if (pContext == java_lang_ref_Reference::getRawContext$()) {
			if (!fastiva_dvmPrecacheReferenceOffsets(pClass)) {
				ALOGE("failed pre-caching Reference offsets");
				dvmThrowInternalError(NULL);
				return NULL;
			}
		}
		fastiva_dvmComputeRefOffsets(pClass);
		d2f_initIFTable(pClass);
	}
	//pClass->sourceFile; //	0x00000000 <Bad Ptr>	const char *

#if FASTIVA_USE_TOKENIZED_FIELD_INFO
	pClass->sfields	= (StaticField*)&pClass[+1];//->ifields + pClass->ifieldCount;
#else
	pClass->sfields	= (StaticField*)pClass->ifields + pClass->ifieldCount;
#endif
	initStaticFields(pContext, pClass->ifieldCount);

	dvmSetClassSerialNumber(pClass);
	dvmAddClassToHash(pClass);

#ifdef _DEBUF
	ALOGD("fastiva loaded %s", pClass->descriptor);
#endif

#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
    dvmUnlockObject(self, (Object*) pClass);
#endif

	return (fastiva_Class_p)pClass;
}


java_lang_Class_p
fm::loadClass(
	unicod* pCode,
	int ucs_len,
    java_lang_ClassLoader_p pLoader
) {
	FASTIVA_NOT_IMPL();
	return 0;
#if 0

	unicod uch;
	const fastiva_ClassContext* pContext;
	int dimension = (*pCode == '[') ? 1 : 0;
	JNI_FindClass fc(pLoader);

	if (dimension != 0) {
		while ((uch = *(++pCode)) == '[') {
			dimension ++;
		}

		ucs_len -= dimension;
		if (uch != 'L') {
			if (ucs_len != 1) {
				return ADDR_ZERO;
			}
			java_lang_Class_p pClass = fm::getPrimitiveClassBySig(uch);
			return pClass;
		}
		pCode ++;
		ucs_len --;
	}

	int utf8_len = fastiva.getUTFLength(pCode, ucs_len);
	char* sig = (char*)alloca(utf8_len+1);
	if (utf8_len > ucs_len) {
		char* end = fastiva.getUTFChars(sig, pCode, ucs_len); 
		char* dst = sig;
		for (int i = utf8_len; i -- > 0; dst++) {
			char ch = *dst;
			switch (ch) {
				case '.':
					*dst = '/';
					break;
				case '/':
					// '/' is not allowed in forName() (cf. just for JNI)
					return ADDR_ZERO;
			}
		}
	}
	else {
		char* dst = sig;
		for (int i = utf8_len; i -- > 0; ) {
			char ch = (char)*pCode ++; 
			switch (ch) {
				case '.':
					ch = '/';
					break;
				case '/':
					// '/' is not allowed in forName() (cf. just for JNI)
					return ADDR_ZERO;
			}
			*dst++ = ch;
		}
	}
	sig[utf8_len] = 0;

	pContext = (fastiva_ClassContext*)fc.findContext(sig, fc.LOAD);

	if (pContext == FASTIVA_NULL) {
		return ADDR_ZERO;
	}

	return fastiva.getArrayClass(pContext, dimension);
#endif
}

static const char* sig_jvoid = "[[[[V";
static const char* sig_jbool = "[[[[Z";
static const char* sig_jbyte = "[[[[B";
static const char* sig_jshort = "[[[[S";
static const char* sig_unicod = "[[[[C";
static const char* sig_jint = "[[[[I";
static const char* sig_jlonglong = "[[[[J";
static const char* sig_jfloat = "[[[[F";
static const char* sig_jdouble = "[[[[D";

void fastiva_initPrimitiveClasses() {

	FASTIVA_PRIMITIVE_CLASS_OF(jvoid) = (fastiva_PrimitiveClass_p)gDvm.typeVoid;
	FASTIVA_PRIMITIVE_CLASS_OF(jcustom) = (fastiva_PrimitiveClass_p)gDvm.typeVoid;
	FASTIVA_PRIMITIVE_CLASS_OF(jbool) = (fastiva_PrimitiveClass_p)gDvm.typeBoolean;
	FASTIVA_PRIMITIVE_CLASS_OF(jbyte) = (fastiva_PrimitiveClass_p)gDvm.typeByte;
	FASTIVA_PRIMITIVE_CLASS_OF(jshort) = (fastiva_PrimitiveClass_p)gDvm.typeShort;
	FASTIVA_PRIMITIVE_CLASS_OF(unicod) = (fastiva_PrimitiveClass_p)gDvm.typeChar;
	FASTIVA_PRIMITIVE_CLASS_OF(jint) = (fastiva_PrimitiveClass_p)gDvm.typeInt;
	FASTIVA_PRIMITIVE_CLASS_OF(jlonglong) = (fastiva_PrimitiveClass_p)gDvm.typeLong;
	FASTIVA_PRIMITIVE_CLASS_OF(jfloat) = (fastiva_PrimitiveClass_p)gDvm.typeFloat;
	FASTIVA_PRIMITIVE_CLASS_OF(jdouble) = (fastiva_PrimitiveClass_p)gDvm.typeDouble;
/*
	kernelData.g_typeSigs[fastiva_Primitives::jvoid] = sig_jvoid + 4;
	kernelData.g_typeSigs[fastiva_Primitives::jbool] = sig_jbool + 4;
	kernelData.g_typeSigs[fastiva_Primitives::jbyte] = sig_jbyte + 4;
	kernelData.g_typeSigs[fastiva_Primitives::jshort] = sig_jshort + 4;
	kernelData.g_typeSigs[fastiva_Primitives::unicod] = sig_unicod + 4;
	kernelData.g_typeSigs[fastiva_Primitives::jint] = sig_jint + 4;
	kernelData.g_typeSigs[fastiva_Primitives::jlonglong] = sig_jlonglong + 4;
	kernelData.g_typeSigs[fastiva_Primitives::jfloat] = sig_jfloat + 4;
	kernelData.g_typeSigs[fastiva_Primitives::jdouble] = sig_jdouble + 4;

	for (int typeID = 0; typeID < fastiva_Primitives::cntType; typeID++) {
		for (int dimension = 1; dimension <= FASTIVA_MAX_ARRAY_DIMENSION; dimension ++) {
			kernelData.g_typeSigs[typeID + dimension * fastiva_Primitives::cntType] = kernelData.g_typeSigs[typeID] - dimension;
		}
	}
*/
}

void fastiva_initPrimitiveArrayClasses() {
	for (int typeID = 0; typeID < fastiva_Primitives::cntType; typeID++) {
		fastiva_Class_p pClass = Kernel::primitiveClasses[typeID];
		*(int*)pClass = (int)java_lang_Class_G$::g_vtable$; // <- init vtable
		pClass->ifc.itableId$ = -1;
		pClass->m_pClass$ = java_lang_Class_C$::getRawStatic$();
		if (typeID >= fastiva_Primitives::jcustom) {
			continue;
		}

		for (int dimension = 1; dimension <= FASTIVA_MAX_ARRAY_DIMENSION; dimension ++) {
			pClass = (fastiva_Class_p)dvmFindArrayClassForElement(pClass);
			*(int*)pClass = (int)java_lang_Class_G$::g_vtable$; // <- init vtable
			/* for scanInstance$() overriding 
			if (dimension == 1) {
				pClass->obj.vtable$ = &kernelData.primitiveArrayVTable;
			}
			else {
				pClass->obj.vtable$ = &kernelData.pointerArrayVTable;
			}
			*/
			Kernel::primitiveClasses[typeID + dimension * fastiva_Primitives::cntType] = (fastiva_PrimitiveClass_p)pClass;
		}
	}
}





#ifdef _WIN32
extern "C" char fastiva_virtualOverridingFuncTable[OVERRIDING_VIRTUAL_PROXY_COUNT][10];
#else
extern "C" int fastiva_virtualOverridingFuncTable[OVERRIDING_VIRTUAL_PROXY_COUNT][2];
#endif

void d2f_initFastivaVirtualSlot(Method* localMeth, int si) {
	FASTIVA_ASSERT(si < OVERRIDING_VIRTUAL_PROXY_COUNT);
	int addr = (int)fastiva_virtualOverridingFuncTable[si];
	((int*)localMeth->clazz->obj.vtable$)[si] = addr;
}


extern ClassObject* g_arrayInterfaces[2];
extern InterfaceEntry g_arrayIftable[2];

ClassObject* d2f_getRawStaticOfClass() {
	ClassObject* clazz = (ClassObject*)FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Class);//->getClass();
	return clazz;
}

ClassObject* d2f_initClassOfJavaLangClass() {
	ClassObject* clazz = d2f_getRawStaticOfClass();
	clazz->clazz = (fastiva_Class_p)clazz;
	clazz->obj.vtable$ = (int*)(void*)java_lang_Class_G$::g_vtable$;
	return clazz;
}


void d2f_initArrayClass(ClassObject* newClass, ClassObject* elementClass) {
	//const fastiva_ClassContext* m_pContext$;
	//const int* obj.vtable$;
	//const int** obj.itables$;
	//int ifc.itableId$;
	//int m_reserved;

	newClass->obj.vtable$ = (int*)(void*)java_lang_Object_G$::g_vtable$;
	*(int*)newClass = (int)java_lang_Class_G$::g_vtable$;
	// &kernelData.pointerArrayVTable; no overring clone;;
	newClass->initStatic$ = NULL;//java_lang_Object_C$::initStatic$;
	newClass->ifc.itableId$ = -1;
    newClass->interfaceCount = 2;
#ifndef FASTIVA
	newClass->interfaces = g_arrayInterfaces;
#endif
    newClass->iftableCount = 2;
    newClass->iftable = g_arrayIftable;
	/*
	newClass->iftable[0].clazz = newClass->interfaces[0];
    newClass->iftable[1].clazz = newClass->interfaces[1];
    dvmLinearReadOnly(newClass->classLoader, newClass->iftable);
	*/
}

void d2f_initStatic(Thread* self, ClassObject* clazz) {
	//char mangedSection$[sizeof(fastiva_ManagedSection)];
	//fastiva_ManagedSection* mangedSection = (fastiva_ManagedSection*)(void*)&mangedSection$[0];

	self->m_inProcessInitStatic ++;

#ifdef _DEBUG
	//ALOGD("initializing %s", clazz->descriptor);
#endif
	TRY$ {
		// push_caller must be inside of TRY$
		if (!strcmp(clazz->descriptor, "Ljava/util/concurrent/atomic/AtomicReferenceArray")) {
			fastiva_debug_break("ZZZ", true);
		}
		JPP_PUSH_CALLER_CLASS$((fastiva_Class_p)clazz);
		clazz->initStatic$();
	}
	CATCH_ANY$ {
		dvmSetException(self, (Object*)catched_ex$);
	}
	self->m_inProcessInitStatic --;

}


static volatile int g_lastITableID = 0;//JPP_IVTABLE_ID::JPP_LIBCORE_IVTABLE_ID_COUNT;

void d2f_initIFTable(ClassObject* clazz) {
	*(void**)clazz = java_lang_Class_G$::g_vtable$;

	// initClassLock must be locked already.
	assert(dvmTryLockMutex(&kernelData.initClassLock) != 0);

	if (dvmIsInterfaceClass(clazz)) {
		assert(clazz->ifc.itableId$ == 0);
		if (clazz->virtualMethodCount == 0) {
			clazz->ifc.itableId$ = -1;
		}
		else {
			clazz->ifc.itableId$ = ++g_lastITableID;
		}
		return;
	}

	if (clazz->iftable == NULL) {
		if (clazz->super != NULL) {
			clazz->obj.itables$ = clazz->super->obj.itables$;
		}
		return;
	}

	const InterfaceEntry* pImpl = clazz->iftable;
	int min = 0x7FFFFFFF, max = 0;
	int iftableCount = clazz->iftableCount;

	int cnt;
	ClassObject* pIFC;
	const InterfaceEntry* pLastImpl = 0;
	for (pImpl = clazz->iftable, cnt = iftableCount; --cnt >= 0; pImpl++) {
		pIFC = pImpl->clazz;
		int id = pIFC->ifc.itableId$;
		assert(id != 0);
		if (id < min) {
			if (id < 0) {
				continue;
			}
			min = id;
		}
		if (id > max) {
			pLastImpl = pImpl;
			max = id;
		}
	}

	if (min > max) {
		return;
	}
	if (min == max) {
		clazz->obj.itables$ = (const int**)(void*)((int*)(void*)&pLastImpl->methodIndexArray - min);
		return;
	}

	while (*kernelData.g_ivtableAllocTop != 0) {
		kernelData.g_ivtableAllocTop ++;
#ifdef _DEBUG
		if (((int)kernelData.g_ivtableAllocTop & (4*1024-1)) == 0) {
			ALOGD("kernelData.g_ivtableAllocTop = %p", kernelData.g_ivtableAllocTop);
		}
#endif
	}

	int* pStart = kernelData.g_ivtableAllocTop;
	//int* pEndOfHeap = (int*)((int)pStart - ((int)pStart % SYSTEM_PAGE_SIZE) + SYSTEM_PAGE_SIZE);

	do {
		pImpl = clazz->iftable;
		cnt = iftableCount;
		for (; --cnt >= 0; pImpl++) {
			pIFC = pImpl->clazz;
			int id = pIFC->ifc.itableId$ - min;
			assert(id != -min);
			if (id < 0) {
				assert(pIFC->ifc.itableId$ == -1);
				continue;
			}
			if (pStart[id] != 0) {
				pStart ++;
				break;
			}
		}
	} while (cnt >= 0);


	for (pImpl = clazz->iftable, cnt = 0; cnt < iftableCount; pImpl++, cnt++) {
		pIFC = pImpl->clazz;
		int id = pIFC->ifc.itableId$ - min;
		assert(id != -min);
		if (id < 0) {
			assert(pIFC->ifc.itableId$ == -1);
ifc_duplicated:
			continue;
		}
#ifdef ANDROID
		// allows dupliacated interface inheritance 
		if (pStart[id] != 0) {
			for (int i = 0; i < cnt; i ++) {
				if (clazz->iftable[i].clazz == pIFC) {
					goto ifc_duplicated;
				}
				//assert(clazz->iftable[i].clazz->ifc.itableId$ != pIFC->ifc.itableId$);
			}
		}
#endif
		if (pStart[id] != 0) {
#ifdef _DEBUG
			ALOGD("wrong iftable of %d: %s", iftableCount, clazz->descriptor);
			ALOGD("wrong iftable id on %p[%d]: %d", pStart, pStart-kernelData.g_ivtableAllocTop, pStart[id]);
			for (int i = 0; i < iftableCount; i ++) {
				ALOGD("iftable: %s = %d", clazz->iftable[i].clazz->descriptor, clazz->iftable[i].clazz->ifc.itableId$);
			}
#endif
			assert(pStart[id] == 0 || pStart[id] == (int)pImpl->methodIndexArray);
		}
		pStart[id] = (int)pImpl->methodIndexArray;
	}

	//fox_semaphore_release(kernelData.g_ivtableLock);
	clazz->obj.itables$ = (const int**)(pStart - min);
}

extern "C" Method* d2f_resolveImplementedInterfaceMethod(ClassObject* thisClass, const Method* method, int methodIdx) {
	
    Method* absMethod = dvmResolveInterfaceMethod(method->clazz, methodIdx);

	if (absMethod == NULL) {
		return NULL;
	}
    /* make sure absMethod->methodIndex means what we think it means */
    assert(dvmIsAbstractMethod(absMethod));

	int i;
	for (i = 0; i < thisClass->iftableCount; i++) {
        if (thisClass->iftable[i].clazz == absMethod->clazz)
            break;
    }
    if (i == thisClass->iftableCount) {
        /* impossible in verified DEX, need to check for it in unverified */
        dvmThrowIncompatibleClassChangeError("interface not implemented");
        return NULL;
    }

	return absMethod;
}

ClassObject* d2f_getCallerClass(Thread* self) {
	ClassObject* pCaller = self->m_pCallerClass;
	if (pCaller != NULL) {
		assert(d2f_isFastivaClass(pCaller));
	}
	else if (ACC_FASTIVA_DIRECT == ACC_FASTIVA_METHOD) {
		pCaller = SAVEAREA_FROM_FP(self->interpSave.curFrame)->method->clazz;
		if (d2f_isFastivaClass(pCaller)) {
			pCaller = dvmGetCallerClass(self->interpSave.curFrame);
		}
	}
	return pCaller;
}


int JNI_FindClass::compare(const void* data) {
	fastiva_Class_p pClass = (fastiva_Class_p)data;
	return strcmp(pClass->descriptor, m_classSig);
}

const fastiva_ClassContext* JNI_FindClass::findContext(const char* sig, Action action) {
	this->m_classSig = sig;
	const char* className = ++sig;

	char delimiter = ';';//this->m_delimiter;
	for (char ch; (ch = *sig) != delimiter; sig++) {
		if (ch == 0) {
			return ADDR_ZERO;
		}
	}

	assert("Should not here!" == 0);

	//fastiva_Module *mi = this->m_pClassLoader == NULL ? g_pLibcoreModule : kernelData.g_appModule;
	const int class_name_len = sig - className - 1;
	uint hashCode = JNI_HashEntry::getHashCode(className, class_name_len);

	const fastiva_ClassContext* pContext = NULL;
	{	
		const JNI_HashEntry* entry = g_pLibcoreModule->m_classes.findEntry(hashCode, this);
		/* @TODO add appModule argument
		if (entry == NULL && this->m_pClassLoader != NULL && kernelData.g_appModule != NULL) {
			entry = kernelData.g_appModule->m_classes.findEntry(hashCode, this);
		}
		*/
		if (action == FIND) {
			if (entry == ADDR_ZERO) {
				return ADDR_ZERO;
			}
			pContext = entry->m_pClass;
			if ((pContext->accessFlags & CLASS_ERROR) != 0) {
				return pContext;
			}
			return NULL;
		}

#if FASTIVA_SUPPORTS_BYTECODE_INTERPRETER

		JNI_ContextSlot* base = (JNI_ContextSlot*)pPackage->m_aContextInfo;
		if (pSlot == ADDR_ZERO) {
			JNI_ContextSlot* new_base;
			int cntContext = pPackage->m_cntContext;
			int sizContext = (cntContext + 0xF) & ~0xF;
			fastiva_Package* pJavaPackage = (fastiva_Package*)pPackage;
			pContext = JNI_RawContext::create(pJavaPackage, className, class_name_len);
			if (sizContext <= cntContext) {
				new_base = (JNI_ContextSlot*)fox_heap_malloc(sizeof(JNI_ContextSlot) * (sizContext + 16));
				if (base != ADDR_ZERO) {
					memcpy((void*)new_base, (void*)base, idxInsert * sizeof(JNI_ContextSlot));
					fox_heap_free(base);
				}
				pJavaPackage->m_aContextInfo = base = new_base;
			}
			else {
				new_base = base;
			}

			memmove(new_base + idxInsert + 1, base + idxInsert, (cntContext - idxInsert) * sizeof(JNI_ContextSlot));

			base[idxInsert].m_hashCode = hashCode;
			base[idxInsert].m_pContext = pContext;
			// sync를 위해 가장 뒤에
			pJavaPackage->m_cntContext ++;
		}
		else {
			pContext = pSlot->m_pContext;
		}

		if (action == JNI_FindClass::LOAD) {
			int state = pContext->markInLoading();
			
			if (state < 0) {
				KASSERT(state == fastiva_ClassContext::NOT_LOADED);
				JNI_RawContext* pRawContext = pContext->toRawContext();
				pContext = fm::loadJavaClassFile(pRawContext);
				while (base[idxInsert].m_pContext != (void*)pRawContext) {
					// laodjavaClassFile 수행중에 다른 컨텍스트가 추가될 수 있다.
					idxInsert ++;
				}
				base[idxInsert].m_pContext = pContext;
				fastiva_ClassContext ** ppContext = JNI_RawContext::getContextSlot(pContext->m_id);
				*ppContext = (fastiva_ClassContext *)pContext;
			}
			KASSERT(!pContext->isRawContext());
		}

		if (pContext->isLoadFailed()) {
			return ADDR_ZERO;
		}
#endif
		return pContext;
	}
}
