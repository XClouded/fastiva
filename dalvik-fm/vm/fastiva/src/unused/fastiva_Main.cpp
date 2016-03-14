#include <precompiled_libcore.h>

#include <kernel/Kernel.h>
#include <kernel/Runnable.h>
#include <fox/Task.h>
#include <fox/Sys.h>
// v3.2206 
#include <java/lang/Throwable.inl>
#include <java/lang/String.inl>
#include <fastiva_main.h>

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h> 
#else
	#include <limits.h>
	#include <unistd.h>
	typedef char TCHAR;
	#define MAX_PATH	PATH_MAX
#endif
#include <fastiva_malloc.h>

#include <stdio.h>
#include <string.h>

typedef void (FOX_FASTCALL(*MAIN_FUNC))(void*, java_lang_String_ap);

#define FASTIVA_SUPPORTS_MIDP	0

#ifdef FASTIVA_J142
	#define printStackTrace0	printStackTrace
#endif
//-immutable_inline -cp C:/fastiva/fastrans/projects/wish/classes -ad d:/java2c/wish/classes -r java

//extern "C" jbool FOX_FASTCALL(fm::runtime_initFastiva)(void* heapArea, int heapSize, void* oemData);
//extern int fastiva_main(const TCHAR* mainClassName, java_lang_String_ap aStr);

//void fm::MainThread::run() {}

class fastiva_MainThread2 : public java_lang_Thread {
	public: FOX_RESTRICT_API void* operator new (size_t siz) {
		return fastiva.allocateEx(
			(fastiva_InstanceContext*)getRawContext$(), siz);
    }

	public: virtual void run();

};

fm::Command g_command;

const TCHAR* fastiva_kernel_getClassPath() {
	return g_command.szClassPath;
}

//static int fastiva_MainThread2 = 0;
static int err_code;


bool fastiva_pumpEvent();

typedef void (FOX_FASTCALL(*MAIN_FUNC2))(java_lang_String_ap);

void fastiva_MainThread2::run(
) {
	TRY$ {
		extern int g_debugRookieMark;
		//const fastiva_InstanceContext* pContext = 
		//	g_command.pMainClass->m_pContext$->toInstanceContext();

		// 임시로 아래 코드 대체;
        if (!FASTIVA_SUPPORTS_MIDP) {
    		((MAIN_FUNC2)g_command.pfnMain)(g_command.aArg);
        }
        else {
		    ((MAIN_FUNC)g_command.pfnMain)(g_command.pMainClass, g_command.aArg);
        }
		g_debugRookieMark += 0x100;
	}
	CATCH_ANY$ {
		fox_printf("!!! Uncaught exception catched in MainThread::run: \n");
		fm::printError(catched_ex$);
	}
};


jbool FOX_FASTCALL(fastiva_GC_init)(void* pHeap, int heapSize);


/** fastiva_init() 
	@0. 필요한 경우, System을 초기 상태로 복구한다.
	@1. heap을 초기화한다.
	@2. class-path 등, VM-Environment 를 초기화한다.
	@3. VM-내부(Kernel)을 초기화한다.
*/

static FOX_BOOL g_initialized = false;

int FASTIVA_DLL_API fastiva_init(
	fastiva_ModuleInfo* pAppModule
	//fastiva_PackageInfo* pAppPackageSlot, 
	//int cntAppPackageSlot,
) {
#ifdef UNDER_CE
	#define FASTIVA_HEAP_SIZE	(1024*1024*2)
#elif defined(_WIN32)
	#define FASTIVA_HEAP_SIZE	(1024*1024*64)
#else
	// android 용으로 400M 설정.
	#define FASTIVA_HEAP_SIZE	(1024*1024*260)
#endif
	if (g_initialized) {
		return 0;
	}

	// main 에서도 호출되고 이중 호출됨. 남겨 둘 것.
	fox_pal_init();

	if (!fastiva_GC_init(0, FASTIVA_HEAP_SIZE)) {
		return -1;
	}

	g_initialized = true;

	//g_command.szClassPath = pszClassPath;
	/* v3
		pszClassPath를 parsing하여 aClassPath를 생성한다.
		Invalid-path를 제거하여 Performance를 높이도록 한다.

			while (*szClassPath == ';') {
				szClassPath ++;
			}
			const char* cp = szClassPath;
			while (true) {
				if ((cp = strchr(cp, ';')) == NULL) {
					break;
				}
				cntPath ++;
				while (*(++cp) == ';') {
				}
			}
			char** aPath = (char**)fastiva_malloc(strlen(szClassPath + cntPath*4));
			char* dcp = &aPath[cntPath];
			const char* cp = szClassPath;
			while (true) {
				*aPath++ = cp;
				while (*cp == ';') {
					cp ++;
				}
				if ((cp = strchr(cp, ';')) == NULL) {
					break;
				}
				while (*(++cp) == ';') {
				}
			}
			*aPath = ADDR_ZERO;
	*/

	if (!fm::initKernel(pAppModule)) {
		sys_util_showFatalError(_T("Fatal Error"), _T("Fastiva initialization fail"));
		return -1;
	}

	//kernelData.g_pResourceLoaderQ = fm::parseResourceLoadingPath("rsc.zip");

	sys_task_setSystemIdleTask(Kernel::g_pSystemTask);

	// 2011.0913 fastiva_start_app 에서 옮겨옴.
	fox_task_resume(Kernel::g_pSystemTask);
	return 0;
}


static TCHAR jre_path[MAX_PATH] = _T("");

void sys_util_setJrePath(const TCHAR* pszFilePath) {
	sys_util_canonicalize((TCHAR*)pszFilePath, jre_path, MAX_PATH);
}

const TCHAR* sys_util_getJrePath() {
	return jre_path;
}

const TCHAR* sys_util_getClassPath() {
	return g_command.szClassPath;
}


int FASTIVA_DLL_API fastiva_start_app(int argc, const TCHAR* argv[], const TCHAR* pszClassPath, void* pfnMain) {


	TRY$ {

		java_lang_String_ap aStr;
		if (argc <= 0) {
			aStr = java_lang_String_A::create$(0);
		}
		else {
			aStr = java_lang_String_A::create$(argc);
			for (int i = 0; i < argc; i ++) {
#ifdef UNICODE
				aStr->set$(i, fastiva.createStringW(argv[i], _tcslen(argv[i])));
#else
				aStr->set$(i, fastiva.createStringA(argv[i], strlen(argv[i])));
#endif
			}
		}

		fastiva_lockGlobalRef(aStr);
		g_command.aArg = aStr;
		//g_command.pMainClass = fm::linkClass(pMainClassContext->m_pClass);
		g_command.pMainThread = new fastiva_MainThread2();//(fastiva_MainThread2*) FASTIVA_NEW(fastiva_MainThread2)();
		g_command.pMainThread->init$();
		g_command.pfnMain = pfnMain;
	#if defined(FASTIVA_J2SE) || defined(FASTIVA_CDC)
		g_command.pMainThread->setName(fastiva.createString("main"));
	#endif
		g_command.pMainThread->start();
	}
	CATCH_ANY$ {
		return false;
	}
/* 2011.0913 fastiva_init 로 위치 이동
	// gc_task를 시작한다.
	fox_task_resume(Kernel::g_pSystemTask);
*/
	return true;
}

/**
 argv를 분석하여, 
 @1. 시스템을 초기화하고(fastiva_init()), 
 @2. main-application이 지정된 경우, 해당 Application을 start한다.
*/
#if FASTIVA_SUPPORTS_MIDP
void FOX_FASTCALL(fastiva_midp_app_main)(void* pMainClass, java_lang_String_ap args);
#endif

int FASTIVA_DLL_API fastiva_start(int argc, const TCHAR* argv[]) {

#ifdef _JPP
	puts("--------------------------------------------------------------");
	puts("  Java to C++ Translator, Fastrans v1.0.0 [Build 2002-11-10]");
	puts("  Copyright(c) 2002-2003 InterWise Corp. All rights reserved");
	puts("--------------------------------------------------------------");
#endif

	sys_util_setExecutedFilePath(argv[0]);
	argc --;
	argv ++;

	const TCHAR* szClassPath = ADDR_ZERO;
	const TCHAR* arg;
	bool isMIDP = false;

	while (argc > 0 && (arg = *argv)[0] == '-') {
		if (argc <= 1) {
show_option_error:
			sys_util_showFatalError(_T("Option Error"), _T("Option valule missing"));
			return -1;
		}

		int cntToken = 2;
		if (strcmp(arg, _T("-cp")) == 0) {
			szClassPath = argv[1];
		}
		else 
		if (strcmp(arg, _T("-home")) == 0) {
			void sys_util_setJrePath(const TCHAR* jre_path);
			sys_util_setJrePath(argv[1]);
		}
		else 
		if (strcmp(arg, _T("-midp")) == 0) {
			isMIDP = true;
			cntToken = 1;
		}
		else 
		if (strcmp(arg, _T("-cwd")) == 0) {
			sys_util_setCurrentWorkingDirectory(argv[1]);
		}
		else {
			sys_util_showFatalError(_T("Unknown Option"), arg);
			goto show_option_error;
		}
		argv += cntToken;
		argc -= cntToken;
	}

	//g_command.argc = argc - 1;
	const TCHAR* pszMainClassPath = *argv++;
	argc --;




	const fastiva_ClassContext* pMainClassContext;
	MAIN_FUNC pfnMain;

	FASTIVA_BEGIN_MANAGED_SECTION(0);
	#if (1 || JPP_JDK_VERSION <= JPP_JDK_MIDC) 
	{
		#if FASTIVA_SUPPORTS_MIDP
			if (false) { // 2011.1023 임시로 막음.
				//pMainClassContext = FASTIVA_RAW_CLASS_CONTEXT_PTR(com_sun_midp_main_MIDletSuiteLoader);
			}
			pfnMain = (MAIN_FUNC)fastiva_midp_app_main;
		#endif
	}
	#else 
	{
		JNI_FindClass fc;
		if (pszMainClassPath == ADDR_ZERO) {
			sys_util_showFatalError(_T("Main Application is Not specified"), _T("Fatal Error"));
			return -1;
		}

		kernelData.g_pResourceLoaderQ = fm::parseResourceLoadingPath(szClassPath);

		pMainClassContext = fc.loadContext_dot(pszMainClassPath);
		if (pMainClassContext == ADDR_ZERO) {
			sys_util_showFatalError(_T("Application Not Found"), pszMainClassPath);
			return -1;
		}


		JNI_FindMethod fm;
		fm.init("main", "([Ljava/lang/String;)V");
		fm.m_accessFlags = ACC_PUBLIC$ | ACC_STATIC$;

		const fastiva_Method* mainMethod = pMainClassContext->getDeclaredMethod(&fm);
		if (mainMethod == ADDR_ZERO) {
			sys_util_showFatalError(_T("main() method Not Found"), pszMainClassPath);
			return -1;
		}
		pfnMain = (MAIN_FUNC)mainMethod->getMethodAddr(NULL, pMainClassContext->m_pClass);
	}
	#endif

	if (false) { // 2011.1023 임시로 막음.
		g_command.pMainClass = fm::importClass(pMainClassContext);
	}
	fastiva_start_app(argc, argv, szClassPath, (void*)pfnMain);
	FASTIVA_END_MANAGED_SECTION();
	return 0;
}

int  fastiva_waitExit() {
	while (kernelData.g_userThreadCount_interlocked > 0) {
#ifdef _WIN32
		Sleep(1000);
#else
		sleep(1); // 초단위.
#endif
	}
	return 0;
}

java_lang_Class_p FOX_FASTCALL(jpp_getPrimitiveArrayClass)(unicod var_type, int dimension) {
	// Jpp를 위하여 추가된 것이다.
	const fastiva_ClassContext* pContext = fm::getPrimitiveContextBySig(var_type);
	return fastiva.getArrayClass(pContext, dimension);
}



//===========================================================================//
//===========================================================================//

bool fastiva_pumpEvent(void) {
#if FASTIVA_SUPPORTS_MIDP
	void FOX_FASTCALL(midp_check_events)(jlonglong timeout);

	for (;;) {
        jlonglong timeout = 5000;//JVM_TimeSlice();
        if (timeout <= -2) {
            break;
        } else {
            int blocked_threads_count;
            //JVMSPI_BlockedThreadInfo * blocked_threads;

            //blocked_threads = SNI_GetBlockedThreads(&blocked_threads_count);
            midp_check_events(timeout);
        }
    }
	//int res = OzWnd::doMessageLoop();
#endif
	return false;
}




struct APP_PARAM {
	int argc;
	const TCHAR** argv;
	fastiva_ModuleInfo* pAppModule;
	void* pfnMain;
};
static fox_Semaphore* g_appTerminated;


static int FOX_FASTCALL(app_crt0)(APP_PARAM* pAppParam) {

	fastiva_init(pAppParam->pAppModule);

	if (FASTIVA_SUPPORTS_MIDP || JPP_JDK_VERSION <= JPP_JDK_MIDC) {
		/*
		여기서 fm::initKNI()를 호출하면, Window를 생성하는 쓰레드와 message loop를 도는 쓰레드가
		같아지면서, 기본 윈도우 메시지들이 메시지루프로 전달된다.
		문제는 이 경우 다른 쓰레드에서 InvalidateRect()를 호출해서, WndProc으로 WM_PAINT가 전달되지 않는다.
		*/
#if FASTIVA_SUPPORTS_MIDP
		fm::initKNI();
#endif
	}

	if (pAppParam->pfnMain == ADDR_ZERO) {
		fastiva_start(pAppParam->argc, pAppParam->argv);
#if		FASTIVA_SUPPORTS_MIDP
	    fastiva_pumpEvent();
#endif
	}
	else {
		FASTIVA_BEGIN_MANAGED_SECTION(0);
		sys_util_setExecutedFilePath(pAppParam->argv[0]);
		pAppParam->argc --;
		pAppParam->argv ++;
		kernelData.g_pResourceLoaderQ = fm::parseResourceLoadingPath(_T("resource.jar"));
		fastiva_start_app(pAppParam->argc, pAppParam->argv, _T("resource.jar"), pAppParam->pfnMain);
		FASTIVA_END_MANAGED_SECTION();
	    fastiva_waitExit();
	}

	fox_semaphore_release(g_appTerminated);

	return 0;
}


int fastiva_main(
	int argc, 
	const TCHAR* argv[], 	
	fastiva_ModuleInfo* pAppModule,
	void* pfnAppMain
) {

	//void* pHeap = fox_heap_malloc(1024*1024);


	//int start_t = ::GetTickCount();
	fox_pal_init();

	g_appTerminated = fox_semaphore_create();

	APP_PARAM appInfo = { argc, argv, pAppModule, pfnAppMain };

#ifndef UNDER_CE
	app_crt0(&appInfo);
#else
	fox_semaphore_lock_GCI(g_appTerminated);

	FOX_HTASK hTask = fox_task_create((FOX_TASK_START_ROUTINE)app_crt0, &appInfo, 5);
	fox_task_resume(hTask);

	fox_semaphore_lock_GCI(g_appTerminated);
	fox_semaphore_release(g_appTerminated);
	fox_semaphore_destroy(g_appTerminated);
#endif

	//int end_t = ::GetTickCount();
	//int elapsed = end_t - start_t;
	return 0;//res;
}




#ifdef _DEBUG
extern "C" {
/* 현재 _DEBUG 모드에서도 /MTd가 아닌 /MT 로 Compile한다.
이에 따라, OutputDebugString과 관계된 Link오류를 막기 위해
아래의 두 함수를 추가한다.
*/
void __cdecl _CrtDbgReport() {
	FASTIVA_DBREAK();
}

void __cdecl _CrtDbgReportW() {
	//FASTIVA_DBREAK();
}
};
#endif

#if 0
int __cdecl _tmain(int argc, const TCHAR* argv_org[]) {
	return fastiva_main(argc, argv_org,
		NULL, &g_fastivaRuntime, ADDR_ZERO);
}

#endif

