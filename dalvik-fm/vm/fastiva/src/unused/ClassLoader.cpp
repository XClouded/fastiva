#include <precompiled_libcore.h>

#include <fastiva_main.h>

#include <fox/Sys.h>
#include <kernel/Kernel.h>
#include <kernel/HeapMark.h>
#include <java/io/InputStream.inl>
// v3.2006
//#include <fastiva/BootstrapClassLoader.h>
#include <java/lang/Thread.inl>
#include <java/lang/Runnable.inl>
#include <pal/fox_file.h>
#include <fox/Heap.h>
#include <fox/Atomic.h>
#include <fastiva/JppException.h>
#include <fastiva/ClassContext.h>

#include "string.h"
#include <fastiva_malloc.h>
#ifndef _WIN32
	#include "alloca.h"
#endif
#include <precompiled_libcore.h>



// fastiva_ModuleInfo fastiva_ModuleInfo;

/**====================== body =======================**/



class fox_ClassLoader : public fastiva_ClassLoader {
public:
	static void relocatePackage(
		fastiva_Package* pPackage, 
		int imageOffsset
	);
	
	static void registerClasses(
		java_lang_Class_p pClass
	);


	static int g_lastClass[sizeof(fastiva_Class_T$) / sizeof(int)];

	static const fastiva_PackageSlot* getFirstPackage() {
		const fastiva_ModuleInfo* pModule = &fastiva_libcore_ModuleInfo;
		return pModule->m_aPackageSlot;
	}
};



int fox_ClassLoader::g_lastClass[sizeof(fastiva_Class_T$) / sizeof(int)];


jbool fm::isLoaded(java_lang_Class_p pClass) {
	return pClass->m_pContext$ != ADDR_ZERO;
}


java_lang_Class_p fm::importClass(
	const fastiva_ClassContext* pContext
) {
	fastiva_Class_p pClass = pContext->m_pClass;
	if (!pClass->isLinked$()) {
		fm::linkClass(pClass);
	}
	return pClass;
}



FOX_HFILE FOX_FASTCALL(__openFileInClassPath)(const char* pszFoxFileName);
//void
//loadClassfile2(const fastiva_InstanceContext*, bool_t fatalErrorIfFail);
/*
const fastiva_ClassContext* fm::findClassContext(
	const char* classSignature
) {
	int str_len = strlen(classSignature);
	return fm::findContext(NULL, classSignature, str_len);
}
*/


static int JVM_isInited = false;
#if 0
const fastiva_ClassContext* fm::findContext(
	java_lang_ClassLoader_p pLoader,
	const char* classPath, 
	int str_len
) {
	CHECK_STACK_FAST();
	KASSERT(classPath[0] != '[');

	if (classPath[0] == 'L') {
		classPath++;
		str_len -= 2;
	}
	else if (str_len == 1) {
		return fm::getPrimitiveContextBySig(classPath[0]);
	}



	char  buf[1024];
	char* packageName = buf + 1;
	char* className = packageName;
	const char* classPath_0 = classPath; 

	if (str_len <= 0 || str_len >= sizeof(buf) - 1) {
		// too long name;
		return ADDR_ZERO;
	}

	// default package에 대해 고려해야 한다.
	// jar와 연동되어야 한다.
	// 라이브러리를 중복하여 load하지 말아야 한다.

	char* pch = packageName;
	int hashCode = 0;
	int packageHashCode = 0;
	for (int i = str_len; i -- > 0; ) {
		char ch = (char)*classPath ++;
		if (ch < '0') {
			if (ch == '.' || ch == '/') {
				className = pch;
				ch = '/';
				packageHashCode = hashCode;
			}
		}
		else {
			hashCode = hashCode * 31 + ch;
		}
 		*pch ++ = ch;
	}

	const fastiva_Package* pPackage;
	
	*pch = 0;
	if (className != packageName) {
		*className ++ = 0;
	}
	else {
		packageName --;
		*packageName = 0;
	}
	
	//LOCK_PACKAGE_Q
	pPackage = fox_ClassLoader::registerJavaPackage(pLoader, packageHashCode, packageName);
	//UNLOCK_PACKAGE_Q

	const fastiva_ClassContext* pContext = ADDR_ZERO;
	int cntContext;
	if (pPackage != ADDR_ZERO) {
		cntContext = pPackage->m_cntContext;
		pContext = fox_ClassLoader::findContext(pPackage, hashCode, className);
	}
	else {
		cntContext = 0;
	}

	if (pContext == ADDR_ZERO) {// && pPackage->m_pLoadedBy == -1 /* mutable-package*/) {
		void* fm::openClassfileInternal(const char* filename);

		if (!JVM_isInited) {
			void InitializeHashtables();
			const char* fastiva_kernel_getClassPath();
			void InitializeClassLoading(char* classPath);

			InitializeHashtables();
			const char* cp = fastiva_kernel_getClassPath();
			InitializeClassLoading((char*)cp);
		}

		strcpy(pch, ".class");
		className[-1] = '/';
		void* filePointer = fm::openClassfileInternal(buf+1);
		className[-1] = 0;

		if (filePointer != ADDR_ZERO) {
			if (pPackage == ADDR_ZERO) {
				// 주의 PackageQ를 다시 locking하여야 한다.
				pPackage = fox_ClassLoader::registerJavaPackage(packageHashCode, packageName, strlen(packageName));
			}

			// LOCK_PACKAGE
			int cntNewContext = pPackage->m_cntContext - cntContext;
			if (cntNewContext > 0) {
				pContext = fox_ClassLoader::findContext(pPackage->m_aContextInfo + cntContext, cntNewContext, hashCode, className);
			}
			if (pContext == ADDR_ZERO) {
				fastiva_ClassContext* fastiava_parseClassFile(fastiva_ClassInfo* ci);

				fastiva_ClassInfo ci;
				ci.m_pBaseName = className;
				ci.m_pPackage = pPackage;
				ci.m_hashCode = hashCode;
				ci.m_filePointerH = &filePointer;
				pContext = fastiava_parseClassFile(&ci);
			}
		}
		// RELEASE_PACKAGE
	}

	return pContext;
}

fastiva_ClassContext* fm::registerClassContext(
	const JNI_RawContext* classInfo, 		
	int accessFlags
) {

	fastiva_ClassContext* pContext;
	char* className;
	if ((accessFlags & ACC_INTERFACE$) != 0) {
		// structure의 optimizing을 위해서, Java-ContstPool을 hidden-header로 처리한다.
		fastiva_InterfaceContext* pIfcContext = (fastiva_InterfaceContext*)((char*)fox_heap_malloc(sizeof(int) +
			sizeof(fastiva_InterfaceContext) + sizeof(fastiva_JniInfo)) + sizeof(int));
		//className = (char*)(pIfcContext + 1) + sizeof(fastiva_JniInfo);
		pContext = pIfcContext;
		pContext->m_pJNI = (fastiva_JniInfo*)(pIfcContext + 1);
	}
	else {
		// structure의 optimizing을 위해서, Java-ContstPool을 hidden-header로 처리한다.
		fastiva_InstanceContext* pObjContext = (fastiva_InstanceContext*)((char*)fox_heap_malloc(sizeof(int) + 
			sizeof(fastiva_InstanceContext) + sizeof(fastiva_JniInfo)) + sizeof(int));
		//className = (char*)(pObjContext + 1) + sizeof(fastiva_JniInfo);
		pContext = pObjContext;
		pContext->m_pJNI = (fastiva_JniInfo*)(pObjContext + 1);

		pObjContext->m_aScanOffset = 0; ??
		pObjContext->m_sizInstance = 0;		// 64K*4 이상의 instance를 생성할 수 없다. (clone에서 사용된다)
		//pObjContext->m_sizScanInfo = 0;

		pObjContext->m_sizInstance = 0;
		pObjContext->m_defInitFlag = 0; ??
		
		pObjContext->m_pSuperContext = 0; // InterfaceContextInfo.m_ppIFC[0] 와 같은 위치에 있어야 한다.
		//void (FASTIVA_STATICCALL(*main$))(java_lang_String_ap);
		pObjContext->m_aScanOffset = 0;
		pObjContext->main$ = 0; ??
	}

	//strcpy(className, classInfo->m_pBaseName);
	pContext->m_id = classInfo->m_id;
	pContext->m_pBaseName = classInfo->m_pBaseName;
	pContext->m_ppIFC = 0; 

	//((void**)pContext)[-1] = classInfo->m_constPool;

	pContext->m_inheritDepth = 0; // ??
	pContext->m_accessFlags = 0;
	pContext->m_pClass = 0;
	pContext->m_pPackage = classInfo->m_pPackage;
//	pContext->m_crc32 = 0;
#ifdef __SUPPORT_SIZ_STATIC
	pContext->m_sizStatic = 0;
#endif
	pContext->m_cntVirtualMethod = 0;
	pContext->initClass$ = 0;
	pContext->initStatic$ = 0;

#if 0
	fastiva_Package* pPackage = (fastiva_Package*)classInfo->m_pPackage;
	// LOCK_PACKAGE()
	fastiva_Package::ContextInfo* base = (fastiva_Package::ContextInfo*)pPackage->m_aContextInfo;
	int cntContext = pPackage->m_cntContext;
	int sizContext = (cntContext + 7) & ~7;
	if (sizContext <= cntContext) {
		fastiva_Package::ContextInfo* new_base = (fastiva_Package::ContextInfo*)fox_heap_malloc(sizeof(fastiva_Package::ContextInfo) * (sizContext + 8));
		if (base != ADDR_ZERO) {
			memcpy((void*)new_base, (void*)base, cntContext * sizeof(fastiva_Package::ContextInfo));
		}
		base = new_base + cntContext;
		pPackage->m_aContextInfo = new_base;
	}
	else {
		base += cntContext;
	}
	base->m_hashCode = classInfo->m_hashCode;
	base->m_pContext = (fastiva_ClassContext*)pContext;
	// sync를 위해 가장 뒤에
	pPackage->m_cntContext ++;
	// @todo 주의!
	// 현재 m_pClassQ는 Package-unloading을 위해 사용되고 있다.
	// Fastiva-Component는 Package에 Class를 추가등록하는 것이 불가능하므로,
	// m_pClassQ가 전체 package를 가지고 있으나, Interpreter의 경우는
	// 그렇치 아니하다. 조정이 필요하다.
//	pPackage->m_pClassQ = (java_lang_Class_p)(void*)&fox_ClassLoader::g_lastClass;
	// RELEASE_PACKAGE();
#endif
	return pContext;

}
#endif



#include <pal/fox_file.h>





const fastiva_ClassContext* fm::validateContext(const fastiva_ClassContext* pMoniker) {
	// 외부 모듈에 속한 Context는 아직 super-class의 vtable을 상속하지 않은 상태이다.
	if (!pMoniker->isRawContext()) {
		return pMoniker;
	}

	if (!FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER) {
		FASTIVA_DBREAK();
		return 0;
	}

	JNI_RawContext* pRawContext = pMoniker->toRawContext();
	
	// thread-safety 확보	
	fox_mutex_lock_GCI(kernelData.g_pRawContextLock);
	if (pRawContext->m_state == JNI_RawContext::IMPORTED) {
		fox_mutex_release(kernelData.g_pRawContextLock);
		return pRawContext->getImportedContext();// (const fastiva_ClassContext*)pRawPackage;
	}

	//LOCK_PACKAGE_Q
	const fastiva_Package* pPackage;
	const char* pPackageName = pRawContext->m_pPackageName;

	int packageHashCode = JNI_HashEntry::getHashCode(pPackageName);
	int len = JNI_HashEntry::getLength(packageHashCode);
	if (len == 0) {
		// default-package는 import될 수 없도록 한다.
		pPackage = fastiva_Module::loadPackage(0, "");
	}
	else {
		pPackage = fastiva_Module::loadPackage(packageHashCode, pPackageName);
		((fastiva_Package*)pPackage)->loadModule();
	}

	int hashCode = JNI_HashEntry::getHashCode(pMoniker->m_pBaseName);
	//UNLOCK_PACKAGE_Q

	const fastiva_ClassContext* pRefContext;
	pRefContext = pPackage->findContextSlot(hashCode, pMoniker->m_pBaseName);

	pRawContext->setImportedContext(pRefContext);
	fox_mutex_release(kernelData.g_pRawContextLock);
	return pRefContext; // pRefContext is not Linked, yet.
}


void fm::linkContext(const fastiva_ClassContext* pContext) {
	if ((pContext->m_pClass->m_mark$ & HM_CONTEXT_RESOLVED) != 0) {
		return;
	}

	pContext->m_pClass->m_mark$ |= HM_CONTEXT_RESOLVED;
	

	/*
	InstanceContext의 ppIVT가 모든 Interface-tree에 대한 list를 가지고 있으나,
	Interface-Class에 대한 import 순서를 저오학히 보장하기 위하여 아래의 code를
	유지한다. 이미 Import된 interface에 대해 중복된 import-class가 호출되는 
	단점이 있으나, performance에 별 영향을 끼치지 않으므로 무시한다.
	*/
	if (pContext->isInterface()) {
		const fastiva_ClassContext* pIFC = pContext->toInterfaceContext();
		const fastiva_ClassContext** ppIFC = pIFC->m_ppIFC;
		if (ppIFC == ADDR_ZERO) {
			*(void**)&pIFC->m_ppIFC = fastiva_Instance_G$::aIVT$;
		}
		else {
			for (; (pIFC = *ppIFC) != NULL; ppIFC++) {
				const fastiva_ClassContext* pIFC = *ppIFC;
				// pIFC는 RawContext를 사용한다. 이를 ClassID로 대체하는 경우,
				// Package Loading시에 import된 모든 외부 Context에 대한 RawContext를
				// 생성해 주어야 하는데, 결과적으로 Loading 속로를 줄이기만 할 뿐, 
				// 별다른 Data 절약효과도 얻을 수 없다.
				if (pIFC->isRawContext()) {
					*ppIFC = pIFC = fm::validateContext(pIFC)->toInterfaceContext();
				}
				//linkContext(pIFC);
				fm::linkClass(pIFC->m_pClass);
			}
		}
		return;
	}

	int isExternal = pContext->m_accessFlags & ACC_EXTERNAL$;
	const fastiva_InstanceContext* pVC = pContext->toInstanceContext();
	const fastiva_InstanceContext* pSuperContext = pVC->m_pSuperContext;

	const fastiva_ImplementInfo* pImpl = pVC->m_aImplemented;
	const fastiva_ImplementInfo* pSuperImpl = pSuperContext->m_aImplemented;
	if (pImpl == ADDR_ZERO) {
		*(const void**)&pVC->m_aImplemented = pSuperImpl;
	}
	else {
		const fastiva_ClassContext* pIFC;
		for (; (pIFC = pImpl->m_pInterfaceContext) != ADDR_ZERO; pImpl++) {
			// pIFC는 RawContext를 사용한다. 이를 ClassID로 대체하는 경우,
			// Package Loading시에 import된 모든 외부 Context에 대한 RawContext를
			// 생성해 주어야 하는데, 결과적으로 Loading 속로를 줄이기만 할 뿐, 
			// 별다른 Data 절약효과도 얻을 수 없다.
			if (pIFC->isRawContext()) {
				*(const void**)&pImpl->m_pInterfaceContext = pIFC = fm::validateContext(pIFC)->toInterfaceContext();
			}

			//linkContext(pIFC);
			fm::linkClass(pIFC->m_pClass);
		}
		// 다운로드된 Component는 super-class의 implemented-list를 포함하지 않는다.
		if (!isEmbedded) {
			*(const void**)&pImpl->m_pIVTable = pSuperImpl;
		}
		pVC->m_pClass->obj.itables$ = fm::createIVTables(pVC->m_aImplemented);
	}

	if (pSuperContext->isRawContext()) {
		pSuperContext = fm::validateContext(pSuperContext)->toInstanceContext();
		*(const void**)&pVC->m_pSuperContext = pSuperContext;
	}
	//linkContext((fastiva_ClassContext*)(pSuperContext));
	fm::linkClass(pSuperContext->m_pClass);

	if (FASTIVA_SUPPORTS_JAVASCRIPT) {
		// 상위 class의 Generic-Interface-Table 상속.
		*(int*)pContext->m_pClass = *(int*)pSuperContext->m_pClass;
	}



	if (!isEmbedded) {
		// validate virtual-table;

		//*(short*)&pContext->m_accessFlags |= ACC_SUPER$;

		int unresolvedSuperAddr = pContext->getPackage()->m_pModule->m_imageOffset;
		//*
		int  cntVM = pSuperContext->getVirtualMethodCount();
		int* pSlot = (int*)pContext->getVirtualTable();
		int* pRef  = (int*)pSuperContext->getVirtualTable();
		for (; cntVM -- > 0; ) {
			/** 
			참고) 아래 Code 수행시 오류가 발새하면 아래의 Link-Option을 추가한다.
			/section:.text,RW /section:.bss,RW /section:.rdata,R /section:.rdata,RW
			*/
			void* pRefFunc = (void*)*pRef;
			void* pSlotFunc = (void*)*pSlot;
			if ((uint)*pSlot == unresolvedSuperAddr) { // check-abstract-func.
				*pSlot = *pRef;
			}
			pSlot ++;
			pRef ++;
		}
	}
}

void fastiva_Runtime::linkClass_$$(void* pVTable, const fastiva_ClassContext* pContext) {
	//const fastiva_ClassContext* pContext = fm::getClassContext(this);//this->m_pContext$;

	pContext->m_pClass->obj.vtable$ = (pVTable);
	fm::linkContext(pContext);

	if (!pContext->isInterface()) {
		fastiva_InstanceContext_cp pSuperContext = pContext->toInstanceContext()->m_pSuperContext;
		if (pSuperContext != NULL) {
			const int cntII = sizeof(fastiva_Instance_I$) / sizeof(void*);
			void** pSuperVTable = (void**)pSuperContext->m_pClass->obj.vtable$;
			void** pSelfVTable = (void**)pVTable;
			for (int i = pSuperContext->m_cntVirtualMethod; --i >= 0; ) {
				*pSelfVTable++ = *pSuperVTable++;
			}
		}
	}


#if 0
	if (!pContext->isInstance()) {
		KASSERT(pVTable == ADDR_ZERO);
		/*
		InstanceContext의 ppIVT가 모든 Interface-tree에 대한 list를 가지고 있으나,
		Interface-Class에 대한 import 순서를 저오학히 보장하기 위하여 아래의 code를
		유지한다. 이미 Import된 interface에 대해 중복된 import-class가 호출되는 
		단점이 있으나, performance에 별 영향을 끼치지 않으므로 무시한다.
		*/

		const fastiva_ClassContext* pIC = pContext->toInterfaceContext();
		fastiva_InterfaceContext** ppIFC = (fastiva_InterfaceContext**)(void*)pIC->m_ppIFC;
		fastiva_InterfaceContext* pIFC;
		if (ppIFC != ADDR_ZERO) {
			for (; (pIFC = *ppIFC) != ADDR_ZERO; ppIFC++) {
				KASSERT(!pIFC->isRawContext());
				fm::importClass(pIFC);
			}
		}
	}
	else {
		KASSERT(pVTable != ADDR_ZERO);
		const fastiva_InstanceContext* pVC = pContext->toInstanceContext();
		const fastiva_InstanceContext* pSuperContext = pVC->m_pSuperContext;

		if (pSuperContext != ADDR_ZERO) {
			KASSERT(!pSuperContext->isRawContext());
			fm::importClass(pSuperContext);
		};

		const fastiva_ClassContext** ppIFC = pVC->m_ppIFC;
		if (ppIFC != ADDR_ZERO) {
			for (; *ppIFC != ADDR_ZERO; ppIFC++) {
				const fastiva_ClassContext* pIFC = *ppIFC;
				KASSERT(!pIFC->isRawContext());
				fm::importClass(pIFC);
			}
		}
	}
#endif

	//Native method를 등록한다. static block에서 native method를 사용할수 있으므로
	//initStatic하기 전에 일단 등록한다.


}

void* fastiva_Runtime::initClass_$$(java_lang_Class_p pClass) {
	return fm::linkClass(pClass);
}



const void** fm::createIVTables(const fastiva_ImplementInfo* aImplemented) {
	//const fastiva_ImplementInfo* aImplemented = pContext->m_aImplemented;
	const fastiva_ImplementInfo* pImpl;
	int min = 0x7FFFFFFF, max = 0;
	const fastiva_ClassContext* pIFC;
	for (pImpl = aImplemented; (pIFC = pImpl->m_pInterfaceContext) != 0; pImpl++) {
		int id = pIFC->ifc.itableId$;
		if (id < min) {
			if (id < 0) {
				continue;
			}
			min = id;
		}
		if (id > max) {
			max = id;
		}
	}

	if (min > max) {
		return NULL;
	}

	int size = max - min;
	fox_semaphore_lock(kernelData.g_ivtableLock);
	while (*kernelData.g_ivtableAllocTop != NULL) {
		kernelData.g_ivtableAllocTop ++;
	}

	int* pStart = kernelData.g_ivtableAllocTop;
	int* pEndOfHeap = (int*)((int)pStart - ((int)pStart % sys_heap_getPageSize()) + sys_heap_getPageSize());

	for (pImpl = aImplemented; (pIFC = pImpl->m_pInterfaceContext) != 0;) {
		int id = pIFC->ifc.itableId$ - min;
		if (id < 0) {
			KASSERT(pIFC->ifc.itableId$ == -1);
			pImpl++;
			continue;
		}
		if (pStart + id >= pEndOfHeap) {
			pEndOfHeap[0] = 0;
			pEndOfHeap += sys_heap_getPageSize() / sizeof(int);
		}
		if (pStart[id] != NULL) {
			pStart ++;
			pImpl = aImplemented;
			continue;
		}
		pImpl++;
	}
	
	for (pImpl = aImplemented; (pIFC = pImpl->m_pInterfaceContext) != 0; pImpl++) {
		int id = pIFC->ifc.itableId$ - min;
		if (id < 0) {
			KASSERT(pIFC->ifc.itableId$ == -1);
			continue;
		}
		pStart[id] = (int)pImpl->m_pIVTable;
	}

	fox_semaphore_release(kernelData.g_ivtableLock);
	return (const void**)(pStart - min);
}

fastiva_Class_p fm::linkClass(
	java_lang_Class* pClass_00
) {
	
	//modified by keenchin
	/*
	java_lang_ExceptionInInitializerError이 발생한후 Class의 static block을 수행하면
	java_lang_NoClassDefFoundError를 발생하여야 한다.
	java_lang_ExceptionInInitializerError가 발생한 후 pContext를 fastiva_throwNoClassDefFoundError와 
	같은 Address로 세팅한다.
	밑에 CATCH_ANY$ 문 참조.

	*/
	fastiva_Class_p volatile pClass = (fastiva_Class_p) pClass_00;


	const fastiva_ClassContext* pContext = pClass->m_pContext$;
	KASSERT(fm::getArrayDimension(pClass) == 0);
	KASSERT(pContext->m_pClass == pClass);

	if (pContext->isPrimitive()) {
		return pClass;
	}
	// 두개의 Thread에 의한 동시 진입을 방지한다.
	fastiva_Synchronize lock(pClass);

	if (pClass->isLinked$()) {
		// 반드시 locking 이후에 검사하여야만 한다.
		if ((pClass->m_mark$ & HM_ININITIALIZER_ERROR) != 0) {
			fastiva_throwNoClassDefFoundError();
		}
		// initStatic$ 중에 재호출된 것이다.
		// fm::linkContext(pContext);
		return pClass;
	}

	while (true) {
		// 2012.10 아래 code 및 m_pClassQ 삭제.
		fastiva_ModuleInfo* pModule = ((fastiva_PackageInfo*)pContext->getPackage())->m_pModule;
		pClass->m_pNext$ = (pModule->m_pClassQ);
		int oldV = (int)pClass->m_pNext$;
		if (fox_util_cmpxchg32((void*)&pModule->m_pClassQ, (int)pClass, oldV)) {
			break;
		}
	}

	TRY$ {
		pContext->initStatic$();
	}
	CATCH_ANY$ {
		pClass->m_mark$ |= HM_ININITIALIZER_ERROR;
		fastiva_throwExceptionInInitializerError(catched_ex$);
	}

	return pClass;
}



int fastiva_Runtime::checkImported(java_lang_Class_p pClass) {
	if (pClass->m_pNext$ == ADDR_ZERO) {
		if ((pClass->m_pContext$->m_accessFlags & ACC_PRELOAD$) == 0) {
			if (kernelData.g_pMainThread != ADDR_ZERO) {
				FASTIVA_DBREAK();
			}
		}
	};
	return true;
}




void fox_ClassLoader::relocatePackage(fastiva_Package* pPackage, int imageOffset) {
	/*
	CHECK_STACK_FAST();
	int* pAddr = (int*)pPackage;
	for (int i = 3; i -- > 0; ) {
		// m_pShadowAddr를 제외한다.
		*pAddr += imageOffset;
		pAddr ++;
	}

	fastiva_ClassContext** ppContext = (fastiva_ClassContext**)pPackage->m_tContext;
	fastiva_ClassContext** ppEndContext = (fastiva_ClassContext**)pPackage->m_pEndOfContext;
	*(int*)&ppEndContext = (int)ppEndContext & ~3;
	*(int*)&pPackage->m_pEndOfContext = (int)ppEndContext;
	
	for (; ppContext < ppEndContext; ppContext ++) {
		fastiva_ClassContext* pContext = *ppContext;
		*ppContext = pContext = (fastiva_ClassContext*)((int)pContext + imageOffset);
		int* pAddr = (int*)pContext;
		for (int i1 = 3; i1 -- > 0; ) {
			*pAddr += imageOffset;
			pAddr ++;
		}
		// aExportedProc$...
		// fastiva_Package$...
		// className$...

		pAddr += 2;  
		for (int i2 = 4; i2 -- > 0; ) {
			if (*pAddr != 0) {
				*pAddr += imageOffset;
			}
			pAddr ++;
		}
		// qArrayClass$...
		// import$class...
		// init$instance...
		// context$...

		for (int i = pContext->m_cntInterface; i -- > 0; ) {
			*pAddr += imageOffset;
			pAddr ++;
		}

		if ((pContext->m_accessFlags & fm::ACC_INTERFACE) != 0) {
			continue;
		}
		
		// 다음은 static function table을 초기화한다.
		// addr_zero를 binary가 load된 image로 변경.
		typedef int FUNC_ADDR;
		FUNC_ADDR* pExportProc = (FUNC_ADDR*)pContext->m_tSharedProc
								 - pContext->m_cntVirtualProc;
		while (pExportProc < (FUNC_ADDR*)pContext) { // final_proc
			*pExportProc += imageOffset;
			pExportProc ++;
		}
	}
	return;
	*/
}

//void fm::cleanStaticData(java_lang_Class_p pClass, int sizClass) {
//	memset((fastiva_Class_p)pClass+1, 0, sizClass - sizeof(fastiva_Class_T$));
//}

fastiva_Class_p fm::initRawClass(const fastiva_ClassContext*  pContext) {
	fastiva_Class_p pClass = pContext->m_pClass;
	fastiva_Class_p pLastClass = (fastiva_Class_p)fox_ClassLoader::g_lastClass;
	memcpy(pClass, fox_ClassLoader::g_lastClass, sizeof(fox_ClassLoader::g_lastClass));
	{
		// pointer 및 primitive field 모두를 o으로 reset한다.
#ifdef __SUPPORT_SIZ_STATIC
		int sizStatic = (int)pContext->m_sizStatic
					  - (int)sizeof(fastiva_Class_T$);
		KASSERT((uint)sizStatic < 0xFFFF);
		memset((char*)pClass + sizeof(fastiva_Class_T$), 0, sizStatic);
#endif
	}
	// FASTIVA_NULL_ADDR 이 ADDR_ZERO와 다른 경우, 이를 초기화하고,
	// Instance의 VirtualTable의 값을 얻어낸다. 특정 Class를 import하는 도중,
	// 해당 class의 super-class에서 해당 class-instance를 생성하거나,
	// static 변수를 참조하는 것이 가능하므로, Unlinked-class 도
	// 몇몇 변수가 미리 초기화되어 있어야 한다.
	// v3.2006 pContext->initClass$((java_lang_Class_p)pClass);


	pClass->m_pContext$ = (pContext);
	return pClass;
}

static const int no_typed_checked[] = { 0, 0 };
void fm::initClassLoader() {
	int __initJavaClassVTable();

	kernelData.g_ivtableHeap = (int*)sys_heap_virtualAlloc(NULL, 1*1024*1024);
	kernelData.g_ivtableAllocTop = kernelData.g_ivtableHeap;

	fastiva_Class_p pClass = (fastiva_Class_p)&fox_ClassLoader::g_lastClass;


#if FASTIVA_SUPPORTS_JAVASCRIPT
	*(int*)pClass = __initJavaClassVTable();
#else
	*(void**)pClass = java_lang_Class_G$::g_vtable$;
#endif
	pClass->obj.vtable$ = 0;
	pClass->m_mark$ = HM_PUBLISHED;
	//pClass->m_jsType$ = __getJavaObjectTypeInScript();
	//pClass->m_pInstance_C$ = ADDR_ZERO;
	//pClass->m_pInterface_C$ = &no_typed_checked;
	fox_monitor_init(&pClass->m_monitor$);
	//v3 fm::INITIALIZE_SYSTEM_LOCK(pClass);
	pClass->m_pNext$ = (ADDR_ZERO);
	*(const void**)&pClass->m_pClass$ = FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Class)->m_pClass;
	pClass->m_pContext$ = (FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Object));
	pClass->m_pCompositeType$ = ((java_lang_Class_p)FASTIVA_NULL);
	pClass->m_pClassLoader$ = ((java_lang_ClassLoader_p)FASTIVA_NULL);

	{
		fastiva_libcore_initModuleInfo();
		g_pLibcoreModule->initAllClasses();
		// 모든 class 구조체를 초기화한 후, Object를 CONTEXT_RESOLVED 상태로 변경해 준다.
		java_lang_Object::getRawContext$()->m_pClass->m_mark$ |= HM_CONTEXT_RESOLVED;
	}

	// @?? 이 시점이 맞나. initAllClasses를 한 부분이 맞나?
	g_pLibcoreModule->m_pClassQ = (fastiva_Class_p)&fox_ClassLoader::g_lastClass;
#if (JPP_JNI_EXPORT_LEVEL > 0)
	kernelData.g_interpreterModule.m_pClassQ = (fastiva_Class_p)&fox_ClassLoader::g_lastClass;
#endif
}

/*
void fm::visitAllContext(PFN_VISITOR fnIterator) {
	FASTIVA_DBREAK();

	const fastiva_PackageInfo* pPackage = fox_ClassLoader::getFirstPackage();
	while (pPackage->m_pModule != 0) {
		FASTIVA_DBREAK();
		const JNI_HashEntry* pSlot = pPackage->m_aContextSlot;
		while (pSlot->m_pContext->m_pClass != 0) {
			fnIterator(pSlot->m_pContext);
			pSlot ++;
		}
		pPackage ++;
	}
}
*/

void fastiva_Package::loadModule0(
	java_lang_ClassLoader_p pLoader
) {
#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER
	CHECK_STACK_FAST();
	const fastiva_Package* pPackage = fm::loadLibrary(m_szPackageName, strlen(m_szPackageName));
	if (pPackage == ADDR_ZERO) {
		this->m_pModule = &kernelData.g_interpreterModule;
		return;
		//registerComponent(pPackage, 0, g_idxInsert);
	}

	pPackage->m_pModule->m_pClassQ = (java_lang_Class_p)(void*)&fox_ClassLoader::g_lastClass;
	//if (imageOffset != 0) {
		//pNewPackage->m_pShadowAddr = (void*)(imageOffset + (int)pNewPackage->m_pShadowAddr);

		/*
		const fastiva_Package* pPackage    = pNewPackage->m_pComponent;
		const fastiva_Package* pEndPackage = pNewPackage->m_pEndOfPackage;
		while (pPackage < pEndPackage) {
			if (pPackage->m_szPackageName == 0) {
				pNewPackage->m_pEndOfPackage = pPackage;
				break;
			}
			((fastiva_Package*)pPackage)->m_pFastiva::Package = pNewPackage;
	#ifndef FASTIVA_TARGET_OS_WIN32
			fox_ClassLoader::relocatePackage((fastiva_Package*)pPackage, imageOffset);
	#endif	
	#ifdef _DEBUG
			fox_ClassLoader::getContext(pPackage, "$");
	#endif
			pPackage ++;
		}
		*/
	//}

	if (pPackage->m_aContextInfo != 0) {
		FASTIVA_DBREAK();
		// 미리 등록된 RawContext를 기준으로 classID를 설정한다.
	}

	//pPackage->m_aContextInfo = aContextInfo;
	//pPackage->m_cntContext = cntContext;

	ContextInfo_p pContextInfo = pPackage->m_aContextInfo;
	for (int cntContext = pPackage->m_cntContext; cntContext -- > 0; ) {
		fm::initRawClass((fastiva_ClassContext*)pContextInfo->m_pContext);
		pContextInfo ++;
	}

	//pNewPackage->m_pNext = kernelData.g_pPackageQ;
	//kernelData.g_pPackageQ = pNewPackage;

	return;
#endif
}

/*
const fastiva_ClassContext* fm::findLoadedMoniker(const fastiva_ClassMoniker* pMoniker) {
	KASSERT(pMoniker != ADDR_ZERO);
	KASSERT((int)pMoniker > CNT_PRIMITIVE);

	if (pMoniker->isImported()) {
		return (const fastiva_ClassContext*)pMoniker->m_packageHashCode;
	}

	const fastiva_Package* pPackage;
	const fastiva_ClassContext* pRefContext;

	pPackage = fox_ClassLoader::findLoadedPackage(pMoniker->m_packageHashCode, pMoniker->m_pPackageName, nameLen);
	if (pPackage == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	pRefContext = fox_ClassLoader::findContext(pPackage, pMoniker->m_classHashCode, pMoniker->m_crc32);
	return pRefContext;
}

const fastiva_ClassContext* fm::loadMoniker(const fastiva_ClassMoniker* pMoniker) {
	KASSERT(pMoniker != ADDR_ZERO);
	KASSERT((int)pMoniker > CNT_PRIMITIVE);

	if (pMoniker->isImported()) {
		return (const fastiva_ClassContext*)pMoniker->m_packageHashCode;
	}

	//LOCK_PACKAGE_Q
	const fastiva_Package* pPackage = fox_ClassLoader::loadPackage(ADDR_ZERO, pMoniker->m_packageHashCode, pMoniker->m_pPackageName);
	//UNLOCK_PACKAGE_Q

	const fastiva_ClassContext* pRefContext = ADDR_ZERO;
	if (pPackage != ADDR_ZERO) {
		pRefContext = fox_ClassLoader::findContext(pPackage, pMoniker->m_classHashCode, pMoniker->m_crc32);
	}
	if (pRefContext == ADDR_ZERO) {
		//fox_debug_puts(pMoniker->m_pClassName);
		//fox_debug_puts(" : super class not found! : DefClass Error");
		fastiva_throwNoClassDefFoundError();
	}

	*(const void**)&pMoniker->m_packageHashCode = pRefContext;
	*(int*)&pMoniker->m_accessFlags = -2;
	// v3.2006.2 fox_ClassLoader::loadClass(pRefContext);
	return pRefContext;
}
*/


/**====================== end ========================**/


#if 0
// ClassNameID는 생성하지 아니한다.
struct fm::ClassNameSlot {
	int m_hashCode;
	const fastiva_ClassContext* m_pContext;
};

static fastiva_InstanceContext* createRawContext(fastiva_Package* pPackage, const char* baseName0) {
	/**
	Inteface와 Instance 구별없이 InstanceContext를 생성한다.
	InstanceContext는 InterfaceContext보다 8-16 byte 정도 더 사용하는 단점이 있으나,
	Interpreter에서 사용되는 InterfaceContext의 숫자가 적으므로 무시할 수 있다.
	*/
	fastiva_ClassContext* pContext;
	fastiva_InstanceContext* pObjContext = (fastiva_InstanceContext*)((char*)fox_heap_malloc(sizeof(int) + 
		sizeof(fastiva_InstanceContext) + sizeof(fastiva_JniInfo) + strlen(baseName0 + 1)) + sizeof(int));
	className = (char*)(pObjContext + 1) + sizeof(fastiva_JniInfo);
	pContext = pObjContext;
	pContext->m_pJNI = (fastiva_JniInfo*)(pObjContext + 1);

	pObjContext->m_aScanOffset = 0;
	pObjContext->m_sizInstance = 0;		// 64K*4 이상의 instance를 생성할 수 없다. (clone에서 사용된다)
	//pObjContext->m_sizScanInfo = 0;

	pObjContext->m_sizInstance = 0;
	//pObjContext->m_defInitFlag = 0;
	
	pObjContext->m_pSuperContext = 0; // InterfaceContextInfo.m_ppIFC[0] 와 같은 위치에 있어야 한다.
	//void (FASTIVA_STATICCALL(*main$))(java_lang_String_ap);
	pObjContext->m_aScanOffset = 0;
	pObjContext->main$ = 0;


	strcpy(className, className0);
	pContext->m_pClassName = className;
	pContext->m_ppIFC = 0;
	pContext->m_id = kernelData.g_cntContext ++;

	//((void**)pContext)[-1] = classInfo->m_constPool;

	pContext->m_inheritDepth = 0;
	pContext->m_accessFlags = 0;
	pContext->m_pClass = 0;
	pContext->m_pPackage = pPackage;
	//pContext->m_crc32 = 0;
#ifdef __SUPPORT_SIZ_STATIC
	pContext->m_sizStatic = 0;
#endif
	pContext->m_cntVirtualMethod = 0;
	pContext->initClass$ = 0;
	pContext->initStatic$ = 0;
}

const static int StringBucketSize = 4096;
static char* g_pStringBucket;
static int   g_lenOfPool = StringBucketSize;

static const char* fm::allocateStringToken(const char* classPath, int len) {
	char* buff = (g_pStringBucket + g_lenOfPool);
	if ((g_lenOfPool += len) > StringBucketSize) {
		buff = g_pStringBucket = (char*)fox_heap_malloc(StringBucketSize);
	}

	int* dst = (int*)buff;
	int* src = (int*)classPath;

	uint cntC = (len + 1) / sizeof(int);
	g_lenOfPool += cntC * sizeof(int);

	for (; cntC -- > 0; ) {
		*dst++ = *src ++;
	}
	return buff;
};
#endif



#if FASTIVA_SUPPORTS_BYTECODE_INTERPRETER
const fastiva_ClassContext* fm::loadJavaClassFile(
	JNI_RawContext* pRawContext
) {
	
	//KASSERT(fox_mutex_isLocked(pRawContext->m_pPackage->m_pModule));

	void* fm::openClassfileInternal(const char* filename);

	if (!JVM_isInited) {
		//void InitializeHashtables();
		//const char* fastiva_kernel_getClassPath();
		//void InitializeClassLoading(char* classPath);

		//InitializeHashtables();
		//const char* cp = fastiva_kernel_getClassPath();
		//InitializeClassLoading((char*)cp);
	}

	int p_len = JNI_HashEntry::getLength(pRawContext->getPackage()->m_hashCode);
	int c_len = strlen(pRawContext->m_pBaseName);
	unicod* classPath = (unicod*)alloca((p_len + c_len + 2 + 6) * 2);
	unicod* dst = classPath;
	unicod cod;
	const char* src;

	if (p_len > 0) {
		src = pRawContext->m_pPackageName;
		while ((cod = *src++) != 0) {
			*dst ++ = cod;
		}
		*dst ++ = '/';
	}
	src = pRawContext->m_pBaseName;
	while ((cod = *src++) != 0) {
		*dst ++ = cod;
	}
	src = ".class";
	while ((*dst = *src++) != 0) {
		dst ++;
	}

	// 추후 classLoader로 옮김.
	Byte_ap pData = fm::loadResource(classPath, dst - classPath, ADDR_ZERO);
	fastiva_ClassContext* volatile pContext = (fastiva_ClassContext*)pRawContext;
#ifndef _ARM_
	if (pData != ADDR_ZERO) {
		// LOCK_PACKAGE
		fastiva_ClassContext* fastiava_parseClassFile(JNI_RawContext* ci, jbyte* buff, int length);

		TRY$ {
			Byte_A::Buffer buf(pData);
			pContext = fastiava_parseClassFile(pRawContext, (jbyte*)buf, pData->length());
		}
		CATCH_ANY$ {
			pContext->markLoadFail();
		}
	}
	else 
#endif
	{
		pContext->markLoadFail();
	}
	return pContext;
}
#endif



/*
java_lang_Class_p JNI_FindClass::findClass(const char* pszSignature) { // no throws  
	int dimension = 0;
	while (*pszSignature == '[') {
		pszSignature ++;
		dimension ++;
	}

	const fastiva_ClassContext* pContext = (fastiva_ClassContext*)
		this->findContext(pszSignature, LOAD);

	if (pContext == ADDR_ZERO) {
		return (java_lang_Class_p)FASTIVA_NULL;
	}
	if (dimension == 0) {
		return pContext->m_pClass;
	}
	else {
		return fastiva.getArrayClass(pContext, dimension);
	}
}
*/
/* 임시로 막음. 필요가 없다(?) 2007.10.
const fastiva_ClassContext* JNI_FindClass::findContext(java_lang_String_p pClassSignature, Action action) {
	unicod *pCode = fm::getUnsafeStringBuffer(pClassSignature);
	int ucs_len = pClassSignature->length();
	int len = fastiva.getUTFLength(pCode, ucs_len);
	char* sig = (char*)alloca(len);
	fastiva.getUTFChars(sig, pCode, ucs_len); 

	return findContext(sig, action);
}
*/

#if 1 // FASTIVA_SUPPORTS_BYTECODE_INTERPRETER
class FileLoader : public fastiva_ResourceLoader {
	java_lang_String_p m_pFilePath;
	java_lang_StringBuilder_p m_sb;
public:
	FileLoader(java_lang_String_p pFilePath) {
		m_pFilePath = pFilePath;
		fm::createGlobalRef(pFilePath);
		m_sb = ADDR_ZERO;
	}

	void init() {
		if (m_sb == ADDR_ZERO) {
			m_sb = FASTIVA_NEW(java_lang_StringBuilder)();//m_pZipFilePath);
			fm::createGlobalRef(m_sb);
			const char* protocol = "file:///";
			for (int i = 0; i < 8; i ++) {
				m_sb->append((unicod)protocol[i]);
			}
			m_sb->append(m_pFilePath);
			m_sb->append((unicod)'/');
		}
	}

	virtual java_lang_String_p getURL(java_lang_String_p rscName) {
		init();

		SYNCHRONIZED$(m_sb);
		m_sb->setLength(m_pFilePath->length() + 9);
		m_sb->append(rscName);
		java_lang_String_p url = m_sb->toString();
		if (fastiva_vm_File_C$__existsImpl(url->substring(8))) {
			return url;
		}
		else {
			return ADDR_ZERO;
		}
	}

	virtual Byte_ap loadResource(java_lang_String_p rscName) {
		FASTIVA_DBREAK(); // MIDP 전용?
		return 0;
		/*
		unicod* filePath = (unicod*)alloca(cpLen + pathLen + 2);
		getAbsolutePath(filePath, resourceName, nameLen)

		FOX_HFILE hFile = fox_file_open(filePath, FOX_READ);
		if (hFile == FOX_HERROR) {
			return ADDR_ZERO;
		}
		int len = fox_file_getSize(hFile);
		Byte_ap pBuff = Byte_A::create$(len);
		Byte_A::Buffer buf(pBuff, 0, len);
		if (fox_file_readFully(hFile, (char*)(jbyte*)buf, len) < 0) {
			pBuff = ADDR_ZERO;
		}
		fox_file_close(hFile);
		return pBuff;
		*/
	}
};



class JarLoader : public fastiva_ResourceLoader {
	java_util_zip_ZipFile_p m_pZipFile;
	//jzfile* m_pZip;
	//unicod* m_pDirectory;
	//int m_cpLen;
	java_lang_String_p m_pZipFilePath;
	java_lang_StringBuilder_p m_sb;

public:
	JarLoader(java_lang_String_p pZipFilePath) {
		m_pZipFile = ADDR_ZERO;
		m_pZipFilePath = pZipFilePath;
		this->m_sb = NULL;
		fm::createGlobalRef(pZipFilePath);
	}

	void init() {
		if (m_sb == ADDR_ZERO) {
			m_sb = FASTIVA_NEW(java_lang_StringBuilder)();//m_pZipFilePath);
			fm::createGlobalRef(m_sb);

			TRY$ {
				m_pZipFile = FASTIVA_NEW(java_util_zip_ZipFile)(m_pZipFilePath);
				fm::createGlobalRef(m_pZipFile);

				const char* protocol = "jar:file://";
				for (int i = 0; i < 11; i ++) {
					m_sb->append((unicod)protocol[i]);
				}
				m_sb->append(m_pZipFilePath);
				m_sb->append((unicod)'/');
			}
			CATCH_ANY$ {
				// ignore.
			}
		}
	}

	virtual java_lang_String_p getURL(java_lang_String_p rscName) {
		init();
		if (m_pZipFile == NULL) {
			return NULL;
		}

		if (m_pZipFile->getEntry(rscName) == FASTIVA_NULL) {
			return (java_lang_String_p)FASTIVA_NULL;
		}

		SYNCHRONIZED$(m_sb);
		m_sb->setLength(m_pZipFilePath->length() + 12);
		m_sb->append(rscName);
		return m_sb->toString();
	}

	virtual Byte_ap loadResource(java_lang_String_p rscName) {
		init();

	    java_util_zip_ZipEntry_p ze = m_pZipFile->getEntry(rscName);
		if (ze == ADDR_ZERO) {
			return ADDR_ZERO;
		}

		int len = (int)ze->getSize();
		Byte_ap pBuff = Byte_A::create$(len);
		java_io_InputStream_p in = m_pZipFile->getInputStream(ze);
		for (int off = 0; off < len;) {
			int cr = in->read(pBuff, off, len - off);
			if (cr == 0) {
				return ADDR_ZERO;
			}
			off += cr;
		}
		return pBuff;
	}
};

fastiva_ResourceLoader* fm::parseResourceLoadingPath(const SYS_TCHAR* classPaths)
{
	if (classPaths == ADDR_ZERO) {
		classPaths = _T(".");
		//return ADDR_ZERO;
	}

	fastiva_ResourceLoader* pFirstLoader = 0;
	fastiva_ResourceLoader* pLastLoader = 0;

	unicod* classPath = (unicod*)alloca(strlen(classPaths)*2 + 2);
	unicod* cp = classPath;
	
	while (true) {
		char ch= *classPaths ++;
		if (ch == ';' || ch == 0) {
			*cp = 0;
			int cp_len = cp - classPath;
			fastiva_ResourceLoader* pLoader;
			FOX_UNICODE_STRING ucs;
			ucs.m_pText = classPath;
			ucs.m_length = cp_len;
            if (fox_file_isDirectoryW(ucs)) {
                pLoader = new FileLoader(fastiva.createStringW(classPath, cp_len));
            }
		    else {
				pLoader = new JarLoader(fastiva.createStringW(classPath, cp_len));
		    }

#ifdef _DEBUG
			if (pLoader == ADDR_ZERO) {
				fox_debug_printf("given classpath (%s) is not dir nor jar.", classPath);
			}
#endif

			cp = classPath;
			if (pLoader != ADDR_ZERO) {
				if (pLastLoader == ADDR_ZERO) {
					pFirstLoader = pLoader;
				}
				else {
					pLastLoader->m_pNextLoader = pLoader;
				}
				pLastLoader = pLoader;
			}
			if (ch == 0) {
				break;
			}
		}
		else {
			*cp ++ = ch;
		}
    }
	return pFirstLoader;
}

Byte_ap fm::loadResource(java_lang_String_p rscName, java_lang_ClassLoader_p pClassLoader) {
	fastiva_ResourceLoader* pResourceLoader = kernelData.g_pResourceLoaderQ;
	while (pResourceLoader != ADDR_ZERO) {
		Byte_ap pData = pResourceLoader->loadResource(rscName);
		if (pData != ADDR_ZERO) {
			return pData;
		}
		pResourceLoader = pResourceLoader->m_pNextLoader;
	}
	return ADDR_ZERO;
}
#endif