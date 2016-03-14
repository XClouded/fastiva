#include <string.h>
#include <kernel/heapMark.h>
//#define GET_SLOT_ID(obj)			(obj->m_marked & HEAP_SLOT_ID_MASK)


struct fox_HeapSlot {
	friend struct fox_ReverseSlotIterator;
	friend struct fox_FreeSlotIterator;

	union {
		java_lang_Object_p m_pObject; 
		fox_HeapSlot* m_pNextSlot;
	};
	//int m_id;

	//static void registerHeapInstance(java_lang_Object_p pObj);
	//static void unregisterHeapInstance(java_lang_Object_p pObj);

	//static void enlargeTable();
	static fox_HeapSlot* popHeapSlot();
	static void init$();

	int getSlotID();

	//static fox_HeapSlot* g_pFreeSlot;
	enum { g_firstSlotID = 1 << SLOT_TABLE_ID_SHIFT };

//private:
	static fox_HeapSlot* g_aSlotTable[LAST_SLOT_TABLE_ID + 1]; // 512 * 64 * 1024 / 4
	static fox_HeapSlot* g_pFirstSlot;
	static uint g_emptySlotCount;
	static volatile uint g_freeSlotID;
	static volatile uint g_lastSlotID;
	static fox_Mutex g_HeapSlotTableLock[1];
	static fox_Mutex g_HeapSlotStaticFieldLock[1];

	static jbool isValidSlotID(uint slot_id);
	static fox_HeapSlot* getHeapSlotEx(uint slot_id);
};




inline uint GET_LAST_SIBLING_ID(uint id) {
	return (id | SLOT_OFFSET_MASK);
}

/*
inline unsigned char GET_SLOT_TABLE_ID(uint id)	{
	return ((HeapMark*)&id)->getHeapSlotTableID();
}
*/

inline unsigned int GET_SLOT_OFFSET(uint id) {
	return (id & SLOT_OFFSET_MASK);
}

/*
struct fox_ReverseSlotIterator {
	fox_HeapSlot* m_pSlot;
	int m_idxTable;
	int m_cntSlot;

	fox_ReverseSlotIterator() {
		int freeSlotID = fox_HeapSlot::g_freeSlotID;
		int lastSlotID = fox_HeapSlot::g_lastSlotID;
		m_idxTable = GET_SLOT_TABLE_ID(lastSlotID);
		if (lastSlotID < freeSlotID) {
			m_cntSlot = SLOT_TABLE_ITEM_COUNT;
		}
		else {
			m_cntSlot = GET_SLOT_OFFSET(freeSlotID) / SLOT_OFFSET_INCREE;
		}
		m_pSlot = fox_HeapSlot::g_aSlotTable[m_idxTable] + m_cntSlot;
	}

	fox_HeapSlot* prev() {
		// KASSERT(m_pSlot->m_pObject != ADDR_ZERO);
		return m_pSlot;
	}

	jbool hasMoreElements() {
		do {
			if (--m_cntSlot < 0) {
				if (--m_idxTable < GET_SLOT_TABLE_ID(fox_HeapSlot::g_firstSlotID)) {
					return false;
				}
				m_cntSlot = SLOT_TABLE_ITEM_COUNT - 1;
				m_pSlot = fox_HeapSlot::g_aSlotTable[m_idxTable] + m_cntSlot;
			}
			else {
				m_pSlot --;
			}
		} while (m_pSlot->m_pObject == ADDR_ZERO);

		return true;//(m_pSlot != fox_HeapSlot::g_pFirstSlot);
	}
};

struct fox_FreeSlotIterator {
	fox_HeapSlot* m_pSlot;
	fox_HeapSlot** m_pCurrTable;
	uint m_currID;
	uint m_lastID;

	fox_FreeSlotIterator() {
		m_currID = fox_HeapSlot::g_firstSlotID;
		m_pCurrTable = fox_HeapSlot::g_aSlotTable + 1;
		m_pSlot = *m_pCurrTable;
		m_lastID = GET_LAST_SIBLING_ID(m_currID);
	}

	fox_HeapSlot* nextFree() {
		KASSERT(m_pSlot->m_pObject == ADDR_ZERO);
		if ((m_currID += SLOT_OFFSET_INCREE) > m_lastID) {
			// tableÀÌ ¹Ù²î¾ú´Ù.
			fox_HeapSlot* pTemp = m_pSlot;
			m_pCurrTable ++;
			m_pSlot = *m_pCurrTable;
			m_lastID = GET_LAST_SIBLING_ID(m_currID);
			return pTemp;
		}
		else {
			return m_pSlot++;
		}
	}

	int currSlotID() {
		return m_currID;
	}

	jbool hasMoreElements() {
		while (m_pSlot->m_pObject != ADDR_ZERO) {
			if ((m_currID += SLOT_OFFSET_INCREE) > m_lastID) {
				// tableÀÌ ¹Ù²î¾ú´Ù.
				m_pCurrTable ++;
				m_pSlot = *m_pCurrTable;
				if (m_pSlot == ADDR_ZERO) {
					return false;
				}
				m_lastID = GET_LAST_SIBLING_ID(m_currID);
			}
			else {
				m_pSlot ++;
			}
		}
		return true;
	}
};

#define GC_FAST_SCAN 1
#define GC_FULL_SCAN 2
#define GC_APP_SCAN  3

#define ASSERT2(t)	if (!(t)) { _asm int 3 }
//typedef void (FAST_API(*SCAN_METHOD))(fm::InstanceRef);
#define GC_FIELD_REF_OF(pObj)	(*(fastiva_Instance_p*)&pObj)
*/
