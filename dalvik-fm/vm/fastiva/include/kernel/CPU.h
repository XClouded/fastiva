#ifndef __KERNEL_CPU_H__
#define __KERNEL_CPU_H__


#include <fastiva/config.h>

#ifdef FASTIVA_TARGET_CPU_i486
	#define _ASM_GET_SP(_sp)						\
		__asm mov _sp, esp;
	
	#define _ASM_SET_THREAD_STACK(thread_stack)		\
		__asm mov eax, esp							\
		__asm mov esp, thread_stack					\
		__asm sub esp, 8							\
		__asm push eax								\

	#define _ASM_CALL_START_THREAD(pthread)			\
		start_fastiva_thread(pthread);				\
		__asm pop esp								\


#endif // FASTIVA_TARGET_CPU_i486


#if defined(FASTIVA_TARGET_CPU_ARM) || defined (FASTIVA_TARGET_CPU_THUMB)

	extern "C" {
		int _asm_get_sp();
		void _asm_call_start_thread(void* pthread);
	}

	#define _ASM_GET_SP(_sp)						\
		(_sp = _asm_get_sp())

	#define _ASM_SET_THREAD_STACK(thread_stack)		// do nothing

	#define _ASM_CALL_START_THREAD(pthread)			\
		_asm_call_start_thread((void*)pthread);

#endif // FASTIVA_TARGET_CPU_ARM || FASTIVA_TARGET_CPU_THUMB

#endif // __KERNEL_CPU_H__

/**====================== end ========================**/