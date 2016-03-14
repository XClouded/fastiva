//=#include <kernel/kernel.h>

#include <Fastiva.h>

#ifdef _WIN32
	#include <tchar.h>
#else
	typedef char TCHAR;
	#ifndef _T
    #define _T(txt) txt
#endif
#endif

extern "C" {
int FASTIVA_DLL_API fastiva_main(
	int argc, 
	const TCHAR* argv[], 	
	fastiva_Module* pAppModule,
	void* pfnMain
);

int FASTIVA_DLL_API fastiva_init(
	fastiva_Module* pAppModule
);
};

extern FASTIVA_RUNTIME_EXPORT void FASTIVA_MODULE_NAME(initModuleInfo)();

#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER
	fastiva_Module* fastiva_getApplicationModule();
#else 
	inline fastiva_Module* fastiva_getApplicationModule() { return ADDR_ZERO; }
#endif



#ifdef UNDER_CE
#define FASTIVA_APP_MAIN()													\
	int WINAPI WinMain(														\
		HINSTANCE hInstance,												\
		HINSTANCE hPrevInstance,											\
		LPTSTR lpCmdLine,													\
		int nCmdShow)														\


#define START_MIDLET(appName)												\
	FASTIVA_MODULE_NAME(initModuleInfo)();			\
	TCHAR moduleName[MAX_PATH + 1];											\
	GetModuleFileName(NULL, moduleName, sizeof(moduleName)/sizeof(TCHAR));	\
	const TCHAR* argv[] = {													\
		moduleName,															\
		_T("-cp"), _T("resource.jar"),										\
		_T("com.sun.midp.main.MIDletSuiteLoader"),							\
		_T(appName)															\
	};																		\
	return fastiva_main(sizeof(argv) / sizeof(argv[0]), argv,				\
		fastiva_getApplicationModule(), &g_fastivaRuntime, ADDR_ZERO);		\

#else

#define FASTIVA_APP_MAIN()													\
	int __cdecl _tmain(int argc, const TCHAR* argv_org[]) 




#define START_MIDLET(appName)												\
	FASTIVA_MODULE_NAME(initModuleInfo)();			\
	const TCHAR* argv[] = {													\
		argv_org[0],														\
		_T("-cp"), _T("resource.jar"), _T("-midp"),							\
		_T("com.sun.midp.main.MIDletSuiteLoader"),							\
		_T(appName)															\
	};																		\
	return fastiva_main(sizeof(argv) / sizeof(argv[0]), argv,				\
	&FASTIVA_MODULE_NAME(ModuleInfo), 0);


#define START_APP(appName, main)											\
	FASTIVA_MODULE_NAME(initModuleInfo)();									\
	return fastiva_main(argc, argv_org,										\
	&FASTIVA_MODULE_NAME(ModuleInfo), main);								\


#endif
	

