#ifndef __FOX_HEAP_BLOCK_H__
#define __FOX_HEAP_BLOCK_H__


#define LOCK_HEAP_ON_MALLOC		1
#define NO_USE_HUGE_HEAP_SLOT	1

enum SYS_HEAP {
#ifdef _DEBUG
	SYS_HEAP_HEADER_SIZ = 68,
#else
	SYS_HEAP_HEADER_SIZ = 28,
#endif
	BLOCK_SIZ = 4 * 1024,
	BLOCK_MASK = BLOCK_SIZ - 1,
};


enum FOX_MEM_TYPE {
	FOX_MEM_SYSTEM	= 1,
	FOX_MEM_OBJECT	= 3,
	//UNMARKED = 2,
	//FOX_MEM_MARKED   = 3,
};

//allocation실패시 null 값을 리턴

void fox_heap_registerObject(void* pObject);

void FOX_FASTCALL(fox_heap_free_ex)(void* ptr, FOX_MEM_TYPE memType);

FOX_BOOL FOX_FASTCALL(fox_heap_checkObject)(void* ptr);


//void* FOX_FASTCALL(fox_heap_getUnmarked)(void* ptr);

//FOX_BOOL FOX_FASTCALL(fox_heap_isObjectMarked)(void* pObj);

//void FOX_FASTCALL(fox_heap_markObject)(void* pObj);

//void FOX_FASTCALL(fox_heap_unmarkObject)(void* pObj);


//extern char* g_pVirtualHeap;
//extern char* g_pVirtualHeapEnd;


void* FOX_FASTCALL(sys_heap_init)(int max_heap_size);

void* FOX_FASTCALL(sys_heap_alloc)(int size);

FOX_BOOL sys_heap_isAllocated(void* pMem);

void FOX_FASTCALL(sys_heap_free)(void *pMem, int size);

void* FOX_FASTCALL(sys_heap_realloc)(void* ptr, int size);

void FOX_FASTCALL(sys_task_spin)();

void FOX_FASTCALL(sys_huge_heap_lock)();

void FOX_FASTCALL(sys_small_heap_lock)();

int FOX_FASTCALL(sys_huge_heap_isLocked)();

int FOX_FASTCALL(sys_small_heap_isLocked)();

void sys_huge_heap_unlock();

void sys_small_heap_unlock();

void FOX_FASTCALL(sys_heap_destroy)();

void sys_huge_heap_compact();

#if 0
void FOX_FASTCALL(sys_heap_markValidSegment)(void* pMem);

void FOX_FASTCALL(sys_heap_demarkInvalidSegment)(void* pMem);
#endif

struct fox_heap_HeapSlot;
struct fox_heap_Space;
struct fox_heap_Block;
struct fox_heap_BlockQ;

struct fox_heap_HeapSlot {
	union {
		void* m_ptr;
		fox_heap_HeapSlot* m_pNextSlot;
	};

	int getID();

	//static fox_heap_HeapSlot* FOX_FASTCALL(popHeapSlot)();
	//static void FOX_FASTCALL(pushHeapSlot)(fox_heap_HeapSlot*);

	static void attachHeapSlot(fox_heap_Space* pSpace, int memType);
	static void dettachHeapSlot(fox_heap_Space* pSpace, int slotID);

	static fox_heap_HeapSlot* g_aHeapSlot;
	static fox_heap_HeapSlot* g_pFreeHeapSlot;
	static fox_heap_HeapSlot* g_pEndOfHeapSlot;
	static FOX_BOOL init(int max_heap_size);
	//static void release();
};


struct fox_heap_Space {
	friend struct fox_heap_Block;
private:
	union {
		fox_heap_Space* m_pNext;
		int m_memInfo;
	};

public:

	fox_heap_Space* getNext() {
		return m_pNext;
	}

	int getMemInfo() {
		return m_memInfo;
	}

	void setMemType(FOX_MEM_TYPE type) {
		m_memInfo |= type;
	}

	fox_heap_Space* getNextSibling(int siz) {
		return (fox_heap_Space*)((char*)this + siz);
	}

	void setMemInfo(int memInfo) {
		this->m_memInfo = memInfo;
	}

	int getMemType() {
		return this->m_memInfo & 3;
	}

	//void setHeapSlot(fox_heap_HeapSlot* pSlot);

	fox_heap_Block* getBlock() {
		fox_heap_Block* pBlock = (fox_heap_Block*)((int)m_memInfo & ~3);
		return pBlock;
	}

	fox_heap_Space* getNextFree() {
		return m_pNext;
	}

private:
	void setNextFree(fox_heap_Space* pNext) {
		this->m_pNext = pNext;
	}
};

struct fox_heap_HugeSpaceHeader {
	int m_sizSpace;
};

struct fox_heap_HugeSpace : fox_heap_HugeSpaceHeader, fox_heap_Space {
};


struct fox_heap_BlockQ {

private:
	fox_heap_Block* m_pTop;
	fox_heap_Block* m_pLast;
	unsigned short m_cntBlock;
	unsigned short m_sizSpace; /* fox_heap_Space 의 크기를 더한 것 */

//	char m_unused[128 - 6 * 4]; // asm_lock의 효율을 높이기 위하여 128 byte boundary 를 맞춘다.

public:

	void init(int sizSpace);

	int getSpaceSize() {
		return m_sizSpace;
	}

	void* allocSpace();
	void unlinkFreeBlock();

// internal functions.
	void pushAvailableBlock(fox_heap_Block* pBlock);

};


struct fox_heap_Block {
private:
	fox_heap_Space* m_pFree;
	fox_heap_BlockQ* m_pOwner;
	unsigned int m_cntAlloc;
	unsigned int m_spinLock;
	void* m_pEndOfBlock;
//	fox_heap_Block* m_pPrev;
	fox_heap_Block* m_pNext;
	fox_heap_Space  m_firstSpace;

public:

	fox_heap_BlockQ* getOwner() {
		return m_pOwner;
	}

//	fox_heap_Block* getPrev() {
//		return m_pPrev;
//	}

	fox_heap_Block* getNext() {
		return m_pNext;
	}

	int getSpaceSize() {
		return m_pOwner->getSpaceSize();
	}

	int getAllocCount() {
		return m_cntAlloc;
	}

	FOX_BOOL isHugeBlock() {
		return this >= (void*)0xFF000000; 	
	}

	fox_heap_Space* getFirstSpace() {
		return &m_firstSpace;
	}

	int getBlockSize() {
		int siz = (int)m_pEndOfBlock - (int)this;
		return siz;
	}
	
	FOX_BOOL hasAvailableSpace() {
		return m_pFree != 0;
	}

	fox_heap_Space* allocSpace();

	void freeSpace(fox_heap_Space* pSpace);

	void checkValidSpace(fox_heap_Space* pSpace);


// =================================================================
// Internal Operations

	static FOX_BOOL isInsideVirtualHeap(void* ptr);

	static FOX_BOOL checkValidPtr(fox_heap_Space* ptr, FOX_MEM_TYPE);

	static FOX_BOOL initHeap(void* pVirtualHeap, int max_heap_size);

	static fox_heap_Block* createBlock(fox_heap_BlockQ* pOwner);

	bool isFull() {
		return m_pFree == ADDR_ZERO;
	}

	void insertAfter(fox_heap_Block* pNext) {
		pNext->m_pNext = this->m_pNext;
		this->m_pNext = pNext;
//		pNext->setSibling(this, this->m_pNext);
	}

	void setNext_unsafe(fox_heap_Block* pNext) {
		this->m_pNext = pNext;
//		pNext->setSibling(this, this->m_pNext);
	}

/*
	void setSibling(fox_heap_Block* pPrev, fox_heap_Block* pNext) {
		#ifdef KERNEL_DEBUG
			KASSERT(m_pPrev == ADDR_ZERO);
			KASSERT(m_pNext == ADDR_ZERO);
		#endif
		this->m_pPrev = pPrev;
		this->m_pNext = pNext;
		if (pPrev != ADDR_ZERO) {
			KASSERT(pPrev->getOwner() == this->getOwner());
			KASSERT(pPrev->isBlank()  == this->isBlank());
			pPrev->m_pNext = this;
		}
		if (pNext != ADDR_ZERO) {
			KASSERT(pNext->getOwner() == this->getOwner());
			KASSERT(pNext->isBlank()  == this->isBlank());
			pNext->m_pPrev = this;
		}
	}
*/

	void init(fox_heap_BlockQ* pOwner);

	void destroy();

	void initFreeSpaceArea(fox_heap_Space* pSpace);

	void appendBlankSpaceArea(void* blank_space);

	bool isBlank() {
		return m_pFree == ADDR_ZERO && m_cntAlloc == 0;
	}

private:
	void lock();
	void unlock();

};

#endif