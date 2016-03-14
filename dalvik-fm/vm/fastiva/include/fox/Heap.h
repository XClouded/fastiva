#ifndef __FOX_HEAP_H__
#define __FOX_HEAP_H__

#include <fox/Config.h>

#ifdef __cplusplus
extern "C" {
#endif	

//==========================================================//
//			HEAP MANAGER									//
//==========================================================//

//allocation실패시 null 값을 리턴
void* FOX_FASTCALL(fox_heap_malloc)(int size);

// 잘못된 mem을 전달하면 Heap 이 깨진다. 
void FOX_FASTCALL(fox_heap_free)(void* mem);

void FOX_FASTCALL(fox_heap_compact)();

FOX_BOOL FOX_FASTCALL(fox_heap_init)(int max_heap_size);

void FOX_FASTCALL(fox_heap_setMaxHeapSize)(int max_heap_size);
	
// 참고) java_lang_Runtime::totalMemory()
int FOX_FASTCALL(fox_heap_getTotalSize)();

// 참고) java_lang_Runtime::freeMemory()
int FOX_FASTCALL(fox_heap_getFreeSize)();

// memory page size를 알아낸다.
int FOX_FASTCALL(sys_heap_getPageSize)();

// memory page 단위로 memory를 할당한다. pMem이 0이면 새로운 주소값을 반환.
void* FOX_FASTCALL(sys_heap_virtualAlloc)(void* pMem, int size);

// virtual-alloc된 memory page 를 해제한다.
void FOX_FASTCALL(sys_heap_virtualFree)(void *pMem, int size);


#ifdef __cplusplus
}
#endif	

#endif // __FOX_HEAP_H__
