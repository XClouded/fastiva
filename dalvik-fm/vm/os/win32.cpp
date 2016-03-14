/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Dalvik.h"

#include "os.h"

#include <sys/time.h>
#include <sys/mman.h>
#include <limits.h>
#include <errno.h>

#include <windows.h>
#include <winnt.h>
#include <fcntl.h>//<io.h>
//#include <utils/threads.h>

#if 0
/*
 * Conversion map for "nice" values.
 *
 * We use Android thread priority constants to be consistent with the rest
 * of the system.  In some cases adjacent entries may overlap.
 */
static const int kNiceValues[10] = {
    ANDROID_PRIORITY_LOWEST,                /* 1 (MIN_PRIORITY) */
    ANDROID_PRIORITY_BACKGROUND + 6,
    ANDROID_PRIORITY_BACKGROUND + 3,
    ANDROID_PRIORITY_BACKGROUND,
    ANDROID_PRIORITY_NORMAL,                /* 5 (NORM_PRIORITY) */
    ANDROID_PRIORITY_NORMAL - 2,
    ANDROID_PRIORITY_NORMAL - 4,
    ANDROID_PRIORITY_URGENT_DISPLAY + 3,
    ANDROID_PRIORITY_URGENT_DISPLAY + 2,
    ANDROID_PRIORITY_URGENT_DISPLAY         /* 10 (MAX_PRIORITY) */
};
#endif

void os_changeThreadPriority(Thread* thread, int newPriority)
{
	FASTIVA_LATE_IMPL();
#if 0
	if (newPriority < 1 || newPriority > 10) {
        ALOGW("bad priority %d", newPriority);
        newPriority = 5;
    }

    int newNice = kNiceValues[newPriority-1];
    pid_t pid = thread->systemTid;

    if (newNice >= ANDROID_PRIORITY_BACKGROUND) {
        set_sched_policy(dvmGetSysThreadId(), SP_BACKGROUND);
    } else if (getpriority(PRIO_PROCESS, pid) >= ANDROID_PRIORITY_BACKGROUND) {
        set_sched_policy(dvmGetSysThreadId(), SP_FOREGROUND);
    }

    if (setpriority(PRIO_PROCESS, pid, newNice) != 0) {
        std::string threadName(dvmGetThreadName(thread));
        ALOGI("setPriority(%d) '%s' to prio=%d(n=%d) failed: %s",
        pid, threadName.c_str(), newPriority, newNice, strerror(errno));
    } else {
        ALOGV("setPriority(%d) to prio=%d(n=%d)", pid, newPriority, newNice);
    }
#endif
}

int os_getThreadPriorityFromSystem()
{
	FASTIVA_LATE_IMPL();
    return THREAD_NORM_PRIORITY;
#if 0
    errno = 0;
    int sysprio = getpriority(PRIO_PROCESS, 0);
    if (sysprio == -1 && errno != 0) {
        ALOGW("getpriority() failed: %s", strerror(errno));
        return THREAD_NORM_PRIORITY;
    }

    int jprio = THREAD_MIN_PRIORITY;
    for (int i = 0; i < NELEM(kNiceValues); i++) {
        if (sysprio >= kNiceValues[i]) {
            break;
        }
        jprio++;
    }
    if (jprio > THREAD_MAX_PRIORITY) {
        jprio = THREAD_MAX_PRIORITY;
    }
    return jprio;
#endif
}

int os_raiseThreadPriority()
{
	FASTIVA_LATE_IMPL();
    return THREAD_NORM_PRIORITY;
#if 0
    /* Get the priority (the "nice" value) of the current thread.  The
     * getpriority() call can legitimately return -1, so we have to
     * explicitly test errno.
     */
    errno = 0;
    int oldThreadPriority = getpriority(PRIO_PROCESS, 0);
    if (errno != 0) {
        ALOGI("getpriority(self) failed: %s", strerror(errno));
    } else if (oldThreadPriority > ANDROID_PRIORITY_NORMAL) {
        /* Current value is numerically greater than "normal", which
         * in backward UNIX terms means lower priority.
         */
        if (oldThreadPriority >= ANDROID_PRIORITY_BACKGROUND) {
            set_sched_policy(dvmGetSysThreadId(), SP_FOREGROUND);
        }
        if (setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_NORMAL) != 0) {
            ALOGI("Unable to elevate priority from %d to %d",
                    oldThreadPriority, ANDROID_PRIORITY_NORMAL);
        } else {
            /*
             * The priority has been elevated.  Return the old value
             * so the caller can restore it later.
             */
            ALOGV("Elevating priority from %d to %d",
                    oldThreadPriority, ANDROID_PRIORITY_NORMAL);
            return oldThreadPriority;
        }
    }
    return INT_MAX;
#endif
}

void os_lowerThreadPriority(int oldThreadPriority)
{
	FASTIVA_LATE_IMPL();
#if 0
    if (setpriority(PRIO_PROCESS, 0, oldThreadPriority) != 0) {
        ALOGW("Unable to reset priority to %d: %s",
                oldThreadPriority, strerror(errno));
    } else {
        ALOGV("Reset priority to %d", oldThreadPriority);
    }
    if (oldThreadPriority >= ANDROID_PRIORITY_BACKGROUND) {
        set_sched_policy(dvmGetSysThreadId(), SP_BACKGROUND);
    }
#endif
}

void* brk_mem = NULL;
extern "C"
void* sbrk(size_t size) {
	if (brk_mem == NULL) {
		brk_mem = ::VirtualAlloc(NULL, 1024*1204*1204, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	}
	void* res = ::VirtualAlloc(brk_mem, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	return res;
}


void* mmap(void *addr, size_t length, int prot, int flags, int fd, size_t offset) {
#ifdef _WIN32
	assert(fd == -1);
	assert(offset == 0);
	int type = MEM_COMMIT;
	int protect = PAGE_EXECUTE_READWRITE;
	if ((prot & (PROT_READ | PROT_WRITE)) == 0) {
		//type = MEM_RESERVE;
	}

	void* res = ::VirtualAlloc(addr, length, type, protect);
	return res;
#else
	return mmap(NULL, length, prot, MAP_PRIVATE, fd, offset);
#endif
}


int munmap(void *addr, size_t length) {
#ifdef _WIN32
	::VirtualFree(addr, 0, length);
	return 0;
#else
	return munmap(NULL, length);
#endif
}

int madvise(void *addr, size_t length, int advice) {
	void* res;
	if (advice == MADV_NORMAL) {
		res = ::VirtualAlloc(addr, length, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	}
	else {
		assert(advice == MADV_DONTNEED);
		// @zee call MEM_DECOMMIT to clear memory with zeros.
		bool ensure_to_clear_memory_with_zeros = true;
		if (!ensure_to_clear_memory_with_zeros) {
			res = ::VirtualAlloc(addr, length, MEM_RESET, 0);
		}
		else {
			BOOL r1 = ::VirtualFree(addr, length, MEM_DECOMMIT);
			if (!r1) {
				return -1;
			}
			res = ::VirtualAlloc(addr, length, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			//res = ::VirtualAlloc(addr, length, MEM_RESET, 0);
		}
	}
	return (res == NULL) ? -1: 0;
}

int mprotect (void *addr, size_t len, int prot) {
	int flags;
	if (prot == PROT_NONE) {
		flags = PAGE_NOACCESS;
	}
	else if ((prot & PROT_EXEC) != 0) {
		flags = PAGE_EXECUTE_READWRITE;
	}
	else if ((prot & PROT_WRITE) == 0) {
		flags = PAGE_READONLY;
	}
	else {
		flags = PAGE_READWRITE;
	}
	DWORD oldFlags;
	BOOL res = ::VirtualProtect(addr, len, flags, &oldFlags);
	return res ? 0 : -1;
}

void SetStdOutToNewConsole()
{
  int hConHandle;
  long lStdHandle;
  FILE *fp;

  // allocate a console for this app
  AllocConsole();

  // redirect unbuffered STDOUT to the console
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  *stdout = *fp;

  setvbuf( stdout, NULL, _IONBF, 0 );
}


// pseudo functions.
// CheckJNI.cpp is for Linux only..
void dvmUseCheckedJniEnv(JNIEnvExt* pEnv) {}
void dvmUseCheckedJniVm(JavaVMExt* pVm) {}
void dvmCheckCallJNIMethod(const u4* args, JValue* pResult, const Method* method, Thread* self) {
	dvmCallJNIMethod(args, pResult, method, self);
}

int __android_log_bwrite(int32_t tag, const void *payload, size_t len) { return len; }
int __android_log_btwrite(int32_t tag, char type, const void *payload, size_t len) { return len; }




extern "C" void __cdecl __chkstk() {}