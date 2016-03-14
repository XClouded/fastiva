#include <Dalvik.h>

#include <kernel/JniConstant.h>
#include <dalvik_Kernel.h>
#include <java/io/InputStream.inl>
// v3.2006
//#include <fastiva/BootstrapClassLoader.inl>
#include <java/lang/Thread.inl>
#include <java/lang/Runnable.inl>
//#include <pal/fox_file.h>
//#include <fox/Heap.h>

//#include <tchar.h>
#include "string.h"
//#include <fastiva_malloc.h>

enum { BUFFER_SIZE = 4096 };

char* JNI_UtfStringPool::allocate(uint len) {
	{	SYNCHRONIZED$(this)
		assert((m_position & 3) == 0);

		char* dst0 = (char*)m_pBuffer + m_position;
		len = (len + 3) & ~3;

		if ((m_position += len) > BUFFER_SIZE) {
			Buffer* pNewBuffer = (Buffer*)/*fox_heap_*/malloc(BUFFER_SIZE);
			pNewBuffer->m_pNextBuffer = m_pBuffer;
			dst0 = pNewBuffer->m_buff;
			this->m_pBuffer = pNewBuffer;
			this->m_position = len + sizeof(BufferHeader);
			dst0 = m_pBuffer->m_buff;
		}
		return dst0;
	}
}

const char* JNI_UtfStringPool::addString(const char* token, int len) {
	assert(len >= 0);
	char* dst0 = allocate(len+1);

	memcpy(dst0, token, len);
	dst0[len] = 0;
	return dst0;
}

/*


int JNI_HashEntry::searchToken(const JNI_HashEntry* aToken, int high, uint hashCode, const char* pszToken) {
	//assert(high >= 0);
	uint lenToken = hashCode & 1023;
	const JNI_HashEntry* pToken;
	int low = 0;

	while (low <= high) {
		int mid = (low + high) / 2;
		pToken = aToken + mid;
		uint slot_hash = pToken->m_hashCode;
		if (slot_hash > hashCode) {
			high = mid - 1;
		}
		else if (slot_hash < hashCode) {
			low = mid + 1;
		}
		else {
			if (memcmp(pToken->m_pszToken, pszToken, lenToken) == 0) {
				return mid;
			}
			const JNI_HashEntry* pPrev = pToken - 1;
			while (pPrev->m_hashCode == hashCode) {
				if (memcmp(pPrev->m_pszToken, pszToken, lenToken) == 0) {
					return (pPrev - aToken);
				}
				pPrev --;
			}
			const JNI_HashEntry* pNext = pToken + 1;
			while (pNext->m_hashCode == hashCode) {
				if (memcmp(pNext->m_pszToken, pszToken, lenToken) == 0) {
					return (pNext - aToken);
				}
				pNext ++;
			}
			return ~mid;
		}
	}
	return ~low;
}
*/

const JNI_HashEntry* JNI_HashMap::findEntry(uint hashCode, KeyComparer* finder) {
	//assert(high >= 0);
	uint lenToken = hashCode & 1023;
	const JNI_HashEntry* pSlot;
	const JNI_HashEntry* aSlot = this->m_aSlot;
	int low = 0;
	int high = this->m_cntSlot - 1;

	while (low <= high) {
		int mid = (low + high) / 2;
		pSlot = aSlot + mid;
		uint slot_hash = pSlot->m_hashCode;
		if (slot_hash > hashCode) {
			high = mid - 1;
		}
		else if (slot_hash < hashCode) {
			low = mid + 1;
		}
		else {
			const JNI_HashEntry* pNext = pSlot;
			while (true) {
				if (finder->compare(pNext->m_pData) == 0) {
					return pNext;
				}
				pNext ++;
				if (pNext->m_hashCode != hashCode) {
					break;
				}
			}
			const JNI_HashEntry* pPrev = pSlot - 1;
			while (pPrev->m_hashCode == hashCode) {
				if (finder->compare(pPrev->m_pData) == 0) {
					return pPrev;
				}
				pPrev --;
			}
			return NULL; //~mid;
		}
	}
	return NULL; //~low;
}
