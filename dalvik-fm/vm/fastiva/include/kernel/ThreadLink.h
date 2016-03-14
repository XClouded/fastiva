#ifndef __FOX_THREAD_H__
#define __FOX_THREAD_H__

#include <fastiva/config.h>

#define FOX_UNLINKED_HTHREAD	((fox_ThreadLink*)0xDCDCDCDC)
#define FASTIVA_NOTIFY_SINGLE		0
#define FASTIVA_NOTIFY_ALL			1
#define FASTIVA_NOTIFY_INTERRUPT	2


class fox_ThreadLink {
	// GreenThread에서 순환에 의한 하위 Priority thread를 수행하기 위해 필요.
public:
	fox_ThreadLink* m_pPrevRunnable;
	fox_ThreadLink* m_pNextRunnable;
	int m_priority;
		
	void unlink();
	void link(fox_ThreadLink* pLeft, fox_ThreadLink* pRight);
};


struct fox_ThreadQ : public fox_ThreadLink {

	void insert(fox_ThreadLink* pThread);

	void reset();

	fox_ThreadLink* firstRunnable() {
		return m_pNextRunnable;
	}

	fox_ThreadLink* lastRunnable() {
		return m_pPrevRunnable;
	}

	jbool isEmpty() {
		return this == m_pNextRunnable;
	}
};


inline void fox_ThreadLink::unlink() {
	m_pPrevRunnable->m_pNextRunnable = this->m_pNextRunnable;
	m_pNextRunnable->m_pPrevRunnable = this->m_pPrevRunnable;
#ifdef _DEBUG
	this->m_pPrevRunnable = FOX_UNLINKED_HTHREAD;
	this->m_pNextRunnable = FOX_UNLINKED_HTHREAD;
#endif
}

inline void fox_ThreadLink::link(
	fox_ThreadLink* prev, 
	fox_ThreadLink* next
) {
	KASSERT(this != prev);
	KASSERT(this != next);
	KASSERT(this->m_pPrevRunnable == FOX_UNLINKED_HTHREAD);
	KASSERT(this->m_pNextRunnable = FOX_UNLINKED_HTHREAD);

	this->m_pPrevRunnable = prev;
	this->m_pNextRunnable = next;

	KASSERT(prev != ADDR_ZERO);
	KASSERT(next != ADDR_ZERO);
	next->m_pPrevRunnable = this;
	prev->m_pNextRunnable = this;
}

inline void fox_ThreadQ::reset() {
	this->m_pPrevRunnable = this;
	this->m_pNextRunnable = this;
	this->m_priority = -2;
}

#endif //__FOX_THREAD_H__



/**====================== end ========================**/