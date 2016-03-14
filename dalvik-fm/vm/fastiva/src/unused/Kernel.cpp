#include <precompiled_libcore.h>

/**
* 웹
*/


#define VC_NET

#include <kernel/Kernel.h>
#include <kernel/Runnable.h>
//#include <fastiva/jni/Runtime.h>
#include <fastiva/JppException.h>
#include <kernel/sys.h>
#include <fox/Task.h>
#include <fox/Heap.h>

#include <java/util/Properties.inl>
#include <java/lang/System.inl>
#include <java/lang/StringBuffer.inl>
#include <java/io/PrintStream.inl>
#include <java/io/FileOutputStream.inl>
#include <java/util/Hashtable.inl>
#include <precompiled_libcore.h>
//#include <fastiva/BootstrapClassLoader.inl>
//#include <fastiva/MainThread.inl>

//#include <windows.h> // for CRITICAL_SECTION


	#include <pal/fox_file.h>
	#include <java/lang/Boolean.inl>
	#include <java/lang/Byte.inl>
	#include <java/lang/Character.inl>
	#include <java/lang/Short.inl>
	#include <java/lang/Integer.inl>
	#include <java/lang/Long.inl>
	#include <java/lang/Float.inl>
	#include <java/lang/Double.inl>
#if JPP_JDK_VERSION >= JPP_JDK_CDC
	#include <java/util/Collections.inl>
	#include <java/io/FileDescriptor.inl>
	#include <java/lang/Number.inl>
#endif
	#include <java/lang/ClassLoader.inl>
	#include <java/lang/Thread.h>
	#include <java/lang/ThreadGroup.h>

#if JPP_JDK_VERSION >= JPP_JDK_CDC && !JPP_JDK_IS_ANDROID()
	#include <sun/io/Converters.inl>
	#include <sun/net/www/protocol/file/Handler.inl>
	#include <sun/net/www/protocol/jar/Handler.inl>
#ifdef FASTIVA_J2SE
	#include <javax/swing/text/html/parser/ParserDelegator.inl>
#endif
#endif


#include <stdio.h>

extern jbool FOX_FASTCALL(fox_task_setSystemPriority)(FOX_HTASK hThread, int priority);
extern jbool FOX_FASTCALL(fox_task_setRunningPriority)(FOX_HTASK hThread, int priority);

#if 0 // v3
void FOX_FASTCALL(fox_scheduler_boostPriority)(FOX_HTASK pTask);
void FOX_FASTCALL(fox_scheduler_restorePriority)(FOX_HTASK pTask);
bool FOX_FASTCALL(fox_scheduler_isBoosted)();
#else
#define fox_scheduler_boostPriority //
#define fox_scheduler_restorePriority //
#define fox_scheduler_isBoosted false//
#endif


#include "string.h"


//Kernel  g_kernel;
Kernel kernelData;

/**
* pragma data_seg를 사용하는 아래와 비슷한 예제를 언급하는 문서가
* 있으니, 반드시 읽어야만 코드이해가 가능한 문서이다.
* 2001 jan MSDN Magazine Under the Hood 에 실린
* "Reduce EXE and DLL Size with LIBCTINY.LIB" 이다. 꼭 읽자.
*
* - jay 2003.2.10
*/
#ifdef FASTIVA_TARGET_CPU_i486
	extern "C" {
		extern FASTIVA_EXPORT int FASTIVA_PACKAGE_LIST_HEADER[1];
		extern FASTIVA_EXPORT int FASTIVA_PACKAGE_LIST_FOOTER[1];
	}
	
	#ifdef __GNUC__
		#define FASTIVA_GET_FIRST_PACKAGE()						\
			(fastiva_Package*)(FASTIVA_PACKAGE_LIST_HEADER + 8);
	#elif defined(VC_NET)
		#define FASTIVA_GET_FIRST_PACKAGE()						\
			(fastiva_Package*)(FASTIVA_PACKAGE_LIST_HEADER + 1);
	#else
		#define FASTIVA_GET_FIRST_PACKAGE()						\
			(fastiva_Package*)(FASTIVA_PACKAGE_LIST_HEADER + 2);
	#endif
	#define FASTIVA_GET_ENDOF_PACKAGE()							\
		(fastiva_Package*)(FASTIVA_PACKAGE_LIST_FOOTER);
#else
	extern "C" {
		/* Must turn off <project> "remove unused section" <read only> */
		extern char FX$PKGS$$Base[];
		extern char FX$PKGS$$Limit[];
	}
	#define FASTIVA_GET_FIRST_PACKAGE() (fastiva_Package*)FX$PKGS$$Base
	#define FASTIVA_GET_ENDOF_PACKAGE() (fastiva_Package*)FX$PKGS$$Limit;
#endif


/**====================== body =======================**/



//const fastiva_Instance_T$::VTable$ fastiva_Instance_T$::m_context$ = {0};
/*
void* fm::runtime_getOEMData() {
	//return g_kernel.oemData;
	return 0;
}
*/

void fm::attachThread(java_lang_Thread_p pThread, FOX_HTASK hTask) {
	fastiva_lockGlobalRef(pThread);
	pThread->m_hTask$ = hTask;//fox_task_currentTask();
	hTask->m_pUserData = pThread;
}



void fm::attachNativeTask(fastiva_Task* pCurrTask) {
	KASSERT(pCurrTask != Kernel::g_pSystemTask);
	if (pCurrTask->m_pNext0 == (fastiva_Task*)1) {
		if (pCurrTask->m_pUserData == ADDR_ZERO) {
			java_lang_Thread_p pJavaThread = FASTIVA_ALLOC(java_lang_Thread);
			KASSERT(kernelData.g_pMainThread != ADDR_ZERO);
			pJavaThread->init$();

			// running 중인 Java-Thread 만이 GlockLocking 되어야 한다.
			fm::attachThread(pJavaThread, pCurrTask);
		}
		fox_mutex_lock(kernelData.g_pLock);
		pCurrTask->m_pNext0 = kernelData.g_pTaskQ_interlocked;
		kernelData.g_pTaskQ_interlocked = pCurrTask;
		fox_mutex_release(kernelData.g_pLock);

		// 2011.0913 Attach 된 native task 도 UserThread의 하나로 본다(?);
		int cntU = fox_util_inc32((uint*)&kernelData.g_userThreadCount_interlocked);
	}
}


#if defined(FASTIVA_J2SE) || defined(FASTIVA_CDC)
void fm::initSystemThread(java_lang_ThreadGroup_p pSystemGroup, java_lang_Thread_p pThread) {

	pThread->set__group(pSystemGroup);
	//setPriority(priority);
    //    InheritableThreadLocal.bequeath(parent, this);
	// v3 그룹이 꼭 존재해야 하는가?
	pSystemGroup->add(pThread);
}
#endif

void fm::initMainThread() {
	// daemon thread를 만든다.
	java_lang_ThreadGroup_C$* pThreadGroupClass = java_lang_ThreadGroup_C$::importClass$();
	kernelData.g_pMainThread = FASTIVA_NEW(java_lang_Thread)(pThreadGroupClass->g_mMain, fastiva.createAsciiStringA("main", 4), java_lang_Thread__NORM_PRIORITY, true);


#if JPP_JDK_VERSION > JPP_JDK_CDC
	kernelData.g_pMainThread->set__contextClassLoader(java_lang_ClassLoader_C$::importClass$()->getSystemClassLoader());

#if FASTIVA_NULL_ADDR != 0
	// v3 그룹이 null이어도 문제가 없는가?
	pThread->set__inheritedAccessControlContext((java_security_AccessControlContext_p)FASTIVA_NULL);
	pThread->set__target((java_lang_Runnable_p)FASTIVA_NULL);
#endif
#if !JPP_JDK_IS_ANDROID()
	pThread->set__lock(FASTIVA_NEW(java_lang_Object)());
#endif

#if 0 // Z-TEMP 2007.09 #ifndef FASTIVA_J142
	//*
	pThread->set__threadLocals(java_util_Collections_C$::importClass$()->g_EMPTY_MAP);
	pThread->set__inheritableThreadLocals(java_util_Collections_C$::importClass$()->g_EMPTY_MAP);
	/*/
	pThread->set__threadLocals((java_lang_ThreadLocal$ThreadLocalMap*)FASTIVA_NULL);
	pThread->set__inheritableThreadLocals((java_lang_ThreadLocal$ThreadLocalMap*)FASTIVA_NULL);
	//*/
#endif
#endif

	// running 중인 Java-Thread 만이 GlockLocking 되어야 한다.
	// 현 Thread가 속한 ThreadGroup도 항상 Reachable 상태가 된다.
	//pThreadGroup->add(pThread);
	fm::attachThread(kernelData.g_pMainThread, fastiva_getCurrentTask());

	// system이란 이름을 가진 thread-group이 생성된다.
	//java_lang_ThreadGroup_C$* pThreadGroupClass = java_lang_ThreadGroup_C$::importClass$();
	kernelData.g_pMainThread->m_group = pThreadGroupClass->g_mMain;
	//fastiva_lockGlobalRef(pThreadGroup);
}


jlonglong fastiva_currTimeMillis = -1;
//jbool Kernel::g_inInitializing;
fastiva_Task* Kernel::g_pSystemTask;
//fastiva_Task* Kernel::g_pSystemProxy;
java_lang_Object_p Kernel::g_pSysEvent;
AtExitProc* Kernel::g_pAtExitProcQ;


/** initKernel
	@0. fox_task를 초기화한다.
    @1. Kernel에서 사용하는 각종 global 변수를 초기화한다.
	@2. System-task(GC-Task를 초기화하고, start한다.)
	@3. fastiva-classLooder를 초기화한다.
	@4. static-link 된 package 들을 등록한다.
	@5. 적절한 순서에 맞추어, 필수 class 들을 미리 import하고,
	@6. java_lang_System을 초기화한다. (각종 property 들).
*/
extern void FOX_FASTCALL(gc_task)(void* param);

jbool fm::initKernel(fastiva_ModuleInfo* pAppModule) {
 

	Kernel::g_pAtExitProcQ = ADDR_ZERO;
	memset(&kernelData, 0, sizeof(kernelData));
	jlonglong currTimeMillis = -1;
	//fastiva_Task* pGCTask = 
	Kernel::g_pSystemTask = (fastiva_Task*)fox_kernel_createTaskContext();
	fox_task_create_ex(gc_task, ADDR_ZERO, 0, Kernel::g_pSystemTask, 0);

	kernelData.g_aPackageSlot = (fastiva_PackageSlot*)sys_heap_virtualAlloc(NULL, sizeof(fastiva_PackageSlot)*1024);

	// WARNING!!! the initializing order is very important !!!

	// v3 KASSERT(java_lang_Object_T$::g_context$.context.m_pSuperContext == ADDR_ZERO);
	// java_lang_Thread_p pSystemThread = fm::attachNativeTask(Kernel::g_pSystemTask);
	// pSystemThread->m_priority = (5);
	fox_mutex_init(kernelData.g_pLock);
	kernelData.g_ivtableLock = fox_semaphore_create();

#if (JPP_JNI_EXPORT_LEVEL > 0)
	fox_mutex_init(kernelData.g_pContextIDLock);
	//fox_mutex_init(kernelData.g_pPackageListLock);
	kernelData.stringPool.init();
	kernelData.g_interpreterModule.m_hDLL = (void*)-1;
#endif

#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER
	fastiva_Runtime* pFastivaRuntime = pModule->m_pRuntime;
	if (pFastivaRuntime != ADDR_ZERO) {
		initThunk(pFastivaRuntime);
	}
#endif

#ifdef _WIN32
	// 1.0+ddE // 두자리수 exponent 표현 선택.
#ifndef UNDER_CE
	_set_output_format(_TWO_DIGIT_EXPONENT);
#endif
#endif


	FASTIVA_BEGIN_MANAGED_SECTION(0);
	TRY$ {
		fm::initClassLoader();
		java_lang_Object_C$::initStatic$();

		fm::initPrimitiveClasses();


		//_asm int 3;
		/*
		fastiva_Package* pPackage	 = FASTIVA_GET_FIRST_PACKAGE();
		fastiva_Package* pEndPackage = FASTIVA_GET_ENDOF_PACKAGE();
 		while (pPackage < pEndPackage) {
			fm::registerComponent(pPackage, ADDR_ZERO);
	#ifdef __GNUC__
			pPackage = (fastiva_Package*)((char*)pPackage + 0x40);
	#else
			pPackage = (fastiva_Package*)((char*)pPackage + 0x28);
	#endif
		}
		*/


		// String class 를 초기화한다.
		// PreloadableClass 중에 StringConstant 를 가진 class 의 경우, 
		// 해당 field assign 시 global publishing 이 발생한다.
		// 그 전에 String class의 m_pVTable 이 초기화되어야하만다.
		java_lang_String_C$::importClass$();

		g_pLibcoreModule->initStringPool();
		g_pLibcoreModule->linkPreloadableClasses(true);

		/** java_lang_String은 linkClass 도중 String-Contant를 생성하기 위해
		필요하다. 예외처리가 필요하다.
		*/
		java_lang_Class_C$::importClass$();

		g_pLibcoreModule->internStringPool();

		/*
		java_lang_String_p aInitInternBuffer[MAX_INIT_INTERN];
		kernelData.g_aInitInternBuffer = aInitInternBuffer;

		java_lang_String_T$ tStr;
		java_lang_String_T$::getRawContext$()->m_pClass->obj.vtable$ = *(void**)&tStr;
		fm::linkClass(java_lang_String_T$::getRawContext$()->m_pClass);
		*/
		Kernel::g_pSysEvent = FASTIVA_NEW(java_lang_Object)();
		
		fm::initSystemException();
		
		java_lang_Class_C$::importClass$();
		java_lang_Thread_C$::importClass$();
		java_lang_System_C$* pSystemC = java_lang_System_C$::importClass$();
		java_lang_StringBuffer_C$::importClass$();
		java_util_Hashtable_C$::importClass$();
		java_lang_ClassLoader_C$::importClass$();



		fastiva_lockGlobalRef(Kernel::g_pSysEvent);
//		fastiva_lockGlobalRef(pSystemThread);
		fastiva_lockGlobalRef(kernelData.g_pInternedString);

#ifdef _WIN32
#if JPP_JDK_VERSION >= JPP_JDK_CDC
		java_io_FileDescriptor_C$* pStatic = java_io_FileDescriptor_C$::importClass$();
		pStatic->g_in->m_descriptor = (int)fox_file_getStandardHandle(FOX_STANDARD_INPUT);
		pStatic->g_out->m_descriptor = (int)fox_file_getStandardHandle(FOX_STANDARD_OUTPUT);
		pStatic->g_err->m_descriptor = (int)fox_file_getStandardHandle(FOX_STANDARD_ERROR);
#else 
		java_io_FileOutputStream_p out = FASTIVA_NEW(java_io_FileOutputStream)((int)fox_file_getStandardHandle(FOX_STANDARD_OUTPUT));
		//java_io_FileOutputStream_p out = FASTIVA_NEW(java_io_FileOutputStream)((int)fox_file_getStandardHandle(FOX_STANDARD_OUTPUT));
		pSystemC->set__props(FASTIVA_NEW(java_util_Properties)());
		pSystemC->set__out(FASTIVA_NEW(java_io_PrintStream)(out));
		pSystemC->set__err(FASTIVA_NEW(java_io_PrintStream)(out));
			//(int)fox_file_getStandardHandle(FOX_STANDARD_OUTPUT);
#endif
#endif

	fm::initMainThread();

	{
		/* 2011.10.25 
		 * ClassCache 사용방식이 변경되어 bootstrap 단계에서 ClassCache 가 초기화되지 않는 문제가 있다.
		 * ClassCache.initStatic() 내의 LangAccess.setInstance() 가 수행되도록 하여야 한다.
		 */
		java_lang_ClassCache_C$::importClass$();
	}

	if (pAppModule != NULL) {
		((fastiva_Module*)pAppModule)->initExteranlModule();
	}

#if defined(FASTIVA_J2SE) || defined(FASTIVA_CDC)

		// Z-TEMP 2007.09 java_lang_System_C$::getRawStatic$()->initializeSystemClass();
		/*
		{
			g_pLibcoreModule->linkPreloadableClasses();
		}

		if (pAppModule != ADDR_ZERO) {
			((fastiva_Module*)pAppModule)->init();
		}
		*/


		java_lang_Boolean_C$::importClass$();
		java_lang_Byte_C$::importClass$();
		java_lang_Character_C$::importClass$();
		java_lang_Short_C$::importClass$();
		java_lang_Integer_C$::importClass$();
		java_lang_Long_C$::importClass$();
		java_lang_Float_C$::importClass$();
		java_lang_Double_C$::importClass$();
		java_lang_Number_C$::importClass$();
#ifndef FASTIVA_CLDC
		//J131 Library에서 .exe로 Link가 되지 않아 forName에서 ClassNotFoundException이 
		//발생하므로 임시로 여기에서 import한다. main 함수가 있는 곳으로 이동되어야 한다.
		//sun_io_Converters_C$::importClass$();
#ifdef FASTIVA_J2SE
		javax_swing_text_html_parser_ParserDelegator_C$::importClass$();
#endif
// Z-TEMP 2007.09		sun_net_www_protocol_file_Handler_C$::importClass$();
// Z-TEMP 2007.09		sun_net_www_protocol_jar_Handler_C$::importClass$();
#endif
		//findNativeMethod_MD에서 java_lang_ClassLoader::findNative0$를 이용하여
		//Native를 찾기 위해서 ClassLoader가 import되어야 한다.
		java_lang_Class_p pClassLoader = (java_lang_Class_p)java_lang_ClassLoader_C$::importClass$();
		java_lang_ClassLoader_C$::getRawStatic$()->getSystemClassLoader();
		
#if !JPP_JDK_IS_ANDROID() 
        java_util_Properties_p props = FASTIVA_NEW(java_util_Properties)();
		java_lang_System_C$::getRawStatic$()->initProperties(props);
		java_lang_System_C$::getRawStatic$()->set__props(props);
#endif
		//sun_awt_NativeLibLoader::loadLibraries()에서 awt.dll을 Loading하기 때문에
		//우선적으로 Loading한다. java_awt_XXX가 import된 후에 sun_awt_NativeLibLoader::loadLibraries()가
		//불리기 때문에 여기서 미리 Loading을 한다.
		//sun_awt_NativeLibLoader_C$::importClass$();
		//sun_awt_NativeLibLoader::loadLibraries();
 		//java_lang_System::loadLibrary(fastiva.createString("jpeg"));
		
#endif

#if (JPP_JNI_EXPORT_LEVEL > 0)
		int idx = kernelData.g_utfHashTable.addRefName("<init>");
		KASSERT(idx == JPP_REF_NAME_ID::init$$);
		idx = kernelData.g_utfHashTable.addRefName("<cinit>");
		KASSERT(idx == JPP_REF_NAME_ID::initStatic$$);
		idx = kernelData.g_utfHashTable.addRefName("main");
		KASSERT(idx == JPP_REF_NAME_ID::main$);
#endif

#if FASTIVA_SUPPORTS_JAVASCRIPT
		fm::initJScript();
#endif

		//fox_task_startScheduler();

	}
	CATCH_ANY$ {
		return false;
	}
	FASTIVA_END_MANAGED_SECTION();
gg:
	return true;
}

#if JPP_JNI_EXPORT_LEVEL == 0
jlonglong fastiva_Runtime::invokeJNI(
	void* pObj,
	void* pMethodName
) {
	FASTIVA_DBREAK();
	return 0;
}
#endif


const int JNI_FIELD_USED	= 0x8000;
const int JNI_METHOD_USED	= 0x4000;

void fastiva_kernel_exitFastiva(int status, bool shutdown) {
	extern void pal_sys_exit(int status);

	//fox_task_stopScheduler();

	#ifdef PRINT_IMPORTED_CLASS_LIST
		void FOX_FASTCALL(fox_freeLibrary)(const void* hDLL);
		fastiva_Package* pBundle = (fastiva_Package*)kernelData.g_pPackageQ;
		while (pBundle != ADDR_ZERO) {
			const fastiva_Package::ContextInfo* pContextInfo = pBundle->m_aContextInfo;
			printf("#package %s\n", pBundle->m_pName);
			for (int cnt = pBundle->m_cntContext; cnt-- > 0; pContextInfo ++) {
				const fastiva_ClassContext* pContext = pContextInfo->m_pContext;
				if (pContext->m_pClass->isLinked$()) {
					int jniLock = fox_ClassLoader::getJniLockCount(pContext);
					if (jniLock >= 0x4000) {
						puts("#define FASTIVA_SUPPORT_JNI");
					}
					printf("\t#include \"%s/%s.cpp\"\n", pBundle->m_pName, pContext->m_pClassName);
					if (jniLock >= 0x4000) {
						puts("#undef FASTIVA_SUPPORT_JNI");
					}
				}
				else {
					printf("//\t#include \"%s/%s.cpp\"\n", pBundle->m_pName, pContext->m_pClassName);
				}
			}

			if (pBundle->m_hDLL != ADDR_ZERO) {
				const void* hDLL = pBundle->m_hDLL;
				pBundle = (fastiva_Package*)pBundle->m_pNext;
				fox_freeLibrary(hDLL);
			}
			else {
				pBundle = (fastiva_Package*)pBundle->m_pNext;
			}
		}
		kernelData.g_pPackageQ = ADDR_ZERO;
		printf("#-----------------------\n");
		printf("#-----------------------\n");
		fflush(stdout);
	#endif

	if (shutdown) {
		FASTIVA_DBREAK();
		pal_sys_exit(status);
	}
}


/**
* constructor 추후 GC 구조까지 반영한 형태로 변경한다.
*/
extern "C" struct JNIEnv* getJNIEnv();

fox_Task* fox_kernel_createTaskContext(int sizContext) {
	//KASSERT(sizContext <= (int)sizeof(fastiva_Task));
	if (sizContext < (int)sizeof(fastiva_Task)) {
		sizContext = (int)sizeof(fastiva_Task);
	}
	fastiva_Task* pTask = (fastiva_Task*)fox_heap_malloc(sizContext);
	memset(pTask, 0, sizContext);
	pTask->init();
#ifdef FASTIVA_SUPPORT_JNI
	#if JPP_JDK_VERSION >= JPP_JDK_CDC
		pTask->fastiva_env_vtable$ = *(void**)getJNIEnv();//fm::getVM_ENV();
	#endif
#endif
	pTask->m_pNext0 = (fastiva_Task*)1;
	pTask->m_scanState = NOT_SCANNED;
	pTask->setState((fox_TaskState)NEW);
	return pTask;
}

/*
FOX_NAKED void fm::invokeInterface(
	fm::Interface_p pInterface,
	int virtual_offset
) {
	_asm {
		mov eax, [ecx];
		sub ecx, [eax];
		mov edx, [eax + edx];
		mov eax, [ecx];
		jmp dword ptr [eax+edx];
	}
}
*/

/*
void fm::eovf_$$() {
}
*/

/*
void fm::lockScheduler() {
	java_lang_Thread_p pThread = kernelData.g_pThreadQ;
	java_lang_Thread_p pPrevThread = ADDR_ZERO;
	while (pThread != ADDR_ZERO) {
		FOX_HTASK hThread = pThread->m_hTask;
		if (hThread == ADDR_ZERO) {
			if (pPrevThread != ADDR_ZERO) {
				pPrevThread->m_pNext = pThread->m_threadQ;
			}
			else {
				kernelData.g_pThreadQ = pThread->m_threadQ;
			}
		}
		else {
			pPrevThread = pThread;
		}
		pThread = pThread->m_threadQ;
	}

#ifdef FASTIVA_USE_NATIVE_THREAD
	::EnterSystemCriticalSection();
	void* pCurrNative = fox_Thread::getCurrentStackEnd();
	Thread* pThread = kernelData.g_pThreadQ;
	while (pThread != ADDR_ZERO) {
		fox_Thread* pNativeThread = (fox_Thread*)pThread->m_hTask;
		if (pNativeThread != ADDR_ZERO 
		&& pNativeThread != 1 // not started
		&& pNativeThread != pCurrNative
		&& pNativeThread->m_pMonitor == ADDR_ZERO 
		&& pNativeThread->m_pWaitObj == ADDR_ZERO) {
			pNativeThread->suspend();
		}
		pThread = pThread->m_pNext;
	}
	::LeaveSystemCriticalSection();
#endif
}
*/
/*
void fm::wakeSystemThread(int priority) {
	fastiva_Synchronize gc_lock(Kernel::g_pSysEvent);
	if (priority <= 0) {
		fox_Task* pCurrTask = (fox_Task*)fox_task_currentTask();
		priority = pCurrTask->getRunningPriority();
	}
	fox_Task* pSystemTask = (fox_Task*)Kernel::g_pSystemTask;
	if (pSystemTask->getRunningPriority() < priority) {
		fox_task_setRunningPriority(Kernel::g_pSystemTask, priority);
	}
	Kernel::g_pSysEvent->notify();
}
*/

/*
void fm::unlockScheduler() {
#ifdef FASTIVA_USE_NATIVE_THREAD
	void* pCurrNative = fm::getCurrentStackEnd();
	Thread* pThread = kernelData.g_pThreadQ;
	while (pThread != ADDR_ZERO) {
		Kernel* pNativeThread = (Kernel*)pThread->m_hTask;
		if (pNativeThread != ADDR_ZERO 
		&& pNativeThread != 1
		&& pNativeThread != pCurrNative
		&& pNativeThread->m_pMonitor == ADDR_ZERO 
		&& pNativeThread->m_pWaitObj == ADDR_ZERO) {
			pNativeThread->resume();
		}
		pThread = pThread->m_pNext;
	}
#endif
}
*/

extern "C" void fox_task_pushStackFrame() {
}

extern "C" void fox_task_setTopStackFrame() {
}

/*
jbool* Bool_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (jbool*)(void*)this; 
}							

jbyte* Byte_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (jbyte*)(void*)this; 
}							

unicod* Unicod_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (unicod*)(void*)this; 
}							

jshort* Short_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (jshort*)(void*)this; 
}							

jint* Int_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (jint*)(void*)this; 
}							

jlonglong* Longlong_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (jlonglong*)(void*)this; 
}							

jfloat *Float_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (jfloat *)(void*)this; 
}							

jdouble* Double_A::cast_array_to_ex$(const void*, fm::Array(*)[1]){
		return (jdouble*)(void*)this; 
}							
*/


/**====================== end ========================**/