#ifndef __KERNEL_HEAPMARK_H__
#define __KERNEL_HEAPMARK_H__

enum { 
	// PhantomReference������ �ش� ���� Object�� �����ϰ� ������ ��Ÿ����.
	// �Ϲ����� Reachable���� �ǹ̰� �ٸ���.
	HM_PUBLISHED = 0x8000,
	HM_FINALIZABLE = 0x4000,
	HM_PHANTOM_REF_REACHABLE = 0x2000,
	HM_CONTEXT_RESOLVED = 0x1000,
	HM_STRONG_REACHABLE = 0x0080,
	HM_SOFT_REACHABLE = 0x0010,
	
	// Array-Class �� dimension�� �����ϱ� ���Ͽ� ���ȴ�.
	HM_ARRAY_CLASS_DIMENSION = 0x0700,
	
	// Native method ��� �Ϸ�.
	HM_NATIVE_REGISTERED = 0x0800,

	// Class �� �׻� Finalized �����̹Ƿ�, HM_NOT_FINALIZED�� �ߺ������ �����ϴ�.
	HM_ININITIALIZER_ERROR = 0x4000,
};

#if 0
#define USE_FAST_GENENRAION_SHIFT


/*
 HeapMark �� bit���� �����ϸ� m_cntRef$�� �Ϻλ����� �Ѵ�.
union HeapMark {
	slotTableID		: 8;
	slotID			: 14;
	stackReachable	: 1;
	publicReachable : 1;
};
*/

/*
PHANTOME_G �� BABY_G�� ������ ref-count�� ��������ν�,
publishing �ܰ迡�� ref-count�� ������ �ʿ並 ������.
GENERATION_TOUCH�� YOUNG, OLD, STATIC�� �� �� ��ȿ�ϴ�.
(BABY_G�� PHANTOM_G�� ���ǹ��ϰ�, ROOT_G�� setStaticField_$$����
����ó���� �����ϴ�.)
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
// HeapSlot ���� define
// HeapSlot ���� ������ integer ���·� �����ȴ�. �̸� HeapMark ���·� �ٲٴ� 
// ���� �ҽ��� �������� ������ ���ҽ�Ų��.
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

	// @todo ���� �ڷ����� Endian�� ���� �����Ǿ�� �Ѵ�.

	//	slotOffset		: 14;
	//	phantomReachable : 1;   // HM_CONTEXT_RESOLVED�� ���ȴ�.
	//	strongReachable : 1;
	unsigned char m_touch;
	unsigned char m_unused;
	//unsigned short	m_slotOffset;

	//enum { HM_STRONG_REACHABLE = 0x01 };
	// PhantomReference������ �ش� ���� Object�� �����ϰ� ������ ��Ÿ����.
	// �Ϲ����� Reachable���� �ǹ̰� �ٸ���.
	enum { HM_PHANTOM_REF_REACHABLE = 0x02 };
	//enum { HM_REFERENT_TOUCHED = HM_STRONG_REACHABLE };

	//  slotTableID		: 8;
	//  slotTable�� �ݵ�� slotOffset�� ������ ���̾�� �Ѵ�.
	//  (offset overflow �� �ڵ����� TableID ����).
	unsigned char	m_slotTableID;

	/** �Ʒ��� Field ���� ������ ��ȣ �������̴�.
	  ��, Mutable, generation, isFinalized Flag�� ������ ���� ���ÿ� �߻����� �ʴ´�.
	  1) isFinalized�� new-instance �� Local-Q�� ����ϱ� ������, finalize() ȣ�� ��������
	  ����ȴ�. finalize() ȣ�� ����, stack������ ������ �� ������, �� ���� StrongReachable ����
	  ����ȴ�.
	  2) generation �� publishInstance�ÿ� addPublicLost�ÿ��� ����ȴ�.
	  �� �� ��� Local-Instance�̹Ƿ�, isMutable�̳�, isFinalized-Flag�� ����� ������ ����.
    */


	//	isMutable		: 1;	// setField �ÿ��� ����ȴ�
	//	generationTouch : 3;	
	//	isFinalized		: 1;	// allocInstance(), runFinalize()������ ����.
	//	generation		: 3;	// publishInstance(), addPublicLost() ������ ����.
	signed char	m_gInfo;
	enum { G_INFO_SHIFT = 24 };
	enum { HM_NOT_FINALIZED = 0x08 } ;// << G_INFO_SHIFT };
	enum { HM_MUTABLE   = 0x80 }; // << G_INFO_SHIFT };

	// REFERENT_TOUCHED BIT�� ���� ���� ����� �ʿ�� ������
	// DEBUGING ������ �ӽ÷� ����Ѵ�.



	/*
	static int getGenerationTouch(int G) {
		// GENERATION_TOUCH �� G-SHIFT-UP �ÿ��� ���ǹ��ϴ�.
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
		// addPublicLost�ÿ��� ȣ��ȴ�.
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
		// demarkReachable �� GC ���� ��, GC-Task������ ȣ��ȴ�.
		// �ٸ� ��� task������ markReachble�� ȣ����� �����Ƿ�
		// thread-safe �ϴ�.
		m_touch = 0;//slotOffset &= ~(HM_STRONG_REACHABLE);
		//fox_util_and32(this, ~(HM_STRONG_REACHABLE));
	}


	int isPhantomRefReachable() {
		return m_unused & HM_PHANTOM_REF_REACHABLE;
	}

	void markPhantomRefReachable() {
		// public, local scan ���� ��, marking���� ����
		// phantom-referent �� ���ؼ��� ȣ�� �ȴ�.
		// mark/demarkStrongReachable �� �浹���� �ʴ´�.
		m_unused |= HM_PHANTOM_REF_REACHABLE;
	}

	int demarkPhantomRefReachable() {
		// finalize() ȣ�� �� �Ŀ� �˻� �ȴ�. scanLocalStack�� 
		// runFinalize() ������ ���� task->disableSuspend() ��
		// ȣ���� �����̹Ƿ�, local-scanning�� ���� ���� �����̴�.
		// thread-safe �ϴ�. 
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
		// finalize-task���� finalize() ȣ�� �������� ȣ��ȴ�.
		m_gInfo &= ~HM_NOT_FINALIZED;
	}

	void demarkFinalized() {
		// allocInstance ���ο����� ȣ��Ǹ�, Local-Q�� 
		// �ش� instance �� ��ϵǱ� ���� ȣ��ǹǷ� thread-safe�ϴ�.
		m_gInfo |= HM_NOT_FINALIZED;
	}

	int isReachable();

	short isContextResolved() {
		// just for fastiv_Class : for non-heap-veriosn only
		// fastiva_Class_T$ �� HeapObject�� �ƴϹǷ�,
		// ���� StackRechable�� marking���� �ʴ´�.
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

static const int HM_INSTANCE			 = 0x00000001; //Interface �� Instane�� �����ϱ� ���� ���ȴ�.


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
	�ڽ��� ���� ����κ����� ref-count�� �ڽ��� ���� ������ 
	MIN_REF_COUNT���� Ŀ�� �� �ִ�. �ٸ� ����� �޸� ROOT_G�� ��쿡�� 
	�̷��� ��Ȳ�� �߻��ϸ� Ư�� Instance�� GC���� �ʴ� ������ �߻��Ѵ�.
	(static-field�� assign�� instance�� ����-���鰳�� child-instance��
	�����ϰ�, child-instance ���� �ش� instance�� �����ϰ� ���� ��)
	�̿� ROOT_G�� overflow�� �߻����� �ʴ� ������ ref-count�� ����ϰų�,
	�ƿ� class�� scan�ϴ� ����� ����Ͽ��߸� �Ѵ�.
	jni-lock�� ����Ͽ�, ������ ref-count�� ����ϴ� ���� ����ϰų�.
	ref-overflow�� �߻��ϸ� ����ó���ϴ� ����� ����Ѵ�.
	root-ref�� ���鿩�� �߻��ϴ� ��Ȳ�� �幰�Ƿ�, (���鿩���� class��
	static-field�� ������ instance�� assign�ϴ� ���� ���� ����.)
	256 �Ǵ� 512 �̻��� �ִ� ref-count�� ������ �� ������, �Ϲ�����
	��쿡�� ������ �߻����� ������, native C++ class ���� ������
	������ ��츦 ����Ѵ�.
	


/** Static-Q
	Clss�� static ������ assign �� ��� instance�� Linked-List�̴�.
	All-Geration Scan�� ���� root ������ �����ϸ�, Class ��ü�� ����
	ScanOffset Info�� �������� �ʾƵ� �Ǵ� ������ �����Ѵ�.

	static-ref-count 0 �̻��� instance�� ���� garbage�� �ƴϴ�.
	static-ref-count�� 0�� �Ǹ�, Old-G �Ǵ� Young-G(OldRef-Count = 0)�� �Ű�����.

	staticQ�� �ΰ��� �����µ�,
		statble-static-Q �� assign �� �ش� Field�� ���� null�� ��쿡
		mutable-static-Q �� assign �� �ش� Field�� ���� null�� �ƴ� ��쿡
		���ȴ�.
		(Class�� ������ ����� ���� �ִ°��� �����ϴ� ���� �� �����ϴ�?)
*/

/** Static-G
	statble-Static-Q�κ��� ���ٰ����� instance�� ���� (static-Q) �̴�.

/** Old-G
	mutable-Static-Q�� ���� ���ٰ����� instance�� ���� (old-Q) �̴�.
	old-Q���� stable-old-Q �� mutable-old-Q�� ������.
*/

/** Young-G
	mutable-old-Q�� ���� ���� ������ instance �� ����(younger-Q)�̴�. 
*/

/** Baby-G
	mutable-younger-Q�� ���� ���� ������ instance ��
	���� ������ instance �� ����(baby-Q)�̴�.

	stable-old-Q���� ���ٰ����� instance ���� �ش� Q�� �Ű�����.
	�̸� �� �� ���� �����ϱ� ����, Young-G �� ref-count��
	0x80 �̻��� ���� MUTABLE_OLD��, 0x00400 �̻��� ����
	STABLE_OLD�� �ű��. ����) �ű� ��, �ݵ��, TYPE-FLAG�� 
	������ �־�� �Ѵ�. �׷��߸�, G-ref-count�� ��Ȯ�� 
	�� �� �ִ�.
*/

/* Batch Generation Shift 1)
   GC �ʱ⿡�� static-stable-Q�� baby-Q�� ������,
   baby-Q�� ��� GC�Ѵ�.

   ���� stable-Link�� mutable-Link�� ��������� �����鼭,
   static-Q�� old-Q, Young-Q, Baby-Q�� �����Ѵ�.

   Mutable-Link �� ������ ������ �ʾ������� ����,
   �ټ� GC�� ȿ���� �����ϴ� ������ �ִ�.
*/

/* On-Time Generation Shift 2)
   ref-count�� �ٲ�ų�, MUTABLE_MARK�� ����Ǵ� ������
   ��������� sub-tree�� ref-count�� �����Ѵ�.

   ���� Program�� Performance ������ ���� �� �ִ�.
*/
#endif
#endif
#endif //__KERNEL_HEAPMARK_H__
