#ifndef __KERNEL_DYNA_CLASS_H__
#define __KERNEL_DYNA_CLASS_H__

#include <fastiva/config.h>


"fastiva/class.h" =====================-=============================
#define	DECL_FASTIVA_COMPONENT
		.........
		JProtected static const fastiva_ClassContext g_classContext;
		JPublic static THIS$Class* import$class();

Java VM ������ init$Static�� �Ҹ���� ����

	class Parent {
		static int id = 101;
		static int child_id = Child.id;
		static int id2 = 102;
		static int child_id2 = Child.id2;
	};
	class Child extends Parent {
		static int id = 901;
		static int parent_id = Parent.id;
		static int id2 = 902;
		static int parent_id2 = Parent.id2;
	}
	
	void main() {
		1. 	print(Child.parent_id) => 101;
		   	print(Parent.child_id) => 0;
		   	print(Child.parent_id2) => 102;
		   	print(Parent.child_id2) => 0;
		   	// Parent::int$static�� ������ ����� �� Child::init$static�� ����Ǿ���.
		   	
		2. 	print(Parent.child_id) => 901;
			print(Child.parent_id) => 101;
			print(Parent.child_id2) => 902;
			print(Child.parent_id2) => 0;
		   	// Parent::int$static�� ����Ǵ� ���� Child::init$static�� ����Ǿ���.
	}


#ifndef GENERATE_RAM_IMAGE
	// FLASH�� ��쿣 code rewritting �� �ݵ�� ��� address��
	// 0xFFFFFFF�� �ֱ�ȭ�Ǿ� �־�� �� �� �ƴ϶�,
	// �ݺ� rewite�� �Ұ����� ����.
	#define IMPLEMENT_IMPORT_CLASS_FUNCTION(CLASS) 
			static CLASS$Class* g_pClass_##THIS = null;
			CLASS$Class* CLASS::import$class() {
			// ����!! // recursive call�� �����ؾ� �Ѵ�.
				if (g_pClass##THIS != null) {
					// �ӵ� optimizing �䱸.
					return g_pClass##THIS;
				}
				g_pClass##THIS = new CLASS$Class(&g_classContext);
				// virtual-table�� �ֱ�ȭ �� �� �־�� �ϹǷ�,
				// funtion call�� (��: createClass()) ��ġ�� �� ����.
				SUSER::import$Class();
				// �ݵ�� g_pStatic�� valid�� ������ ���� ��,
				// Superr::import$Class()�� call�ؾָ� �Ѵ�, (recursive ����)
				initStatic();
				return g_pClass##THIS;
			}
#else
	#define IMPLEMENT_IMPORT_CLASS_FUNCTION(CLASS)
			NAKED CLASS$Class* CLASS::import$class() {
				begin:  // g_pClass�� data�������� �������� �ʴ´�.
				fm::importSuperClass(new CLASS$Class(&g_classContext));
				// g_classContex�� �ݵ�� ClassContext Segment ���� �־�� �Ѵ�.
				initStatic();
				goto begin;
			}
#endif

"ClassLoader.cpp" ====================================================
java_lang_Class* fm::importSuperClass(class* pSuper, class* pClass) {
	// maybe implemeted by asembly
	int* nemonic = LR - proper_offset;

	ClassContext* pRawContext = pClass->m_pContext;
	ClassContext* pRefContext = pSuper->m_pContext;
	if (pRawCotext->m_strPackage != pRefContext->m_strPackage) {
		// ���ڿ��� ���� �ʿ���� addr�� ���ص� �����ϴ�.
		int sizRefInstnce = pRefContext->m_sizInstance;
		int sizAlloc = sizRefInstnce + pRawConext->pRefContext->m_sizInstance;
		//GCHANDLE* h = allocateHandle(sizAlloc);
		GCBUFFER buffer(sizAlloc);
		int* refObj = (int*)(buffer + 0);
		int* rawObj = (int*)(buffer + sizRefInstance);
		pRawContext->m_pfnInitRaw(&rawObj);
		pRefContext->m_pfnInitRaw(&refObj);
		validateVirtaulTable();
	}	
	nemonic[0] = _asm mov eax, pClass;
	nemonic[1] = _asm ret;
	return pClass;
}

"FastivaLib.cpp" ====================================================
#define IsINSIDE_TEXT_SEGMENT(seg, ptr)
		(ptr >= START_OF_SEGMENT(seg) && ptr < END_OF_SEGMENT(seg))
		
#ifndef SEPERATED_CONTEXT_INITIALING
void initFastiva(fm::Kernel* pKernel) {
	ClassContext* pContext = START_OF_SEGMENT(ClassContext_SEGMENT);
	while (pContext < END_OF_SEGMENT(ClassContext_SEGMENT) {
		ClassContext* pSuperContext = pContext->m_pusperContext;
		if (!IsINSIDE_SEGMENT(ClassContext_SEGMENT, pContext->m_pusperContext)) {
			pContext->m_pusperContext = (*(STATIC_PROC)pSuperContext)();
		}
		else {
			pContext->m_pusperContext = getRelocatedArrd(pSuperContext);
		}
		pContext = pContext->m_pusperContext
	}
}
#else
void initRawContext(pContext) {
	.........
}
#endif

java_lang_Class::java_lang_Class(ClassContext* pContext) 
	: m_pContext(getRelocatedAddr(m_pContext)) 
{
#ifdef SEPERATED_CONTEXT_INITIALING
	initRawContext(pContext)
#endif
}

java_lang_Class* fm::importSuperClass(java_lang_Class* pClass) {
	// maybe implemeted by asembly
	Class* pSuperClass = pClass->m_pContext->m_superContext->m_import$Class();
	fm::validateVTable(pSuperClass, pClass);
}

"Fastiva.h" =========================================================
class Fastiva {
	class ClassContext {
		....
		STATIC_PROC m_init$static;
		ClassContext* m_superContext;
		int m_cntVTableItem;
	};
};

"java_lang_Class.h" =================================================
class java_lang_Class {
	JPrivate fastiva_ClassContext* const m_pContext;
	#ifdef FASTIVA_EMBBDED_RUNTIME
	protected: java_lang_Class(ClassContext* pContext)
		: m_pContext(pContext) {}
	#else
	protected: java_lang_Class(ClassContext* pContext);
	#endif
};

"java source file.cpp" ==============================================
#pragma SEGMENT ".fm::classContext""
const fastiva_ClassContext THIS::g_class {
	......
	m_pfnInitRaw = rawInitProc;
	&SUPER::g_classContext;
}; // maybe implemented by ASSEMBLER
#pragma SEGMENT ".text"

IMPLEMENT_IMPORT_CLASS_FUNCTION(THIS);

"Thunk assembler files"==============================================
SEGMENT .TEXT
	EXPORT ClassName::g_classContext;
	EXPORT ClassName::import$class();
	EXPORT ClassName::constructor_thunk_functions();
	EXPORT ClassName::static_thunk_functions();
	EXPORT ClassName::virtual_psuedo_functions();
	struct ThunkTable {
		int crc;
		aConstructorThunk[];
		aFinalThunk[];
		aStaticThunk[];
		char* className;
	};
	
ClassName::g_classContext // as a func
	call fm::getClassContext("ClassName");
	
ClassName::import$Class // as a func
	call fm::import$Class(&ThunkTable);
	return r0;
	
"ClassLoader.cpp" -==================================================
java_lang_Class* fm::import$class(StaticThunkTable) {
	pContext = fm::getClassContext(className);
	rewrite_thunk_proc();
	rewrite_thunk_import$Class_proc();
}

" *.classInfo.asm "=======================================================
SEGMENT .TEXT
	IMPORT CLASS::construtor_proc (all exported);
	IMPORT CLASS::final_proc (all exported);
	IMPORT CLASS::static_proc (all exported);
	IMPORT CLASS::vitaul_proc (all exported);
	IMPORT CLASS::vtable;
	
	CHECK VTABLE SIZE HEARE!!!!

	EXPORT CLASS::g_classContext;
	struct ThunkRefTable {
		int crc;
		aConstructor[];
		aFinalProc[];
		aStaticProc[];
		aVirtualProc[];
	};
	
#endif // __KERNEL_THUNK_H__


/* About thunk of ARM7
A.	BL always PC-relativa. 
	����, ROM-image�� fixed-addr�� �����Ѵ��ص�,
	single-step���� ������ �� ����. �̿� ������ ���� 2 step code�� �����ϰ� �ȴ�.
		1. 	LDR r0, func_addr
			BL	[r0]
		�Ǵ� 
		2.	LDR	pc, {pc} + 4
			DCD	func_addr
	��� jump�� 2 step�̶��, thunk�� ��������� ���� performance ���ϴ� ����.
	���� thunk�� ����ϸ� code-size�� ���̴� ȿ���� �� �� �ִ�.

B. 	g_pStatic spend more meories.
	������ ���� g_pStatic�� �̿��ϸ�, �ڵ� ����� ���� �� ���� �� ������,
		g_pStatic->m_pfnStaticProc[idx](...);
	������ ������ assembly code�� ������ ���� 3 step�̵ȴ�.
		call import$Class();
		LDR r12, [r0] + idx;  // parameter ������ ���ؼ� r0�� r12�� ����
		... set pararms 
		BL  {r12}
		
	��, thunk�� ����� ���, �׻� table��ü�� �����ؾ� �ϴ� ���� �����̳�,
		Constructor, Static function, Final function���� �� ���ڰ�
		�����Ƿ� Thunk�� ����ϴ� ���� ȿ�����̴�.
		static thunk function�� ���� �ʱ�ȭ�� package loading�ÿ� �ϰ�,
		Constructor�� Final function�� import$Class�� ȣ��� ���� �Ѵ�.
		// 	static thunk function�� ���� �ʱ�ȭ�� imprt$Class���� �ϸ� ������
		���� ������ �߻��Ѵ�.
			1.	BL import$Class // RL �� ���� ����� ������ �� ����.
			2.  B  import$Class	// import$Class() ���� �� branch�� func�� �� �� ����.
			/// �ƴϴ� !!! �����ϴ�.
			2-0. B import$static
			
			import$static:
				// STMFD sp!, {rl}  
				call import$Class();
				// LDMFD sp!, {rl}
				mov pc, rl - 4;  // static_proc�� call�ϴ� code�� ���ư�
								 // static_thnk_func�� �ٽ� call�Ѵ�.
			���� thunk function�� 2 step �̻����� ������ �Ѵ�.
		Thunk Constructor�� ����ϸ� Code size ����ȿ���� ũ��,
		Thunk Final function�� ����ϸ�, �ణ�� Code size ����ȿ����
		performace����� ���� �� �ִ�.

C.	Virtual function direct call�� ��� ó���� ���ΰ�?
	Virtual function�� �� ���ڰ� �������� �ұ��ϰ�, dirct virtual function call��
	�ſ� ���������� ���̹Ƿ� thunk�� ó���ϸ� size�� ���ʿ��ϰ� �����Ѵ�.
	g_pStatic�� ����ϴ� �͵� �ذ�å�̳�, Header file�� �ſ� ����������.
	������ �ذ�å�� ��� virtual thunk function�� ������ object�� geneate�ϰ�,
	���� ���� link�ϴ� ����̴�. �׷��� ���ؼ�, virtual funtion�� offset��
	�˾Ƴ� ����� �����ϰų�, exported func���� ������ index�� �ο��ؾ� �Ѵ�..
	function���� ������ index�� �ο��ϸ�, Runtime ���ο� ������ table�� �ʿ��ϴ�.
	�׷���, �� �����, psuedo function �� ��Ī �浹�� �߻���Ű�� ������ �ִ�.
	�������� ������ �ҽ������ �߻��ϴ� ���̺귯���� Runtime���� ����ġ�� �߻��� �� �ִٴ�  
	���̴�. �̸� �ذ��� �� �ִ� ����� �ǵ��� ������ �и��� Assembly�� ������� �ʴ� ����
	������, ARM7 C-Compiler������ virtual function pointer ����� ������ thunk-code��
	�����ϱ� ������ assembly ����� �Ұ����ϴ�. ���� assembly ������ Option��
	�־� ���� file�� export-table�� thunk-proc�� ������ �� �ֵ��� �Ѵ�.
	
	ARM7�� ���, ������ function���� compile�ϸ� jmp code�� �����س���.
		int f1() { return 0; }
		int f2() { return f1(); } // b f1;

	���� Function���� Link�� �����ϸ� virtaul-thunk�� ������ ���� ó���Ѵ�.
		Foo::virtual_fn() {
			return g_pVFTable$Foo->virtual_fn();
		}
		
	�׷���, Contructor, Final, Static table�� ���� assembly�� ����ϸ� code�� 
	table�� ���ÿ� ������ �� �־ ȿ���� ���� �� �ִ�.
	
	���: export�� ��� �Լ��� ���� export-table�� �����.
		  virtual thunk�� function �Լ������� link ��Ų��.
		  static table�� �� ��(Constructor, Final)�� table�� �ѷ� �и��Ѵ�.
		
			


* Array // ������ �̿��ϸ�, ArrayItem�� ���� inherit�� ������ ���Ƿ�
		// item assign �� Type checking ����� �߰��� �� �ִ�.
	#define DECL_CLASS(CLASS)
		typedef CLASS* CLASS##_p;
		typedef $OPtr$<CLASS> CLASS##_item_p;

	#define DECL_INTERFACE(CLASS)
		typedef $IPtr$<CLASS> CLASS##_p;
		typedef $IPtr$<CLASS> CLASS##_item_p;
	


/**====================== end ========================**/