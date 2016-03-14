#include <precompiled_libcore.h>

#include <Kernel/Kernel.h>
#include <Kernel/Runnable.h>
#include <fastiva/Runtime.h>

/*
const int* FOX_FASTCALL(java_lang_Class_getInterfaceTable$)(
	java_lang_Class_p pClass,
	const fastiva_ClassContext* pContext
) {
	return pClass->getInterfaceTable$(pContext);
}
const int* FOX_FASTCALL(java_lang_Thread_getInterfaceTable$)(
	java_lang_Thread_p pThread,
	const fastiva_ClassContext* pContext
) {
	return pThread->getInterfaceTable$(pContext);
}
*/

extern "C" int FOX_FASTCALL(fm::importMoniker_impl)(int* pStack, int ret_addr_import_moniker);



#ifdef _ARM_

extern "C" void FOX_NO_RETURN jump_to_real_method_of_proxy(int* pStack, int func_addr);
extern "C" void wait_while_thunk_replace();

#else

static void FOX_NAKED jump_to_real_method_of_proxy(int* pStack, int func_addr) {
#ifdef __GNUC__
	//GCC������ __asm__���� �ڵ��� return_import_class���� Label�� ���ο� Label�̸����� 
	//������ �ʱ� ������ return_import_class�� �������� �����ϸ� �ȵȴ�.
	__asm__ __volatile__(
		"popl %edx						\n\t"
		"cmpl $0,%edx                	\n\t"
		"je        return_import_class1	\n\t"
																	
	"jump_to_real_method1:        		\n\t"
		"popl %edx                   	\n\t"
		"addl $8,%esp                	\n\t"
		"jmp  *%eax                  	\n\t"
																	
	"return_import_class1:        		\n\t"
		"popl %edx                   	\n\t"
		"addl $4,%esp                	\n\t"
		"ret                         	\n\t"
	);
#else
	_asm {
		int 3; // code ���� �ʿ�.
		mov esp, ecx
		mov eax, edx
		pop ecx;
		pop edx;
		add esp, 8; // ret_addr_of_importMoniker$ �� return_addr_of_importClass �� ����.
		jmp eax;
	}
#endif
}


/*
	[esp +  0] = ecx;
	[esp +  4] = edx;
	[esp +  8] = ret_addr : who called importClass_$();
	[esp + 12] = ret_addr : who called importClass();
*/

#if 0
FOX_NAKED void* fm::importClass_$$() {
#ifdef _ARM_
	FASTIVA_DBREAK();
	return 0;
#else

#ifdef __GNUC__
	
	__asm__ __volatile__(
        "pushl %edx                  \n\t" 
        "pushl %EAX                  \n\t" 
        "movl %esp,%EAX              \n\t" 
        "call fm::importMoniker_impl \n\t" 

        "movl 12(%esp),%edx          \n\t"
        "movl -4(%edx),%ecx          \n\t"
        "addl %edx,%ecx              \n\t"
        "cmpl %eax,%ecx              \n\t"
        "je        return_import_class2 \n\t"
                                                                                
"jump_to_real_method2:        \n\t"
        "leal -5(%edx),%eax          \n\t"
        "popl %ecx                   \n\t" 
        "popl %edx                   \n\t" 
        "addl $8,%esp                \n\t"
        "jmp  *%eax                  \n\t"

"return_import_class2:        \n\t"
        "popl %ecx                   \n\t" 
        "popl %edx                   \n\t" 
        "addl $4,%esp                \n\t"
        "jmp  *%eax                  \n\t"
	);
#else
	_asm push edx;
	_asm push ecx;
	//_asm xor ecx, ecx;
	_asm mov ecx, esp;
	_asm call fm::importClass_internal;

	_asm mov  edx, [esp+12];		// edx = ret_addr : who called importClass$();
	_asm mov  ecx, [edx-4];			// ecx = call_offset
	_asm add  ecx, edx				// ecx = method_addr = call_offset + pc + 4
	_asm cmp  ecx, eax;				// if (method_addr == import_class_of_each_class)
	_asm je	  return_import_class;  // thunk ó���� ������ ����, call importClass$()����
									// jmp each-func-addr�� �ٲ�� �ִ�.

jump_to_real_method:
	_asm lea  eax, [edx-5]
	_asm pop  ecx;
	_asm pop  edx;
	_asm add  esp, 8
	_asm jmp  eax
	
return_import_class:
	_asm pop  ecx;
	_asm pop  edx;
	_asm add  esp, 4
	_asm jmp  eax
#endif
#endif
}

#endif



#endif


#ifndef _ARM_
#pragma pack(1)
struct ProxyMethod_thunk {
	char call_method; 
	int  func_addr;
	short mov_eaxp;
	int  offset;
	char ret;

	int getOffset() {
		return offset;
	}

	bool isThunkOf(int addr) {
		return call_method == (char)0xE8
			&& mov_eaxp == (short)0xC7
			&& addr == func_addr + (int)(&mov_eaxp);
	}

	void setJumpAddr(void* addr) {
		char* pThunk = (char*)&mov_eaxp;
		// multi-thread safety�� ���ؼ� �ϴ� �κ� ���� ����.
		pThunk[0] = (char)0xE9; // jmp;
		*(int*)(pThunk + 1) = (int)addr - (int)pThunk - 5;

		// func_addr�� �ٲٴ� ���� ������ ������ �ʵ��� �ӽ� code�� �����Ѵ�.
		this->call_method = (char)0xA9; // test eax, 32bit_v;  

		this->func_addr = (int)addr - (int)&func_addr - 4;
		this->call_method = (char)0xE9; // jmp;
	}
};


struct ImportClass_thunk {
//C6 05 48 E8 44 00 00  mov  byte ptr [javax_microedition_midlet_MIDlet::g_context$ (44E848h)],0 
//E8 77 79 FD FF		call @ILT+1900(@fm::importMoniker$@0) (401771h) 
//C3					ret              
	union {
		struct {
			short mov_0; 
			int   pMoniker;
			char  dummy;
			char  call_method; 
			int   func_addr;
			char  ret;
		} m_proxy;
		struct {
			char mov_eax;
			int  pClass;
			char ret;
		} m_thunk;
	};


	fastiva_ClassContext* getContextMoniker() {
		return (fastiva_ClassContext*)m_proxy.pMoniker;
	}

	bool isResolved() {
		return m_thunk.mov_eax == (char)0xB8; // move eax, ??
	}

	ProxyMethod_thunk* getFirstProxyThunk() {
		return (ProxyMethod_thunk*)((int)this + 16);
	}

	bool enterReplaceMode() {
		return xchg8(&m_proxy.dummy, 0xFF) == 0;
		//code0 = encodeBranchCommand_arm((int)wait_while_thunk_replace - (int)&code0);
	}

	void setImportedClass(java_lang_Class_p pClass) {
		m_thunk.mov_eax = (char)0xB8; // mov eax;
		m_thunk.pClass = (int)pClass;
		m_thunk.ret    = (char)0xC3; // return;
	}

	static FOX_NAKED char FOX_FASTCALL(xchg8)(volatile void* dest, int v) {
		#ifdef __GNUC__
			__asm__ __volatile__ (
				"movl %0, %%edx \n\t"
				"movl %1, %%eax \n\t"
				"FOX_ASM_LOCK xchgl (%%edx), %%eax \n\t"
				:
				: "m"(dest), "m"(v)
			);	
		#else
			__asm {
				mov eax, edx;
				FOX_ASM_LOCK xchg byte ptr [ecx], al;
				ret;
			}
		#endif
	}
}

};



#pragma pack()
#else
/*
00000		 |?importClass$@java_lang_ArithmeticException_C$@@SIPAV1@XZ| PROC ; java_lang_ArithmeticException_C$::importClass$

  00000		 |$LN5@importClas|
  00000	e52de004	 str         lr, [sp, #-4]!
  00004		 |$M25567|
  00004	eb000000	 bl          |fm::importMoniker$|
  00008	e59f3004	 ldr         r3, [pc, #4]
  0000c	e5803000	 str         r3, [r0]
  00010	e49df004	 ldr         pc, [sp], #4
  00014		 |$LN6@importClas|
  00014		 |$LN7@importClas|
  00014	00000000	 DCD         |?g_context$@java_lang_ArithmeticException@@2UInstanceContextInfo@Fastiva@@B|



  00000		 |?init$@java_lang_ArithmeticException@@QAAPAXXZ| PROC ; java_lang_ArithmeticException::init$
  00000		 |$LN5@init$|
  00000	e52de004	 str         lr, [sp, #-4]!
  00004		 |$M25576|
  00004	eb000000	 bl          |?importClass$@java_lang_ArithmeticException_C$@@SIPAV1@XZ|
  00008	e3a03028	 mov         r3, #0x28
  0000c	e5803000	 str         r3, [r0]
  // ������ �� �� ������ �Ʒ� �ڵ尡 �����.
  00010	e6000010	 DCD         0xe6000010
  //
  00014	e49df004	 ldr         pc, [sp], #4
*/

static int getTargetAddress_arm(int* code_addr) {
	int bl_cmd = *code_addr;
	if ((bl_cmd & 0xFF000000) != 0xEB000000) {
		return 0;
	}
	int offset;
	if ((bl_cmd & 0x800000) != 0) {
		offset = (bl_cmd | 0xFF000000) << 2;
	}
	else {
		offset = (bl_cmd & 0xFFFFFF) << 2;
	}

	int target_address = (int)(code_addr) + offset + 8 ; // pc ��� offset�� 8byte �߰�.
	return target_address;
}

static int encodeBranchCommand_arm(int offset) {
	int offset_24 = ((offset - 8) >> 2) & 0xFFFFFF;
	return 0xeA000000 + offset_24;
}

struct ProxyMethod_thunk {
	int code0; // e52de004   str lr, [sp, #-4]!
	int code1; // eb000000	 bl  |?importClass$@java_lang_ArithmeticException_C$@@SIPAV1@XZ|
	int code2; // e3a03028	 mov r3, #0x28 = vtalbe offset;
	int code3; // e5803000	 str r3, [r0]
	//int code4; // e6000010	 DCD garbage ??
	int code5; // e49df004	 ldr pc, [sp], #4

	bool isThunkOf(int addr) {
		if (code0 != 0xe52de004) {
			return false;
		}
		int target = getTargetAddress_arm(&code1);
		return target == addr;
	}

	int getOffset() {
		KASSERT((code2 & ~0xFFF) == 0xe3a03000);
		int imm8 = code2;
		int shift = (imm8 & 0xF00);
		if (shift == 0) {
			return imm8 & 0xFFF;
		}
		/* ����! ���� code�� wce-compiler �� ���׿� ���� ������ ����� ���´�.
		jlonglong rotated_imm8 = (imm8 & 0xFF);
		rotated_imm8 += rotated_imm8 << 32;
		*/

		ulonglong rotated_imm8 = (imm8 & 0xFF);
		rotated_imm8 |= rotated_imm8 << 32;
		rotated_imm8 >>= (shift >> 7);
		return (int)rotated_imm8;

	}

	void setJumpAddr(void* func_addr) {
		code3 = (int)func_addr;
		/*
		code0 = 0xe59ff004; // LDR pc, [pc, #4];
		/*/
		code0 = encodeBranchCommand_arm((int)func_addr - (int)&code0);
		// wait_while_thunk_replace() �Լ� Loop�� ����� ret_addr�� ó���ϱ����Ͽ�
		// code1,2 �� �����Ѵ�.
		code2 = encodeBranchCommand_arm((int)func_addr - (int)&code2);
		// ���� code2�� ���� ������ ��, code1�� �����Ѵ�.
		code1 = 0xE8BD4000; // stmia  sp!, {lr} // LR�� �����ϰ�, sp�� �������� ����.
	}
};


struct ImportClass_thunk {
	int code0; //	e52de004	  str  lr, [sp, #-4]!
	int code1; //	eb000000	  bl   |fm::importMoniker$|
	int code2; //	e59f3004	  ldr  r3, [pc, #4] = code6;
	int code3; //	e5803000	  str  r3, [r0]
	int code4; //	e49df004	  ldr  pc, [sp], #4
	int code5; //				  DCD  |?g_context$@java_lang_ArithmeticException@@2UInstanceContextInfo@Fastiva@@B|

	fastiva_ClassContext* getContextMoniker() {
		int machine_code = code2;

		if ((machine_code & ~0xFFF) != 0xe59f3000) {
			FASTIVA_DBREAK();
		}
		// ARM���� PC ���� ������ �� ���� (LDR r3, [pc, #0x1C])
		// ���� PC offset�� 8�� ���� ���� ���ȴ�.
		int offset = (machine_code & 0xFFF) + 8;
		int dcd = ((int)&code2 + offset); // ClassMoniker �ּҰ� ����� address.
		fastiva_ClassContext* pContext = *(fastiva_ClassContext**)dcd;
		return pContext;
	}

	void setImportedClass(java_lang_Class_p pClass) {
		KASSERT((code0 & 0xFF000000) == 0xEA000000);
		code3 = (int)pClass;
		code0 = 0xe59f0004; //LDR RO, [pc, #4] = code3(=pCLASS);
		code1 = 0xE1A0F00E;// MOV PC, LR;
	}

	ProxyMethod_thunk* getFirstProxyThunk() {
		return (ProxyMethod_thunk*)((int)this + 24);
	}

	bool isResolved() {
		return *(volatile int*)&code0 != 0xe52de004;
	}

	int markImporting() {
		return fox_util_xchg32(&code4, 0);
	}

	bool enterReplaceMode() {
		// ����!!) bl�� ����ϸ�, rl�� ����� �� ���� �ǹǷ� bl ��ſ� b�� ����Ѵ�.
		// bl�� ����Ϸ���, stack�� ���� rl�� �����Ͽ� �ξ�߸� �ϴµ�,
		// code0 �� ����Ǵ� ������ code0�� ���࿩��(rl �� push�Ǿ�����) ��
		// ��Ȯ�� �Ǵ��� �� ����. 
		// �̿� b(jmp) ��ɰ� rl�� �̿��Ͽ� loop�� �����Ѵ�.
		int bl_cmd = encodeBranchCommand_arm((int)wait_while_thunk_replace - (int)&code0);
		int ord_cmd = fox_util_xchg32(&code0, bl_cmd);
		return (ord_cmd & 0xFF000000) != 0xeA000000;
	}
};
#endif


#ifdef _ARM_
	#define STATIC_THUNK_RET_OFFSET		8
	#define NEXT_DEBUG_THUNK(thunk)	(ProxyMethod_thunk*)((int)thunk + sizeof(ProxyMethod_thunk))
#else 
	#define STATIC_THUNK_RET_OFFSET		12
	#define NEXT_DEBUG_THUNK(thunk)	(ProxyMethod_thunk*)((int)thunk + 16)
#endif

#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER

int FOX_FASTCALL(fm::importMoniker_impl)(
	int* pStack, int ret_addr_of_importMoniker 
) {

	const fastiva_ClassContext* pContext;
	int func_addr_of_importClass = ret_addr_of_importMoniker - STATIC_THUNK_RET_OFFSET;
	ImportClass_thunk* pStaticThunk = (ImportClass_thunk*)func_addr_of_importClass;

	pContext = pStaticThunk->getContextMoniker();

	KASSERT(pContext->isRawContext());
	pContext = fm::validateContext(pContext);

	if (pContext == ADDR_ZERO) {
		/* 
		link ���н� �ߺ����� link �õ��� �����ϱ� ���Ͽ� jump-addr�� �����ϴ� Code�̴�.
		Performance�� �� ������ �����Ƿ� ������� �ʾƵ� �����ϴ�.

		void* jump_addr = fastiva_throw_NoClassDefFoundError;
		ProxyMethod_thunk* pProxyThunk = pStaticThunk->getFirstProxyThunk();
		((ProxyMethod_thunk*)pStaticThunk)->setJumpAddr(jump_addr);
		while (pProxyThunk->isThunkOf(func_addr_of_importClass)) {
			pProxyThunk->setJumpAddr(jump_addr);
			pProxyThunk = NEXT_DEBUG_THUNK(pProxyThunk);
		}
		*/
		fastiva_throwNoClassDefFoundError();
	}

	fastiva_Class_p pClass = pContext->m_pClass;
	#ifdef _ARM_
		int ret_addr_of_importClass = pStack[5]; 
		ProxyMethod_thunk* pCallee = (ProxyMethod_thunk*)(ret_addr_of_importClass - 8);
	#else
		int ret_addr_of_importClass = pStack[3];
		ProxyMethod_thunk* pCallee = (ProxyMethod_thunk*)(ret_addr_of_importClass - 5);
	#endif

	// Multi-thread ȯ�濡�� code�� �ٲٴ� ���� �ſ� �����ϴ�.
	// �� Thunk�� ���� Loop�� �����ϵ��� coding�ϰ�, 
	// Runtime�� Code�� �����ϴ� ���������� ���ο��� 
	// ������ Loop�� �����ϵ��� �����Ѵ�.
	int real_proc_addr = 0;
	fastiva_Synchronize lock(pClass);
	if (pStaticThunk->isResolved()) {
		// �ٸ� thread�� ���� resolve �Ǿ���.
		return 0;//(int)pCallee;
	}

	if (pStaticThunk->enterReplaceMode()) {
		fm::linkClass(pClass);
		ProxyMethod_thunk* pProxyThunk = pStaticThunk->getFirstProxyThunk();
		/*
		if (pProxyThunk->isThunkOf(func_addr_of_importClass)) {
		    // allocate() �Լ��� ���� ���� ó��.
			if (pProxyThunk->getOffset() == 0) {
				pStaticThunk = (ImportClass_thunk*)pProxyThunk;
				pStaticThunk->mov_eax = (char)0xB9; // = mov_ecx
				pStaticThunk->offset = (int)pContext;
				pStaticThunk->call_method = (char)0xE9;
				pStaticThunk->func_addr = 
					(int)((java_lang_Object_p(FOX_FASTCALL(*))(const fastiva_ClassContext*))fastiva.allocate)
					- (int)(&pStaticThunk->func_addr + 1);
				pProxyThunk = FIRST_PROXY_METHOD(pProxyThunk);
			}
			else {
				goto init_thunk;
			}
		}
		*/

		char* _vtable = (char*)pContext->getVirtualTable();
		while (pProxyThunk->isThunkOf(func_addr_of_importClass)) {
init_thunk:
			int offset = pProxyThunk->getOffset();
			KASSERT((uint)offset < 0xFFFF);
			void* func_addr = *(void**)(_vtable + offset);
			pProxyThunk->setJumpAddr(func_addr);
			if (pCallee == pProxyThunk) {
				real_proc_addr = (int)func_addr;
			}
			pProxyThunk = NEXT_DEBUG_THUNK(pProxyThunk);
		}

		// ���������� importClass()�� replace �Ѵ�.
		pStaticThunk->setImportedClass(pClass);

#ifdef _ARM_
		{
			void fm::discardCodeCache();
			fm::discardCodeCache();
		}
#endif

	}
	else {
		// Class Link�� �Ϸ���� ���� ���¿��� static �Լ��� �� ȣ��Ǿ���.
		// Link�� ���� ���� Thread�� �ƴ� �ٸ� Threa�� ���� ȣ��Ǿ��ٸ�,
		// linkClass()���ο��� ����ȭ�� �̷������,
		// ���� Thread ������ �ݺ� ȣ��Ǿ��ٸ�, ���� Link�� ������� ���� 
		// ���¿��� �Ʒ��� Code�� ����ȴ�. �ٸ� Thread�� ���� importClass��
		// ȣ��� ������ �����Ƿ�, Thunk�� �ּҴ� �������� �ʰ�, 
		// jump�� address�� �ٽ� �����Ͽ��� �Ѵ�.
		// ����) �Ʒ��� Code�� ����� Ȯ���� ���� 0�� ������.
		// �ܺ� Component ���� �ٸ� Componenent�� ���� class�� initStatic$()�� �����ϴ� ����,
		// ó�� Component�� �ٽ� �����ǰ�, �� �ٽ� ������ imprtMoniker$() ��
		// ȣ��Ǵ� ��쿡�� �Ʒ� Code�� ����Ǵµ�, initStatic$() ���� ���߿�
		// �̿� ���� ��Ȳ�� �߻��� Ȯ���� ���� ����.

		// ���� ret_addr_of_importMoniker$ �� stack�� ����Ǿ� ���� �ʴ�.

		if (pCallee->isThunkOf(func_addr_of_importClass)) {
			// proxy-method ���ο��� ImportClass() �� ȣ��� ���̴�.
			int func_addr;
			int offset = pCallee->getOffset();
			if (offset == 0) {
				FASTIVA_DBREAK();
				// thread-safe�ϰ� allocate()�� ���� thunk�� ������ �� ����.
				func_addr = (int)((java_lang_Object_p(FOX_FASTCALL(*))(const fastiva_ClassContext*))fastiva.allocate);
				pStack[0] = (int)pContext; // r0 or ecx = pContext;
			}
			else {
				char* _vtable = (char*)pContext->getVirtualTable();
				func_addr = *(int*)(_vtable + offset);
			}
			real_proc_addr = func_addr;
		}
	}

	if (real_proc_addr != 0) {
		/* �� ��ġ���� jump�ؼ��� �ȵȴ�. r4-r11 register�� save�Ǿ� ���� �ʴ�.*/
		// jump_to_real_method_of_proxy(pStack, real_proc_addr);
	}

	return real_proc_addr;
}
#endif

#ifdef _ARM_
extern "C" java_lang_Class_p fm::importMoniker$();

#else
FOX_NAKED java_lang_Class_p fm::importMoniker$(
) {
#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER

#ifdef __GNUC__
	__asm__ __volatile__ (
        "pushl %edx                  \n\t" 
        "pushl %ecx                  \n\t" 
        "movl %esp,%ecx              \n\t" 
        "call fm::importMoniker_impl \n\t" 

        "movl 12(%esp),%edx          \n\t" 
        "movl -4(%edx),%ecx          \n\t"               
        "addl %edx,%ecx              \n\t"
        "cmpl %eax,%ecx              \n\t" 
        "je        return_import_class3 \n\t" 

"jump_to_real_method3:        \n\t"
        "leal -5(%edx),%eax          \n\t"
        "popl %ecx                   \n\t" 
        "popl %edx                   \n\t" 
        "addl $8,%esp                \n\t"
        "jmp  *%eax                  \n\t"

"return_import_class3:        \n\t"
        "popl %ecx                   \n\t" 
        "popl %edx                   \n\t" 
        "addl $4,%esp                \n\t"
        "jmp  *%eax                  \n\t"
	);
#else
	_asm {
		push edx;
		push ecx;
		mov ecx, esp;
		mov edx, [esp + 8];
		call fm::importMoniker_impl;

		pop  ecx;
		pop  edx;

		cmp  eax, 0;
		jne  jump_to_real_method

		add  esp, 4  // importMoniker�� ret_addr�� ����
		pop  eax;    // eax = ret_addr of importClass$;
		sub  eax, 5;
		jmp  eax;    // importClass$�� ȣ���� ��ġ�� ���ư� �ٽ� importClass$�� ȣ���Ѵ�.

		test  eax, 0x12345678

jump_to_real_method:
		add  esp, 8; // ret_addr_of_importMoniker$ �� return_addr_of_importClass �� ����.
		jmp  eax;
	}
#endif
#else
	_asm { ret }
#endif
}
#endif

//fastiva_Runtime g_fastivaRuntime = { 0 };

void fm::initThunk(fastiva_Runtime* pFastivaRuntime) {
#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER

#if 0 // ndef _ARM_
	#define INIT_THUNK(thunk)								\
			pFastivaRuntime.thunk.mov_addr_eax = 0xB8;					\
			pFastivaRuntime.thunk.addr = (void *)thunk;		\
			pFastivaRuntime.thunk.jmp_eax = (int)0xE0FF;	

	#define INIT_THUNK2(class, thunk)								\
			pFastivaRuntime.thunk.mov_addr_eax = 0xB8;					\
			pFastivaRuntime.thunk.addr = (void *)class::thunk;		\
			pFastivaRuntime.thunk.jmp_eax = (int)0xE0FF;	
#else
	#define INIT_THUNK(thunk)								\
			*(void**)&pFastivaRuntime->thunk = (void *)thunk;		\

	#define INIT_THUNK2(class, thunk)								\
			*(void**)&pFastivaRuntime->thunk = (void *)class::thunk;		\

#endif

//	INIT_THUNK(java_lang_Class_getInterfaceTable$);
//	INIT_THUNK(java_lang_Thread_getInterfaceTable$);
	INIT_THUNK(fm::importMoniker$);
	INIT_THUNK(fastiva_throwArrayIndexOutOfBoundsException);

	INIT_THUNK2(Fastiva, allocate);
	INIT_THUNK2(Fastiva, allocateEx);
	INIT_THUNK2(Fastiva, allocateMultiArray);
	INIT_THUNK2(Fastiva, allocateMultiPrimitiveArray);
	INIT_THUNK2(Fastiva, allocatePrimitiveArray);
	INIT_THUNK2(Fastiva, allocatePointerArray);
	INIT_THUNK2(Fastiva, allocateInitializedArray);

	INIT_THUNK2(Fastiva, setInstanceField);

	typedef void (FOX_FASTCALL(*setStaticObjectField))(
		fastiva_Class_p pClass, 
		fastiva_Instance_p pValue, 
		void* pFieldPos
	);

	typedef void (FOX_FASTCALL(*setStaticInterfaceField))(
		fastiva_Class_p pClass, 
		fastiva_Interface_p pValue, 
		void* pFieldPos
	);

	*(void**)&pFastivaRuntime->setStaticField = (setStaticObjectField)fm::setStaticField;
	*(void**)&pFastivaRuntime->setStaticFieldEx = (setStaticInterfaceField)fm::setStaticField;
	INIT_THUNK2(Fastiva, setArrayItem);
	INIT_THUNK2(Fastiva, setAbstractArrayItem);


	INIT_THUNK2(Fastiva, invokeJNI);
	INIT_THUNK2(Fastiva, createString);
	INIT_THUNK2(Fastiva, createStringA);
	INIT_THUNK2(Fastiva, createStringW);
	INIT_THUNK2(Fastiva, createUTFString);
	INIT_THUNK2(Fastiva, getUnicodeLengthOfUTF8);
	INIT_THUNK2(Fastiva, createUTFStringA);
	INIT_THUNK2(Fastiva, getUTFLength);
	INIT_THUNK2(Fastiva, getUTFChars);
	INIT_THUNK2(Fastiva, createAsciiString);
	INIT_THUNK2(Fastiva, createAsciiStringA);
	INIT_THUNK2(Fastiva, memset);
	INIT_THUNK2(Fastiva, getTickCount);
	INIT_THUNK2(Fastiva, monitorEnter);
	INIT_THUNK2(Fastiva, monitorExit);

	INIT_THUNK2(Fastiva, beginSynchronized);
	INIT_THUNK2(Fastiva, endSynchronized);

	INIT_THUNK2(Fastiva, linkSynchronized);
	INIT_THUNK2(Fastiva, unlinkSynchronized);

	INIT_THUNK2(Fastiva, throwException);
	INIT_THUNK2(Fastiva, pushExceptionHandler);
	INIT_THUNK2(Fastiva, removeExceptionHandler);
	INIT_THUNK2(Fastiva, rethrow);

	INIT_THUNK2(Fastiva, dispatchNativeException);
	INIT_THUNK2(Fastiva, beginManagedSection_$$);
	INIT_THUNK2(Fastiva, endManagedSection_$$);
	INIT_THUNK2(Fastiva, enterNativeSection_$$);
	INIT_THUNK2(Fastiva, leaveNativeSection_$$);
	
	
	INIT_THUNK2(Fastiva, checkInstanceOf);
	INIT_THUNK2(Fastiva, checkImplemented);
	INIT_THUNK2(Fastiva, isInstanceOf);
	INIT_THUNK2(Fastiva, isImplemented);
	INIT_THUNK2(Fastiva, checkArrayInstanceOf);
	INIT_THUNK2(Fastiva, checkPrimitiveArrayInstanceOf);
	INIT_THUNK2(Fastiva, isArrayInstanceOf);
	INIT_THUNK2(Fastiva, isPrimitiveArrayInstanceOf);
/*
	INIT_THUNK2(Fastiva, checkInternalClass);
	INIT_THUNK2(Fastiva, newInstance);
*/	
	INIT_THUNK2(Fastiva, jniProlog);
	INIT_THUNK2(Fastiva, jniEpilog);
	INIT_THUNK2(Fastiva, lockGlobalRef);
	INIT_THUNK2(Fastiva, releaseGlobalRef);
//	INIT_THUNK2(Fastiva, lockLocalRef);
//	INIT_THUNK2(Fastiva, releaseLocalRef);
	INIT_THUNK2(Fastiva, checkImported);
	INIT_THUNK2(Fastiva, linkClass_$$);
	INIT_THUNK2(Fastiva, initClass_$$);
#endif
}

fastiva_Package* FOX_FASTCALL(fm::loadLibrary)(const char* pLibName, int nameLen) {
#if !FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER
	return (fastiva_Package*)FASTIVA_NULL;
#else

	CHECK_STACK_FAST();
	fastiva_Package* pBundle = ADDR_ZERO;
	typedef fastiva_Package* (*INIT_FASTIVA_PROC2)(void* pKernel);
	extern fox_Task fastivaMainThread;
	extern void fastiva_GC_disable_GC(java_lang_Object_p pObj);
	//extern void fastiva_GC_doLightGC();
	extern Byte_ap fm::loadExcutable(java_lang_String_p pStr);
	java_lang_Object* fm::getClassLoader(java_lang_String_p pStr);

#ifdef FASTIVA_TARGET_CPU_THUMB
	java_lang_String_p pStr = createFastivaString(pLibName);
	Byte_ap buff = fm::loadExcutable(pStr);
	// zip ���� ����� ���Ͽ� �ܺο��� loading�ϵ��� �Ͽ���.
	if ((java_lang_Object*)buff == FASTIVA_NULL) {
		fox_debug_printf("[ERROR] load lib fail %s ..\n", pLibName);
		return ADDR_ZERO;
	}
	//fastiva_GC_disable_GC(buff);
	extern Kernel  g_kernel;
	char* imageOrigin = (char*)&buff[0];
	if (memcmp(imageOrigin + 10, "FV10", 4) != 0) {
		// file�� ���� ó���� �ƴ϶�, 10��° ����Ʈ���� �˻�.
		return ADDR_ZERO;
	}
	INIT_FASTIVA_PROC2 initFastiva = (INIT_FASTIVA_PROC2)((int)imageOrigin+1);
	pBundle = (*initFastiva)(&g_kernel); // +1 is Thunk Masking
	//pBundle->m_pShadowAddr = (char*)pBundle->m_pShadowAddr + (int)imageOrigin;
#else
	//if (g_fastivaRuntime.allocate == 0) { //.mov_addr_eax != 0xB8) {
	//	fm::initThunk();
	//}

	FASTIVA_INIT_LIB_PROC FOX_FASTCALL(fastiva_win32_loadLibrary)(const char* pLibName, int nameLen);
	FASTIVA_INIT_LIB_PROC pfnInitLib = fastiva_win32_loadLibrary(pLibName, nameLen);
	if (pfnInitLib == ADDR_ZERO) {
		fox_debug_printf("[ERROR] load lib %s Fail!\n", pLibName);
		return ADDR_ZERO;
	}

	fastiva_Module* pModule = pfnInitLib(ADDR_ZERO);//&g_fastivaRuntime);
	pModule->init();
	/*
	pBundle->m_pCodeImage = (Byte_A::HEADER*)buff;
	/*/
	//Byte_ap buff = Byte_A::create$(4);
	//pBundle->m_pCodeImage = (Byte_A::HEADER*)buff;
	//char* imageOrigin = (char*)&buff[0];
	//*/
	//pBundle->m_pShadowAddr = (void*)((char*)pBundle->m_pShadowAddr - (char*)&buff[0]);
#endif
	//fm::registerComponent(pBundle, (int)imageOrigin);
	//fox_debug_printf("[Fastiva] load lib %s OK!\n", pLibName);
	
	@todo libName�� ������ �̸��� ���� package�� ��ȯ�Ѵ�.
	return pBundle;
#endif //FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER

}

