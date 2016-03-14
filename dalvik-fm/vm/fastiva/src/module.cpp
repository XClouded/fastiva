#include <Dalvik.h>
#include <dalvik_Kernel.h>
#include <kernel/Module.h>
#include <string.h>

#include <java/net/URISyntaxException.h>
#include <libcore/io/OsConstants.h>
#include <00_java_lang.inl>
#include <00_java_util.inl>
#include <00_java_lang_annotation.inl>
#if FASTIVA_TARGET_ANDROID_VERSION >= 40400
#include <libcore/reflect/AnnotationMember.inl>
#include <libcore/reflect/AnnotationFactory.inl>
typedef libcore_reflect_AnnotationMember_A AnnotationMember_A;
typedef libcore_reflect_AnnotationMember AnnotationMember;
typedef libcore_reflect_AnnotationMember_ap AnnotationMember_ap;
typedef libcore_reflect_AnnotationMember_p AnnotationMember_p;
typedef libcore_reflect_AnnotationFactory_C$ AnnotationFactory_C$;
#else
#include <org/apache/harmony/lang/annotation/AnnotationMember.inl>
#include <org/apache/harmony/lang/annotation/AnnotationFactory.inl>
typedef org_apache_harmony_lang_annotation_AnnotationMember_A AnnotationMember_A;
typedef org_apache_harmony_lang_annotation_AnnotationMember AnnotationMember;
typedef org_apache_harmony_lang_annotation_AnnotationMember_ap AnnotationMember_ap;
typedef org_apache_harmony_lang_annotation_AnnotationMember_p AnnotationMember_p;
typedef org_apache_harmony_lang_annotation_AnnotationFactory_C$ AnnotationFactory_C$;
#endif


/*
bool fastiva_Package::loadModule(java_lang_ClassLoader_p pLoader) {
	if (this->m_pModule == ADDR_ZERO) {
		loadModule0(pLoader);
	}
	return this->m_pModule->m_hDLL != (void*)-1;
}
*/

bool fastiva_Package::isMuttable() const {
	return this->m_pModule == ADDR_ZERO || this->m_pModule->m_hDLL == (void*)-1;
}

#if 0
const fastiva_ClassContext* fastiva_Module::findClass(
	uint hashCode, 
	const char* pClassSig
) const {
	//assert(!this->isMuttable() || fox_monitor_isLocked(&m_pModule->m_monitor));
	int insertIndex;

	class ClassFinder : JNI_HashMap::Finder {
		ClassFinder(const char* pClassSig);

	};

	int idx = m_classes.findEntry(hashCode, pClassSig);
	
	if (idx < 0) {
		return ADDR_ZERO;
	}
	return this->m_classes.m_aSlot[idx].m_pContext;

#if 0 // 2011.0610
	int high = this->m_cntSlot - 1;
	int low = 0;
	int len = JNI_HashEntry::getLength(hashCode);

	const JNI_HashEntry* aSlot = this->m_classes.m_aSlot;
	const JNI_HashEntry* pSlot;
	const fastiva_ClassContext* pContext;

	int mid = 0;
	while (low <= high) {
		mid = (low + high) / 2;
		pSlot = aSlot + mid;
		/**
		�Ʒ��� ���� �����?����ϸ�?�ȵȴ�. �ؽ��ڵ��� ������ MAX_INT (0x7FFFFFFF) �� ���� �� �ִ� ���?slot_hash ���� �ؽ��ڵ庸�� ���� ��쿡��?
		diff �� ���� ���� �� �� �ִ� !!!
		int diff = slot_hash - hashCode;
		*/
		uint slot_hash = pSlot->m_hashCode;
		if (slot_hash > hashCode) {
			high = mid - 1;
		}
		else if (slot_hash < hashCode) {
			low = mid + 1;
		}
		else {
			if (memcmp(pSlot->m_pContext->m_pBaseName, pszBaseName, len) == 0) {
				goto context_found;
			}
			const fastiva_PackageInfo::ContextInfo* pTemp = pSlot;
			int mm = mid;
			while (low < --mid && (--pSlot)->m_hashCode == hashCode) {
				if (memcmp(pSlot->m_pContext->m_pBaseName, pszBaseName, len) == 0) {
					goto context_found;
				}
			}
			pSlot = pTemp;
			mid = mm;
			while (high >= ++mid && (++pSlot)->m_hashCode == hashCode) {
				if (memcmp(pSlot->m_pContext->m_pBaseName, pszBaseName, len) == 0) {
					goto context_found;
				}
			}
			assert(low == mid);
		}
	}

	if (pInsertIndex != ADDR_ZERO) {
		*pInsertIndex = low;
	}
	return ADDR_ZERO;

context_found:
	return pSlot->m_pContext;
#endif 
}
#endif

extern java_lang_String_p STRING_POOL[];
java_lang_String_p STRING_POOL[1];

static void initInitialClass(const fastiva_ClassContext* pContext) {
#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
	fastiva_Class_p newClass = (fastiva_Class_p)dvmMalloc(pContext->m_sizStatic, ALLOC_NON_MOVING);
	pContext->m_pClass0[0] = newClass;
	dvmReleaseTrackedAlloc((Object*) newClass, NULL);
#endif
}

void fastiva_Module::prepareRawClasses() {
	initInitialClass(FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Object));
	initInitialClass(FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Class));
	initInitialClass(FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Cloneable));
	initInitialClass(FASTIVA_RAW_CLASS_CONTEXT_PTR(java_io_Serializable));
	return;

	const JNI_HashEntry* pPSlot = m_classes.m_aSlot;
	uint lastHashCode = 0;

	if ((sizeof(fastiva_Class) & 7) != 0) {
		dvmAbort();
	}

#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
	for (int i = m_cntSlot; --i >= 0; pPSlot ++) {
		// the hashCode of default package := 0
		assert(pPSlot->m_hashCode > lastHashCode || (i == m_cntSlot -1 && pPSlot->m_hashCode == 0));
		lastHashCode = pPSlot->m_hashCode;
		const JNI_HashEntry* pCSlot = pPSlot->m_pPackage->m_classes.m_aSlot;
		for (int k = pPSlot->m_pPackage->m_cntSlot; --k >= 0; pCSlot++) {
			fastiva_Class_p newClass =(fastiva_Class_p)dvmMalloc(pCSlot->m_pContext->m_sizStatic, ALLOC_NON_MOVING);
			pCSlot->m_pContext->m_pClass0[0] = newClass;
			dvmReleaseTrackedAlloc((Object*) newClass, NULL);
		}
	}
#endif
}

void fastiva_Module::initRawClasses() {
	const JNI_HashEntry* pPSlot = m_classes.m_aSlot;
	uint lastHashCode = 0;

	dvmLockMutex(&kernelData.initClassLock);
	for (int i = m_classes.m_cntSlot; --i >= 0; pPSlot ++) {
		assert(pPSlot->m_hashCode >= lastHashCode || (i == m_classes.m_cntSlot -1 && pPSlot->m_hashCode == 0));
		lastHashCode = pPSlot->m_hashCode;
		fm::initRawClass(pPSlot->m_pClass);
		((ClassObject*)pPSlot->m_pClass)->classLoader = m_pClassLoader;
	}
	dvmUnlockMutex(&kernelData.initClassLock);

}

void fastiva_Module::initAllClasses() {
	fastiva_PrimitiveClass_p* ppClass = Kernel::primitiveClasses;
#if 0
	const char** typeSigs = kernelData.g_typeSigs;

	if (true) {
		ppClass += JPP_CLASS_ID::cntPredefinedClassID;
		typeSigs += JPP_CLASS_ID::cntPredefinedClassID;
	}
	else {
		ppClass += JPP_EXTERNAL_CLASS_ID_START;
		typeSigs += JPP_EXTERNAL_CLASS_ID_START;
	}
	this->m_fnInitClassIDs((java_lang_Class_p*)ppClass, typeSigs);
#endif

	if (m_pClassLoader == NULL) {
		dvmLockMutex(&kernelData.initClassLock);
		fm::initRawClass(FASTIVA_RAW_CLASS_PTR(java_lang_Class));
		fm::initRawClass(FASTIVA_RAW_CLASS_PTR(java_lang_Object));
		fm::initRawClass(FASTIVA_RAW_CLASS_PTR(java_lang_Cloneable));
		fm::initRawClass(FASTIVA_RAW_CLASS_PTR(java_io_Serializable));
		dvmUnlockMutex(&kernelData.initClassLock);
	}
	if (true) {
		//return;
	}
	this->initRawClasses();

}

extern fastiva_Instance_p DEBUG_INSTANCE;
//static int cntCS = 0;


void fastiva_Module::initStringPool() {
#ifdef FASTIVA_PRELOAD_STRING_CONSTANTS
	&& unused &&
	unicod const* const * ppConstString = this->m_aConstString;
	java_lang_String_p* ppString = this->m_stringPool;

	while (*ppConstString != ADDR_ZERO) {
		const unicod* pCS = *ppConstString ++;
		java_lang_String_p pStr = fm::createStringConstant(pCS);
#ifdef ANDROID	
		pStr = (java_lang_String_p)dvmLookupImmortalInternedString(pStr)
#endif
		*ppString++ = pStr;
	}
	*ppString = ADDR_ZERO;
#endif
}

//*
void fastiva_Module::internStringPool() {
#ifdef FASTIVA_PRELOAD_STRING_CONSTANTS
	java_lang_String_p* ppString;
#ifndef ANDROID	
	pString = this->m_stringPool;
	const int* pVTable = ((fastiva_Class_p)java_lang_String_C$::getRawStatic$())->obj.vtable$;
	while (*ppString != ADDR_ZERO) {
		java_lang_String_p pStr = *ppString ++;
		*(int*)pStr = (int)pVTable;
	}
	ppString = this->m_stringPool;
	int cntCS = 0;
	while (*ppString != ADDR_ZERO) {
		java_lang_String_p pStr = *ppString;
		*ppString = fm::internString(pStr);
		ppString ++;
		cntCS ++;
	}
#endif
	//int a = 3;
#endif
}
//*/

void fastiva_Module::linkPreloadableClasses(bool isLibCore) {
	// registerPackage ���Ŀ� �ݵ��?ȣ��Ǿ�� �Ѵ�.
	// initClass ȣ�� ���Ŀ� link�� �����ϱ� ���Ͽ� �и��Ǿ���.

	const JNI_HashEntry* pCSlot = m_classes.m_aSlot;
	for (int i = m_classes.m_cntSlot; --i >= 0; pCSlot ++) {
		fastiva_Class_p pClass = pCSlot->m_pClass;
		if ((pClass->accessFlags & ACC_PRELOAD$) != 0) {
			if (!fm::isLinked$(pClass)) {
				pClass->initStatic$();
			}
		}
	}
}

void fastiva_Module::initExteranlModule() {
	SYNCHRONIZED$(kernelData.g_pPackageListLock)
	this->initStringPool();
	this->internStringPool();
	this->initRawClasses();
	this->linkPreloadableClasses(false);
}

struct AnnotationBuilder {

	const fastiva_AnnotationItem* m_pItem;
	fastiva_Module* m_pModule;

	AnnotationBuilder(fastiva_Module* pModule, java_lang_ClassLoader_p classLoader) {
		this->m_pModule = pModule;
		this->classLoader = classLoader;
	}

	struct MemberNV {
		int name;
		java_lang_Object_p value;

		AnnotationMember_p createAnnotationMember(fastiva_Module* pModule) {
			AnnotationMember_p member;
			java_lang_String_p str = fastiva.getImmortalString(pModule, name);
			member = FASTIVA_NEW(AnnotationMember)(str, value);
			return member;
		}
	};

	java_lang_ClassLoader_p classLoader;

	java_lang_annotation_Annotation_ap processAnnotations(int cntAnno);

	java_lang_annotation_Annotation_p processAnnotation();

	java_lang_Object_p processValue(fastiva_Class_p valueType);

	void processAnnotationMember(MemberNV* nv);

};

void fastiva_Module::initDefaultAnnotationValues(fastiva_Class_p clazz) {
	fastiva_Module* fastiva_getModule(const ClassObject* clazz);

	assert(clazz->ifc.annoDefaults$ != NULL && clazz->ifc.annoDefaults$[0].member.type == 0);
	fastiva_Module *mi = fastiva_getModule(clazz);
	AnnotationBuilder b(mi, (java_lang_ClassLoader_p)clazz->classLoader);
	fastiva_AnnotationItem* dst = clazz->ifc.annoDefaults$ + 1;

	int cntMember = dst[-1].i;
	b.m_pItem = dst;
	//AnnotationMember_ap members = AnnotationMember_A::create$(cntMember);
	for (int i= 0; i < cntMember; i ++) {
		b.processAnnotationMember((AnnotationBuilder::MemberNV*)dst);
		kernelData.g_pAnnotationsList->add_(dst->defVal.value);
		dst ++;
	}
	clazz->ifc.annoDefaults$[0].defVal.value = (java_lang_Object_p)1;
	//clazz->ifc.annoDefaults$ = members;
}

java_lang_annotation_Annotation_ap AnnotationBuilder::processAnnotations(int cntAnno) {
	java_lang_annotation_Annotation_ap annos = java_lang_annotation_Annotation_A::create$(cntAnno);
	fastiva_Class_p c =  m_pItem->member.type;
	int name_id = m_pItem->member.name;

	void* value = NULL;
	if (name_id == FASTIVA_ANNOTATION_ENCLOSING_CLASS) {
		m_pItem++;
		value = c;
	}
	else if (name_id == FASTIVA_ANNOTATION_ENCLOSING_METHOD) {
		m_pItem++;
		char* name = (char*)m_pItem->method.name;
		char* sig = (char*)m_pItem->method.sig;
		Method* meth = dvmFindVirtualMethodHierByDescriptor(c, name, sig);
		if (meth == NULL) {
			/* search private methods and constructors; non-hierarchical */
			meth = dvmFindDirectMethodByDescriptor(c, name, sig);
		}
		FASTIVA_ASSERT(meth != NULL);
		value = meth;
		m_pItem++;
	}

	for (int i = 0; i < cntAnno; i ++) {
		if (i == 1) {
			i = 1;
		}
		annos->set$(i, processAnnotation());
	}
	annos->just_for_8byte_align_padding = (int)value;
	return annos;
}

java_lang_annotation_Annotation_p AnnotationBuilder::processAnnotation() {
	fastiva_Class_p annoClass = m_pItem->member.type;
	int cntMember = m_pItem->i;
	m_pItem ++;

	AnnotationMember_ap members = AnnotationMember_A::create$(cntMember);
	for (int i= 0; i < cntMember; i ++) {
		MemberNV nv;
		processAnnotationMember(&nv);
		members->set$(i, nv.createAnnotationMember(m_pModule));
	}
	java_lang_annotation_Annotation_p anno = 
		AnnotationFactory_C$::importClass$()->createAnnotation_(annoClass, members);
	return anno;
}

void AnnotationBuilder::processAnnotationMember(MemberNV* nv) {
	int name_id = m_pItem->member.name;
	nv->name = name_id & 0x7FFFFFFF;
	fastiva_Class_p itemType = m_pItem->member.type;
	m_pItem ++;

	if (name_id > 0) {
		nv->value = processValue(itemType);
	}
	else {
		int len = m_pItem->i;
		m_pItem ++;
		java_lang_Object_ap array;
		switch ((int)itemType) {
			case fastiva_Primitives::jbool:
				array = (java_lang_Object_ap)java_lang_Boolean_A::create$(len);
				break;
			case fastiva_Primitives::jbyte:
				array = (java_lang_Object_ap)java_lang_Byte_A::create$(len);
				break;
			case fastiva_Primitives::jshort:
				array = (java_lang_Object_ap)java_lang_Short_A::create$(len);
				break;
			case fastiva_Primitives::unicod:
				array = (java_lang_Object_ap)java_lang_Character_A::create$(len);
				break;
			case fastiva_Primitives::jint:
				array = (java_lang_Object_ap)java_lang_Integer_A::create$(len);
				break;
			case fastiva_Primitives::jlonglong:
				array = (java_lang_Object_ap)java_lang_Long_A::create$(len);
				break;
			case fastiva_Primitives::jfloat:
				array = (java_lang_Object_ap)java_lang_Float_A::create$(len);
				break;
			case fastiva_Primitives::jdouble:
				array = (java_lang_Object_ap)java_lang_Double_A::create$(len);
				break;
			default:
				array = (java_lang_Object_ap)fastiva.allocatePointerArray(itemType, len);
		}

		for (int i = 0; i < len; i ++) {
			java_lang_Object_p v = processValue(itemType);
			array->set$(i, v);
		}
		nv->value = array;
	}
}

java_lang_Object_p AnnotationBuilder::processValue(fastiva_Class_p valueType) {
	const fastiva_AnnotationItem* pItem = m_pItem ++;
	java_lang_Object_p value;

	switch ((int)valueType) {
		case fastiva_Primitives::jbool:
			value = FASTIVA_NEW(java_lang_Boolean)(pItem->i != 0);
			break;
		case fastiva_Primitives::jbyte:
			value = FASTIVA_NEW(java_lang_Byte)(pItem->i);
			break;
		case fastiva_Primitives::jshort:
			value = FASTIVA_NEW(java_lang_Short)(pItem->i);
			break;
		case fastiva_Primitives::unicod:
			value = FASTIVA_NEW(java_lang_Character)(pItem->i);
			break;
		case fastiva_Primitives::jint:
			value = FASTIVA_NEW(java_lang_Integer)(pItem->i);
			break;
		case fastiva_Primitives::jlonglong:
			value = FASTIVA_NEW(java_lang_Long)(pItem->j);
			break;
		case fastiva_Primitives::jfloat:
			value = FASTIVA_NEW(java_lang_Float)(pItem->f);
			break;
		case fastiva_Primitives::jdouble:
			value = FASTIVA_NEW(java_lang_Double)(pItem->d);
			break;
		default: 
			if (dvmIsAnnotationClass(valueType)) {
				return processAnnotation();
			}
			else {
				assert(!dvmIsInterfaceClass(valueType));
				value = fastiva.getImmortalString(m_pModule, pItem->i);
				if (valueType != java_lang_String::getRawContext$()) {
					assert(java_lang_Enum_C$::importClass$()->isAssignableFrom_(valueType));
					assert((valueType->accessFlags & ACC_ENUM) != 0);
					value = java_lang_Enum_C$::importClass$()->valueOf_(valueType, (java_lang_String_p)value);
				}
			}
	}
	return value;
}

static java_lang_annotation_Annotation_p _runtimeRetainAnnotation = NULL;
java_lang_annotation_Annotation_p fastiva_Module::getRuntimeRetainAnnotation() {

	if (_runtimeRetainAnnotation == NULL) {
		AnnotationMember_ap members = AnnotationMember_A::create$(1);
		java_lang_String_p name = fm::createUTFString("value");
		//name = (java_lang_String_p)dvmLookupImmortalInternedString(name);
		java_lang_Object_p value = java_lang_annotation_RetentionPolicy_C$::importClass$()->get__RUNTIME();
		AnnotationMember_p member;
		member = FASTIVA_NEW(AnnotationMember)(name, value);
		members->set$(0, member);
		fastiva_Class_p annotationClazz = java_lang_annotation_Retention_C$::getRawStatic$();
		_runtimeRetainAnnotation = 
			AnnotationFactory_C$::importClass$()->createAnnotation_(annotationClazz, members);
		kernelData.g_pAnnotationsList->add_(_runtimeRetainAnnotation);
	}
	return _runtimeRetainAnnotation;
}

void* fastiva_Module::getAnnotations(void* annotationInfo) {
	if (annotationInfo == NULL) {
		return NULL;
	}
	int info = *(int*)annotationInfo;
	java_lang_Object_p annos = FASTIVA_GET_CACHED_ANNOTATIONS(info);
	if (annos != NULL) {
		return annos;
	}
	int offset = FASTIVA_ANNOTATIONS_DATA_OFFSET(info);
	int cntAnno = FASTIVA_ANNOTATIONS_DATA_LENGTH(info);

	AnnotationBuilder b(this, this->m_pClassLoader);
	b.m_pItem = &this->m_pAnnotationsData[offset];
	java_lang_annotation_Annotation_ap res = b.processAnnotations(cntAnno);
	*(int*)annotationInfo = (int)res;
	kernelData.g_pAnnotationsList->add_(res);
	return res;
}




java_lang_Class_p
fm::getRawClass(
	const char* sig,
	java_lang_ClassLoader_p pLoader
) {
	JNI_FindClass fc(pLoader);
	int dimension = 0;
	while (*sig == '[') {
		dimension ++;
		sig ++;
	}
	fastiva_Class_p pContext = (fastiva_Class_p)fc.findContext(sig, fc.LOAD);

	if (pContext == FASTIVA_NULL) {
		return ADDR_ZERO;
	}

	return fastiva.getArrayClass(pContext, dimension);
}



#if 0
extern const fastiva_ClassContext** contextTable;

void fastiva_libcore_linkClass_inTime(int idx) {
	const fastiva_ClassContext* pContext = contextTable[idx];
	fm::linkClass(pContext->m_pClass);
	*pContext->m_pStaticSlot = pContext->m_pClass;
}
#endif


