#include <precompiled_libcore.h>

#include "kernel/instance_Q.h"
#include <fox/kernel/TaskContext.h>
#if 0
//FOX_NAKED 
void fm::SafeInstanceQ::insert(fastiva_Instance_p pNewEntry) {
#if 1 // def _ARM_
	while (true) {
		fastiva_Instance_p pFirst = this->m_pFirst;
		pNewEntry->m_pNext$ = pFirst;
		/**
		pNewEntry->m_pNext$ �� ���������� ����� �� �����Ƿ�, �ݵ�� pFirst�� ����� ��
		*/
		if (fox_util_cmpxchg32(&this->m_pFirst, (int)pNewEntry, (int)pFirst)) {
			break;
		}
	}
#else
	_asm {
		mov eax, [ecx].m_pFirst;
	set_next_entry:
		mov [edx].m_pNext$, eax;
		FOX_ASM_LOCK cmpxchg [ecx].m_pFirst, edx
		// ���� thread���� ������ instance�� �߰��Ͽ��� ����.
		jne set_next_entry;
		ret;
	}
#endif
}

//FOX_NAKED 
void fm::SafeInstanceQ::insert(fastiva_Instance_p pNewRoot, fastiva_Instance_p pEnd) {
#if 1 //def _ARM_
	while (true) {
		fastiva_Instance_p pFirst = this->m_pFirst;
		pEnd->m_pNext$ = pFirst;
		/**
		pNewEntry->m_pNext$ �� ���������� ����� �� �����Ƿ�, �ݵ�� pFirst�� ����� ��
		*/
		if (fox_util_cmpxchg32(&this->m_pFirst, (int)pNewRoot, (int)pFirst)) {
			break;
		}
	}
#else
	_asm {
		push ebx;
		mov  ebx, [esp + 8];
		mov  eax, [ecx].m_pFirst;
	set_next_entry:
		mov  [ebx].m_pNext$, eax;
		FOX_ASM_LOCK cmpxchg [ecx].m_pFirst, edx
		// ���� thread���� ������ instance�� �߰��Ͽ��� ����.
		jne set_next_entry;
		pop ebx;
		ret 4;
	}
#endif
}

#endif
