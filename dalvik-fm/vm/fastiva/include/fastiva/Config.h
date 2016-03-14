#ifndef __FASTIVA_CONFIG_H__
#define __FASTIVA_CONFIG_H__

#include <fox/Config.h>
#include <fox/Monitor.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define JPP_ENANBLE_HUGE_PRECOMPILED_HEADER 1
#ifdef __GNUC__
	#ifndef GCC_VERSION
		#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
	#endif
	#if defined __arm__ || defined __thumb__
		#define _ARM_
	#elif defined(__i386__) || defined(__x86_64__)
		#define _X86_
	#endif
#elif _MSC_VER
	#if defined _M_ARM || defined _M_ARMT
		#define _ARM_
	#elif defined _M_IX86
		#define _X86_
	#endif
#endif

 




#ifdef _WIN32
    #define FOX_CPU_REGISTER_COUNT 16
    typedef int fox_jmp_buf[FOX_CPU_REGISTER_COUNT];
    typedef fox_jmp_buf jmp_buf;
    extern "C" {
	    int __cdecl _setjmp(fox_jmp_buf _Buf);
    };
    #define FASTIVA_SETJMP _setjmp
#elif defined(ANDROID)
	#include <setjmp.h>

	typedef jmp_buf fox_jmp_buf;
   #define FASTIVA_SETJMP(buf) setjmp(buf)
   #define FASTIVA_LONGJMP(buf, v) longjmp(buf, v)
#else
	$$ set jmpbuf with preprocessed file 
#endif

#define FASTIVA_VIRTUAL					//
#define NO_ENABLE_SCRIPT				&&& obsolete flags &&&
#define JPP_VM_IS_INTERPRETABLE			&&& obsolete flags &&&
#define JPP_VM_IS_GENERAL_PURPOSE		&&& obsolete flags &&&
#define JPP_USE_PRECOMPILED_HEADER		&&& obsolete flags &&&
#define FASTIVA_SUPPORTS_PACKAGLE_LOADER	&&& obsolete flags &&&


#define FASTIVA_PRELOAD_STATIC_INSTANCE			1

#define FASTIVA_FAST_JNI						1
#ifdef FASTIVA_SUSPEND_BY_SIGNAL
	#undef FASTIVA_FAST_JNI
	#define FASTIVA_FAST_JNI					1
#endif

#define FASTIVA_BUILD_TARGET_EXTERNAL_COMPONENT			0
#define FASTIVA_BUILD_TARGET_EMBEDDED_COMPONENT			1
#define FASTIVA_BUILD_TARGET_RUNTIME_LIBCORE			2
#define FASTIVA_BUILD_TARGET_RUNTIME_KERNEL				3


#undef	EMBEDDED_RUNTIME

typedef unsigned char       u1;
typedef unsigned short      u2;
typedef unsigned int		u4;
typedef unsigned long long  u8;
typedef signed char         s1;
typedef signed short        s2;
typedef signed int		    s4;
typedef signed long long    s8;
typedef u4 size_t;
#ifndef NULL
#define NULL 0
#endif

#if defined ANDROID || (FASTIVA_BUILD_TARGET >= FASTIVA_BUILD_TARGET_EMBEDDED_COMPONENT) 
#	define	EMBEDDED_RUNTIME
#else 
#	error "Not EMBDDED_RUNTIME"
#endif

#define FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER		0
#define FASTIVA_SUPPORTS_JAVASCRIPT						0
#define FASTIVA_SUPPORTS_BYTECODE_INTERPRETER			0
#define FASTIVA_SUPPORTS_REFLECTION						1
#define FASTIVA_SUPPORTS_JNI							1
#define FASTIVA_SUPPORTS_DYNAMIC_ARRAY					0
#define FASTIVA_SUPPORT_JNI_STUB						1


#ifndef JPP_JNI_EXPORT_LEVEL
	/**
	 * 0 : NONE
	 * 1 : only explicitly @JniAccess members and Class.newInstance method only
	 * 2 : public members and above
	 * 3 : protected members and above
	 * 4 : all memebers
	 */
	#if   FASTIVA_SUPPORTS_REFLECTION
		#define JPP_JNI_EXPORT_LEVEL			4
	#elif FASTIVA_SUPPORTS_BYTECODE_INTERPRETER
		#define JPP_JNI_EXPORT_LEVEL			3
	#elif FASTIVA_SUPPORTS_JAVASCRIPT
		#define JPP_JNI_EXPORT_LEVEL			2
	#elif FASTIVA_SUPPORTS_JNI
		#define JPP_JNI_EXPORT_LEVEL			1
	#else 
		#define JPP_JNI_EXPORT_LEVEL			0
	#endif
#endif


#define JPP_JDK_CLDC10			1000
#define JPP_JDK_CLDC11			1010
#define JPP_JDK_MIDC			1100
#define JPP_JDK_CDC				2000

#define JPP_JDK_ANDROID(VER)	(VER*10000 + 1)

#define JPP_JDK_VERSION			JPP_JDK_ANDROID(11)

#define JPP_JDK_IS_ANDROID()	((JPP_JDK_VERSION & 1) != 0)

#ifndef FASTIVA_SUPPORT_JNI
	//#define FASTIVA_SUPPORT_JNI
#endif

#ifdef EMBEDDED_RUNTIME
	#define FASTIVA_DLL_API		FOX_EXPORT_API
#else
	#define FASTIVA_DLL_API		FOX_IMPORT_API
#endif


#define FASTIVA_SUPPORT_JNI 1
/*
#define FASTIVA_NO_JNI_PACKAGE_fastiva
#define FASTIVA_NO_JNI_PACKAGE_java_lang
#define FASTIVA_NO_JNI_PACKAGE_java_lang_ref
#define FASTIVA_NO_JNI_PACKAGE_java_lang_reflect
#define FASTIVA_NO_JNI_PACKAGE_java_io
#define FASTIVA_NO_JNI_PACKAGE_java_net
#define FASTIVA_NO_JNI_PACKAGE_java_util
#define FASTIVA_NO_JNI_PACKAGE_java_util_zip
#define FASTIVA_NO_JNI_PACKAGE_java_util_jar
#define FASTIVA_NO_JNI_PACKAGE_java_security
#define FASTIVA_NO_JNI_PACKAGE_sun_misc
#define FASTIVA_NO_JNI_PACKAGE_sun_reflect
*/

#undef FASTIVA_NO_CPP_MULTI_INHERIT

#define FASTIVA_NO_PROXY_THUNK
#ifdef FASTIVA_J142
	#define FASTIVA_J2SE
#endif

#ifdef UNICODE
	#define SYS_TCHAR unsigned short
#else
	#define SYS_TCHAR char
#endif

typedef struct fox_Task* FOX_HTASK;

#define FOX_HTASK_NOT_STARTED	((FOX_HTASK)1)
#define FASTIVA_HMUTEX_UNUSED	((FOX_HTASK)-1)


/**
* define target CPU
*/
#ifdef EMBEDDED_RUNTIME
	#ifndef FASTIVA_IMMUTABLE_IMAGE
		#define FASTIVA_IMMUTABLE_IMAGE
	#endif
#endif

#ifdef FASTIVA_IMMUTABLE_IMAGE
	#define FASTIVA_GLOBAL
#else
	#define FASTIVA_GLOBAL	// const 
#endif

#ifdef _WIN32
#pragma const_seg("fm::package_0$")
#pragma const_seg("fm::package_5$")
#pragma const_seg("fm::package_9$")
#define FASTIVA_EXPORT						__declspec(dllexport) FASTIVA_GLOBAL
#define FASTIVA_ALIGN_SEGMENT(seg_name)		__declspec(allocate(#seg_name))
#endif

#if defined(_M_IX86) && !defined(i486)
	#define i486
#endif

#if defined(i486) || defined(i386)
	#define FASTIVA_TARGET_CPU_i486
	#define FASTIVA_CALL(fn)	fn		//
	#define FASTIVA_JNI_CALL __stdcall
//#elif defined(_ARM_)
//	#define FASTIVA_TARGET_CPU_ARM
#elif defined(__TARGET_FEATURE_THUMB)
	#define FASTIVA_TARGET_CPU_THUMB
#endif

#define FASTIVA_THISCALL(fn)			FOX_THISCALL(fn)
//#define FASTIVA_STATICCALL(fn)			FOX_THISCALL(fn)
#define FASTIVA_PARAM_T(type)			type//##_PARAM_T$

/**
* define target OS
*/

#if defined(WIN32) || defined(_WIN32)
	#define FASTIVA_TARGET_OS_WIN32
#elif defined(_WIN32_WCE)
	#define FASTIVA_TARGET_OS_WINCE
#elif defined(__linux__)
	#define FASTIVA_TARGET_OS_LINUX
#elif defined(SCOM_HAL)
	#define FASTIVA_TARGET_OS_SCOM
#elif defined(EZPHONE)
	#define FASTIVA_TARGET_OS_EZ_HAL
#endif


#if defined(DEBUG) && !defined(_DEBUG)
	#define _DEBUG
#endif

#ifdef __GNUC__
	#define FASTIVA_PURE_API		__attribute__((pure))
	#define FASTIVA_PURE_CONST_API	const __attribute__((pure))

	#pragma GCC diagnostic ignored "-Winvalid-offsetof"
	#pragma GCC diagnostic ignored "-Wcomment"
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wunused-label"
	#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#else
	#define FASTIVA_PURE_API		// IGNORE
	#define FASTIVA_PURE_CONST_API	const
	typedef unsigned int size_t;
#endif

typedef bool				jbool;
typedef signed char			jbyte;		
typedef FOX_UNICODE_CHAR	unicod;
typedef signed short		jshort;		
typedef signed int			jint;		
typedef float				jfloat;	
typedef double				jdouble;

typedef unsigned char		ubyte;		
typedef unsigned short		ushort;		
typedef unsigned int		uint;		

#ifdef _WIN32
	typedef signed __int64		jlonglong;		
	typedef unsigned __int64	ulonglong;		
#else
	typedef signed long long	jlonglong;		
	typedef unsigned long long	ulonglong;		
#endif


#ifdef FASTIVA_CPD540
	#define FASTIVA_NULL_ADDR	0x1231
#elif defined(_WIN32) || (__linux__)
	#define FASTIVA_NULL_ADDR	0
#else
	#define FASTIVA_NULL_ADDR	0x5001231
#endif
#define FASTIVA_NULL		((const void*)FASTIVA_NULL_ADDR)


void fastiva_debug_textout(const char* msg);

//extern int g_method_call;
//extern int g_interface_call;

#define CHECK_STACK(local_size, func)	//g_method_call ++;
//	fastiva_StackFrame FASTIVA_CHECK_STACK$(func);

void fastiva_checkStackFast();
#define	CHECK_STACK_FAST()	//fastiva_checkStackFast();



#ifdef FASTIVA_TARGET_CPU_i486
#define	SIZE_OF_IMPORT_CLASS_CODE 16 //class loading Assembly code
#ifdef FASTIVA_TARGET_OS_WIN32
	#if defined(KVM_DLL_EXPORTS) || defined(MIDP_DLL_EXPORTS)
		#define DLL_EXPORT __declspec(dllexport)
	#elif defined(KVM_DLL_IMPORTS)
		#define DLL_EXPORT __declspec(dllimport)
	#else
		#define DLL_EXPORT
	#endif
#else
	#define DLL_EXPORT
#endif
#elif defined(FASTIVA_TARGET_CPU_ARM)
#define	SIZE_OF_IMPORT_CLASS_CODE 16 //class loading Assembly code
#ifdef FASTIVA_TARGET_OS_WINCE
	#if defined(KVM_DLL_EXPORTS) || defined(MIDP_DLL_EXPORTS)
		#define DLL_EXPORT __declspec(dllexport)
	#elif defined(KVM_DLL_IMPORTS)
		#define DLL_EXPORT __declspec(dllimport)
	#else
		#define DLL_EXPORT
	#endif
#endif
#endif

//#ifdef _DEBUG
//#else
//	#define _FASTIVA_DBREAK()
//#endif


#ifdef FASTIVA_TARGET_OS_WIN32
	//#pragma warning(disable : 4715) // not all control paths return a value;
	//#pragma warning(disable : 4716) // no return;
	#pragma warning(disable : 4251) // class 'java_util_concurrent_CopyOnWriteArrayList_0Slice_EXVT$' needs to have dll-interface to be used by clients of struct 'java_util_concurrent_CopyOnWriteArrayList_0Slice_G$'
	#pragma warning(disable : 4275) // non dll-interface struct 'fastiva_Interface_G$' used as base for dll-interface struct 'java_io_ObjectInput_G$'
	#pragma warning(disable : 4101) // unreferenced local variable
	#pragma warning(disable : 4102) // unreferenced label;
	//#pragma	warning(disable : 4166) // illegal calling convention for constructor/destructor
	//#pragma	warning(disable : 4251) // class 'Val_A<int>' needs to have dll-interface
	//#pragma	warning(disable : 4786) // long name(for debugging) truncated 
	#pragma	warning(disable : 4200) // nonstandard extension used (zero-sized array in struct/union)
	#pragma	warning(disable : 4509) // nonstandard extension used: 'Foo::funcion' uses SEH and 'cleaner$$' has destructor
	//#pragma	warning(disable : 4035) // no return value
	//#pragma	warning(disable : 4291) // no return value
	//#pragma	warning(disable : 4800) // no return value
#ifdef _ARM_
	#define FASTIVA_NAKED_API __declspec(noinline)
#else
	#define FASTIVA_NAKED_API __declspec(naked)
#endif
#endif

#ifdef FASTIVA_TARGET_CPU_i486
#ifdef _MSC_VER
	#define _ASM_RET() __asm ret
#elif defined(__GNUC__)
	#define _ASM_RET() __asm__ __volatile__("ret");
#endif
#else 
	#define _ASM_RET()	//FASTIVA_DBREAK();
#endif

#ifdef FASTIVA_CPD540
	extern volatile unsigned long fm::curr_tick;
	#define fastiva_getTickCount0() fm::curr_tick
	#define FOX_JNI_CALL 
#elif defined(FASTIVA_TARGET_OS_LINUX)
	#define FOX_JNI_CALL __stdcall
	extern "C" unsigned long fox_sys_time_getTickCount();
	#define fastiva_getTickCount0()	fox_sys_time_getTickCount() 
#elif !defined(_WIN32) 
	extern volatile unsigned long rollovertime_ms;
	#define fastiva_getTickCount0() rollovertime_ms
	#define FOX_JNI_CALL 
#elif defined(UNDER_CE)
	unsigned long __stdcall GetTickCount();
	#define FOX_JNI_CALL __stdcall
	#define fastiva_getTickCount0()	GetTickCount()
#else 
	#define FASTIVA_GET_STACK_FRAME(var) {					\
		int addr = (int)&this->var;						\
		__asm mov eax, dword ptr [addr]					\
		__asm mov dword ptr [eax], ebp					\
	}
	unsigned long __declspec(dllimport) __stdcall GetTickCount();
	#define FOX_JNI_CALL __stdcall
	#define fastiva_getTickCount0()	GetTickCount()
#endif

#define FASTIVA_NOT_IMPL()  assert(false)
#define FASTIVA_DBREAK()	// ignore assert(false)
#ifdef _WIN32
	#define FASTIVA_LATE_IMPL() // ignore assert(false)
#else
	#define FASTIVA_LATE_IMPL() assert(false)
#endif
#define FASTIVA_IGNORE()	// ignore
#define ASSERT_K(exp)		assert(exp)

#ifndef FASTIVA_DBREAK
	#ifdef _DEBUG
		#define _DEBUG_ASSERT
		#define fox_debug_printf		fox_printf
		#define FASTIVA_DBREAK()		{ fox_debug_trace(__FUNCTION__, __FILE__, __LINE__); FOX_BREAK(); }
	#else
		static __inline void fox_debug_printf(const char*format,...) {}
		#define FASTIVA_DBREAK()		// ignore
	#endif
	#define FASTIVA_NOT_IMPL()	FASTIVA_DBREAK()
#endif


#ifdef __cplusplus
}
#endif
#endif // __FASTIVA_CONFIG_H__

/*
#define FASTIVA_SHORT_INTERFACE
*/
/**====================== end ========================**/
