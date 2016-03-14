#include <precompiled_libcore.h>

#include "kernel/kernel.h"
#include "kernel/cpu.h"		// _ASM_GET_SP() macro
#include "kernel/Runnable.h"		// _ASM_GET_SP() macro
#include <pal/fox_file.h>
#include <pal/util.h>
#include "fox/Sys.h"		// declarations of fox_thread... functions
#include "fox/task.h"		// declarations of fox_thread... functions
#include "fox/Atomic.h"		// declarations of fox_thread... functions
#include "kernel/sys.h"

#include <windows.h> // for CRITICAL_SECTION
#include <stdio.h> // for SPRINTF
#include <fastiva_malloc.h>
#include <float.h>

//#undef _T
#include <tchar.h>
#include <fox/Sys.h>
typedef unsigned int uint;

typedef union {
	unsigned char mode : 2;
	unsigned char resered : 3;
	unsigned char rm : 3;
}X86_CODE;

static int getIDivJumpOffset(unsigned int baseAddr) {
	int ip = 0;
	X86_CODE code = {0};
	
	char *eip = (char *)(void *)baseAddr;
	if (eip[0] != 0xF7) {; //eip는 항상 0xF7(X86의 IDIV code)
		FASTIVA_DBREAK();
	}

	code = *(X86_CODE*)&eip[1];

	switch(code.mode) {
	case 0:  //mod 00
		if (code.rm == 0x05) { //disp32
			return 6;
		}
		else if (code.rm == 0x04) {            //[--][--] 한 byte더 읽어서 판별
			if ((eip[2] & 0x07) != 0x05) {
				return 0x03;        
			}
			else {
				return 0x07;
			}
		}

	case 1:
		if (code.rm == 0x04) { //[--][--]+disp8
			return 0x04;
		}
		else {
			return 0x03;                      
		}

	case 2:
		if (code.rm == 0x04) { //[--][--]+disp32
			return 0x07;
		}
		else {
			return 0x06;
		}

	case 3:
		return 0x02;
	}
	return 0;
}


#if 0 
int fm::getTickCount() {
	return ::GetTickCount();
}
#endif


#ifdef _ARM_
extern "C" void CacheSync(int flags);
/*
/* Flags for CacheSync/CacheRangeFlush */
#define CACHE_SYNC_DISCARD      0x001   /* write back & discard all cached data */
#define CACHE_SYNC_INSTRUCTIONS 0x002   /* discard all cached instructions */
#define CACHE_SYNC_WRITEBACK    0x004   /* write back but don't discard data cache*/
#define CACHE_SYNC_FLUSH_I_TLB  0x008   /* flush I-TLB */
#define CACHE_SYNC_FLUSH_D_TLB  0x010   /* flush D-TLB */
#define CACHE_SYNC_FLUSH_TLB    (CACHE_SYNC_FLUSH_I_TLB|CACHE_SYNC_FLUSH_D_TLB)    /* flush all TLB */
#define CACHE_SYNC_L2_WRITEBACK 0x020   /* write-back L2 Cache */
#define CACHE_SYNC_L2_DISCARD   0x040   /* discard L2 Cache */

#define CACHE_SYNC_ALL          0x07F   /* sync and discard everything in Cache/TLB */
//*/
void fm::discardCodeCache() {
	CacheSync(CACHE_SYNC_ALL);
}
#endif


extern void sys_reset_StackOverflow(void* catched_ex) {
	/**
	The _resetstkoflw function recovers from a stack overflow condition, 
	allowing a program to continue instead of failing with a fatal exception error. 
	If the _resetstkoflw function is not called, there are no guard page after 
	the previous exception. The next time that there is a stack overflow, 
	there are no exception at all and the process terminates without warning.

	이 함수는 stack이 완전하게 복구된 이후에 호출되어야만 한다.
	_resetstkoflw should never be called from:
	1. A filter expression.
	2. A filter function.
	3. A function called from a filter function.
	4. A catch block.
	5. A __finally block

	*/
#ifdef UNDER_CE
	__debugbreak();
#else
	_resetstkoflw();
#endif
}

void fox_GC_doLocalGC();
static int g_pass_trap = 0; // 정해진 개수만큼 break 무시
static int g_in_trap = false;

int fastiva_Runtime::dispatchNativeException(void* pExInfo) {
	//fastiva_Task* pTask = fastiva_getCurrentTask();

	//fox_unexpected_handler((_EXCEPTION_POINTERS*)pExInfo);
	_EXCEPTION_POINTERS *pExceptionInfo = (_EXCEPTION_POINTERS*)pExInfo;
	// The DemandLoadDll_ExceptionFilter function changes a 
	// thread's program counter. We can restrict the amount
	// of CPU-dependent code by defining the PROGCTR macro below.
	#if defined(_X86_)
	#define PROGCTR(Context)  ((Context)->Eip)
	#endif

	#if defined(_MIPS_)
	#define PROGCTR(Context)  ((Context)->Fir)
	#endif

	#if defined(_ALPHA_)
	#define PROGCTR(Context)  ((Context)->Fir)
	#endif

	#if defined(_PPC_)
	#define PROGCTR(Context)  ((Context)->Iar)
	#endif

	#if defined(_ARM_)
	#define PROGCTR(Context)  ((Context)->Pc)
	#endif
	uint addr;
	uint reg_ecx;


	switch (pExceptionInfo->ExceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION:
#ifdef UNDER_CE
			addr = 0; //pExceptionInfo->ExceptionRecord->ExceptionInformation[0];
#else
			addr = pExceptionInfo->ExceptionRecord->ExceptionInformation[1];
#endif
			//pTask = fastiva_getCurrentTask();
			//if (pTask != 0 && pTask->m_trap) {
			//	return EXCEPTION_CONTINUE_SEARCH;
			//}

			if (addr == 9999) {
				fox_Task* pTask = fox_task_currentTask();
				int eip = pTask->m_gcInterruptContext.Eip;
				pTask->m_gcInterruptContext = *pExceptionInfo->ContextRecord;
				int* _esp = (int*)pTask->m_gcInterruptContext.Esp;
				pTask->m_gcInterruptContext.Eax = *_esp ++;
				pTask->m_gcInterruptContext.Esp += 4;
				pTask->m_gcInterruptContext.Eip = eip;
				pExceptionInfo->ContextRecord->Eip = (int)fox_GC_doLocalGC;
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			if (--g_pass_trap < 0) {
				if (g_in_trap) {
					g_in_trap = false;
					//return EXCEPTION_CONTINUE_EXECUTION;
				}
				else {
					g_in_trap = true;
					return EXCEPTION_CONTINUE_SEARCH;
				}
			}
#ifdef _DEBUG
			if ((int)addr <= 0x400) { //== (int)FASTIVA_NULL) {
				PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwNullPointerException;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
#else
			if (addr <= 0x400) { //3FF(int)FASTIVA_NULL) {
				PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwNullPointerException;
				return EXCEPTION_CONTINUE_EXECUTION;
			}
#endif
			return EXCEPTION_CONTINUE_SEARCH;

		case EXCEPTION_INT_DIVIDE_BY_ZERO: 
			PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwDivideByZeroException;
			return EXCEPTION_CONTINUE_EXECUTION;


		case EXCEPTION_STACK_OVERFLOW: 
			PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwStackOverflowError;
			return EXCEPTION_CONTINUE_EXECUTION;

		case EXCEPTION_FLT_STACK_CHECK: 
			//PROGCTR(pExceptionInfo->ContextRecord) = (int)fastiva_throwStackOverflowError;
			//return EXCEPTION_CONTINUE_EXECUTION;
		
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: 
			// The thread tried to access an array element that is out of bounds 
			// and the underlying hardware supports bounds checking.

		case EXCEPTION_INT_OVERFLOW: 
			// The result of an integer operation caused a carry out of the most 
			// significant bit of the result. ((signed int)0x80000000 / (int)-1)
		{
#ifdef UNDER_CE
			KASSERT(0);
#else
			unsigned int XEIP = pExceptionInfo->ContextRecord->Eip;
			unsigned int jumpAddr = XEIP;
			jumpAddr += getIDivJumpOffset(XEIP);
			pExceptionInfo->ContextRecord->Eax = 0x80000000;
			pExceptionInfo->ContextRecord->Edx = 0x00000000;
			PROGCTR(pExceptionInfo->ContextRecord) = (int)jumpAddr;
#endif
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		case EXCEPTION_FLT_DENORMAL_OPERAND: 
			// One of the operands in a floating-point operation is denormal. 
			// A denormal value is one that is too small to represent as a standard 
			// floating-point value. 

		case EXCEPTION_FLT_DIVIDE_BY_ZERO: 
			// The thread tried to divide a floating-point value by a 
			// floating-point divisor of zero. 
		
		case EXCEPTION_FLT_INEXACT_RESULT: 
			// The result of a floating-point operation cannot be 
			// represented exactly as a decimal fraction. 
		
		case EXCEPTION_FLT_INVALID_OPERATION: 
			// This exception represents any floating-point exception not included in this list. 
		
		case EXCEPTION_FLT_OVERFLOW: 
			// The exponent of a floating-point operation is greater than the magnitude 
			// allowed by the corresponding type. 
		
		case EXCEPTION_FLT_UNDERFLOW: 
			// The exponent of a floating-point operation is less than the magnitude 
			// allowed by the corresponding type. 

		case EXCEPTION_ILLEGAL_INSTRUCTION: 
			// The thread tried to execute an invalid instruction. 

		case EXCEPTION_IN_PAGE_ERROR: 
			// The thread tried to access a page that was not present, 
			// and the system was unable to load the page. 
			// For example, this exception might occur if a network connection is lost 
			// while running a program over the network. 

		case EXCEPTION_INVALID_DISPOSITION: 
			// An exception handler returned an invalid disposition to the 
			// exception dispatcher. Programmers using a high-level language 
			// such as C should never encounter this exception. 

		case EXCEPTION_NONCONTINUABLE_EXCEPTION: 
			// The thread tried to continue execution after a noncontinuable exception occurred. 

		case EXCEPTION_PRIV_INSTRUCTION: 
			// The thread tried to execute an instruction whose operation is not 
			// allowed in the current machine mode. 

		case EXCEPTION_BREAKPOINT: 	
			// A breakpoint was encountered. 

		case EXCEPTION_SINGLE_STEP: 
			// A trace trap or other single-instruction mechanism signaled 


		case EXCEPTION_DATATYPE_MISALIGNMENT: 
			// The thread tried to read or write data that is misaligned on hardware 
			// that does not provide alignment. For example, 16-bit values must be 
			// aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on. 
			break;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

HINSTANCE hPrevDLL = NULL;
HINSTANCE hCurrDLL = NULL;
void (*tt_main)();

//#include <fastiva/JNI/runtime.h>


static HINSTANCE FOX_FASTCALL(__findLocalLibrary)(
	const TCHAR* pszPath, int lenPath, 
	const TCHAR* pszFile, int lenFile
) {
	char path[2048];
	if (lenPath + lenFile >= sizeof(path)) {
		return NULL;
	}
	memcpy(path, pszPath, lenPath);
	path[lenPath++] = '/';
	memcpy(path + lenPath, pszFile, lenFile);
	strcpy(path + lenPath + lenFile, ".dll");
	//FASTIVA_ENTER_NATIVE_SECTION();

#ifdef UNDER_CE
	WCHAR wPath[2048];
	int len = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
		(LPCSTR) path, -1,
		(LPWSTR) wPath, 2048);

	HINSTANCE hDLL = ::LoadLibrary(wPath);
#else
	HINSTANCE hDLL = ::LoadLibrary(path);
#endif
	return hDLL;
	//FASTIVA_LEAVE_NATIVE_SECTION();
}

static FOX_HFILE FOX_FASTCALL(__openLocalFile)(
	const TCHAR* pszPath, int lenPath, 
	const TCHAR* pszFile, int lenFile
) {
	FOX_UNICODE_CHAR path[2048];
	if (lenPath + lenFile >= sizeof(path)) {
		return NULL;
	}
	int i;
	FOX_UNICODE_CHAR* pch = path;
	for (i = lenPath; i -- > 0; ) {
		*pch ++ = *pszPath++;
	}
	//memcpy(path, pszPath, lenPath);
	//path[lenPath++] = '/';
	*pch ++ = '/';
	for (; *pszFile != 0; ) {
		*pch ++ = *pszFile ++;
	}
	*pch = 0;
	//strcpy(path + lenPath, pszFile);
	//path[lenPath] = 0;
	//strcpy(path + lenPath + lenFile, ".dll");
	//FASTIVA_ENTER_NATIVE_SECTION();

	FOX_HFILE hFile = fox_file_open(path, FOX_READ);
	return hFile;
	//FASTIVA_LEAVE_NATIVE_SECTION();
}

FOX_HFILE FOX_FASTCALL(__openFileInClassPath)(
	const TCHAR* pszFoxFileName
) {
	const TCHAR* fastiva_kernel_getClassPath();
	const TCHAR* cp = fastiva_kernel_getClassPath();
	const TCHAR* d;
	int len_file = _tcslen(pszFoxFileName);

	if (cp == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	while ((d = _tcschr(cp, ';')) != NULL) {
		int len_path = d - cp;
		FOX_HFILE hFile = __openLocalFile(cp, len_path, pszFoxFileName, len_file);
		if (hFile != FOX_HERROR) {
			return hFile;
		}
		cp = d + 1;
	}

	return __openLocalFile(cp, _tcslen(cp), pszFoxFileName, len_file);
}


static HINSTANCE FOX_FASTCALL(__findLocalLibrary)(const TCHAR* pszFoxFileName) {
	const TCHAR* fastiva_kernel_getClassPath();
	const TCHAR* cp = fastiva_kernel_getClassPath();
	const TCHAR* d;
	int len_file = _tcslen(pszFoxFileName);

	if (cp == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	while ((d = _tcschr(cp, ';')) != NULL) {
		int len_path = d - cp;
		HINSTANCE hDLL = __findLocalLibrary(cp, len_path, pszFoxFileName, len_file);
		if (hDLL != NULL) {
			return hDLL;
		}
		cp = d + 1;
	}

	return __findLocalLibrary(cp, _tcslen(cp), pszFoxFileName, len_file);
}



HINSTANCE FOX_FASTCALL(__findLibrary)(const TCHAR* pszFoxFileName) {
	HINSTANCE hDLL = ::LoadLibrary(pszFoxFileName);//, NULL, DONT_RESOLVE_DLL_REFERENCES);//LOAD_LIBRARY_AS_DATAFILE);
	if (hDLL == NULL) {
		int a = GetLastError();
		hDLL = __findLocalLibrary(pszFoxFileName);//, NULL, LOAD_LIBRARY_AS_DATAFILE);
	}
	return hDLL;
}

static java_lang_String_p g_pSystemDir = ADDR_ZERO;
HANDLE g_hFastivaInstance = NULL;

//*
//*/

java_lang_String_p fm::util_getSystemDir() {
	if (g_pSystemDir == ADDR_ZERO) {
		/*
		TCHAR moduleName[MAX_PATH + 1];
		int len = GetModuleFileName((HMODULE)g_hFastivaInstance, moduleName, sizeof(moduleName)/sizeof(TCHAR));
		int dir_len = -1;
		for (; --len >= 0; ) {
			if (moduleName[len] == '\\') {
				moduleName[len] = '/';
				if (dir_len < 0) {
					// '/'를 포함한다.
					dir_len = len + 1;
				}
			}
		}
		*/
		const TCHAR* moduleName = sys_util_getCurrentDirectory();
		int dir_len = strlen(moduleName);

		//moduleName[dir_len] = 0;
		//fox_printf(moduleName);

		if (sizeof(TCHAR) == sizeof(unicod)) {
			g_pSystemDir = fastiva.createStringW((unicod*)moduleName, dir_len);
		}
		else {
			g_pSystemDir = fastiva.createStringA((char*)moduleName, dir_len);
		}
		fastiva_lockGlobalRef(g_pSystemDir);
	}
	return g_pSystemDir;
}


static bool __getFoxFileName(TCHAR* pszFoxFileName, const char* pszPackageName, int nameLen) {
	/*
	int len = sys_util_getCurrentDirectory(pszFoxFileName);
	pszFoxFileName += len;
	//*/
	for (;nameLen-- > 0;) {
		char ch = *pszPackageName;
		if (ch == '/') {
			ch = '_';
		}
		KASSERT(ch != '.');
		*pszFoxFileName++ = ch;
		pszPackageName ++;
	}
	/*
	*pszFoxFileName ++ = '.';
	*pszFoxFileName ++ = 'd';
	*pszFoxFileName ++ = 'l';
	*pszFoxFileName ++ = 'l';
	//*/
	*pszFoxFileName = 0;
	return true;
}

FASTIVA_INIT_LIB_PROC FOX_FASTCALL(fastiva_win32_loadLibrary)(const char* pszPackageName, int nameLen) {

	HINSTANCE hDLL;
	if (pszPackageName[0] == 0) {
		hDLL = __findLocalLibrary(_T("default$"));//, NULL, LOAD_LIBRARY_AS_DATAFILE);
	}
	else if (nameLen < 1024) {
		TCHAR libName[1024];
		if (!__getFoxFileName(libName, pszPackageName, nameLen)) {
			return ADDR_ZERO;
		}
		hDLL = __findLibrary(libName);
	}
	else {
		return 0;
	}

	if (hDLL == NULL) {
		int a = GetLastError();
		return 0;
	}
	hPrevDLL = hCurrDLL;
	hCurrDLL = hDLL;


	extern Kernel  g_kernel;
#ifndef UNDER_CE
	FASTIVA_INIT_LIB_PROC initFastiva = (FASTIVA_INIT_LIB_PROC)::GetProcAddress(hDLL, TEXT("initFastiva"));
#else
	FASTIVA_INIT_LIB_PROC initFastiva = (FASTIVA_INIT_LIB_PROC)::GetProcAddressA(hDLL, "initFastiva");
#endif
	if (initFastiva == NULL) {
		FreeLibrary(hDLL);
	}
	return initFastiva;
}

/*
#if defined(FASTIVA_J2SE) || defined(FASTIVA_CDC)
void* fm::findNativeMethod_MD(const char* method_name) {
	/*
	if (isImportClassLoader == false) {
		java_lang_Class_p pClassLoader = java_lang_ClassLoader_C$::importClass$();
		pClassLoader->m_pContext->initStatic$();
		isImportClassLoader = true;
		return (void*)FASTIVA_NULL;
	}
	//*

	java_lang_String_p pMethodName = fastiva.createString(method_name);
	jlonglong fn = java_lang_ClassLoader::findNative0$((java_lang_ClassLoader_p)FASTIVA_NULL, pMethodName);

	if (fn == 0) {
		return (void*)FASTIVA_NULL;
	}
	else {
		return (void *)fn;
	}
}
#endif
*/


jbool FOX_FASTCALL(fastiva_GC_init)(void* heapArea, int heapSize);
extern jbool FOX_FASTCALL(fm::kernel_initFastiva)(void* oemData);

void GetFastivaLibPath(char *pBuf, int bufSize);

jbool initJRE() {
	//char buf[256];
	//GetFastivaLibPath(buf, sizeof(buf));
#ifndef _JPP
	//strcat(buf, "\\java.dll");
	return LoadLibrary("java.dll") != NULL;
#endif
	return true;
}

/*
void fm::memset(void* buff, int filler, int length) {
	::memset(buff, filler, length);
}
*/

static jbool isImportClassLoader = false;

static HMODULE __hAWTDll = NULL; 

void* fm::findNativeMethod_MD(const char* method_name) {
	//java_lang_String_p pMethodName = fastiva.createString(method_name);
	//jlonglong fn = java_lang_ClassLoader::findNative0$((java_lang_ClassLoader_p)FASTIVA_NULL, pMethodName);
	if (__hAWTDll == NULL) {
#ifdef UNDER_CE
		WCHAR dllPath[MAX_PATH];
		KASSERT(0);
		//GetCurrentDirectory(MAX_PATH, dllPath);	// 대응되는 wce 함수 없음
		wcscat(dllPath, _T("\\awt_g.dll"));
		//__hAWTDll = GetModuleHandle(dllPath);
		__hAWTDll = LoadLibraryW(dllPath);
#else
		char dllPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, dllPath);
		strcat(dllPath, "\\awt_g.dll");
		//__hAWTDll = GetModuleHandle(dllPath);
		__hAWTDll = LoadLibrary(dllPath);
#endif
		if (__hAWTDll != FASTIVA_NULL) {
			// awt_g.dll에 있는 _JNI_OnLoad@8함수를 호출하여 JavaVM을 전달해야 한다.
			typedef jint (__stdcall *JNI_OnLoad_t)(const void*, void*);
			JNI_OnLoad_t JNI_OnLoad = (JNI_OnLoad_t)GetProcAddress(__hAWTDll, _T("_JNI_OnLoad@8"));
			(*JNI_OnLoad)(0/*getVM()*/, (void*)FASTIVA_NULL);
		}
		
	}

	if(__hAWTDll == FASTIVA_NULL) {
		return ADDR_ZERO;
	}

#ifdef UNDER_CE
	WCHAR wMethodName[256];
	int len = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS,
		(LPCSTR) method_name, -1,
		(LPWSTR) wMethodName, 256);
	FARPROC proc =  GetProcAddress(__hAWTDll, wMethodName);
#else
	FARPROC proc =  GetProcAddress(__hAWTDll, method_name);
#endif
	if (proc == NULL) {
		return ADDR_ZERO;
	}
	else {
		return (void *)proc;
	}
}

