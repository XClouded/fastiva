#ifndef __FASTIVA_META_CLASS_H__
#define __FASTIVA_META_CLASS_H__
// java/lang/Object.h 에 의해 include 된다.

#include "Common.h"

/*
 * Native function pointer type.
 *
 * "args[0]" holds the "this" pointer for virtual methods.
 *
 * The "Bridge" form is a super-set of the "Native" form; in many places
 * they are used interchangeably.  Currently, all functions have all
 * arguments passed in, but some functions only care about the first two.
 * Passing extra arguments to a C function is (mostly) harmless.
 */
typedef void (*DalvikBridgeFunc)(const u4* args, JValue* pResult,
    const Method* method, struct Thread* self);
typedef void (*DalvikNativeFunc)(const u4* args, JValue* pResult);


/* Use the top 16 bits of the access flags field for
 * other class flags.  Code should use the *CLASS_FLAG*()
 * macros to set/get these flags.
 */
enum ClassFlags {
    CLASS_ISFINALIZABLE        = (1<<31), // class/ancestor overrides finalize()
    CLASS_ISARRAY              = (1<<30), // class is a "[*"
    CLASS_ISOBJECTARRAY        = (1<<29), // class is a "[L*" or "[[*"
    CLASS_ISCLASS              = (1<<28), // class is *the* class Class

    CLASS_ISREFERENCE          = (1<<27), // class is a soft/weak/phantom ref
                                          // only ISREFERENCE is set --> soft
    CLASS_ISWEAKREFERENCE      = (1<<26), // class is a weak reference
    CLASS_ISFINALIZERREFERENCE = (1<<25), // class is a finalizer reference
    CLASS_ISPHANTOMREFERENCE   = (1<<24), // class is a phantom reference

    CLASS_MULTIPLE_DEFS        = (1<<23), // DEX verifier: defs in multiple DEXs

    /* unlike the others, these can be present in the optimized DEX file */
    CLASS_ISOPTIMIZED          = (1<<17), // class may contain opt instrs
    CLASS_ISPREVERIFIED        = (1<<16), // class has been pre-verified
};

enum ClassStatus {
    CLASS_ERROR         = -1,

    CLASS_NOTREADY      = 0,
    CLASS_IDX           = 1,    /* loaded, DEX idx in super or ifaces */
    CLASS_LOADED        = 2,    /* DEX idx values resolved */
    CLASS_RESOLVED      = 3,    /* part of linking */
    CLASS_VERIFYING     = 4,    /* in the process of being verified */
    CLASS_VERIFIED      = 5,    /* logically part of linking; done pre-init */
    CLASS_INITIALIZING  = 6,    /* class init in progress */
    CLASS_INITIALIZED   = 7,    /* ready to go */
};


struct InterfaceEntry {
    /* pointer to interface class */
    ClassObject*    clazz;

    /*
     * Index into array of vtable offsets.  This points into the ifviPool,
     * which holds the vtables for all interfaces declared by this class.
     */
    int*            methodIndexArray;
};


/*
 * Data objects have an Object header followed by their instance data.
 */
struct DataObject : java_lang_Object {
    /* variable #of u4 slots; u8 uses 2 slots */
    u4              instanceData[1];
};

/*
 * Strings are used frequently enough that we may want to give them their
 * own unique type.
 *
 * Using a dedicated type object to access the instance data provides a
 * performance advantage but makes the java/lang/String.java implementation
 * fragile.
 *
 * Currently this is just equal to DataObject, and we pull the fields out
 * like we do for any other object.
 */
struct StringObject : java_lang_Object {
    /* variable #of u4 slots; u8 uses 2 slots */
#ifndef FASTIVA
    u4              instanceData[1];
#endif

    /** Returns this string's length in characters. */
    int length() const;

    /**
     * Returns this string's length in bytes when encoded as modified UTF-8.
     * Does not include a terminating NUL byte.
     */
    int utfLength() const;

    /** Returns this string's char[] as an ArrayObject. */
    ArrayObject* array() const;

    /** Returns this string's char[] as a u2*. */
    const u2* chars() const;
};


/*
 * Array objects have these additional fields.
 *
 * We don't currently store the size of each element.  Usually it's implied
 * by the instruction.  If necessary, the width can be derived from
 * the first char of obj->clazz->descriptor.
 */
struct ArrayObject : java_lang_Object {
    /* number of elements; immutable after init */
    u4              length;
#if FASTIVA_SUPPORTS_DYNAMIC_ARRAY
    u8*              contents;
#else

#ifdef FASTIVA
    u4              just_for_8byte_align_padding;
#endif
    /*
     * Array contents; actual size is (length * sizeof(type)).  This is
     * declared as u8 so that the compiler inserts any necessary padding
     * (e.g. for EABI); the actual allocation may be smaller than 8 bytes.
     */
    u8              contents[1];
#endif
};

/*
 * For classes created early and thus probably in the zygote, the
 * InitiatingLoaderList is kept in gDvm. Later classes use the structure in
 * Object Class. This helps keep zygote pages shared.
 */
struct InitiatingLoaderList {
    /* a list of initiating loader Objects; grown and initialized on demand */
    Object**  initiatingLoaders;
    /* count of loaders in the above list */
    int       initiatingLoaderCount;
};

/*
 * Generic field header.  We pass this around when we want a generic Field
 * pointer (e.g. for reflection stuff).  Testing the accessFlags for
 * ACC_STATIC allows a proper up-cast.
 */
struct Field {
    ClassObject*    clazz;          /* class in which the field is declared */
    const char*     name;
    const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    u4              accessFlags;
#ifdef FASTIVA
	const char*		genericSig;
    void*           annotationsId;
#endif
};


/*
 * Static field.
 */
struct StaticField : Field {
    JValue          value;          /* initially set from DEX for primitives */
};

/*
 * Instance field.
 */
struct InstField : Field {
    /*
     * This field indicates the byte offset from the beginning of the
     * (Object *) to the actual instance data; e.g., byteOffset==0 is
     * the same as the object pointer (bug!), and byteOffset==4 is 4
     * bytes farther.
     */
    int             byteOffset;
#ifdef FASTIVA
	int				just_to_make_same_size_with_StaticField;
#endif
};

/*
 * Class objects have many additional fields.  This is used for both
 * classes and interfaces, including synthesized classes (arrays and
 * primitive types).
 *
 * Class objects are unusual in that they have some fields allocated with
 * the system malloc (or LinearAlloc), rather than on the GC heap.  This is
 * handy during initialization, but does require special handling when
 * discarding java.lang.Class objects.
 *
 * The separation of methods (direct vs. virtual) and fields (class vs.
 * instance) used in Dalvik works out pretty well.  The only time it's
 * annoying is when enumerating or searching for things with reflection.
 */
struct ClassObject : java_lang_Object {
#include "dalvik_class_fields.hxx"
};

/*
 * A method.  We create one of these for every method in every class
 * we load, so try to keep the size to a minimum.
 *
 * Much of this comes from and could be accessed in the data held in shared
 * memory.  We hold it all together here for speed.  Everything but the
 * pointers could be held in a shared table generated by the optimizer;
 * if we're willing to convert them to offsets and take the performance
 * hit (e.g. "meth->insns" becomes "baseAddr + meth->insnsOffset") we
 * could move everything but "nativeFunc".
 */
struct Method {
    /* the class we are a part of */
    ClassObject*    clazz;

    /* access flags; low 16 bits are defined by spec (could be u2?) */
    u4              accessFlags;

    /*
     * For concrete virtual methods, this is the offset of the method
     * in "vtable".
     *
     * For abstract methods in an interface class, this is the offset
     * of the method in "iftable[n]->methodIndexArray".
     */
    u2             methodIndex;

    /*
     * Method bounds; not needed for an abstract method.
     *
     * For a native method, we compute the size of the argument list, and
     * set "insSize" and "registerSize" equal to it.
     */
    u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;

    /* method name, e.g. "<init>" or "eatLunch" */
    const char*     name;

    /*
     * Method prototype descriptor string (return and argument types).
     *
     * TODO: This currently must specify the DexFile as well as the proto_ids
     * index, because generated Proxy classes don't have a DexFile.  We can
     * remove the DexFile* and reduce the size of this struct if we generate
     * a DEX for proxies.
     */
    DexProto        prototype;

    /* short-form method descriptor string */
    const char*     shorty;

    /*
     * The remaining items are not used for abstract or native methods.
     * (JNI is currently hijacking "insns" as a function pointer, set
     * after the first call.  For internal-native this stays null.)
     */

    /* the actual code */
    const u2*       insns;          /* instructions, in memory-mapped .dex */

    /* JNI: cached argument and return-type hints */
    int             jniArgInfo;

    /*
     * JNI: native method ptr; could be actual function or a JNI bridge.  We
     * don't currently discriminate between DalvikBridgeFunc and
     * DalvikNativeFunc; the former takes an argument superset (i.e. two
     * extra args) which will be ignored.  If necessary we can use
     * insns==NULL to detect JNI bridge vs. internal native.
     */
    DalvikBridgeFunc nativeFunc;

    /*
     * JNI: true if this static non-synchronized native method (that has no
     * reference arguments) needs a JNIEnv* and jclass/jobject. Libcore
     * uses this.
     */
    bool fastJni;

    /*
     * JNI: true if this method has no reference arguments. This lets the JNI
     * bridge avoid scanning the shorty for direct pointers that need to be
     * converted to local references.
     *
     * TODO: replace this with a list of indexes of the reference arguments.
     */
    bool noRef;

    /*
     * JNI: true if we should log entry and exit. This is the only way
     * developers can log the local references that are passed into their code.
     * Used for debugging JNI problems in third-party code.
     */
    bool shouldTrace;

#ifdef FASTIVA
    bool            inProfile;
	union {
		const void* fastivaMethod;
		const RegisterMap* registerMap;
	};
	const char* genericSig;
	void* annotations;
	void* paramAnnotations;
#else
    /*
     * Register map data, if available.  This will point into the DEX file
     * if the data was computed during pre-verification, or into the
     * linear alloc area if not.
     */
    const RegisterMap* registerMap;

    /* set if method was called during method profiling */
    bool            inProfile;
#endif

};


#include <fastiva/Array.h>
#include <java/lang/Object.ptr>

#define fastiva_MetaClass_G$	java_lang_Object_G$
#define StringObject_G$			java_lang_Object_G$
#define StringObject_I$			java_lang_Object_I$

// super-class of java.lang.Class
FASTIVA_CLASS fastiva_MetaClass : public ClassObject {
public:
	void setField_$$(fastiva_Instance_p pValue, void* pField) {
		fastiva.setStaticField(this, pValue, pField);
	}
};

struct fastiva_Interface_G$ : public java_lang_Object_G$ {
	enum { 
		isInterface$ = 1,
		g_vtable$ = 0
	};
};

struct fastiva_AnnotationItem {
	union {
		struct {
			jint			name;
			fastiva_Class_p	type;
		} member;
		struct {
			const char*		name;
			const char*		sig;
		} method;
		struct {
			const char*		name;
			java_lang_Object_p value;
		} defVal;
		jbool		z;
		jint		i;
		jlonglong	j;
		jfloat		f;
		jdouble		d;
	};
	fastiva_AnnotationItem(jint v) {
		this->i = v;
	}
	fastiva_AnnotationItem(jdouble v) {
		this->d = v;
	}
	fastiva_AnnotationItem(jlonglong v) {
		this->j = v;
	}
	fastiva_AnnotationItem(jfloat v) {
		this->f = v;
	}
	fastiva_AnnotationItem(int name, fastiva_Class_p clazz) {
		this->member.name = name;
		this->member.type = clazz;
	}
	fastiva_AnnotationItem(const char* name, const char* sig) {
		this->method.name = name;
		this->method.sig = sig;
	}
};



#define FASTIVA_DECL_DEFAULT_ANNOTATION_VALUE_LIST() \
	static fastiva_AnnotationItem g_vtable$[];

#define FASTIVA_PRIMITIVE_TYPE_ID(T)		(fastiva_Class_p)(fastiva_Primitives::T)

#define FASTIVA_REFERENCE_TYPE_ID(T)		FASTIVA_RAW_CLASS_CONTEXT_PTR(T)

// type이 'J'가 아닌 경우엔, dimension 은 항상 0이다.
#define FASTIVA_ANNOTATION_MEMBER(name, type) \
	fastiva_AnnotationItem((int)name, type),

#define FASTIVA_ANNOTATION_VALUE(v) \
	fastiva_AnnotationItem(v),

#define FASTIVA_ANNOTATION_MEMBER_ARRAY(name, type, cntItem) \
	fastiva_AnnotationItem((int)name | 0x80000000, type), \
	fastiva_AnnotationItem(cntItem),

#define FASTIVA_BEGIN_ANNOTATION(CLASS, cntMember) \
	fastiva_AnnotationItem(cntMember, FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS)), 

#define FASTIVA_BEGIN_DEFAULT_ANNOTATION_VALUE_LIST(CLASS, cntMember) \
	fastiva_AnnotationItem CLASS##_G$::g_vtable$[] = { \
		fastiva_AnnotationItem(cntMember, NULL), 

#define FASTIVA_END_DEFAULT_ANNOTATION_VALUE_LIST() \
	};




/*
#define FASTIVA_REFERENCE_TYPE(CLASS, DIMENSION)	\
		FASTIVA_JNI_TYPE_ID_ARRAY$(DIMENSION, CLASS)

#define FASTIVA_PRIMITIVE_TYPE(TYPE, DIMENSION)	\
		FASTIVA_JNI_TYPE_ID_ARRAY$(DIMENSION, TYPE)



*/
#endif // __FASTIVA_META_CLASS_H__

