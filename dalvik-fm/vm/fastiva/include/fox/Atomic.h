#ifndef __FOX_ATOMIC_H__
#define __FOX_ATOMIC_H__

#include <fox/Config.h>

#if SINGLE_TASK_APPLICATION
	extern volatile int g_fastivaActiveTaskCount; && not used &&
	#define FASTIVA_INC_ACTIVE_TASK_COUNT() fox_util_inc32(&g_fastivaActiveTaskCount);  fox_task_yield();
	#define FASTIVA_DEC_ACTIVE_TASK_COUNT() fox_util_dec32(&g_fastivaActiveTaskCount);
#else
	static const int g_fastivaActiveTaskCount = 0xffff;
	#define FASTIVA_INC_ACTIVE_TASK_COUNT() //
	#define FASTIVA_DEC_ACTIVE_TASK_COUNT() //
#endif

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif	

int FOX_FASTCALL(fox_util_add32)(volatile void* dest, int v);

int FOX_FASTCALL(fox_util_sub32)(volatile void* dest, int v);

int FOX_FASTCALL(fox_util_inc32)(volatile void* dest);

int FOX_FASTCALL(fox_util_dec32)(volatile void* dest);


void FOX_FASTCALL(fox_util_or32)(volatile void* dest, int v);

void FOX_FASTCALL(fox_util_and32)(volatile void* dest, int v);

int FOX_FASTCALL(fox_util_xchg32)(volatile void* dest, int v);

char FOX_FASTCALL(fox_util_xchg8)(volatile void* dest, int v);

void FOX_FASTCALL(fox_util_spinLock32)(volatile int* pLock);

void FOX_FASTCALL(fox_util_spinUnlock32)(volatile int* pLock);

// returns true if exchanged;
FOX_BOOL FOX_FASTCALL(fox_util_cmpxchg32)(volatile void* dest, int newV, int oldV);

#ifdef __cplusplus
}
#endif	


#else
#include <sys/atomics.h>

#if JPP_NO_SMP_MEMORY_BARRIER
  #define barrier()       // nothing
  #define barrier_after() // nothing
#else
  #define barrier() __asm__ __volatile__("dmb": : :"memory")
  #define barrier_after() //__asm__ __volatile__("dmb": : :"memory")
#endif

inline int FOX_FASTCALL(fox_util_add32)(volatile void* dest, int v) {
	if (g_fastivaActiveTaskCount <= 1) {
		return *(int*)dest += v;
	}

	while (true) {
		int old = *(int*)dest;
		int _new = old + v;
		barrier();
		if (!__atomic_cmpxchg(old, _new, (int*)dest)) {
		barrier_after();
			return _new;
		}
	}
}

inline int FOX_FASTCALL(fox_util_sub32)(volatile void* dest, int v) {
	if (g_fastivaActiveTaskCount <= 1) {
		return *(int*)dest -= v;
	}
	while (true) {
		int old = *(int*)dest;
		int _new = old - v;
		barrier();
		if (!__atomic_cmpxchg(old, _new, (int*)dest)) {
		barrier_after();
			return _new;
		}
	}
}


inline int FOX_FASTCALL(fox_util_inc32)(volatile void* dest) {
	if (g_fastivaActiveTaskCount <= 1) {
		return (*(int*)dest += 1);
	}
	barrier();
	int r = __atomic_inc((int*)dest) + 1;
	barrier_after();
	return r;
}


inline int FOX_FASTCALL(fox_util_dec32)(volatile void* dest) {
	if (g_fastivaActiveTaskCount <= 1) {
		return (*(int*)dest -= 1);
	}
	barrier();
	int r = __atomic_dec((int*)dest) - 1;
	barrier_after();
	return r;
}



inline int FOX_FASTCALL(fox_util_xchg32)(volatile void* dest, int x) {
	if (g_fastivaActiveTaskCount <= 1) {
		int v = *(int*)dest;
		*(int*)dest = x;
		return v;
	}
	barrier();
	int r = __atomic_swap(x, (int*)dest);
	barrier_after();
	return r;
}


inline FOX_BOOL FOX_FASTCALL(fox_util_cmpxchg32)(volatile void* dest, int newV, int oldV) {
	if (g_fastivaActiveTaskCount <= 1) {
		if (*(int*)dest == oldV) {
			*(int*)dest = newV;
			return true;
		}
		return false;
	}
	barrier();
	int r = !__atomic_cmpxchg(oldV, newV, (int*)dest);
	barrier_after();
	return r;
}


inline void FOX_FASTCALL(fox_util_or32)(volatile void* dest, int v) {
	if (g_fastivaActiveTaskCount <= 1) {
		*(int*)dest |= v;
	}
	while (true) {
		int old = *(int*)dest;
		int _new = old | v;
	barrier();
		if (!__atomic_cmpxchg(old, _new, (int*)dest)) {
	barrier_after();
			break;
		}
	}
}

inline void FOX_FASTCALL(fox_util_and32)(volatile void* dest, int v) {
	if (g_fastivaActiveTaskCount <= 1) {
		*(int*)dest &= v;
	}
	while (true) {
		int old = *(int*)dest;
		int _new = old & v;
	barrier();
		if (!__atomic_cmpxchg(old, _new, (int*)dest)) {
	barrier_after();
			break;
		}
	}
}

#endif

#endif // __FOX_ATOMIC_H__
