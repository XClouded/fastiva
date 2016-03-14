#ifndef _WIN32
//.c�����̱� ������ kernel�� include�� ��� �����Ͽ����� ���´�.
//GCC���� �������Ҷ��� g++�� �������ϱ� ������ �� ������ ����.
//.c�� .cpp�� �ٲٸ� �ùٸ��� ������ �ȴ�.
#include <kernel/kernel.h>
#endif

int __declspec(naked) _setjmp3(void* buf) {
#ifdef __GNUC__
	__asm__ __volatile__ (
		"movl 4(%esp), %ecx		\n\t"
		"movl %ebp, 4*4(%ecx)	\n\t"
		"movl %ebx, 4*1(%ecx)	\n\t"
		"movl %edi, 4*2(%ecx)	\n\t"
		"movl %esi, 4*3(%ecx)	\n\t"
		"movl %esp, 4*0(%ecx)	\n\t"
		"movl (%esp), %eax		\n\t"
		"movl %eax, 4*5(%ecx)	\n\t"
		"movl %esp, 4*8(%ecx)	\n\t"
		"xorl %eax, %eax		\n\t"
		"ret"
	);
#else
	__asm {
		mov ecx, dword ptr [esp + 4];
		mov dword ptr [ecx + 4*4], ebp;
		mov dword ptr [ecx + 4*1], ebx;
		mov dword ptr [ecx + 4*2], edi;
		mov dword ptr [ecx + 4*3], esi;
		mov dword ptr [ecx + 4*0], esp;
		mov eax, dword ptr [esp]; // ret_addr
		mov dword ptr [ecx + 4*5], eax;
		mov dword ptr [ecx + 4*8], esp;
		xor eax, eax;
		ret;
	};
#endif
}

