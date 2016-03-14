#ifndef __KERNEL_HEAPMARK_H__
#define __KERNEL_HEAPMARK_H__

enum { 
	// PhantomReference에서만 해당 현재 Object를 참조하고 있음을 나타낸다.
	// 일반적인 Reachable과는 의미가 다르다.
	HM_PUBLISHED = 0x8000,
	HM_FINALIZABLE = 0x4000,
	HM_PHANTOM_REF_REACHABLE = 0x2000,
	HM_CONTEXT_RESOLVED = 0x1000,
	HM_STRONG_REACHABLE = 0x0080,
	HM_SOFT_REACHABLE = 0x0010,
	
	// Array-Class 에 dimension을 저장하기 위하여 사용된다.
	HM_ARRAY_CLASS_DIMENSION = 0x0700,
	
	// Native method 등록 완료.
	HM_NATIVE_REGISTERED = 0x0800,

	// Class 는 항상 Finalized 상태이므로, HM_NOT_FINALIZED와 중복사용이 가능하다.
	HM_ININITIALIZER_ERROR = 0x4000,
};

#if 0
#define USE_FAST_GENENRAION_SHIFT


/*
 HeapMark 의 bit수가 부족하면 m_cntRef$를 일부사용토록 한다.
union HeapMark {
	slotTableID		: 8;
	slotID			: 14;
	stackReachable	: 1;
	publicReachable : 1;
};
*/

/*
PHANTOME_G 와 BABY_G는 동일한 ref-count를 사용함으로써,
publishing 단계에서 ref-count를 변경할 필요를 없엔다.
GENERATION_TOUCH는 YOUNG, OLD, STATIC에 한 해 유효하다.
(BABY_G와 PHANTOM_G는 무의미하고, ROOT_G는 setStaticField_$$에서
예외처리가 가능하다.)
*/
enum GC_Generation {
	PHANTOM_G,
	BABY_G,
	YOUNG_2G,
	YOUNG_3G,
	YOUNG_4G,
	YOUNG_5G,
	STATIC_G,
	ROOT_G,
};

#define LOCAL_G		PHANTOM_G

#define MIN_LOCAL_REF_COUNT		0x00000000
#define MIN_1G_REF_COUNT		0x00000000
#define MIN_2G_REF_COUNT		0x00000001
#define MIN_3G_REF_COUNT		0x00000008
#define MIN_4G_REF_COUNT		0x00000040
#define MIN_5G_REF_COUNT		0x00000200
#define MIN_STATIC_REF_COUNT	0x00001000
#define MIN_ROOT_REF_COUNT		0x00010000

static const int g_aMinGenerationRefCount[] = {
	MIN_LOCAL_REF_COUNT,
	MIN_1G_REF_COUNT,
	MIN_2G_REF_COUNT,
	MIN_3G_REF_COUNT,
	MIN_4G_REF_COUNT,
	MIN_5G_REF_COUNT,
	MIN_STATIC_REF_COUNT,
	MIN_ROOT_REF_COUNT
};


static inline uint GC_getMinRefCount(int G) {
	KASSERT(G >= 0 && G <= ROOT_G);
	return (uint)g_aMinGenerationRefCount[G];
}


static const int HM_CONTEXT_RESOLVED	 = 0x00000002; // == stack_reachable; 


//==========================================================================//
// HeapSlot 관련 define
// HeapSlot 관련 정보는 integer 형태로 관리된다. 이를 HeapMark 형태로 바꾸는 
// 것이 소스의 가독성을 오히려 감소시킨다.
// 512(Max-Slot-Table-Count) * 32K / 
//==========================================================================//

#define LAST_SLOT_TABLE_ID			0xFF
#define SLOT_TABLE_ID_SHIFT			16

#define SLOT_TABLE_SIZE				(64 * 1024)
#define SLOT_OFFSET_INCREE			sizeof(fox_HeapSlot)
#define SLOT_TABLE_ITEM_COUNT		(SLOT_TABLE_SIZE / SLOT_OFFSET_INCREE)

#define SLOT_TABLE_ID_MASK			(LAST_SLOT_TABLE_ID << SLOT_TABLE_ID_SHIFT)
#define SLOT_OFFSET_MASK			((SLOT_TABLE_ITEM_COUNT - 1) * SLOT_OFFSET_INCREE)
#define HEAP_SLOT_ID_MASK			(SLOT_OFFSET_MASK | SLOT_TABLE_ID_MASK)


class HeapMark {

	// @todo 다음 자료형은 Endian에 따라 조정되어야 한다.

	//	slotOffset		: 14;
	//	phantomReachable : 1;   // HM_CONTEXT_RESOLVED로 사용된다.
	//	strongReachable : 1;
	unsigned char m_touch;
	unsigned char m_unused;
	//unsigned short	m_slotOffset;

	//enum { HM_STRONG_REACHABLE = 0x01 };
	// PhantomReference에서만 해당 현재 Object를 참조하고 있음을 나타낸다.
	// 일반적인 Reachable과는 의미가 다르다.
	enum { HM_PHANTOM_REF_REACHABLE = 0x02 };
	//enum { HM_REFERENT_TOUCHED = HM_STRONG_REACHABLE };

	//  slotTableID		: 8;
	//  slotTable은 반드시 slotOffset과 연이은 값이어야 한다.
	//  (offset overflow 시 자동으로 TableID 증가).
	unsigned char	m_slotTableID;

	/** 아래의 Field 들의 변경은 상호 독립적이다.
	  즉, Mutable, generation, isFinalized Flag의 변경은 결코 동시에 발생하지 않는다.
	  1) isFinalized는 new-instance 를 Local-Q에 등록하기 직전과, finalize() 호출 직전에만
	  변경된다. finalize() 호출 전에, stack내에서 참조될 수 있으나, 이 때는 StrongReachable 만이
	  변경된다.
	  2) generation 은 publishInstance시와 addPublicLost시에만 변경된다.
	  둘 다 모두 Local-Instance이므로, isMutable이나, isFinalized-Flag가 변경될 염려는 없다.
    */


	//	isMutable		: 1;	// setField 시에만 변경된다
	//	generationTouch : 3;	
	//	isFinalized		: 1;	// allocInstance(), runFinalize()내에서 변경.
	//	generation		: 3;	// publishInstance(), addPublicLost() 내에서 변경.
	signed char	m_gInfo;
	enum { G_INFO_SHIFT = 24 };
	enum { HM_NOT_FINALIZED = 0x08 } ;// << G_INFO_SHIFT };
	enum { HM_MUTABLE   = 0x80 }; // << G_INFO_SHIFT };

	// REFERENT_TOUCHED BIT를 구지 따로 사용할 필요는 없으나
	// DEBUGING 용으로 임시로 사용한다.



	/*
	static int getGenerationTouch(int G) {
		// GENERATION_TOUCH 는 G-SHIFT-UP 시에만 유의미하다.
		KASSERT(G >= YOUNG_G && G < ROOT_G);
		return (0x4 << G_INFO_SHIFT) << G; // YOUNG_TOUCH = 0x10;
	}
	*/

public:

	int getGeneration() {
		return m_gInfo & 7;
	}

	int getFlag32() {
		return *(int*)(void*)this;
	}

	void setGeneration(int G) {
		KASSERT(G >= 0 && G <= ROOT_G);
		// addPublicLost시에만 호출된다.
		m_gInfo = (m_gInfo & ~0x7) | G;
	}

	bool isPublished() {
		return getGeneration() != LOCAL_G;
	}

	/*
	void markSeniorTouch(int G) {
		KASSERT(G >= YOUNG_G && G < ROOT_G);
		KASSERT(G > this->getGeneration());
		fox_util_or32(this, getGenerationTouch(G));
	}

	bool isSeniorTouched(int senior_G) {
		return (*(int*)(void*)this) & (getGenerationTouch(senior_G));
	}
	*/

	bool isMutable() {
		return m_gInfo < 0;
	}

	void markMutable() {
		m_gInfo |= HM_MUTABLE;
	}

	void markReferentTouched() {
		markStrongReachable();
	}

	void markPublishedBaby();

	void markPublishedReachableBaby();

	static int getPublishingMark(int G, bool mark);

	void markPusblished(int mark_flag);

	void markStrongReachable();

	void demarkReachable() {
		// demarkReachable 은 GC 종료 후, GC-Task에서만 호출된다.
		// 다른 모든 task에서도 markReachble이 호출되지 않으므로
		// thread-safe 하다.
		m_touch = 0;//slotOffset &= ~(HM_STRONG_REACHABLE);
		//fox_util_and32(this, ~(HM_STRONG_REACHABLE));
	}


	int isPhantomRefReachable() {
		return m_unused & HM_PHANTOM_REF_REACHABLE;
	}

	void markPhantomRefReachable() {
		// public, local scan 종료 후, marking되지 않은
		// phantom-referent 에 대해서만 호출 된다.
		// mark/demarkStrongReachable 과 충돌되지 않는다.
		m_unused |= HM_PHANTOM_REF_REACHABLE;
	}

	int demarkPhantomRefReachable() {
		// finalize() 호출 이 후에 검사 된다. scanLocalStack과 
		// runFinalize() 내에서 서로 task->disableSuspend() 를
		// 호출한 상태이므로, local-scanning이 되지 않은 상태이다.
		// thread-safe 하다. 
		if (isPhantomRefReachable()) {
			m_unused &= ~HM_PHANTOM_REF_REACHABLE;
			return true;
		}
		return false;
	}

	int isFinalized() {
		return (m_gInfo & HM_NOT_FINALIZED) == 0;
	}

	void markFinalized() {
		// finalize-task에서 finalize() 호출 직전에만 호출된다.
		m_gInfo &= ~HM_NOT_FINALIZED;
	}

	void demarkFinalized() {
		// allocInstance 내부에서만 호출되면, Local-Q에 
		// 해당 instance 가 등록되기 전에 호출되므로 thread-safe하다.
		m_gInfo |= HM_NOT_FINALIZED;
	}

	int isReachable();

	short isContextResolved() {
		// just for fastiv_Class : for non-heap-veriosn only
		// fastiva_Class_T$ 는 HeapObject가 아니므로,
		// 결코 StackRechable로 marking되지 않는다.
		return *(int*)(void*)this & HM_CONTEXT_RESOLVED;
	}

	/*
	unsigned short getHeapSlotOffset() {
		return m_slotOffset & (short)~3;
	}

	unsigned char getHeapSlotTableID() {
		return m_slotTableID;
	}
	*/

};



/*

static const int HM_PUBLISHED			 = 0x80000000; //HM_PUBLIC_REACHABLE | HM_NOT_LOCAL;
static const int HM_STRONG_REACHABLE	 = 0x40000000;
static const int HM_PHANTOM_REACHABLE	 = 0x20000000;

static const int HM_TOUCHED_REFERENT	 = 0x08000000;
static const int HM_NOT_FINALIZED		 = 0x04000000;
static const int HM_STACK_ROOT			 = 0x02000000;
static const int HM_NATIVE_REGISTERED	 = 0x01000000;

static const int HM_INSTANCE			 = 0x00000001; //Interface 와 Instane를 구별하기 위해 사용된다.


static const int HM_BROADCASTED			 = HM_STRONG_REACHABLE | HM_PUBLISHED;

static const int HM_REACHABLE			= HM_STRONG_REACHABLE
										| HM_PHANTOM_REACHABLE;

//static const int HM_REACHABLE			= Hm_dISIBLE
//										| HM_WEAK_REACHABLE;

static const int HM_LOCAL_REACHABLE		= HM_PUBLISHED
										| HM_REACHABLE;
										//| HM_WEAK_REACHABLE;

#define GC_markPublished(obj)			fox_util_or32(&obj->m_marked, HM_PUBLISHED) // mark-change
#define GC_isPublished(obj)				((int)obj->m_marked < 0)

#define GC_isLocal(obj)					((int)obj->m_marked >= 0)
#define GC_markReferentTouched(obj)		fox_util_or32(&obj->m_marked, HM_TOUCHED_REFERENT) // mark-change
#define GC_isTouchedReferent(obj)		((obj->m_marked & HM_TOUCHED_REFERENT) != 0)
//#define GC_demarkTouchable(obj)			fox_util_and32(&obj->m_marked, ~(HM_STRONG_REACHABLE|HM_WEAK_REACHABLE))

//#define GC_isReference(obj)			((obj->m_marked & HM_REFERENCE) != 0)

#define GC_markBroadcasted(obj)			fox_util_or32(&obj->m_marked, HM_BROADCASTED) // mark-change
#define GC_isBroadcasted(obj)			((uint)obj->m_marked >= HM_BROADCASTED)

//#define GC_isVisible(obj)				((obj->m_marked & Hm_dISIBLE) != 0)
#define GC_isReachable(obj)				((obj->m_marked & HM_REACHABLE) != 0)
#define GC_isLocalReachable(obj)		((obj->m_marked & HM_LOCAL_REACHABLE) != 0)
#define GC_isPublicReachable(obj)		((uint)obj->m_marked >= (HM_PHANTOM_REACHABLE|HM_PUBLISHED))
#define GC_demarkReachable(obj)			fox_util_and32(&obj->m_marked, ~(HM_REACHABLE|HM_STACK_ROOT|HM_TOUCHED_REFERENT))

#define GC_isStrongReachable(obj)		((obj->m_marked & HM_STRONG_REACHABLE) != 0)
#define GC_isStrongTouched(obj)			((obj->m_marked & (HM_STRONG_REACHABLE|HM_STACK_ROOT)) != 0)
#define GC_markStrongTouched(obj)		fox_util_or32(&obj->m_marked, HM_STACK_ROOT)

#define GC_markWeakReachable(obj)		fox_util_or32(&obj->m_marked, HM_WEAK_REACHABLE) // mark-change
//#define GC_markLocalWeakReachable(obj)	fox_util_or32(&obj->m_marked, HM_WEAK_REACHABLE) // mark-change
//#define GC_markPublicWeakReachable(obj)	fox_util_or32(&obj->m_marked, HM_WEAK_REACHABLE|HM_PUBLISHED) // mark-change
//#define GC_isWeakReachable(obj)			((obj->m_marked & HM_WEAK_REACHABLE) != 0)

#define GC_markLocalReachable(obj)		fox_util_or32(&obj->m_marked, HM_STRONG_REACHABLE) // mark-change
#define GC_markPhantomReachable(obj)	fox_util_or32(&obj->m_marked, HM_PHANTOM_REACHABLE|HM_PUBLISHED) // mark-change

//#define GC_isRescued(obj)				((obj->m_marked & (HM_STRONG_REACHABEL|HM_WEAK_REACHABLE)) != 0)

#define GC_isMovable(obj)				(((obj->m_marked & HM_STACK_ROOT) | obj->m_jniLock) == 0)


*/

#ifdef ______________comment_________________________________________

Generation-Overflow;
	자신의 하위 세대로부터의 ref-count가 자신이 속한 세대의 
	MIN_REF_COUNT보다 커질 수 있다. 다른 세대와 달리 ROOT_G의 경우에는 
	이러한 상황이 발생하면 특정 Instance가 GC되지 않는 문제가 발생한다.
	(static-field에 assign된 instance가 수십-수백개의 child-instance를
	참조하고, child-instance 들이 해당 instance를 참조하고 있을 때)
	이에 ROOT_G는 overflow가 발생하지 않는 별도의 ref-count를 사용하거나,
	아예 class를 scan하는 방식을 사용하여야만 한다.
	jni-lock을 고려하여, 별도의 ref-count를 사용하는 것을 고려하거나.
	ref-overflow가 발생하면 예외처리하는 방법을 고려한다.
	root-ref가 수백여개 발생하는 상황은 드물므로, (수백여개의 class의
	static-field에 동일한 instance를 assign하는 경우는 거의 없다.)
	256 또는 512 이상의 최대 ref-count를 설정할 수 있으면, 일반적인
	경우에는 문제가 발생하지 않으나, native C++ class 내의 참조가
	가능한 경우를 대비한다.
	


/** Static-Q
	Clss의 static 변수에 assign 된 모든 instance의 Linked-List이다.
	All-Geration Scan을 위한 root 정보를 제공하며, Class 객체에 대해
	ScanOffset Info를 생성하지 않아도 되는 이점을 제공한다.

	static-ref-count 0 이상인 instance는 결코 garbage가 아니다.
	static-ref-count가 0이 되면, Old-G 또는 Young-G(OldRef-Count = 0)로 옮겨진다.

	staticQ는 두개로 나뉘는데,
		statble-static-Q 는 assign 시 해당 Field의 값이 null인 경우에
		mutable-static-Q 는 assign 시 해당 Field의 값이 null이 아닌 경우에
		사용된다.
		(Class의 내용이 변경되 적이 있는가로 구분하는 것이 더 적절하다?)
*/

/** Static-G
	statble-Static-Q로부터 접근가능한 instance의 집합 (static-Q) 이다.

/** Old-G
	mutable-Static-Q로 부터 접근가능한 instance의 집합 (old-Q) 이다.
	old-Q또한 stable-old-Q 와 mutable-old-Q로 나뉜다.
*/

/** Young-G
	mutable-old-Q로 부터 접근 가능한 instance 의 집합(younger-Q)이다. 
*/

/** Baby-G
	mutable-younger-Q로 부터 접근 가능한 instance 와
	새로 생성된 instance 의 집합(baby-Q)이다.

	stable-old-Q에서 접근가능한 instance 들은 해당 Q로 옮겨진다.
	이를 좀 더 쉽게 구현하기 위해, Young-G 중 ref-count가
	0x80 이상인 것은 MUTABLE_OLD로, 0x00400 이상인 것은
	STABLE_OLD로 옮긴다. 주의) 옮길 때, 반드시, TYPE-FLAG를 
	변경해 주어야 한다. 그래야만, G-ref-count를 정확히 
	할 수 있다.
*/

/* Batch Generation Shift 1)
   GC 초기에는 static-stable-Q와 baby-Q로 나뉘며,
   baby-Q는 모두 GC한다.

   이후 stable-Link와 mutable-Link를 재귀적으로 나누면서,
   static-Q와 old-Q, Young-Q, Baby-Q를 생성한다.

   Mutable-Link 를 나누는 시점이 늦어짐으로 인해,
   다소 GC의 효율이 감소하는 문제가 있다.
*/

/* On-Time Generation Shift 2)
   ref-count가 바뀌거나, MUTABLE_MARK가 변경되는 시점에
   재귀적으로 sub-tree의 ref-count를 변경한다.

   실행 Program의 Performance 문제가 재기될 수 있다.
*/
#endif
#endif
#endif //__KERNEL_HEAPMARK_H__
