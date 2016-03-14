#ifndef __FOX_HEAP_H__
#define __FOX_HEAP_H__

#include <fox/Config.h>

#ifdef __cplusplus
extern "C" {
#endif	

//==========================================================//
//			HEAP MANAGER									//
//==========================================================//

//allocation���н� null ���� ����
void* FOX_FASTCALL(fox_heap_malloc)(int size);

// �߸��� mem�� �����ϸ� Heap �� ������. 
void FOX_FASTCALL(fox_heap_free)(void* mem);

void FOX_FASTCALL(fox_heap_compact)();

FOX_BOOL FOX_FASTCALL(fox_heap_init)(int max_heap_size);

void FOX_FASTCALL(fox_heap_setMaxHeapSize)(int max_heap_size);
	
// ����) java_lang_Runtime::totalMemory()
int FOX_FASTCALL(fox_heap_getTotalSize)();

// ����) java_lang_Runtime::freeMemory()
int FOX_FASTCALL(fox_heap_getFreeSize)();

// memory page size�� �˾Ƴ���.
int FOX_FASTCALL(sys_heap_getPageSize)();

// memory page ������ memory�� �Ҵ��Ѵ�. pMem�� 0�̸� ���ο� �ּҰ��� ��ȯ.
void* FOX_FASTCALL(sys_heap_virtualAlloc)(void* pMem, int size);

// virtual-alloc�� memory page �� �����Ѵ�.
void FOX_FASTCALL(sys_heap_virtualFree)(void *pMem, int size);


#ifdef __cplusplus
}
#endif	

#endif // __FOX_HEAP_H__
