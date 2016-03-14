#ifndef __FASTIVA_JNI_CONSTANT_H__
#define __FASTIVA_JNI_CONSTANT_H__

struct JNI_UtfStringPool : fastiva_Instance {
private:
	enum { BUFFER_SIZE = 4096 };

	struct Buffer;
	struct BufferHeader {
		Buffer* m_pNextBuffer;
	};
	struct Buffer : public BufferHeader {
		char m_buff[BUFFER_SIZE - sizeof(BufferHeader)];
	};

	Buffer* m_pBuffer;
	int m_position;

public:
	void init() {
		KASSERT(m_pBuffer == ADDR_ZERO);
		m_position = BUFFER_SIZE;
	}

	char* FOX_FASTCALL(allocate)(uint len);

	const char* FOX_FASTCALL(addString)(const char* token, int len);
};



#include <fastiva/ClassContext.h>
#include <stdarg.h>


struct JNI_UtfHashTable : fastiva_Instance {
private:
	enum { BUFFER_SIZE = 4096 };

	struct Entry {
		ushort m_hashCode;
		ushort m_id;
		Entry* m_pNextEntry;
		const char* getToken() {
			return (char*)(this + 1);
		}
	};
	enum	{ SLOT_COUNT = 512 }; //jpp_component_UtfID::cntString / 1024 + 1; }
	Entry*  m_aHashSlot[SLOT_COUNT];//jpp_component_UtfID::cntString / slotSize];


	struct KeyTable {
		enum { capacity = BUFFER_SIZE / sizeof(void*) };
		union {
			KeyTable* m_pNextTable;
			const char* m_aKeyword[capacity];
		};
	};
	KeyTable*   m_pFirstTable;
	KeyTable*   m_pLastTable;
	int			m_cntKeyword;

public:

	int addRefName(const char* name);	

	int findRefName(const char* name);

	int addArguments(ushort* aArgType, int hashCode);

	const char* getKeyword(uint id);

private:
	int addEntryEx(uint hashCode, const char* key);

	Entry* findEntry(uint hashCode, const char* key);

	int addKeyword(const char* keyword);
};


struct JNI_RawContext {
	struct Buffer;
	struct ContextList;

	union {
		const char* m_pBaseName;
		JNI_RawContext* m_pNextFree;
	};

	union {
		const  fastiva_ClassContext* m_pImportedContext;
		const  char* m_pPackageName;
	};

	short  m_state;
	ushort m_inheritDepth;

	enum {
		NOT_IMPORTED = -1,
		IMPORTED = -2,
	};

	fastiva_Package* getPackage() {
		return (fastiva_Package*)(((fastiva_PackageInfo*)m_pPackageName) - 1);
	}

	const fastiva_ClassContext* getImportedContext() const {
		KASSERT(m_state == IMPORTED);
		return m_pImportedContext;
	}

	void setImportedContext(const fastiva_ClassContext* pContext) {
		KASSERT(m_state == NOT_IMPORTED);
		m_pImportedContext = pContext;
		m_state = IMPORTED;
	}


	static fastiva_ClassContext* create(fastiva_Package* pPackage, const char* pszBaseName, int nameLen);

	static void free2(JNI_RawContext* pRetiree);

	static fastiva_ClassContext** FOX_FASTCALL(getContextSlot)(
		int classID
	);


private:
	static JNI_RawContext* allocateBuffer();
};


/* @zee 2010.06.01 필요치 않아 없엠.
struct fastiva_PackageInfo { //: public JNI_HashEntry {
	int m_hashCode;
	const char* m_pszPackageName;
	const char* getPackageName() const {
		return m_pszPackageName;//(char*)m_pPackage + sizeof(fastiva_Package);
	}
	const fastiva_Package* getPackage() const {
		return (fastiva_Package*)(m_pszPackageName - sizeof(fastiva_PackageInfo));
	}
};
*/

#ifndef ANDROID
struct JNI_FindField {
	ushort m_name;
	ushort m_type;
	jint  m_accessFlags;

	const fastiva_ClassContext* m_pFoundContext;	// [out]

	void init(const char* name, const char* typeSignature) {
		this->m_name = findFieldName(name);
		this->m_type = findFieldType(typeSignature);
	}

	void init(const char* name, ushort fieldType) {
		this->m_name = findFieldName(name);
		this->m_type = fieldType;
	}

	void init(ushort nameID, ushort fieldType) {
		this->m_name = nameID;
		this->m_type = fieldType;
	}

	// 0 : continue, -1 : fail, +1 : match found.
	virtual int matches(const fastiva_Field* pField);

	static const char* getFieldName(uint nameID);

	static int getFieldType(uint typeID, char* buff, int sizBuff);

	// 0 : not found;
	static ushort findFieldName(const char* name);

	static ushort findFieldType(const char* name);

	static ushort registerFieldName(const char* name);

#ifdef _DEBUG
	void dump();
#endif
};

struct JNI_FindMethod { //: JNI_FindClass {
	FASTIVA_TOKEN_T m_name;
	unsigned short m_accessFlags; 
	unsigned short m_offset; // offset from vtable or func-addr;
	FASTIVA_TOKEN_T m_retType;
	FASTIVA_TOKEN_T m_args;
	unsigned char m_cntArg;
	unsigned char m_argSize; // the stack size of paramters / size of stack-slot

	void init(const char* name, const char* typeSignature) {
		this->m_name = findMethodName(name);
		setMethodType(typeSignature);
	}

	void init(const char* name, ushort retType = fastiva_Primitives::jvoid) {
		this->m_name = findMethodName(name);
		this->m_retType = retType;
		this->m_args = fastiva_Primitives::jvoid;
		this->m_argSize = this->m_cntArg = 0;
	}

	void initConstructor(const char* typeSignature) {
		this->m_retType = fastiva_Primitives::jvoid;
		this->m_name = JPP_REF_NAME_ID::init$$;
		setMethodType(typeSignature);
	}

	void initDefaultConstructor() {
		this->m_retType = fastiva_Primitives::jvoid;
		this->m_name = JPP_REF_NAME_ID::init$$;
		this->m_args = FASTIVA_SIG_void;
		this->m_argSize = this->m_cntArg = 0;
	}

	// 0 : continue, -1 : fail, +1 : match found.
	int matches(const fastiva_Method* pMethod);

	static const char* getMethodName(uint nameID);

	static ushort registerMethodName(const char* name);

	static ushort findMethodName(const char* name);

	static int getArgumentsType(int argsID, char* buff, int sizBuff);

#ifdef _DEBUG
	void dump();
#endif
private:

	void setMethodType(const char* typeSignature);

};

#endif
/*
class JNI_ArgIterator {
#ifdef ANDROID
	uint __unsed;
#endif
	uint* m_pArgType;
	uint  m_reserved;
public:
	// returns ArgumentCount. if the value is 1, do not use iterator
	int init(uint argListID);

	uint nextID() {
		return *m_pArgType ++;
	}

	java_lang_Class_p nextArgType();

	const fastiva_ClassContext* nextArgContext(int* array_dimension);

	const uint* currentArgTypePointer() {
		return m_pArgType;
	}
};
*/

//#include "kernel/BootstrapConstantPool.h"


#endif // __FASTIVA_JNI_CONTEXT_H__

