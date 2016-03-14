#ifndef __KERNEL_DEVICE_H__
#define __KERNEL_DEVICE_H__

#include <fastiva/config.h>
#include "KvmEvent.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SUPPORTS_COLOR	       1
#define SUPPORTS_POINTER       2
#define SUPPORTS_MOTION        4
#define SUPPORTS_REPEAT        8
#define SUPPORTS_DOUBLEBUFFER 16

#define ALERT_INFO         1
#define ALERT_WARNING      2
#define ALERT_ERROR        3
#define ALERT_ALARM        4
#define ALERT_CONFIRMATION 5


typedef struct {
	int  screenWidth;
	int	 screenHeight;
	int	 backgroundColor;
	int  support_flags;
	char* appName;
	void* pScreenBuffer;

	void (*setScreenBuffer)(void* buffer);
	void (*showError)(char* err);
	void (*setTimer)(int type, int millis);
	int  (*playSound)(int alertType);
	int  (*getKvmEvent)();
	void (*refreshScreen)(int x, int y, int width, int height);
} DisplayDevice;


#if defined(FASTIVA_TARGET_OS_WIN32) || defined(FASTIVA_TARGET_OS_WINCE)
	int _declspec(dllexport) start_fastiva(DisplayDevice * pDevice);
#endif

#ifdef __cplusplus
};
#endif

#endif // __KERNEL_DEVICE_H__

/**====================== end ========================**/