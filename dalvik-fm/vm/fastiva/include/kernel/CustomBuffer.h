#ifndef __CUSTOM_BUFFER_H__
#define __CUSTOM_BUFFER_H__


#include <kernel/Kernel.h>


template <class ITEM_T> //, class CONTROL> 
class fastiva_Vector : public fastiva_ArrayHeader {
protected:
	int		m_cntItem;
	ITEM_T  m_aItem[64];

public:


	bool isValid() { 
		return this->m_cntItem <= this->m_length; 
	}

	bool isValid(uint idx) {
		return isValid() && idx < (uint)this->m_cntItem;
	}

	bool isFull() {
		KASSERT(this->isValid());
		return this->m_cntItem >= this->m_length;
	}

/*
	ITEM_T get$(uint idx) {
		KASSERT(isValid(idx));
		return CONTROL::get$(this, &m_aItem[idx]);
	}

	void set$(uint idx, ITEM_T item) {
		KASSERT(isValid(idx));
		CONTROL::set$(this, &m_aItem[idx], item);
	}
*/

	const ITEM_T* getBuffer(uint idx, int len = 0) {
		KASSERT(isValid());
		KASSERT(len >= 0);
		KASSERT(idx + len <= (uint)getItemCount());
		return &this->m_aItem[idx];
	}

	int getMaxLength() {
		KASSERT(isValid());
		return this->m_length;
	}

	int getItemCount() {
		KASSERT(isValid());
		return this->m_cntItem;
	}

	void setItemCount(int count) {
		this->m_cntItem = count;
		KASSERT(isValid());
	}

	void append(ITEM_T item) {
		KASSERT(!this->isFull());
		*(ITEM_T*)getBuffer(this->m_cntItem++) = item;
	}

	static int getAllocSize(int length) {
		return (int)&((fastiva_Vector*)0)->m_aItem[length];
	} 


};



#endif // __CUSTOM_BUFFER_H__


/**====================== end ========================**/