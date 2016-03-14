#ifndef __KERNEL_DYNA_CLASS_H__
#define __KERNEL_DYNA_CLASS_H__

#include <fastiva/config.h>


"fastiva/class.h" =====================-=============================
#define	DECL_FASTIVA_COMPONENT
		.........
		JProtected static const fastiva_ClassContext g_classContext;
		JPublic static THIS$Class* import$class();

Java VM 내에서 init$Static이 불리우는 순서

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
		   	// Parent::int$static이 수행이 종료된 후 Child::init$static이 수행되었다.
		   	
		2. 	print(Parent.child_id) => 901;
			print(Child.parent_id) => 101;
			print(Parent.child_id2) => 902;
			print(Child.parent_id2) => 0;
		   	// Parent::int$static이 수행되는 도중 Child::init$static이 수행되었다.
	}


#ifndef GENERATE_RAM_IMAGE
	// FLASH인 경우엔 code rewritting 시 반드시 대상 address가
	// 0xFFFFFFF로 최기화되어 있어야 할 뿐 아니라,
	// 반복 rewite가 불가능해 진다.
	#define IMPLEMENT_IMPORT_CLASS_FUNCTION(CLASS) 
			static CLASS$Class* g_pClass_##THIS = null;
			CLASS$Class* CLASS::import$class() {
			// 주의!! // recursive call을 방지해야 한다.
				if (g_pClass##THIS != null) {
					// 속도 optimizing 요구.
					return g_pClass##THIS;
				}
				g_pClass##THIS = new CLASS$Class(&g_classContext);
				// virtual-table을 최기화 할 수 있어야 하므로,
				// funtion call로 (예: createClass()) 대치할 수 없다.
				SUSER::import$Class();
				// 반드시 g_pStatic을 valid한 값으로 만든 후,
				// Superr::import$Class()를 call해애만 한다, (recursive 방지)
				initStatic();
				return g_pClass##THIS;
			}
#else
	#define IMPLEMENT_IMPORT_CLASS_FUNCTION(CLASS)
			NAKED CLASS$Class* CLASS::import$class() {
				begin:  // g_pClass를 data영역에서 참조하지 않는다.
				fm::importSuperClass(new CLASS$Class(&g_classContext));
				// g_classContex는 반드시 ClassContext Segment 내에 있어야 한다.
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
		// 문자열을 비교할 필요없이 addr만 비교해도 무방하다.
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
	따라서, ROM-image를 fixed-addr에 생성한다해도,
	single-step으로 점프할 수 없다. 이에 다음과 같이 2 step code를 생성하게 된다.
		1. 	LDR r0, func_addr
			BL	[r0]
		또는 
		2.	LDR	pc, {pc} + 4
			DCD	func_addr
	모든 jump가 2 step이라면, thunk를 사용함으로 인한 performance 저하는 없다.
	또한 thunk를 사용하면 code-size를 줄이는 효과를 볼 수 있다.

B. 	g_pStatic spend more meories.
	다음과 같이 g_pStatic을 이용하면, 코드 사이즈를 줄일 수 있을 것 같으나,
		g_pStatic->m_pfnStaticProc[idx](...);
	실제로 생성된 assembly code는 다음과 같이 3 step이된다.
		call import$Class();
		LDR r12, [r0] + idx;  // parameter 설정을 위해서 r0를 r12로 대피
		... set pararms 
		BL  {r12}
		
	단, thunk를 사용할 경우, 항상 table전체를 포함해야 하는 것이 문제이나,
		Constructor, Static function, Final function들은 그 숫자가
		적으므로 Thunk를 사용하는 것이 효율적이다.
		static thunk function에 대한 초기화는 package loading시에 하고,
		Constructor와 Final function은 import$Class가 호출될 때에 한다.
		// 	static thunk function에 대한 초기화를 imprt$Class에서 하면 다음과
		같은 문제가 발생한다.
			1.	BL import$Class // RL 이 깨져 제대로 복귀할 수 없다.
			2.  B  import$Class	// import$Class() 수행 후 branch할 func을 알 수 없다.
			/// 아니다 !!! 가능하다.
			2-0. B import$static
			
			import$static:
				// STMFD sp!, {rl}  
				call import$Class();
				// LDMFD sp!, {rl}
				mov pc, rl - 4;  // static_proc를 call하는 code로 돌아가
								 // static_thnk_func을 다시 call한다.
			따라서 thunk function을 2 step 이상으로 만들어야 한다.
		Thunk Constructor를 사용하면 Code size 절감효과가 크고,
		Thunk Final function을 사용하면, 약간의 Code size 절감효과와
		performace향상을 기할 수 있다.

C.	Virtual function direct call은 어떻게 처리할 것인가?
	Virtual function은 그 숫자가 많음에도 불구하고, dirct virtual function call은
	매우 간헐적으로 쓰이므로 thunk로 처리하면 size가 불필요하게 증가한다.
	g_pStatic을 사용하는 것도 해결책이나, Header file이 매우 복잡해진다.
	적절한 해결책은 모든 virtual thunk function을 각각의 object로 geneate하고,
	사용될 때만 link하는 방법이다. 그러기 위해선, virtual funtion의 offset을
	알아낼 방법을 강구하거나, exported func마다 별도의 index를 부여해야 한다..
	function마다 별도의 index를 부여하면, Runtime 내부에 별도의 table이 필요하다.
	그러나, 이 방법도, psuedo function 과 명칭 충돌을 발생시키는 문제가 있다.
	무엇보다 문제는 소스변경시 발생하는 라이브러리와 Runtime간에 불일치가 발생할 수 있다는  
	점이다. 이를 해결할 수 있는 방법은 되도록 별도로 분리된 Assembly를 사용하지 않는 것이
	좋으나, ARM7 C-Compiler에서는 virtual function pointer 연산시 무조건 thunk-code를
	생성하기 때문에 assembly 사용이 불가피하다. 따라서 assembly 생성시 Option을
	주어 동일 file로 export-table과 thunk-proc를 생성할 수 있도록 한다.
	
	ARM7의 경우, 다음의 function들을 compile하면 jmp code를 생성해낸다.
		int f1() { return 0; }
		int f2() { return f1(); } // b f1;

	따라서 Function단위 Link가 가능하면 virtaul-thunk는 다음과 같이 처리한다.
		Foo::virtual_fn() {
			return g_pVFTable$Foo->virtual_fn();
		}
		
	그러나, Contructor, Final, Static table의 경우는 assembly를 사용하면 code와 
	table을 동시에 생성할 수 있어서 효율을 높일 수 있다.
	
	결론: export된 모든 함수에 대해 export-table을 만든다.
		  virtual thunk는 function 함수단위로 link 시킨다.
		  static table과 그 외(Constructor, Final)의 table을 둘로 분리한다.
		
			


* Array // 다음을 이용하면, ArrayItem에 대한 inherit가 가능해 지므로
		// item assign 시 Type checking 기능을 추가할 수 있다.
	#define DECL_CLASS(CLASS)
		typedef CLASS* CLASS##_p;
		typedef $OPtr$<CLASS> CLASS##_item_p;

	#define DECL_INTERFACE(CLASS)
		typedef $IPtr$<CLASS> CLASS##_p;
		typedef $IPtr$<CLASS> CLASS##_item_p;
	


/**====================== end ========================**/