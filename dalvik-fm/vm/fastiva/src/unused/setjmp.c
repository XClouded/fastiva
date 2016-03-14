#ifndef _WIN32
//.c파일이기 때문에 kernel을 include할 경우 컴파일에러가 나온다.
//GCC에서 컴파일할때는 g++로 컴파일하기 때문에 별 문제가 없다.
//.c를 .cpp로 바꾸면 올바르게 컴파일 된다.
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

