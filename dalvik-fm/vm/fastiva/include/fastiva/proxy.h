#ifndef __FASTIVA_PROXY_H__
#define __FASTIVA_PROXY_H__

// fastiva/ClassContext.h 를 include하지 않는다.
#define __FASTIVA_CLASS_CONTEXT_H__
#undef  EMBEDDED_RUNTIME
#define EMBEDDED_RUNTIME

#define FASTIVA_GENERATE_PROXY_LIB

#include <Fastiva.h>
#include <fox/Mutex.h>
#include <java/lang/String.ptr>
#include <java/lang/Object.ptr>
#include <fastiva/Runtime.h>
//#include <fastiva/Runtime.inl>


#pragma	warning(disable : 4716) // nonstandard extension used: 'Foo::funcion' uses SEH and 'cleaner$$' has destructor

#if 0
#pragma pack(1)
struct fastiva_ClassContext {
	ushort m_id;
	ushort m_accessFlags;
	//const fastiva_Method* m_pStatic0;
	const char* m_pPackageName;
	const char* m_pBaseName;
	//const char* m_pPackageName;
	jint m_packageHashCode;
	jint m_classHashCode;
	jint m_crc32;
/*
	char  c$$0;
	int   c$$1;
	char  c$$2;
	char  c$$3;
	char  c$$4[1];
	jbool isProxy() const {
		return (uint)m_accessFlags >= (uint)-2;
	}
	
	jbool isImported() const {
 		KASSERT(isProxy());
		return m_accessFlags == -2;
	}
*/	
	// fastiva_ClassMoniker* m_pNext;
	// search-class 횟수를 줄이기 위해 superMoniker를 사용.
};
#pragma pack()

struct fastiva_InterfaceContextInfo {
	const fastiva_ClassContext context;
};


struct fastiva_InstanceContextInfo {
	const fastiva_ClassContext context;
};



#define FASTIVA_GENERATE_PROXY_LIB






struct fastiva_ProxyInfo { 
	const char* m_tSharedProc;
	const char* m_pPackage;
	const char* m_pClassName;
};


#define FASTIVA_DECL_INTERFACE_MONIKER(IFC, SLOT, INSTANCE)					\
//	static const int IFC##__##INSTANCE##_IVT$ = (int)INSTANCE::SLOT##_IVT$;

extern "C" {
	// fm::importMoniker$는 paramenter를 가질 수 없다.
	// 그 과정에서 호출된 함수의 원 parameter register값이 변경되기 때문이다.
	java_lang_Class_p fm::importMoniker$();
};

/*
#include <fastiva/Unknown.h>

struct fastiva_Runtime {
	static void* __cdecl importContext(char* c$$);
};

struct fm::Proxy_T$ : public fastiva_Instance_T$ {
	private: const fastiva_ClassContext* const m_pContext0;
	private: java_lang_Class_p m_pNextComposite;
	protected: java_lang_Class_p m_pStatic;
};

#define FASTIVA_IMPL_PROXY(CLASS, PACKAGE_HASH, CLASS_HASH, CRC32)		\
	struct CLASS##_T$ {													\
		static const fastiva_ClassMoniker m_context$;					\
	};																	\
	const fastiva_ClassMoniker CLASS##_T$::m_context$ = {				\
		-1,																\
		0,																\
		PACKAGE_PATH,													\
		PACKAGE_HASH,													\
		CLASS_HASH,														\
		CRC32,															\
		(char)0xB8, (int)fastiva_Runtime::importContext,				\
		(char)0xFF, (char)0xD0,											\
	};

*/
#if 0
#define FASTIVA_IMPL_INSTANCE_PROXY(CLASS, PACKAGE_HASH, CLASS_HASH, CRC32)		\
	const fastiva_InstanceContextInfo CLASS##_G$::g_context$ = {{					\
		-1,																\
		0,																\
		PACKAGE_PATH,													\
		PACKAGE_HASH,													\
		CLASS_HASH,														\
		CRC32,															\
}};


#define FASTIVA_IMPL_INTERFACE_PROXY(CLASS, PACKAGE_HASH, CLASS_HASH, CRC32)		\
	const fastiva_InterfaceContextInfo CLASS##_G$::g_context$ = { {					\
		-1,																\
		0,																\
		PACKAGE_PATH,													\
		PACKAGE_HASH,													\
		CLASS_HASH,														\
		CRC32,															\
} };
#else
#define FASTIVA_IMPL_INSTANCE_PROXY(CLASS, PACKAGE_NAME, CLASS_NAME, CRC32)		\
	const fastiva_InstanceContextInfo CLASS##_G$::g_context$ = {{					\
		0xC3C3, -1,														\
		PACKAGE_PATH,													\
		CLASS_NAME,														\
		CRC32,															\
}};


#define FASTIVA_IMPL_INTERFACE_PROXY(CLASS, PACKAGE_NAME, CLASS_NAME, CRC32)		\
	const fastiva_InterfaceContextInfo CLASS##_G$::g_context$ = { {					\
		0xC3C3, -1,														\
		PACKAGE_PATH,													\
		CLASS_NAME,														\
		CRC32,															\
} };
#endif

//#pragma warning(disable : C2561) // unreferenced local variable
// 주의!!! importClass$()가 가장 끝에, 즉 모든 Thunk 의 앞에 위치하여야 한다.
#ifdef _ARM_
#define FASTIVA_IMPL_IMPORT_MONIKER(CLASS)									\
	CLASS##_C$* CLASS##_C$::getRawStatic$() {									\
		return CLASS_C$::importClass$();										\
	}																		\
	const fastiva_ClassContext* CLASS::getRawContext$() {					\
		return CLASS_C$::importClass$()->m_pContext$;\
	}																		\
	FASTIVA_NAKED_API CLASS##_C$* CLASS_C$::importClass$() { 					\
		*(int*)fm::importMoniker$() = (int)&CLASS##_G$::g_context$;			\
    }

#else

#define FASTIVA_IMPL_IMPORT_MONIKER(CLASS)									\
	CLASS##_C$* CLASS##_C$::getRawStatic$() {									\
		return importClass$();												\
	}																		\
	const fastiva_ClassContext* CLASS::getRawContext$() {					\
		return CLASS##_C$::importClass$()->m_pContext$;					\
	}																		\
	FASTIVA_NAKED_API CLASS##_C$* CLASS_C$::importClass$() { 				\
		*(char*)&CLASS##_G$::g_context$ = 0;										\
		*(int*)fm::importMoniker$();									\
		_ASM_RET()															\
    }																		\

#endif

//#define FASTIVA_DECL_ALLOCATE_PROXY(CLASS)									\
//	FASTIVA_IMPL_ALLOCATE_THUNK(CLASS)


extern "C" {
	void fm::invokeStatic_$$(fastiva_ClassMoniker* pMoniker);
	void fm::invokeNonVirtual_$$(fastiva_ClassMoniker* pMoniker);
	void fm::invokeSpecial_$$(fastiva_ClassMoniker* pMoniker);
};

#endif

#endif // __FASTIVA_PROXY__H__
