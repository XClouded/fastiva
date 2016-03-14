#ifndef __KVM_EVENT_H__
#define __KVM_EVENT_H__


#include <fastiva/config.h>

#ifdef __cplusplus
extern "C" {
#endif

const int KVM_KEY_UP = 1;
const int KVM_KEY_DOWN = 6;
const int KVM_KEY_LEFT = 2;
const int KVM_KEY_RIGHT = 5;
const int KVM_KEY_FIRE = 8;
const int KVM_KEY_SOFT1 = 9;	// GAME_A
const int KVM_KEY_SOFT2 = 0x10;	// GAME_B
const int KVM_KEY_CANCEL = 11;
const int KVM_KEY_END = 0x0A;
const int KVM_KEY_GAME_D = 12;
const int KVM_KEY_0 = 48;
const int KVM_KEY_1 = 49;
const int KVM_KEY_2 = 50;
const int KVM_KEY_3 = 51;
const int KVM_KEY_4 = 52;
const int KVM_KEY_5 = 53;
const int KVM_KEY_6 = 54;
const int KVM_KEY_7 = 55;
const int KVM_KEY_8 = 56;
const int KVM_KEY_9 = 57;
const int KVM_KEY_STAR = 42;
const int KVM_KEY_POUND = 35;

enum KVMEventTypes {
    invalidKVMEvent    = -1,
    keyDownKVMEvent    = 0,
    keyUpKVMEvent      = 1,
    keyRepeatKVMEvent  = 2,
    penDownKVMEvent    = 3,
    penUpKVMEvent      = 4,
    penMoveKVMEvent    = 5,
    timerKVMEvent      = 6,
    commandKVMEvent    = 7,
    repaintKVMEvent    = 8,
    appStopKVMEvent    = 9,
    keyTypedKVMEvent   = 10,
    lastKVMEvent       = 10
};

/*  The event record. */
typedef struct {
    enum KVMEventTypes type:8;
    signed int         chr:24;
    short              screenX;
    short              screenY;
} KvmEvent;

enum fm::KeyEventType {
    keyDownEvent    = 0,
    keyUpEvent      = 1,
    keyRepeatEvent  = 2,
    keyTypedEvent   = 10
};

enum fm::PenEventType {
    penDownEvent    = 3,
    penUpEvent      = 4,
    penMoveEvent    = 5
};

#ifdef FASTIVA_TARGET_OS_WIN32
	_declspec(dllexport) void fm::sendKeyEvent(fm::KeyEventType type, int key);
	_declspec(dllexport) void fm::sendPenEvent(fm::PenEventType type, int x, int y);
#else
	void fm::sendKeyEvent(fm::KeyEventType type, int key);
	void fm::sendPenEvent(fm::PenEventType type, int x, int y);
#endif
 

typedef int (*GET_EVENT_PROC)();

#ifdef __cplusplus
}
#endif

#endif // __KVM_EVENT_H__


/**====================== end ========================**/