#include <kernel/Kernel.h>
#include <fox/Atomic.h>
#include <fox/Heap.h>
#include <fox/RWLock.h>
#include <fox/kernel/TaskContext.h>

struct PublicInstanceQ : public fastiva_Instance {

public:
	unsigned int m_cntDeref;
	unsigned int m_cntGC;
	unsigned int m_cntUnlink;
	//fastiva_Instance_p m_pLastInstance;

	void reset() { //FOX_HTASK pOwnerTask
		//m_pOwnerTask = pOwnerTask;
		m_cntGC = 0;
		m_cntUnlink = 0;
		m_cntDeref = 0;
		//m_pLastInstance = this;
		this->m_pNext$ = ADDR_ZERO;
	}

	void checkThreadSafe() {
		KASSERT(fox_task_currentTask() == (fox_Task*)Kernel::g_pSystemTask);
	}

	void reset(fastiva_Instance_p pFirst) {
		checkThreadSafe();
		KASSERT(this->m_pNext$ == ADDR_ZERO);
		this->m_pNext$ = pFirst;
	}

	fastiva_Instance_p getFirst() {
		return this->m_pNext$;
	}

	fastiva_Instance_p removeAll() {
		checkThreadSafe();
		fastiva_Instance_p pFirst = this->m_pNext$;
		this->m_pNext$ = ADDR_ZERO;
		return pFirst;
	}

	void FOX_FASTCALL(insert)(fastiva_Instance_p pRookie) {
		checkThreadSafe();
		pRookie->m_pNext$ = this->m_pNext$;
		this->m_pNext$ = pRookie;
	}

	void FOX_FASTCALL(insert)(fastiva_Instance_p pRoot, fastiva_Instance_p pEnd) {
		checkThreadSafe();
		pEnd->m_pNext$ = this->m_pNext$;
		this->m_pNext$ = pRoot;
	}
};


struct fm::SafeInstanceQ { // : public fastiva_Instance {

private:
	fastiva_Instance_p m_pFirst;
	fox_RWLock* m_pLock;
public:
	void init(fox_RWLock* pLock) {
		m_pLock = pLock;
		m_pFirst = ADDR_ZERO;
	}

	void reset(fastiva_Instance_p pRoot) {
		KASSERT(fox_rwlock_isWriteLocked(m_pLock));
		KASSERT(m_pFirst == ADDR_ZERO);
		// finalize-Q에 대해서만 호출된다. thread-safe 하다.
		m_pFirst = pRoot;
	}

	fastiva_Instance_p getFirst() {
		return m_pFirst;
	}

	fastiva_Instance_p removeAll() {
		KASSERT(fox_rwlock_isWriteLocked(m_pLock));
		fastiva_Instance_p pFirst = m_pFirst;
		m_pFirst = ADDR_ZERO;
		return pFirst;
	}

	void FOX_FASTCALL(insert)(fastiva_Instance_p pRookie) {
		KASSERT(fox_rwlock_isWriteLocked(m_pLock));
		pRookie->m_pNext$ = m_pFirst;
		m_pFirst = pRookie;
	}

	void FOX_FASTCALL(insert)(fastiva_Instance_p pRoot, fastiva_Instance_p pEnd) {
		KASSERT(fox_rwlock_isWriteLocked(m_pLock));
		pEnd->m_pNext$ = m_pFirst;
		this->m_pFirst = pRoot;
	}
};


