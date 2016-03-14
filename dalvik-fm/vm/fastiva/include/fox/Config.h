#ifndef __FOX_CONFIG_H__
#define __FOX_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif	



#define FOX_NATIVE_DLL_API __declspec(dllimport)

#define FOX_TASK_MAX_PRIORITY	15
#define ADDR_ZERO				0

typedef int			 	FOX_BOOL;
#define FOX_TRUE		1
#define FOX_FALSE		0

typedef char FOX_CHAR;
typedef const FOX_CHAR* FOX_STRING;

typedef unsigned short FOX_KSC16_CHAR;
typedef FOX_KSC16_CHAR* FOX_KSC16_STRING;

//typedef unsigned short FOX_UNICODE_CHAR;
#ifdef __cplusplus
#ifdef _WIN32
	typedef wchar_t  FOX_UNICODE_CHAR;          /* unsigned 16 bits */
#else 
	typedef unsigned short  FOX_UNICODE_CHAR;          /* unsigned 16 bits */
#endif
#else 
	typedef unsigned short FOX_UNICODE_CHAR;
#endif

//typedef const FOX_UNICODE_CHAR * FOX_UNICODE_STRING;

typedef struct  {
	FOX_UNICODE_CHAR * m_pText;
	int m_length;
} FOX_UNICODE_STRING ;

#ifdef UNDER_CE
	struct fox_Task;
	#define fox_NativeTaskContext fox_Task 
#else
	struct fox_NativeTaskContext;
#endif

#ifdef FASTIVA_TARGET_CPU_ARM
	#define __stdcall 	// ignnore 
	#define __cdecl   	// ignnore
	#define __fastcall  // ignnore
	#define __thiscall  // ignnore

#elif defined(__GNUC__)
	#define __stdcall 	__attribute__((__stdcall__))
	#define __cdecl   	__attribute__((__cdecl__))
	#define __fastcall  __attribute__((fastcall))
	#define __thiscall  __attribute__((thiscall))

	#ifndef __cdecl
		#define __cdecl __attribute__((cdecl))
	#endif
	#ifndef __declspec
		#define __declspec(e) __attribute__((e))
	#endif
#endif  //endif __GNUC__

    	#define FOX_FASTCALL(fn)	/*__fastcall*/ fn
    	#define FOX_THISCALL(fn)	/*__thiscall*/ fn



#if defined(_WIN32) || defined(_WIN32_WCE)
	#define FOX_NAKED			__declspec(naked)
    #define FOX_NO_RETURN		__declspec(noreturn)
    #define FOX_NO_VTABLE		__declspec(novtable)
    #define FOX_RESTRICT_API	__declspec(restrict)
	#define FOX_EXPORT_API		__declspec(dllexport)
	#define FOX_IMPORT_API 		__declspec(dllimport)
#elif defined(__GNUC__)
	#define FOX_NAKED			__attribute__ ((naked))
	#define FOX_NO_RETURN		__attribute__((noreturn))
	#define FOX_NO_VTABLE		// NDK_TODO
	#define FOX_RESTRICT_API	__attribute__((malloc))
	#define FOX_EXPORT_API		__attribute__((visibility("default")))
	#define FOX_IMPORT_API 		// Not used??
#ifndef __CC_ARM
	typedef long long 	__int64;
#endif
#else
	typedef long long 	__int64;
#endif

typedef __int64		FOX_LONGLONG;

typedef unsigned int FOX_TIME; // milli second(ms)
typedef FOX_LONGLONG FOX_TIMESTAMP; // milli second(ms)

typedef const void* FOX_HANDLE;


#ifdef _WIN32
	static const int DEBUG_BUFF_SIZE = 4096;
	extern char debug_buff[];
	extern int sprintf_s(char *buffer, size_t sizeBuff, const char *format, ...);
	__declspec(dllimport) void __stdcall OutputDebugStringA(const char *msg);
	#define fox_printf(...)	(sprintf_s(debug_buff, DEBUG_BUFF_SIZE, __VA_ARGS__)); OutputDebugStringA(debug_buff); 
#else
	int __android_log_print(int prio, const char *tag, const char *fmt, ...);
	#define fox_printf(...)	((void)__android_log_print(6, "Fastiva", __VA_ARGS__))
#if GCC_VERSION >= 40500
	#define __assume(cond) do { if (!(cond)) __builtin_unreachable(); } while (0)
#else
	#define __assume(cond) 		// android has not the __builtin_unreachable().
#endif
#endif

#ifdef _WIN32
	#define FOX_BREAK()		__debugbreak()
#else
	extern void fox_debug_trap();
	#define FOX_BREAK()		fox_debug_trap()
#endif

#ifndef _WIN32
	#define __FUNCTION__  __func__
#endif

extern void fox_debug_trace(const char* func, const char* file, int line);
extern void fox_exit(int errcode);


#if defined(_DEBUG_ASSERT) && !defined(NO_DEBUG_ASSERT)
	#define KASSERT(t)	if (!(t))	{ fox_printf("ASSERT FAIL: (%s) %s <%s:%d>\n", #t, __FUNCTION__, __FILE__, __LINE__); fox_exit(-1); }
#else
	#define KASSERT(t)	__assume((t))
#endif

#ifdef __cplusplus
	struct FOX_DEBUG_TRACE_FRAME { 
		struct fastiva_Task* pTask;
		FOX_DEBUG_TRACE_FRAME(const char* func, const char* file, int line);
		~FOX_DEBUG_TRACE_FRAME();
	};
	#define FOX_STACK_TRACE()	FOX_DEBUG_TRACE_FRAME _debug_trace_frame(__FUNCTION__, __FILE__, __LINE__)
#else 
	#define FOX_STACK_TRACE()	fox_debug_trace(__FUNCTION__, __FILE__, __LINE__)
#endif

/*
#ifdef _DEBUG_TRACE
	#define JPP_PROLOGUE()	FOX_STACK_TRACE();
#else
	#define JPP_PROLOGUE()	// ignore;
#endif
*/


#define FOX_DEBUG_BREAK()	FASTIVA_DBREAK()

#define FOX_HERROR 	((FOX_HANDLE)-1)
	




#ifdef __cplusplus
}
#endif	


#endif // __FOX_CONFIG_H__
