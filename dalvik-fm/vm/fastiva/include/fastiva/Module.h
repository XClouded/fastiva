#ifndef __FASTIVA_MODULE__H__
#define __FASTIVA_MODULE__H__

#include <fastiva/Types.h>
#include <fastiva/Package.h>

JPP_TYPEDEF_PRELOADED_POINTER(java_lang_annotation_Annotation);

#if (JPP_JNI_EXPORT_LEVEL > 0)
	#include <fastiva/jni_def.h>
#endif


struct fastiva_InitArgs {
	java_lang_ClassLoader_p m_pLoader;
	//const char* fileName;
	fastiva_RuntimeProxy* m_pProxy;
};


extern unicod const* const FASTIVA_MODULE_NAME(constStrings$)[];
//extern int FASTIVA_MODULE_NAME(constStringCount$));

#define JPP_BEGIN_MODULE_INFO()												\
	static const fastiva_PackageSlot s_packageSlots$[] = {

#define JPP_END_MODULE_INFO()												\
	};																			

#define JPP_INIT_PACKAGE_SLOT(PACKAGE)										\
	{ PACKAGE##$package::hashCode$, JPP_PACKAGE_INFO_PTR(PACKAGE) },			


#define JPP_INIT_LIBCORE_CLASS_ID_LIST(cnt)								\
	static const JNI_HashEntry s_all_classes$[] = {	\

#define JPP_INIT_EXTERNAL_CLASS_ID_LIST(cnt)							\
	static const JNI_HashEntry s_all_classes$[] = {	\

#define JPP_INIT_CLASS_ID_SLOT(CLASS)									\
	{ CLASS##_G$::classHash$, { FASTIVA_RAW_CLASS_PTR(CLASS) } },

#define JPP_INIT_ARRAY_CLASS_ID_SLOT(CLASS, dimension, suffix)			\
	{ CLASS##_G$::classHash$, { FASTIVA_RAW_CLASS_PTR(CLASS) } },

#define JPP_END_INIT_CLASS_ID_LIST()									\
	};


//@zee 2011.0602 사용치 않음.
//#define JPP_INIT_APP_PACKAGE_ID_LIST(cnt, startHash, lastHash)			
//		const fastiva_PackageSlot g_aAppPackageSlot[] = 

//#define JPP_APP_PACKAGE_SLOT_COUNT()										
//		(sizeof(g_aAppPackageSlot) / sizeof(g_aAppPackageSlot[0]))


#define JPP_INIT_LIBCORE_REF_NAME_ID_LIST(cnt)								\
	static const JNI_HashEntry s_refNames$[] =		

#define JPP_INIT_EXTERNAL_REF_NAME_ID_LIST(cnt)								\
	static const JNI_HashEntry s_refNames$[] =		

#define JPP_INIT_REF_NAME_ID_SLOT(hashCode, len, name)						\
		{ hashCode, name },





#define JPP_JNI_ARG_VAL$(type)		&& OBSOLETE &&	FASTIVA_JNI_TYPE_ID_VAL$(type) 
#define JPP_JNI_ARG_PTR$(CLASS)		&& OBSOLETE &&	FASTIVA_JNI_TYPE_ID_PTR$(CLASS)
#define JPP_JNI_ARG_JOBJ$(CLASS)	&& OBSOLETE &&		FASTIVA_JNI_TYPE_ID_PTR$(CLASS)
#define JPP_JNI_ARG_ARRAY$(dim, CLASS)	&& OBSOLETE &&		FASTIVA_JNI_TYPE_ID_ARRAY$(dim, CLASS)

#ifdef _WIN32
#define FASTIVA_BEGIN_ANNOTATIONS_RAW_DATA(cnt)	\
	fastiva_AnnotationItem fastiva_AnnotationsData$[] = {
#else
#define FASTIVA_BEGIN_ANNOTATIONS_RAW_DATA(cnt)	\
	const fastiva_AnnotationItem fastiva_AnnotationsData$[] = {
#endif

#define FASTIVA_DECLARING_CLASS(CLASS) \
	fastiva_AnnotationItem(FASTIVA_ANNOTATION_DECLARING_CLASS, FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS)),

#define FASTIVA_ENCLOSING_CLASS(CLASS) \
	fastiva_AnnotationItem(FASTIVA_ANNOTATION_ENCLOSING_CLASS, FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS)),

#define FASTIVA_ENCLOSING_METHOD(CLASS, name, sig) \
	fastiva_AnnotationItem(FASTIVA_ANNOTATION_ENCLOSING_METHOD, FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS)), \
	fastiva_AnnotationItem(name, sig),

#define FASTIVA_ANNOTATION_DECLARING_CLASS 0x7FFFFFFF
#define FASTIVA_ANNOTATION_ENCLOSING_CLASS 0x7FFFFFFE
#define FASTIVA_ANNOTATION_ENCLOSING_METHOD 0x7FFFFFFD

#define FASTIVA_END_ANNOTATION()  // IGNORE

#define FASTIVA_END_ANNOTATIONS_RAW_DATA()	};

#define FASTIVA_BEGIN_ANNOTATIONS_INFO(cnt)	\
	int fastiva_AnnotationCache$[] = {

#define FASTIVA_ANNOTATIONS_INFO(offset, length)	(offset << 16) + (length << 2) + 1, 

#define FASTIVA_ANNOTATIONS_DATA_OFFSET(info)		(info >> 16)
#define FASTIVA_ANNOTATIONS_DATA_LENGTH(info)		((info & 0xFFFF) >> 2)

#define FASTIVA_GET_CACHED_ANNOTATIONS(info)		((java_lang_Object_p)((info & 1) == 0 ? info : 0))

#define FASTIVA_END_ANNOTATIONS_INFO()	0 };


#define FASTIVA_BEGIN_PARAMETER_ANNOTATIONS_INFO(cnt)	\
	int fastiva_ParameterAnnotationCache$[] = {

#define FASTIVA_PARAMETER_ANNOTATIONS_INFO(cnt, ...)	cnt, __VA_ARGS__, 

#define FASTIVA_END_PARAMETER_ANNOTATIONS_INFO()	0 };


#define JPP_INIT_LIBCORE_ARGLIST_ID_LIST(cnt)								\
	static const JNI_HashEntry s_argLists$[] =		

#define JPP_INIT_EXTERNAL_ARGLIST_ID_LIST(cnt)								\
	static const JNI_HashEntry s_argLists$[] =		

#define JPP_INIT_ARGLIST_ID_SLOT(hashCode, nameLen, index, name)			\
	{ (hashCode & ~1023) + (hashCode & 1023) * 2 + 2, (char*)g_aArgumentList_##index##$ },

#define JPP_BEGIN_INIT_ARGLIST_POOL(offset)					\
	namespace fastiva_ArgListPool$ {

#define JPP_END_INIT_ARGLIST_POOL()									\
	};

#define JPP_ARG_CNT_FLAG	0xFFFF0000

#define JPP_INIT_ARGLIST_ID(cnt, ARGS) \
	const char* FASTIVA_ARGLIST_NAME_##cnt ARGS[cnt+1] = {	\
		(const char*)(cnt | JPP_ARG_CNT_FLAG), \
		FASTIVA_ARGLIST_SIG_##cnt ARGS \
	};



#define FASTIVA_INIT_EMPTY_CONSTANT_STRING()								\
	unicod const* const FASTIVA_MODULE_NAME(constStrings$)[] = { 0 };		\
	java_lang_String_p FASTIVA_STRING_POOL[1] = {0};

#define FASTIVA_BEGIN_CONSTANT_STRING()										\
	unicod const* const FASTIVA_MODULE_NAME(constStrings$)[] = {

#define FASTIVA_END_CONSTANT_STRING()										\
	0 };																	\
	java_lang_String_p FASTIVA_STRING_POOL[FASTIVA_ARRAY_ITEM_COUNT(FASTIVA_MODULE_NAME(constStrings$))];





#if JPP_JNI_EXPORT_LEVEL <= 0
	static const int s_refNames$ = 0;
	static const int s_argLists$ = 0;
#endif




struct fastiva_ConstantUcs16String {
	short m_len; 
#ifdef __GNUC__
	unicod m_str[7600]; //CDC에서는 최대 7500개가 넘는게 존재한다.
#else
	unicod m_str[0];
#endif
};

struct fastiva_ConstantAsciiString {
	short m_len; 
#ifdef __GNUC__
	char m_str[1440];  //CDC에서 1440개가 넘는것이 존재한다.
#else
	char m_str[0];
#endif
};

struct fastiva_ConstantString {
	short m_len; 
	union {	unicod m_aUnicod[1];
			const char m_aByte[2];
	};
};


struct JNI_HashEntry {
	uint m_hashCode;
	union {
		const void* m_pData;
		fastiva_Class_p m_pClass;
	};

	static int makeHashCode(int hashCode, int len) {
		return (hashCode << 10) + len;
	}

	static int getLength(int hashCode) {
		return hashCode & 1023;
	}

	static int getHashCode(const char* key) {
		int hashCode = 0;
		const char* origin = key;
		for (char ch; (ch = *key) != 0; key ++) {
			hashCode = hashCode * 31 + ch;
		}
		int len = key - origin;
		if (len >= 1024) {
			FASTIVA_DBREAK();
			//fastiva_throwInternalError();
		}
		return makeHashCode(hashCode, len);
	}

	static int getHashCode(const char* key, int len) {
		if (len >= 1024) {
			FASTIVA_DBREAK();
			//fastiva_throwInternalError();
		}
		int hashCode = 0;
		for (int i = len; i -- > 0;) {
			hashCode = hashCode * 31 + (*key ++);
		}
		return makeHashCode(hashCode, len);
	}

};

struct JNI_HashMap {
	const JNI_HashEntry* m_aSlot;
	int m_cntSlot;

	struct KeyComparer {
		virtual int compare(const void* data) = 0;
		virtual ~KeyComparer() {};
	};

	void init(const JNI_HashEntry* aSlot, int cntSlot) {
		this->m_aSlot = aSlot;
		this->m_cntSlot = cntSlot;
	}

	const JNI_HashEntry* findEntry(uint hashCode, KeyComparer* finder);

	//static int searchToken(const JNI_HashEntry* aToken, int high, uint hashCode, Finder* finder);
	//static int searchSlot(const JNI_HashEntry* const aSlot, int high, uint hashCode, Finder* finder);
};



struct JNI_FindClass : public JNI_HashMap::KeyComparer {
	friend struct JNI_FindMethod;
public:
	typedef enum {
		FIND = 0,
		LOAD = 1,
		REGISTER = 2
	} Action;

	JNI_FindClass(java_lang_ClassLoader_p pLoader = ADDR_ZERO) {
		this->m_pClassLoader = pLoader;
	}

	const fastiva_ClassContext* loadContext_dot(
		const SYS_TCHAR* classSig // null-terminated pure class name without any prefixes, i.e ('[', 'L').
							 // pakckage delimiter is '.' (not '/');
	);

	const fastiva_ClassContext* findContext(
		const char* classSig,
		Action action = FIND
	);

	static java_lang_Class_p loadRawClass(uint classID);

	static const fastiva_ClassContext* getClassContext(
		int classID
	);
	//java_lang_Class_p findClass(const char* classSignature); // no throws 

	static ushort parseValueType(const char* typeSignature, java_lang_ClassLoader_p pLoader); // it throws internal error (invalid type-sig);

	static int getTypeSig(uint typeID, char* buff, int sizBuff);

	virtual int compare(const void* data);

private:
	const char* m_classSig;
	java_lang_ClassLoader_p m_pClassLoader;

	int parseArgType();
	const fastiva_ClassContext* findContext(Action action);
	fastiva_Package* findPackage(uint hashCode, uint length);
};

const char* d2f_getTypeDescriptor(u4 typeID);

class JNI_ArgIterator {
	uint* m_pArgType;
	uint  m_reserved;
    const void* dexProto; // must be 0;
    u2 parameterCount;
	u2 point;

	// returns ArgumentCount. if the value is 1, do not use iterator
public:
	int init(uint argListID);

	uint nextID() {
		if (point == parameterCount) {
			return -1;
		}
		point ++;
		return *m_pArgType ++;
	}

	java_lang_Class_p nextArgType();

	const fastiva_ClassContext* nextArgContext(int* array_dimension);

	const uint* currentArgTypePointer() {
		return m_pArgType;
	}
};



class fastiva_Module {
public:
	JNI_HashMap m_classes;
	java_lang_ClassLoader_p m_pClassLoader;
	int   m_cntMarked;
	const void* m_pLoadedBy;
	//char  m_szFileName[256];
	java_lang_Class_p m_pClassQ;
	unicod const* const* m_aConstString;
	java_lang_String_p* m_stringPool;
	const fastiva_Package* m_pProxyQ;
	const fastiva_ClassContext** m_ppImportedContext;
	int   m_imageOffset;
	const JNI_HashEntry* m_aRefName;
	int   m_cntRefName;
	const void* m_aArgList;
	const void* m_aArgListEnd;
	const fastiva_AnnotationItem* m_pAnnotationsData;
	int* m_pAnnotationsInfo;
	void (*m_fnInitClassIDs)(java_lang_Class_p* classes, const char** typeSigs);
	const void* m_hDLL;

	void init(
		const JNI_HashEntry* contextSlots, 
		int cntContext, 
		const JNI_HashEntry* pRefNames, 
		int cntRefName,
		const void* pArgLists,
		int argListSize,
		const fastiva_AnnotationItem* pAnnotationsData,
		int* pAnnotationsInfo,
		void (*initClassIDs)(java_lang_Class_p* classes, const char** typeSigs),
		unicod const* const* aConstString,
		java_lang_String_p* stringPool
	) {
		m_classes.init(contextSlots, cntContext);//fm::Package_9$ - this->m_aPackage;//FASTIVA_ARRAY_ITEM_COUNT(fm::aPackageSlot$);
		this->m_aRefName = pRefNames;
		this->m_aArgList = pArgLists;
		this->m_pAnnotationsData = pAnnotationsData;
		this->m_pAnnotationsInfo = pAnnotationsInfo;
		this->m_cntRefName = cntRefName;
		this->m_aArgListEnd = (void*)((int)pArgLists + argListSize);
		this->m_stringPool = stringPool;
		this->m_aConstString = aConstString;
		this->m_fnInitClassIDs = initClassIDs;
	}
	fox_Monitor m_monitor;


	void prepareRawClasses();

	const fastiva_ClassContext* findClass(
		uint hashCode, 
		const char* pClassSig
	) const;

	void initAllClasses();

	void initStringPool();

	void internStringPool();

	void linkAllClasses();

	void linkPreloadableClasses(bool isLibCore);
		
	void initExteranlModule();

	void* getAnnotations(void* id);

	static void initDefaultAnnotationValues(fastiva_Class_p clazz);

	static java_lang_annotation_Annotation_p getRuntimeRetainAnnotation();

private:
	void initRawClasses();

};

#define g_pLibcoreModule ((fastiva_Module*)&fastiva_libcore_ModuleInfo)
/*
struct fastiva_ModuleInfo { //: fastiva_Instance {
	const JNI_HashEntry* m_aContextSlot;
	int   m_cntContext;
	java_lang_ClassLoader_p classLoader;
	int   m_cntMarked;
	const void* m_pLoadedBy;
	java_lang_Class_p m_pClassQ;
	unicod const* const* m_aConstString;
	java_lang_String_p* m_stringPool;
	const fastiva_Package* m_pProxyQ;
	const fastiva_ClassContext** m_ppImportedContext;
	int   m_imageOffset;
	const JNI_HashEntry* m_aRefName;
	int   m_cntRefName;
	const void* m_aArgList;
	const void* m_aArgListEnd;
	const fastiva_AnnotationItem* m_pAnnotationsData;
	int* m_pAnnotationsInfo;
	void (*m_fnInitClassIDs)(java_lang_Class_p* classes, const char** typeSigs);
	const void* m_hDLL;

	void init(
		const JNI_HashEntry* contextSlots, 
		int cntContext, 
		const JNI_HashEntry* pRefNames, 
		int cntRefName,
		const void* pArgLists,
		int argListSize,
		const fastiva_AnnotationItem* pAnnotationsData,
		int* pAnnotationsInfo,
		void (*initClassIDs)(java_lang_Class_p* classes, const char** typeSigs),
		unicod const* const* aConstString,
		java_lang_String_p* stringPool
	) {
		this->m_aContextSlot = contextSlots;
		this->m_cntContext = cntContext;//fm::Package_9$ - this->m_aPackage;//FASTIVA_ARRAY_ITEM_COUNT(fm::aPackageSlot$);
		this->m_aRefName = pRefNames;
		this->m_aArgList = pArgLists;
		this->m_pAnnotationsData = pAnnotationsData;
		this->m_pAnnotationsInfo = pAnnotationsInfo;
		this->m_cntRefName = cntRefName;
		this->m_aArgListEnd = (void*)((int)pArgLists + argListSize);
		this->m_stringPool = stringPool;
		this->m_aConstString = aConstString;
		this->m_fnInitClassIDs = initClassIDs;
	}
};
*/

struct fastiva_ImportedPackageInfo {
	int m_hashCode;
	union {
		char m_szPackageName[4];
		const fastiva_Package* m_pImportedPackage;
	};
};

inline int fastiva_getAbstractMethodAddress() {
#if FASTIVA_SUPPORTS_EXTERNAL_PACKAGE_LOADER
	class FOX_NOVTABLE abstract_Object$ {
		virtual void abstract_fn() = 0;
	};
	jbyte c2[sizeof(java_lang_Object)];
	// NDK_TODO 생성자 바로 호출할 수 없음
	((abstract_Object$*)c2)->abstract_Object$::abstract_Object$();
	return (*(int**)&c2)[0];
#else
	return 0;
#endif
}



/*
#define FASTIVA_DEFINE_STRING_DATA_ASCII(len, name)						\
	const fastiva_ConstantAsciiString str_##name##$ = { -len, 

#define FASTIVA_DEFINE_STRING_DATA_UCS16(len, name)						\
	const fastiva_ConstantUcs16String str_##name##$ = { +len, 

#define FASTIVA_STRING_DATA_PTR(name)									\
	(java_lang_String_p)(void*)&str_##name##$
*/



#endif // __FASTIVA_MODULE__H__
