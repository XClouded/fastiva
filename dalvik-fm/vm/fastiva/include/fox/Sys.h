#ifndef __FOX_SYS_H__
#define __FOX_SYS_H__

#include <fox/Config.h>
#include <fox/errcode.h>

#ifdef __cplusplus
extern "C" {
#endif	

FOX_BOOL fox_pal_init();

#undef _T

#ifdef UNICODE
	typedef wchar_t TCHAR;
	#define _T(t)	L##t
#else
	typedef char TCHAR;
	#define _T(t)	t
#endif

void FOX_FASTCALL(sys_util_setCurrentWorkingDirectory)(const TCHAR* path);

const TCHAR* FOX_FASTCALL(sys_util_getCurrentDirectory)();

// 마지막 seperator('/')를 포함한다.
int  FOX_FASTCALL(sys_util_getCurrentDirectoryLength)();

// 문자열의 끝을 반환한다.
wchar_t* FOX_FASTCALL(sys_util_copyCurrentDirectoryInto)(wchar_t* path_buf);

void FOX_FASTCALL(sys_util_setExecutedFilePath)(const TCHAR* pszFilePath);

const TCHAR* FOX_FASTCALL(sys_util_getExecutedFilePath)();

int FOX_FASTCALL(sys_util_canonicalize)(TCHAR *orig_path, TCHAR *result, int size);

void FOX_FASTCALL(sys_util_showFatalError)(const TCHAR* title, const TCHAR* message);

#ifdef __cplusplus
}
#endif	

#endif // __FOX_SYS_H__
