#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
#include <kernel/HeapMark.h>
#include <java/io/InputStream.inl>
// v3.2006
//#include <fastiva/BootstrapClassLoader.inl>
#include <java/lang/Thread.inl>
#include <java/lang/Runnable.inl>
#include <pal/fox_file.h>
#include <fox/Heap.h>

#include <precompiled_libcore.h>
//#include <tchar.h>
#include "string.h"
#include <fastiva_malloc.h>
#ifndef _WIN32
	#include "alloca.h"
#endif

extern fastiva_ModuleInfo fastiva_component_ModuleInfo;


#if 0 // OBOSLETE
const fastiva_ClassContext* JNI_FindClass::getClassContext(
	int classID
) {
	classID &= FASTIVA_JNI_LAST_CLASS_ID;
	if (classID <= fastiva_Primitives::jvoid) {
		return fm::getPrimitiveContext(classID);
	}

	return *JNI_RawContext::getContextSlot(classID);
}
#endif




java_lang_Class_p JNI_FindClass::loadRawClass(uint id) {
#if 1
	//KASSERT(id < JPP_EXTERANLA_CLASS_ID_START + fastiva_component_moduleInfo.m_cntIndexedClasses);
	int ccc = JPP_CLASS_ID::cntPredefinedClassID;
	if (id > ccc){
		int a = 3;
	}
	return fastiva.primitiveClasses[id];
#else // 2012.0710
	int context_id = id & FASTIVA_JNI_LAST_CLASS_ID;
	const fastiva_ClassContext* pContext;
	if (context_id <= fastiva_Primitives::jvoid) {
		if (context_id == 8){
			int a = 3;
		}
		pContext = fm::getPrimitiveContext(context_id);//g_aPrimitiveContext(id);
	}
	else if (true) {
		pContext = (const fastiva_ClassContext*)(id & FASTIVA_JNI_LAST_CLASS_ID);
	}
	else {
		int package_id = (id >> 16) - 1;
		int class_id = id & 0xFFFF;
		const fastiva_PackageSlot* packageSlot;
		if ((package_id & 0x8000) == 0) {
			packageSlot = fastiva_libcore_ModuleInfo.m_aPackageSlot + package_id;
		}
		else {
			packageSlot = fastiva_component_ModuleInfo.m_aPackageSlot + (package_id & 0x7FFF);
		}
		pContext = packageSlot->m_pPackage->m_aContextSlot[class_id].m_pContext;
	}
#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER
	else {
		fastiva_ClassContext** ppContext = JNI_RawContext::getContextSlot(context_id);
		pContext = *ppContext;

		int state = pContext->markInLoading();
		
		if (state >= 0) {
			goto context_loaded;
		}

		fastiva_Package* pPackage = (fastiva_Package*)pContext->getPackage();
		pPackage->loadModule();
		pContext = *ppContext;
		if (pContext->isRawContext()) {
			KASSERT(pPackage->isMuttable());
			{	SYNCHRONIZED$(pPackage->m_pModule)
				int index;
				int hashCode = JNI_HashEntry::getHashCode(pContext->m_pBaseName);
				JNI_ContextSlot* pSlot = (JNI_ContextSlot*)pPackage->findContextSlot(hashCode, pContext->m_pBaseName, &index);
				KASSERT(pSlot->m_pContext == (void*)pContext);
				JNI_RawContext* pRawContext = pContext->toRawContext();
				pContext = fm::loadJavaClassFile(pRawContext);
				while (pSlot->m_pContext != (void*)pRawContext) {
					// laodjavaClassFile 수행중에 다른 컨텍스트가 추가될 수 있다.
					pSlot ++;
				}
				pSlot->m_pContext = pContext;
				*ppContext = (fastiva_ClassContext*)pContext;
			}
		}
	}
#endif
context_loaded:
	int dimension = id / FASTIVA_JNI_ARRAY_DIMENSION_BITS(1);
	if (pContext->isRawContext()) {
		pContext = fm::validateContext(pContext);
	}
	java_lang_Class_p pClass = fastiva.getArrayClass(pContext, dimension);
	return pClass;
#endif
};

const fastiva_ClassContext* JNI_FindClass::loadContext_dot(const SYS_TCHAR* classSig) {
	char* classPath = (char*)alloca(strlen(classSig) + 1);
	char* dst = classPath;
	while ((*dst = *classSig) != 0) {
		if (*dst == '.') {
			*dst = '/';
		}
		dst ++;
		classSig ++;
	}
	this->init(classPath);
	m_delimiter = 0;
	return findContext(LOAD);
}

const fastiva_ClassContext* JNI_FindClass::findContext(Action action) {
	const char* sig = m_sig;
	const char* packageName = sig;
	const char* className = sig;

	//int hashCode = 0;
	//int packageHashCode = 0;
	//int package_name_len = 0;
	char delimiter = this->m_delimiter;
	for (char ch; (ch = *sig) != delimiter; sig++) {
		if (ch == 0) {
			return ADDR_ZERO;
		}
		if (ch == '/') {
			className = sig;
		}
	}
	m_sig = sig + 1;

	const fastiva_Package* pPackage;
	if (className != packageName) {
		int	package_name_len = className - packageName;
		className ++; // '/' 제거.
		uint hashCode = JNI_HashEntry::getHashCode(packageName, package_name_len);//hashCode * 31 + ch;
		pPackage = fastiva_Module::loadPackage(hashCode, packageName);
	}
	else {
		pPackage = fastiva_Module::loadPackage(0, "");
	}

	
	if (action == LOAD) {
		((fastiva_Package*)pPackage)->loadModule(this->m_pClassLoader);
	}

	int idxInsert;
	const int class_name_len = sig - className;
	uint hashCode = JNI_HashEntry::getHashCode(className, class_name_len);

	const fastiva_ClassContext* pContext;
	if (!pPackage->isMuttable()) {
		pContext = pPackage->findContextSlot(hashCode, className, &idxInsert);
		KASSERT(pContext == ADDR_ZERO || (!pContext->isRawContext() && !pContext->isLoadFailed()));
		return pContext;
	}

	{	
		SYNCHRONIZED$(pPackage->m_pModule)
		pContext = pPackage->findContextSlot(hashCode, className, &idxInsert);
		if (action == FIND) {
			if (pContext == ADDR_ZERO || pContext->isLoadFailed()) {
				return ADDR_ZERO;
			}
			return pContext;
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

ushort JNI_FindClass::parseValueType(const char* typeSignature, java_lang_ClassLoader_p pLoader) {
	JNI_FindClass fc;
	fc.init(typeSignature);
	fc.m_delimiter = ';';
	int id = fc.parseArgType();
	if (id < 0) {
		fastiva_throwInternalError("invalid value type signature");
	}
	return (ushort)id;
}


int JNI_FindClass::parseArgType() {
	switch (*m_sig++) {
		case ')':
			return -1;

		case 'D':
			return fastiva_Primitives::jdouble;
		case 'J': 
			return fastiva_Primitives::jlonglong;

		case 'V':
			return fastiva_Primitives::jvoid;
		case 'B':
			return fastiva_Primitives::jbyte;
		case 'C':
			return fastiva_Primitives::unicod;
		case 'S':
			return fastiva_Primitives::jshort;
		case 'I':
			return fastiva_Primitives::jint;
		case 'Z':
			return fastiva_Primitives::jbool;
		case 'F':
			return fastiva_Primitives::jfloat;
		
		case 'L': {
			const fastiva_ClassContext* pContext = this->findContext(FIND);
			if (pContext == ADDR_ZERO) {
				return -1;
			}
			return (int)pContext;
		}

		case '[': {
			int dimension = 1;
			while (*m_sig == '[') {
				m_sig ++;
				dimension ++;
			}
			return parseArgType() | FASTIVA_JNI_ARRAY_DIMENSION_BITS(dimension);
		}

		default: {
			return -2;
			//KASSERT("signature error" == 0);
		}
	}
}