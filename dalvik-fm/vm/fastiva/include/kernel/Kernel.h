#ifndef __KERNEL_KERNEL_H__
#define __KERNEL_KERNEL_H__


#include <Fastiva.h>
#include <fastiva/Util.h>
#include <fastiva/Heap.h>
#include <fastiva/ClassContext.h>
#include <kernel/Module.h>
#include <java/lang/Object.inl>

#include <java/lang/String.inl>
#include <java/lang/Thread.inl>
#include <java/util/Hashtable.inl>
#include <kernel/nativeRuntime.h>

#ifndef FASTIVA_CLDC
	#include <java/lang/ThreadGroup.inl>
#endif
#include <fastiva/Exception.h>
#include <kernel/JniContext.h>
#include <kernel/JniConstant.h>

#if NO_DALVIK
#include <fox/Task.h>
#include <pal/fox_file.h>
#include <fox/Mutex.h>
#endif

#if FASTIVA_SUPPORTS_JAVASCRIPT
#include <kernel/jscript.h>
#endif

#ifdef _DEBUG
	#define KERNEL_DEBUG = 1;
#endif

#ifdef KERNEL_DEBUG 
	#define ASSERT_K(t)		if (!(t)) { FASTIVA_DBREAK(); }
#else
	#define ASSERT_K(t)		// bypass
#endif

#define FASTIVA_THREAD_PRIORITY(priority)		(priority*(FOX_TASK_MAX_PRIORITY/10)+1)

#define g_pLibcoreModule ((fastiva_Module*)&fastiva_libcore_ModuleInfo)

extern "C" {
	void fastiva_divide_by_zero_handler();
}

struct KernelTrap {
	KernelTrap();
	~KernelTrap();
};

struct AtExitProc {
	void(*func) (void);
	AtExitProc* next;
};

#define ENABLE_KERNEL_TRAP()  KernelTrap __kernel__trap__;
#define ASSERT_K0(t)	if (KERNEL_DEBUG && !(t))	_FASTIVA_DBREAK();
#define ASSERT_K1(t)	if (KERNEL_DEBUG > 1 && !(t))	_FASTIVA_DBREAK();
#define ASSERT_K2(t)	if (KERNEL_DEBUG > 2 && !(t))	_FASTIVA_DBREAK();

static const int GC_MAX_MONITOR_LOCK_COUNT	= 127; // 최대 255번까지
static const int GC_MONITOR_LOCK_MASK		= 0xFF;
static const int GC_MONITOR_LOCK_SHIFT		= 0;
static const int GC_MONITOR_LOCK_INCREE		= 1;

template <class BASE_T> 
class FASTIVA_STACK_OBJECT : public BASE_T {
};


struct fastiva_PrimitiveInfo {
	FASTIVA_DECL_PRIMITIVE_TYPE_INFOS();
};

struct fastiva_Text16 : FOX_UNICODE_STRING {
	Unicod_ap m_pUnicod;
	unicod* m_pUCS;

	fastiva_Text16(java_lang_String_p pStr, int offset, int length) {
		this->init(pStr->m_value, pStr->m_offset + offset, length);
	}

	fastiva_Text16(java_lang_String_p pStr) {
		this->init(pStr->m_value, pStr->m_offset, pStr->m_count);
	}

	fastiva_Text16(Unicod_ap buff, int offset, int length) {
		this->init(buff, offset, length);
	}

	~fastiva_Text16() {
		if (m_pUCS != ADDR_ZERO) {
			delete m_pUCS;
		}
	}

	void init(Unicod_ap buff, int offset, int length) {
		this->m_pUnicod = buff;
		unicod* pUCS = (unicod*)Unicod_A::Buffer(buff, offset,length);
		this->m_length = length;
		if (pUCS[length] != 0) {
			m_pUCS = new unicod[length + 1];
			memcpy(m_pUCS, pUCS, length * 2);
			pUCS = m_pUCS;
			pUCS[length] = 0;
		}
		else {
			m_pUCS = ADDR_ZERO;
		}
		this->m_pText = pUCS;
	}

};

#define FASTIVA_PRIMITIVE_CLASS_OF(T)							\
	(fastiva.primitiveClasses[fastiva_Primitives::T])

#define FASTIVA_PRIMITIVE_CONTEXT_OF(T)									\
	(fastiva.primitiveContexts[fastiva_Primitives::T])

//#define FASTIVA_PRIMITIVE_MONIKER_OF(pContext)							\
//	(const fastiva_ClassMoniker*)										\
//		(pContext - Kernel::g_aPrimitiveContext)

#define FASTIVA_boolean	jbool	
#define FASTIVA_byte	jbyte	
#define FASTIVA_char	unicod	
#define FASTIVA_short	jshort	
#define FASTIVA_int		jint		
#define FASTIVA_float	jfloat	
#define FASTIVA_long	jlonglong	
#define FASTIVA_double	jdouble	

#define FASTIVA_SIGNATURE_LOADER	((java_lang_ClassLoader_p)-1)
//typedef struct fm::Kernel KERNEL;
typedef void*(FOX_FASTCALL(*FASTIVA_DEFAULT_CONSTRUCTOR_FN_TYPE))(java_lang_Object_p);
//struct fastiva_PackageInfo;
extern "C" void FOX_NO_RETURN FOX_FASTCALL(fastiva_throwIllegalThreadStateException)();
extern "C" void FOX_NO_RETURN FOX_FASTCALL(fastiva_throwUnsatisfiedLinkError)();
extern "C" void FOX_NO_RETURN FOX_FASTCALL(fastiva_throwThreadDeath)();



#define MAX_INIT_INTERN	256
#define MAX_RAW_CLASS_CONTEXT


struct fox_HeapSlot;


struct fox_StackSlot { 
	void* m_sp; 
	java_lang_Object_p m_pInstance;
};

struct TextBuff : public Unicod_A::Buffer {
private:
	typedef Unicod_A::Buffer SUPER_T$;

	int m_pos;

public:
	TextBuff(int max_size) : SUPER_T$(Unicod_A::create$(max_size)) {}

	void checkBuffer(int size) {
		ASSERT_K(m_pArray->length() - (m_pItem - getBufferPtr()) >= size);
	}

	void append(const char* str, jint len);

	void append(const unicod* str, jint len);

	void append(java_lang_String_p s);

	java_lang_String_p toString();

};

extern "C" void FOX_NO_RETURN FOX_FASTCALL(fastiva_throwInternalError)(const char* pMessage);
struct fox_Finalizer;
struct StackFrame;

typedef fastiva_ModuleInfo* (__cdecl *FASTIVA_INIT_LIB_PROC)(fastiva_Runtime* pRuntime);


/*
FOX_RESTRICT_API fastiva_ArrayHeader* FOX_FASTCALL(allocateCustomArray)(
	uint primitiveTypeID,//const fastiva_ClassMoniker*,
	int  length,
	uint headerSize,
	uint itemSize
);
*/


struct Kernel { //: public Fastiva {


public:
	Kernel() {}

#if PROTOTYPE_GC
	class ArrayClass : public java_lang_Class {
#if FASTIVA_SUPPORTS_JAVASCRIPT
	public:
		//virtual java_lang_Object_p js_getPrototype$(java_lang_Object_p pObj);

		FASTIVA_VIRTUAL JsValueEx js_getProperty$(java_lang_Object_p pObj, PropertyID attrID);
		//FASTIVA_VIRTUAL void js_setProperty$(java_lang_Object_p pObj, JsBinOperation* op_ref);
		FASTIVA_VIRTUAL bool js_hasProperty$(java_lang_Object_p pObj, PropertyID attrID);
		//FASTIVA_VIRTUAL bool js_deleteProperty$(java_lang_Object_p pObj, PropertyID attrID);

		FASTIVA_VIRTUAL JsValueEx js_getArrayItem$(java_lang_Object_p pObj, uint index) ;
		FASTIVA_VIRTUAL ASSIGN_RESULT js_setArrayItem$(java_lang_Object_p pObj, JsBinOperation* op_ndx);
		FASTIVA_VIRTUAL bool js_hasArrayItem$(java_lang_Object_p pObj, uint index);
		FASTIVA_VIRTUAL bool js_deleteArrayItem$(java_lang_Object_p pObj, uint index);
#endif
		FASTIVA_VIRTUAL void markReachableTree$(fastiva_Instance_p pValue);
		FASTIVA_VIRTUAL void derefLocalTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedBabyTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedReachableBabyTree$(fastiva_Instance_p pObj);
	};

	class PrimitiveArrayClass : public ArrayClass {
		FASTIVA_VIRTUAL void markReachableTree$(fastiva_Instance_p pValue);
		FASTIVA_VIRTUAL void derefLocalTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedBabyTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedReachableBabyTree$(fastiva_Instance_p pObj);
	};

	class PropertyArrayClass : public ArrayClass {
		FASTIVA_VIRTUAL void markReachableTree$(fastiva_Instance_p pValue);
		FASTIVA_VIRTUAL void derefLocalTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedBabyTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedReachableBabyTree$(fastiva_Instance_p pObj);
	};


#if FASTIVA_SUPPORTS_JAVASCRIPT
	class MetaClass : public java_lang_Class {
		FASTIVA_VIRTUAL JsValueEx js_getProperty$(java_lang_Object_p pObj, PropertyID attrID);
		FASTIVA_VIRTUAL ASSIGN_RESULT js_setProperty$(java_lang_Object_p pObj, JsBinOperation* op_ref);
		FASTIVA_VIRTUAL bool js_hasProperty$(java_lang_Object_p pObj, PropertyID attrID);
		//FASTIVA_VIRTUAL bool js_deleteProperty$(java_lang_Object_p pObj, PropertyID attrID);
	};

	class StringClass : public java_lang_Class {
	public:
		StringClass();
		FASTIVA_VIRTUAL JsValueEx js_getProperty$(java_lang_Object_p pObj, PropertyID attrID);
		//FASTIVA_VIRTUAL bool js_hasProperty$(java_lang_Object_p pObj, PropertyID attrID);
		//FASTIVA_VIRTUAL bool js_deleteProperty$(java_lang_Object_p pObj, PropertyID attrID);
	};


#if 0
	class PropertiesClass : public ArrayClass {
		FASTIVA_VIRTUAL void markReachableTree$(fastiva_Instance_p pValue);
		FASTIVA_VIRTUAL void derefLocalTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedBabyTree$(fastiva_Instance_p pObj);
		FASTIVA_VIRTUAL void markPublishedReachableBabyTree$(fastiva_Instance_p pObj);
	};
#endif
#endif
#endif

	//const void** arrayIVTables;
	java_lang_Object::VTABLE$ primitiveArrayVTable;
	java_lang_Object::VTABLE$ pointerArrayVTable;
	java_lang_Object::VTABLE$ proxyArrayVTable;

	struct ClassInfo {
		const fastiva_Package* m_pPackage;
		const char* m_pBaseName;
		//void* m_constPool;
		jint  m_hashCode;
		void* m_filePointerH;
		int m_superClassIndex;
	};

	//static const fastiva_ClassContext* getContextArray(const fastiva_PackageInfo* pPackage);



	//static java_lang_Class g_aPrimitiveArrayClass[fastiva_Primitives::cntType-1];
	// v3 static const fastiva_ArrayHeader::VTable$ aArrayContext$[4];
	//static java_lang_Class_T$ aArrayClass[FASTIVA_MAX_ARRAY_DIMENSION];

	static struct fastiva_Task* g_pSystemTask;
	//static struct fastiva_Task* g_pSystemProxy;

	//fox_main에서 생성한 mainThread
	//CDC의 java/lang/runtime.java에 있는 safeExit에서 사용된다.
	//static FOX_HTASK g_hMainThread;

	//static jbool g_inInitializing;
	static AtExitProc* g_pAtExitProcQ;

	static java_lang_Throwable_p g_pOutOfMemoryError;
	static java_lang_Throwable_p g_pNullPointerException;
	static java_lang_Throwable_p g_pIllegalArgumentException;
	static java_lang_Throwable_p g_pStringIndexOutOfBoundsException;
	static java_lang_Throwable_p g_pArrayIndexOutOfBoundsException;
	static java_lang_Throwable_p g_pIllegalMonitorStateException;
	static java_lang_Throwable_p g_pArithmeticException;
	static java_lang_Throwable_p g_pCloneNotSupportedException;
	static java_lang_Throwable_p g_pIOException;
	static java_lang_Throwable_p g_pIllegalAccessException;
	static java_lang_Throwable_p g_pArrayStoreException;
	static java_lang_Throwable_p g_pClassCastException;
	static java_lang_Throwable_p g_pInterruptedException;
	static java_lang_Throwable_p g_pClassNotFoundException;
	static java_lang_Throwable_p g_pNoClassDefFoundError;
	static java_lang_Throwable_p g_pVirtualMachineError;
#if JPP_JDK_VERSION < JPP_JDK_CDC
	#define g_pStackOverflowError			g_pVirtualMachineError
	#define g_pUnsatisfiedLinkError			g_pVirtualMachineError
	#define g_pThreadDeath				g_pVirtualMachineError
	#define g_pNoSuchFieldError				g_pVirtualMachineError
	#define g_pNoSuchFieldException			g_pVirtualMachineError
	#define g_pNoSuchMethodError			g_pVirtualMachineError
	#define g_pNoSuchMethodException		g_pVirtualMachineError
	#define g_pExceptionInInitializerError	g_pVirtualMachineError
	#define g_pAbstractMethodError			g_pVirtualMachineError
	#define g_pInstantiationException		g_pVirtualMachineError
	#define g_pIllegalThreadStateException	g_pIllegalAccessException
#else
	static java_lang_Throwable_p g_pIllegalThreadStateException;
	static java_lang_Throwable_p g_pInstantiationException;
	static java_lang_Throwable_p g_pStackOverflowError;
	static java_lang_Throwable_p g_pUnsatisfiedLinkError;
	static java_lang_Throwable_p g_pThreadDeath;
	static java_lang_Throwable_p g_pNoSuchFieldError;
	static java_lang_Throwable_p g_pNoSuchFieldException;
	static java_lang_Throwable_p g_pNoSuchMethodError;
	static java_lang_Throwable_p g_pNoSuchMethodException;
	static java_lang_Throwable_p g_pExceptionInInitializerError;
	static java_lang_Throwable_p g_pAbstractMethodError;
#endif

	static java_lang_Object_p	g_pSysEvent; 
	static java_lang_Object_p g_pGCLock;
	enum { MAX_PACKAGE_COUNT = 512 };

	//static java_lang_Thread_p	g_pGCThread;
	//static jbool g_isSystemRunning;
	fox_Mutex g_pLock[1];
	fox_Mutex g_pRawContextLock[1];
	java_util_Hashtable_p g_pInternedString;

	fastiva_Instance g_pPackageListLock[1];

#if (JPP_JNI_EXPORT_LEVEL > 0)
	fox_Mutex g_pContextIDLock[1];
	JNI_RawContext::ContextList* g_pContextList;
	int g_cntContextID;

	JNI_UtfHashTable  g_utfHashTable;
	JNI_RawContext::Buffer* g_pContextBuffer;
	JNI_RawContext* g_pFreeRawContext;

#endif

	fastiva_Module g_interpreterModule;

	int g_cntPackage;
	fastiva_PackageSlot* g_aPackageSlot;

	JNI_UtfStringPool stringPool;

	struct fastiva_Task* g_pTaskQ_interlocked; 
	java_lang_Thread_p g_pMainThread; 
	//java_lang_ThreadGroup_p g_pMainThread; 
	java_lang_Class_p g_pClassQ;
	java_lang_Object_p g_pStaticInstance;
	uint next_screen_refresh_t;
	jint g_userThreadCount_interlocked;
	jint g_activeThreadCount_interlocked;
	jint g_screenRefreshLocked;
	java_lang_ClassLoader_p g_pBootstrapClassLoader;
	java_lang_String_p* g_aInitInternBuffer;
	jint g_cntinitIntern;
	jint g_cntContext;
	fastiva_ResourceLoader* g_pResourceLoaderQ;
	
	fox_Semaphore* g_ivtableLock;
	int* g_ivtableHeap;
	int* g_ivtableAllocTop;

#if FASTIVA_SUPPORTS_JAVASCRIPT
	JsSlotMap* g_pEmptySlotMap;
#endif
	int g_pArrayClass_GenericVT;

	struct {
		//fox_Semaphore pLock[1];
		int valueType;
		void* pValue;
		void* opThread;
		void* baseObject;
		struct JsBinOperation* opAssign;
	} jsBinAssign;

	//static const fastiva_PrimitiveInfo g_aPrimitiveContext[fastiva_Primitives::cntType];
	//static jbyte g_aPrimitiveClass[fastiva_Primitives::cntType][sizeof(fastiva_Class_T$)];
	//static jbyte g_aPrimitiveArrayClass[fastiva_Primitives::cntType-1][sizeof(fastiva_Class_T$)];

};

namespace fm {
	struct Command {
		const SYS_TCHAR* szClassPath;
		java_lang_String_ap aArg;
		java_lang_Class* pMainClass;
		java_lang_Thread_p pMainThread;
		void* pfnMain;
	};


	extern Command g_command;

	java_lang_String_p util_getSystemDir();

	//void FOX_FASTCALL(disableGC)(fastiva_Instance_p pObj);

	//void FOX_FASTCALL(enableGC)(fastiva_Instance_p pObj);

	void FOX_FASTCALL(gc_markPublicReachable)(fastiva_Instance_p pObj);
	
	void FOX_FASTCALL(gc_markFinalizerReachable)(fastiva_Instance_p pObj);

	void FOX_FASTCALL(gc_markStrongReachable)(fastiva_Instance_p pObj);

	void FOX_FASTCALL(gc_markLocalReachable)(fastiva_Instance_p pObj);

	void FOX_FASTCALL(gc_markPublished)(fastiva_Instance_p pObj);

	void FOX_FASTCALL(gc_derefLocalRef)(fastiva_Instance_p pObj);

	void printError(java_lang_Throwable_p);



	/************************************************************************/
	/* Array allocation  도중에는 initClass가 수행되지 않느다.
	따라서 pClass에 사용될 수 있는 class는 이미 import된 class 이거나
	(Fastiva 내에서는 import되기 전에는 class-poineter를 사용할 수 없다. 
	RawClass의 concept이 존재하지 않기 때문이다)
	ArrayClass 또는 Primitive-class(initClass가 필요없으므로)만 사용가능하다.*/
	/************************************************************************/
	void initArrayVTable();

	void scanPointerArray(java_lang_Object_ap pArray, FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner);

	fastiva_ArrayHeader* cloneArray(fastiva_ArrayHeader* pArray);

#if FASTIVA_SUPPORT_JNI_STUB
	void scanJavaProxyArray(Int_ap pArray, void* pEnv0, FASTIVA_JPROXY_SCAN_METHOD method);

	java_lang_Object_p cloneJavaProxyArray(Int_ap pArray);
#endif

	fastiva_ArrayHeader* FOX_FASTCALL(allocateArray)(
		java_lang_Class_p pClass,
		int dimension
		);

	void setArrayDimension(java_lang_Class_p pClass, int dimension);

	int getArrayDimension(java_lang_Class_p pClass);

	int getArrayDimension_asObjectArray(java_lang_Class_p pClass);


	//========================================================================//
	// String
	//========================================================================//

	Unicod_ap byteToCharArray(const ubyte* pbyte, int len);

	Byte_ap charToByteArray(const unicod* pcode, int len);

	void writeUnicod(java_io_OutputStream_p out, int ch1);

	int readUnicod(java_io_InputStream_p in);

	int getMBCSLength(const unicod* str, int len);

	int getUnicodeLength(const ubyte* str, int len);

	java_lang_String_p createStringConstant(const unicod* str);
	
	inline unicod* getUnsafeStringBuffer(java_lang_String_p pStr) {
		int offset = 0;
	#ifndef FASTIVA_CLDC
		offset = pStr->m_offset;
	#endif
		return (unicod*)pStr->m_value->getBuffer_unsafe$() + offset;
	}

	java_lang_String_p internString(java_lang_String_p pRookie);

	// return null if not interned
	java_lang_String_p getInternedString(java_lang_String_p pRookie);

	java_lang_String_p createStringConstant(fastiva_ConstantString* pCS);

	//========================================================================//
	// Util
	//========================================================================//

	inline java_lang_Object_p* getUnsafeBuffer(fastiva_ArrayHeader* pArrayHeader) {
		return (java_lang_Object_p*)pArrayHeader->getBuffer_unsafe$();
	}

	java_lang_String_p FOX_FASTCALL(toString)(jdouble double0, jbool forJscriptFormat);

	double FOX_FASTCALL(stringToNumber)(java_lang_String_p pString0);

	//java_lang_String_p createStringConstant(fastiva_ConstantString* pCS);

	//========================================================================//
	// Class
	//========================================================================//

	const void** createIVTables(const fastiva_ImplementInfo* aImplemented);

	java_lang_Class_p loadClass(unicod* pName, int ucs_len, java_lang_ClassLoader_p pLoader);
	
	int getClassNameUTF8(char* name, int length, const fastiva_ClassContext* pContext);

	const fastiva_ClassContext* getPrimitiveContext(uint primitiveTypeID);

	const fastiva_ClassContext* getPrimitiveContextBySig(unicod	 primitiveTypeID);

	const java_lang_Class_p getPrimitiveClass(uint primitiveTypeID);

	java_lang_Class_p getPrimitiveClassBySig(unicod signature);

	java_lang_Class_p makeCompositeType(java_lang_Class_p itemType);

	int getPrimitiveArrayItemSize(uint primitiveTypeID);

	int getPrimitiveArrayItemSize(const fastiva_ClassContext* pContext);

	int getPrimitiveTypeID(fastiva_Class_p pClass);

	int getPrimitiveReflectionType(fastiva_Class_p pClass);

	inline const fastiva_InstanceContext* getInstanceContext(const java_lang_Class* pClass) {
		return ((fastiva_Class_p)pClass)->m_pContext$->toInstanceContext();
	}

	inline const fastiva_InstanceContext* getInstanceContext(const fastiva_Instance_p pObj) {
		return getInstanceContext(pObj->getClass$());
	}

	//========================================================================//
	// Class-Loader
	//========================================================================//

	fastiva_ResourceLoader* parseResourceLoadingPath(const SYS_TCHAR* classPaths);

	Byte_ap loadResource(java_lang_String_p rscName, java_lang_ClassLoader_p pClassLoader);

	__int64 initInterfaceTrack(const fastiva_ClassContext* pIfcCtx);



	void FOX_FASTCALL(registerPackage)(
		fastiva_Package* pPackage, 
		uint hashCode,
		int insertIdx = -1
	);

	fastiva_Package* FOX_FASTCALL(loadLibrary)(const char* packageName, int nameLen);
	void FOX_FASTCALL(notifyFatalError)();


	jbool isLoaded(java_lang_Class_p pClass);

	void initClassLoader();

	void initPrimitiveClasses();

	void FOX_FASTCALL(linkContext)(const fastiva_ClassContext* pContext);

	#if FASTIVA_SUPPORTS_BYTECODE_INTERPRETER
	const fastiva_ClassContext* FOX_FASTCALL(loadJavaClassFile)(JNI_RawContext* pRawContext);
	#endif

	const fastiva_ClassContext* FOX_FASTCALL(validateContext)(const fastiva_ClassContext* pContext);

	fastiva_Class_p initRawClass(const fastiva_ClassContext*  pContext);

	fastiva_Class_p FOX_FASTCALL(linkClass)(java_lang_Class* pClass);

	java_lang_Class_p importClass(const fastiva_ClassContext* pContext);

	void FOX_FASTCALL(checkInternalClass)(void* packageInfo, const fastiva_ClassContext* pContext);

	java_lang_Class_p importClass(const fastiva_ClassContext* pContext);


	void FOX_FASTCALL(registerComponent)(
		fastiva_Package*, 
		int imageOffset,
		const fastiva_ClassContext* aContext,
		int cntContext
	);


	//========================================================================//
	// ??? GC
	//========================================================================//

	void FOX_FASTCALL(pushStackFrame)(StackFrame* pRookie);

	void FOX_FASTCALL(popStackFrame)(StackFrame* pRetiree);

	void markStackTouched(fastiva_Instance_p pObj);

	void FOX_FASTCALL(gc_scanLocalStack)(fastiva_Task* pCurrStack);

	//========================================================================//
	// Initialization
	//========================================================================//

	void startApp(const char* pszClassName);

	void initSystemException();

	#if defined(FASTIVA_J2SE) || defined(FASTIVA_CDC)
	void FOX_FASTCALL(initSystemThread)(
		java_lang_ThreadGroup_p pSystemGroup,
		java_lang_Thread_p pSystemThread
	);
	#endif

	void initMainThread(
	);



	void* getVM_ENV();

	jbool FOX_FASTCALL(initKernel)(fastiva_ModuleInfo* pAppModlue);//

	void FOX_FASTCALL(startApplication)(
		const char* mainClassName, 
		java_lang_String_ap args
	);

	java_lang_Object_p FOX_FASTCALL(newInstance)(
		java_lang_Class_p pClass,
		const fastiva_ClassContext* pCallerContext
	);

	void initThunk(fastiva_Runtime* pFastivaRuntime);


	jbool FOX_FASTCALL(kernel_initFastiva)(void* oemData);


	//========================================================================//
	// GC
	//========================================================================//


	void FOX_FASTCALL(getScanMethod_OBSOLETE)(int t); // obsolete

	jbool isPublished(fastiva_Instance_p pObj);

	void FOX_FASTCALL(registerHeapInstance)(
		const fastiva_InstanceContext* pContext,
		fastiva_Instance_p pObj
	);

	fastiva_ArrayHeader* FOX_FASTCALL(allocArrayInstance)(
		uint siz, 
		java_lang_Class_p pClass,
		int cntItem
	);

	java_lang_Object_p FOX_FASTCALL(clone)(
		fastiva_Instance_p pObj
	);

	fastiva_ArrayHeader* allocateMultiArray(
		java_lang_Class_p pClass, 
		const int* aLength
	);

	class LateStackMarker : public fastiva_Rewinder {
		java_lang_Object_p m_pObj;
		LateStackMarker(java_lang_Object_p pStackObj) {
			this->m_pObj = pStackObj;
		}

		~LateStackMarker() {
			markStackTouched(m_pObj);
		}
	};



	//========================================================================//
	// JNI
	//========================================================================//

	void FOX_JNI_CALL callNativeMethod(void* ENV, java_lang_Object_p obj);


	java_lang_Object_p createGlobalRef(java_lang_Object_p pObj);

	java_lang_Object_p deleteGlobalRef(java_lang_Object_p pObj);

	java_lang_Object_p createWeakGlobalRef(java_lang_Object_p pObj);

	java_lang_Object_p deleteWeakGlobalRef(java_lang_Object_p pObj);

	java_lang_Object_p createLocalRef(java_lang_Object_p pObj);

	java_lang_Object_p deleteLocalRef(java_lang_Object_p pObj);

	jbool isSameObject(fastiva_Instance_p pObj1, fastiva_Instance_p pObj2);

	jint EnsureLocalCapacity(jint capacity);

	jint PushLocalFrame(jint capacity);

	java_lang_Object_p popLocalFrame(java_lang_Object_p pObj);

	jint FOX_FASTCALL(registerNatives)(java_lang_Class_p pClass, 
		const JNINativeMethod_ex *methods,
		jint nMethods);


	void FOX_FASTCALL(registerNatives)(java_lang_Class_p pClass);

	jint FOX_FASTCALL(unregisterNatives)(java_lang_Class_p pClass);

	void* FOX_FASTCALL(getNativeMethodAddr)(
		const fastiva_InstanceContext* pContext,
		const fastiva_MethodInfo* pInfo
	);

	void* FOX_FASTCALL(findNativeMethod_MD)(const char* method_name);

	//========================================================================//
	// Thread
	//========================================================================//

	void FOX_FASTCALL(thread_crt0)(void* param);

	void FOX_FASTCALL(attachThread)(java_lang_Thread_p pThread, FOX_HTASK hTask);

	void FOX_FASTCALL(attachNativeTask)(fastiva_Task* hTask);

	void detachNativeTask(fastiva_Task* hTask);

	void FOX_FASTCALL(wakeSystemThread)(int runningPriority);


	//========================================================================//
	// JScript
	//========================================================================//

	//static volatile uint g_nextGC_T;
	void initJScript();

	void initReflectionTable();

	//========================================================================//
	// Reflection
	//========================================================================//

	ushort getUtfID(java_lang_String_p pName);

	#if FASTIVA_SUPPORTS_JAVASCRIPT

	JsReflectionTable* FOX_FASTCALL(getReflectionTable)(java_lang_Class_p pClass);

	JsValueEx getInfiniteArrayValue(com_wise_jscript_JsInfiniteArray_p array, uint index);

	ASSIGN_RESULT setInfiniteArrayValue(com_wise_jscript_JsInfiniteArray_p array, JsBinOperation* op_ndx);

	void removeJsArrayItems(com_wise_jscript_JsInfiniteArray_p array, uint indexFrom);

	java_lang_Object_p convertJsValue(java_lang_Class_p type, JsVariant value);

	void FOX_FASTCALL(assignJsValue)(fastiva_Instance_p pObj, void* slot, JsBinOperation* op, int type);

	ASSIGN_RESULT assignJsValue(fastiva_Instance_p pObj, int type, void* slot, JsGeneric_p pValue);


	inline JsSlotMap* getEmptySlotMap() {
		return kernelData.g_pEmptySlotMap;
	}

	#endif

	void initKNI();

	// for JNI
	inline void __registerJNIFieldClass(const fastiva_ClassContext* pContext) {
		// v3 pContext->m_pClass->m_jniLock |= JNI_FIELD_USED;
		FASTIVA_DBREAK();
	}

	inline void __registerJNIMethodClass(const fastiva_ClassContext* pContext) {
		// v3 pContext->m_pClass->m_jniLock |= JNI_METHOD_USED;
		FASTIVA_DBREAK();
	}


	// =============================
	//  MISC Utils.
	// ==============================

	java_lang_String_p getErrorString(int no);

	void throwIOException(int eno);

	#ifdef _DEBUG
	void dump_curentMethod();

	void dumpStackTrace(const fastiva_Method* pMethod);
	#endif

	struct SafeInstanceQ;
	struct PublicInstanceQ;

	void FOX_FASTCALL(insert)(SafeInstanceQ*, fastiva_Instance_p pRookie);

	void FOX_FASTCALL(insert)(SafeInstanceQ*, fastiva_Instance_p pRoot, fastiva_Instance_p pEnd);

	void dumpStackTrace(const fastiva_Method* pMethod);

};


#define FASTIVA_SYSTEM_MODULE	0x8000000



extern "C" {
	void FOX_FASTCALL(fastiva_debug_textout)(const char* msg);
	void FOX_FASTCALL(debug_textout)(const char* format, ...);
	void fastiva_throwStackOverflowError();
	void fastiva_checkStackFast();
	void fastiva_throwExceptionInInitializerError(java_lang_Throwable_p pReason);
	void fastiva_throwNoClassDefFoundError();
};

struct fastiva_StackFrame {
	friend struct Kernel;
	fastiva_StackFrame* m_pPrevFrame;
	ubyte*   m_pCurrentIP;
	int*	 m_pBaseSP;
	int*	 m_pEndOfSP;
	const fastiva_Method* m_pMethod; /* Pointer to the method currently under execution */
	fastiva_StackFrame();
	~fastiva_StackFrame();
};

extern Kernel kernelData;

#endif // __KERNEL_KERNEL_H__


/**====================== end ========================**/