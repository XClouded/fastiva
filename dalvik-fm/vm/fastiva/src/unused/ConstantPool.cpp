#include <precompiled_libcore.h>
#include <kernel/Kernel.h>
#include <kernel/HeapMark.h>
#include <java/io/InputStream.inl>
// v3.2006
//#include <java/lang/BootClassLoader.inl>
#include <java/lang/Thread.inl>
#include <java/lang/Runnable.inl>
#include <pal/fox_file.h>
#include <fox/Heap.h>

#include <precompiled_libcore.h>
#include "string.h"
#include <fastiva_malloc.h>

extern fastiva_ModuleInfo fastiva_component_ModuleInfo;

/*
const char* JNI_UtfStringPool::addString(const char* token, int len) {
	KASSERT(len >= 0);
	char* dst0 = allocate(len+1);

	memcpy(dst0, token, len);
	dst0[len] = 0;
	return dst0;
}
*/

JNI_UtfHashTable::Entry* JNI_UtfHashTable::findEntry(uint hashCode, const char* key) {
	// @todo thread-safety 검사.
	// ??? KASSERT(fox_monitor_isLocked(this->m_monitor$));
	int idxSlot = (hashCode >> 16) % SLOT_COUNT;//~id / slotSize;
	Entry* entry = m_aHashSlot[idxSlot];
	int len = hashCode & 1023;
	while (entry != ADDR_ZERO) {
		if (entry->m_hashCode == (ushort)hashCode
		&&  memcmp(key, entry->getToken(), len) == 0) {
			return entry;
		}
		entry = entry->m_pNextEntry;
	}
	return ADDR_ZERO;
}

int JNI_UtfHashTable::addEntryEx(uint hashCode, const char* key) {
	{   SYNCHRONIZED$(this) 
		Entry* pEntry = findEntry(hashCode, key);
		if (pEntry != ADDR_ZERO) {
			return pEntry->m_id;
		}

		int len = JNI_HashEntry::getLength(hashCode);
		Entry* entry = (Entry*)kernelData.stringPool.allocate(sizeof(Entry) + len + 1);
		char* keyword = (char*)entry->getToken();
		memcpy(keyword, key, len);
		keyword[len] = 0;
		entry->m_hashCode = hashCode;
		int idxSlot = (hashCode >> 16) % SLOT_COUNT;//~id / slotSize;
		int idx = addKeyword(keyword);
		entry->m_pNextEntry = m_aHashSlot[idxSlot];
		m_aHashSlot[idxSlot] = entry;
		return (entry->m_id = (ushort)idx);
	}
}

int JNI_UtfHashTable::findRefName(const char* name) {
	int hashCode = JNI_HashEntry::getHashCode(name);
	int id;
	id = JNI_HashEntry::searchToken(
		fastiva_libcore_ModuleInfo.m_aRefName, fastiva_libcore_ModuleInfo.m_cntRefName - 1,
		hashCode, name);
	if (id >= 0) {
		id += JPP_REF_NAME_ID::cntPredefinedID;
		return id;
	}
	id = JNI_HashEntry::searchToken(
		fastiva_component_ModuleInfo.m_aRefName, fastiva_component_ModuleInfo.m_cntRefName,
		hashCode, name);
	if (id >= 0) {
		id += JPP_EXTERNAL_REF_NAME_ID_START;
	}
	else {
		Entry* pEntry = findEntry(hashCode, name);
		if (pEntry == ADDR_ZERO) {
			return -1;
		}
		id = pEntry->m_id;
	}
	return id;
}


int JNI_UtfHashTable::addRefName(const char* name) {
	int id = findRefName(name);
	if (id >= 0) {
		id += JPP_REF_NAME_ID::cntPredefinedID + 1;
	}
	else {
		int hashCode = JNI_HashEntry::getHashCode(name);
		id = this->addEntryEx(hashCode, name);
	}
	return id;
}


int JNI_UtfHashTable::addArguments(ushort* aArgType, int hashCode) {
	int id = JNI_HashEntry::searchToken(
		fastiva_libcore_ModuleInfo.m_aArgList, fastiva_libcore_ModuleInfo.m_cntArgList - 1, 
		hashCode, (char*)aArgType);

	if (id >= 0) {
		return id + JPP_LIBCORE_ARGLIST_ID_START;
	}

	id = JNI_HashEntry::searchToken(
		fastiva_component_ModuleInfo.m_aArgList, fastiva_component_ModuleInfo.m_cntArgList - 1, 
		hashCode, (char*)aArgType);

	if (id >= 0) {
		return id + JPP_EXTERNAL_ARGLIST_ID_START;
	}

	FASTIVA_DBREAK(); // not implemented;
	//id = this->addEntryEx(hashCode, (char*)aArgType) + JPP_EXTERNAL_ARGLIST_ID_START + fastiva_component_moduleInfo.m_cntArgList;
	// 새로 등록된 ARGS-ID는 CLASS-ID 영역 밖에서 시작한다.
	return id;
}

const char* JNI_UtfHashTable::getKeyword(uint id) {
	if (id >= 0xFFFF) {
		FASTIVA_DBREAK();
		return "<Invalid Name ??>";
	}

	KeyTable* pIndex = m_pFirstTable;
	for (int i = id / KeyTable::capacity; i -- > 0; ) {
		pIndex = pIndex->m_pNextTable;
	}
	const char* key = pIndex->m_aKeyword[id % KeyTable::capacity];
	return key;
}

int JNI_UtfHashTable::addKeyword(const char* keyword) {
	int idx = m_cntKeyword % KeyTable::capacity;
	if (idx == 0) {
		KeyTable* pNewTable = (KeyTable*)fox_heap_malloc(sizeof(KeyTable));
		pNewTable->m_pNextTable = ADDR_ZERO;
		if (m_pLastTable == ADDR_ZERO) {
			m_pFirstTable = pNewTable;
		}
		else {
			m_pLastTable->m_pNextTable = pNewTable;
		}
		this->m_pLastTable = pNewTable;
		// m_aKeyword[0] 에는 m_pNextTable가 저장된다.
		idx = 1;
		m_cntKeyword ++;
	}
	m_pLastTable->m_aKeyword[idx] = keyword;
	return m_cntKeyword++;
}

ushort JNI_FindField::registerFieldName(const char* name) {
	return (ushort)kernelData.g_utfHashTable.addRefName(name);
}

ushort JNI_FindField::findFieldName(const char* name) {
	return (ushort)kernelData.g_utfHashTable.findRefName(name);
}

ushort JNI_FindField::findFieldType(const char* typeSig) {
	return JNI_FindClass::parseValueType(typeSig, ADDR_ZERO);
}



const char* JNI_FindField::getFieldName(uint nameID) {
	int idx = nameID;
	const char* name;
	if (idx >= JPP_EXTERNAL_REF_NAME_ID_START) {
		nameID -= JPP_EXTERNAL_REF_NAME_ID_START;
		name = fastiva_component_ModuleInfo.m_aRefName[nameID].m_pszToken;
	}
	else if (idx > JPP_REF_NAME_ID::cntPredefinedID) {
		nameID -= JPP_REF_NAME_ID::cntPredefinedID;
		name = fastiva_libcore_ModuleInfo.m_aRefName[nameID].m_pszToken;
	}
	else {
		name = kernelData.g_utfHashTable.getKeyword(idx);
	}
	return name;
}

int JNI_FindField::getFieldType(uint typeID, char* buff, int sizBuff) {
	int sig_len = JNI_FindClass::getTypeSig(typeID, buff, sizBuff);
	KASSERT(sig_len < sizBuff);
	buff[sig_len] = 0;
	return sig_len;
}

ushort JNI_FindMethod::findMethodName(const char* name) {
	return (ushort)kernelData.g_utfHashTable.findRefName(name);
}


ushort JNI_FindMethod::registerMethodName(const char* name) {
	return (ushort)kernelData.g_utfHashTable.addRefName(name);
}

const char* JNI_FindMethod::getMethodName(uint nameID) {
	return JNI_FindField::getFieldName(nameID);
}


int JNI_ArgIterator::init(uint argsID) {
	
	if (FASTIVA_IS_VALID_CLASS_ID(argsID)) {
		if (argsID == fastiva_Primitives::jvoid) {
			return (this->parameterCount = 0);
		}
		m_pArgType = &m_reserved;
		m_reserved = argsID;
		return (this->parameterCount = 1);
	}

	//int idx = argsID & FASTIVA_JNI_LAST_CLASS_ID;
	//idx -= 1;

	if (argsID >= JPP_EXTERNAL_ARGLIST_ID_START) {
		argsID -= JPP_EXTERNAL_ARGLIST_ID_START;
		m_pArgType = (FASTIVA_TOKEN_T*)fastiva_component_ModuleInfo.m_aArgList[argsID].m_pszToken;
	}
	else {
		//KASSERT(argsID > JPP_ARGLIST_ID::cntPredefinedID);
		argsID -= JPP_LIBCORE_ARGLIST_ID_START;
		m_pArgType = (FASTIVA_TOKEN_T*)fastiva_libcore_ModuleInfo.m_aArgList[argsID].m_pszToken;
	}
	/*else {
		//argsID -= FASTIVA_JNI_LAST_CLASS_ID + 1;
		//m_pArgType = (FASTIVA_TOKEN_T*)kernelData.g_utfHashTable.getKeyword(argsID);
	}*/
	int cntArg = *m_pArgType ++;
	return (this->parameterCount = cntArg);
}




int JNI_FindMethod::getArgumentsType(int argsID, char* buff, int sizBuff) {
	char* buff_org = buff;

	JNI_ArgIterator iter;
	int cntArg = iter.init(argsID);
	while (cntArg -- > 0) {
		int len = JNI_FindClass::getTypeSig(iter.nextID(), buff, sizBuff);
		if (len <= 0) {
			return 0;
		}
		buff += len;
		sizBuff -= len;
	}
	buff[0] = 0;
	int sig_len = buff - buff_org;
	KASSERT(sizBuff > 0);
	return sig_len;
}


void JNI_FindMethod::setMethodType(const char* typeSig) {
	if (*typeSig ++ != '(') {
        fastiva_throwInternalError("invalid method type signatures");
	}

	JNI_FindClass fc;
	fc.init(typeSig);
	fc.m_delimiter = ';';

	int argSize = 0;
	ushort aArgType[256];
	ushort* pArgType = aArgType + 1;

	int type;
	int cntArg = 0;
	while ((type = fc.parseArgType()) >= 0) {
		switch (type) {
			case fastiva_Primitives::jdouble:
			case fastiva_Primitives::jlonglong:
				argSize ++;
		}
		cntArg ++;
		argSize ++;
        if (argSize > 255) {
            fastiva_throwInternalError("too many method arguments");
        }
		*pArgType ++ = type;
	}
	if (fc.m_sig[-1] != ')') {
        fastiva_throwInternalError("invalid method type signatures");
	}
	int arg_sig_len = fc.m_sig - typeSig - 1;
	int hashCode = JNI_HashEntry::getHashCode(typeSig, arg_sig_len);
	this->m_argSize = argSize;
	this->m_cntArg = cntArg;

	this->m_retType = fc.parseArgType();
	int args_len;
	if (cntArg == 0) {
		this->m_args = fastiva_Primitives::jvoid;
	}
	else 
	if (cntArg == 1 && FASTIVA_IS_VALID_CLASS_ID(aArgType[+1])) {
		// ARG 갯수가 하나이고, ARRAY가 아닌 경우,
		// CLASS-ID를 그대로 사용한다.
		this->m_args = aArgType[+1];
	}
	else {
		int sig_len = (int)pArgType - (int)aArgType;
		aArgType[0] = cntArg;
		hashCode = (hashCode & ~1023) + sig_len;
		this->m_args = kernelData.g_utfHashTable.addArguments(aArgType, hashCode);
	}
}


struct JNI_RawContext::Buffer {

	struct Header {
		/**
		  RawContext 는 real-context loading 전에 잠시 사용되는 것이므로
		  현재 설정된 크기를 벗어나는 일은 거의 발생하지 않는다.
		  따라서 구지 array를 사용하지 않고 m_pNextTable을 사용하였다.
		*/
		Buffer* m_pNextBuffer;
	} m_header;

	enum { capacity = (4096 - sizeof(Buffer::Header)) / sizeof(JNI_RawContext) };

	JNI_RawContext m_aRawContext[capacity];
};

struct JNI_RawContext::ContextList {

	enum { capacity = 4096 / sizeof(JNI_RawContext*) };

	union {
		fastiva_ClassContext* m_apContext[capacity];
		/**
		  Context-IF를 실제 RawContext 로 변경하는 일은 매우 드물게 발생한다.
		  따라서 구지 array를 사용하지 않고 m_pNextList을 사용하였다.
		*/
		ContextList* m_pNextList;
	};
};

/**
Reflection에서 Field-Type과 Method-Type을 얻기 위해 사용된다?
*/
fastiva_ClassContext** JNI_RawContext::getContextSlot(int idx) {
	KASSERT(FASTIVA_IS_VALID_CLASS_ID(idx));

	if (idx >= JPP_EXTERNAL_ARGLIST_ID_START) {
		idx -= JPP_EXTERNAL_ARGLIST_ID_START;
		return (fastiva_ClassContext**)&fastiva_component_ModuleInfo.m_ppImportedContext[idx];
	}
	else if (idx > JPP_CLASS_ID::cntPredefinedClassID) {
		// primitive context는 g_aBootstrapClassContext 에 저장되지 않는다.
		// hashCode에 의한 sorting이 불가능하기 때문이다.
		idx -= JPP_CLASS_ID::cntPredefinedClassID + 1;
		return (fastiva_ClassContext**)&fastiva_libcore_ModuleInfo.m_ppImportedContext[idx];
	}
	idx -= fastiva_Primitives::cntType;
	KASSERT((uint)idx < (uint)kernelData.g_cntContextID);

	fox_mutex_lock_GCI(kernelData.g_pContextIDLock);
	int idxContext = idx % ContextList::capacity;
	int idxTable = idx / ContextList::capacity;
	KASSERT(idxContext > 0); // 첫번째 slot은 m_pNextList를 저장하기 위해 사용된다.

	ContextList* pList = kernelData.g_pContextList;
	while (--idxTable >= 0) {
		pList = pList->m_pNextList;
	}
	fox_mutex_release(kernelData.g_pContextIDLock);
	return pList->m_apContext + idxContext;
}

fastiva_ClassContext* JNI_RawContext::create(fastiva_Package* pPackage, const char* pszBaseName, int nameLen) {
	fox_mutex_lock_GCI(kernelData.g_pContextIDLock);
	JNI_RawContext* pRookie = kernelData.g_pFreeRawContext;
	if (pRookie == ADDR_ZERO) {
		pRookie = allocateBuffer();
	}
	kernelData.g_pFreeRawContext = pRookie->m_pNextFree;
	pRookie->m_state = -1;
	pRookie->m_pBaseName = kernelData.stringPool.addString(pszBaseName, nameLen);
	pRookie->m_pPackageName = pPackage->m_pszName;

	/*
	RawContext를 생성하는 회수만큼, id 요구시마다 contextID를 증가시킨다.
	*/
	int cntContextID = kernelData.g_cntContextID;
	ContextList* pLastList = kernelData.g_pContextList;
	int idx = cntContextID % ContextList::capacity;
	if (idx == 0) {
		// 첫번째 Slot은 비워둔다. m_pNextList로 사용.
		cntContextID ++;
		idx = 1;

		ContextList* pNewList = (ContextList*)fox_heap_malloc(sizeof(ContextList));
		pNewList->m_pNextList = ADDR_ZERO;
		if (pLastList == ADDR_ZERO) {
			kernelData.g_pContextList = pLastList = pNewList;
		}
		else {
			while (pLastList->m_pNextList != ADDR_ZERO) {
				pLastList = pLastList->m_pNextList;
			}
			pLastList->m_pNextList = pNewList;
		}
	}
	else {
		while (pLastList->m_pNextList != ADDR_ZERO) {
			pLastList = pLastList->m_pNextList;
		}
	}
	// 2012.06.07 임시로 막음. pRookie->m_id = cntContextID + fastiva_Primitives::cntType;
	kernelData.g_cntContextID = ++cntContextID;
	pLastList->m_apContext[idx] = (fastiva_ClassContext*)pRookie;
	fox_mutex_release(kernelData.g_pContextIDLock);
	return (fastiva_ClassContext*)pRookie;
}

void JNI_RawContext::free2(JNI_RawContext* pRetiree) {
	pRetiree->m_pNextFree = kernelData.g_pFreeRawContext;
	kernelData.g_pFreeRawContext = pRetiree;
}


JNI_RawContext* JNI_RawContext::allocateBuffer() {
	Buffer* pNewBuffer =(Buffer*)fox_heap_malloc(sizeof(Buffer));

	JNI_RawContext* pContext = pNewBuffer->m_aRawContext;
	kernelData.g_pFreeRawContext = pContext;

	for (int i = Buffer::capacity; --i > 0; /*마지막 하나 생략*/) {
		pContext->m_pNextFree = pContext + 1;
		pContext ++;
	}
	pContext->m_pNextFree = ADDR_ZERO;
	KASSERT(pContext == pNewBuffer->m_aRawContext + Buffer::capacity - 1);

	pNewBuffer->m_header.m_pNextBuffer = ADDR_ZERO;
	Buffer* pLastBuffer = kernelData.g_pContextBuffer;
	if (pLastBuffer == ADDR_ZERO) {
		kernelData.g_pContextBuffer = pNewBuffer;
	}
	else {
		while (pLastBuffer->m_header.m_pNextBuffer != ADDR_ZERO) {
			pLastBuffer = pLastBuffer->m_header.m_pNextBuffer;
		}
		pLastBuffer->m_header.m_pNextBuffer = pNewBuffer;
	}
	return pContext;
}


#ifdef _DEBUG
void JNI_FindField::dump() {
	char buff[1024];
	const char* name = this->getFieldName(this->m_name);
	fox_debug_printf(name);
	buff[0] = ':';
	int len = this->getFieldType(this->m_type, buff + 1, sizeof(buff) - 2) + 1;
	buff[len++] = '\n';
	buff[len] = 0;
	fox_debug_printf(buff);
}

void JNI_FindMethod::dump() {
	char buff[1024];
	const char* name = this->getMethodName(this->m_name);
	fox_debug_printf(name);
	buff[0] = '(';
	int len = this->getArgumentsType(this->m_args, buff + 1, sizeof(buff) - 2) + 1;
	buff[len] = ')';
	buff[++len] = 0;
	fox_debug_printf(buff);
	len = JNI_FindClass::getTypeSig(this->m_retType, buff, sizeof(buff) - 2);
	buff[len++] = '\n';
	buff[len] = 0;
	fox_debug_printf(buff);
}



int JNI_FindClass::getTypeSig(uint typeID, char* buff, int sizBuff) {
	KASSERT(typeID < 0xFFFF);
	if (sizBuff < 2) {
		return 0;
	}

	char* dst = buff;
	java_lang_Class_p pClass = fastiva.primitiveClasses[typeID];
	int array_depth = 0;
	while (pClass->m_pComponentType$ != NULL) {
		array_depth ++;
		pClass = pClass->m_pComponentType$;
	}
	const fastiva_ClassContext* pContext = pClass->m_pContext$;

	//int array_depth = typeID / FASTIVA_JNI_ARRAY_DIMENSION_BITS(1);
	if ((sizBuff -= array_depth) <= 1) {
		goto insufficient_buffer;
	}
	for (int d = array_depth; d -- > 0; ) {
		*dst++ = '[';
	}

	//typeID &= FASTIVA_JNI_LAST_CLASS_ID;
	if (typeID <= fastiva_Primitives::jvoid) {
		*dst++ = pContext->m_pBaseName[0];
		*dst = 0;
		return array_depth + 1;
	}

	if (array_depth > 0) {
		*dst++ = 'L';
		sizBuff -= 2;
	}

	//fastiva_ClassContext** ppContext = JNI_RawContext::getContextSlot(typeID);
	//const fastiva_ClassContext* pContext = *ppContext;
	fastiva_Package* pPackage = (fastiva_Package*)pContext->getPackage();

	int p_len = strlen(pPackage->m_pszName);
	int c_len = strlen(pContext->m_pBaseName);

	const char* src;
	char ch;

	if (p_len > 0) {
		if (sizBuff < p_len + c_len + 2) {
			goto insufficient_buffer;
		}
		src = pPackage->m_pszName;
		while ((ch = *src++) != 0) {
			*dst ++ = ch;
		}
		*dst ++ = '/';
	}
	else {
		if (sizBuff < c_len + 1) {
			goto insufficient_buffer;
		}
	}
	src = pContext->m_pBaseName;
	while ((ch = *src++) != 0) {
		*dst ++ = ch;
	}
	
	if (array_depth > 0) {
		// bufSize 검사는 위에서 미리 됨.
		*dst ++ = ';';
	}
	*dst = 0;
	return dst - buff;

insufficient_buffer:
	buff[0] = '?';
	buff[1] = 0;
	return 0;
}

#endif