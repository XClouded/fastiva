    /* leave space for instance data; we could access fields directly if we
       freeze the definition of java/lang/Class */
#ifdef FASTIVA
	union {
		struct {
			const int*  vtable$;
			const int** itables$;
		} obj;
		struct {
			fastiva_AnnotationItem* annoDefaults$;
			int itableId$;
		} ifc;
	};
	void (*initStatic$)();//const fastiva_ClassContext* m_pContext$;
#else
    u4              instanceData[CLASS_FIELD_SLOTS];
#endif

    /* UTF-8 descriptor for the class; from constant pool, or on heap
       if generated ("[C") */
    const char*     descriptor;
#ifdef FASTIVA
	union { 
		void (*initVTable$)(void*);
		char* descriptorAlloc;
	};
#else
    char*           descriptorAlloc;
#endif

    /* access flags; low 16 bits are defined by VM spec */
    u4              accessFlags;

    /* VM-unique class serial number, nonzero, set very early */
    u4              serialNumber;

    /* DexFile from which we came; needed to resolve constant pool entries */
    /* (will be NULL for VM-generated, e.g. arrays and primitive classes) */
    DvmDex*         pDvmDex;

    /* state of class initialization */
    ClassStatus     status;

    /* if class verify fails, we must return same error on subsequent tries */
    ClassObject*    verifyErrorClass;

    /* threadId, used to check for recursive <clinit> invocation */
    u4              initThreadId;

    /*
     * Total object size; used when allocating storage on gc heap.  (For
     * interfaces and abstract classes this will be zero.)
     */
    size_t          objectSize;

    /* arrays only: class object for base element, for instanceof/checkcast
       (for String[][][], this will be String) */
    ClassObject*    elementClass;

    /* arrays only: number of dimensions, e.g. int[][] is 2 */
    int             arrayDim;

    /* primitive type index, or PRIM_NOT (-1); set for generated prim classes */
    PrimitiveType   primitiveType;

    /* superclass, or NULL if this is java.lang.Object */
    ClassObject*    super;

    /* defining class loader, or NULL for the "bootstrap" system loader */
    Object*         classLoader;

    /* initiating class loader list */
    /* NOTE: for classes with low serialNumber, these are unused, and the
       values are kept in a table in gDvm. */
    InitiatingLoaderList initiatingLoaderList;

    /* array of interfaces this class implements directly */
    int             interfaceCount;
#ifndef FASTIVA
    ClassObject**   interfaces;
#endif

    /* static, private, and <init> methods */
    int             directMethodCount;
    Method*         directMethods;

    /* virtual methods defined in this class; invoked through vtable */
    int             virtualMethodCount;
    Method*         virtualMethods;

    /*
     * Virtual method table (vtable), for use by "invoke-virtual".  The
     * vtable from the superclass is copied in, and virtual methods from
     * our class either replace those from the super or are appended.
     */
    int             vtableCount;
    Method**        vtable;

    /*
     * Interface table (iftable), one entry per interface supported by
     * this class.  That means one entry for each interface we support
     * directly, indirectly via superclass, or indirectly via
     * superinterface.  This will be null if neither we nor our superclass
     * implement any interfaces.
     *
     * Why we need this: given "class Foo implements Face", declare
     * "Face faceObj = new Foo()".  Invoke faceObj.blah(), where "blah" is
     * part of the Face interface.  We can't easily use a single vtable.
     *
     * For every interface a concrete class implements, we create a list of
     * virtualMethod indices for the methods in the interface.
     */
    int             iftableCount;
    InterfaceEntry* iftable;

    /*
     * The interface vtable indices for iftable get stored here.  By placing
     * them all in a single pool for each class that implements interfaces,
     * we decrease the number of allocations.
     */
#ifdef FASTIVA
	union {
		void*			m_annotationsInfo;
    int             ifviPoolCount;
	};
	union {
		const fastiva_ClassContext** m_pEnclosing_n_DeclaredClasses;;
    int*            ifviPool;
	};
#else
    int             ifviPoolCount;
    int*            ifviPool;
#endif


    /* instance fields
     *
     * These describe the layout of the contents of a DataObject-compatible
     * Object.  Note that only the fields directly defined by this class
     * are listed in ifields;  fields defined by a superclass are listed
     * in the superclass's ClassObject.ifields.
     *
     * All instance fields that refer to objects are guaranteed to be
     * at the beginning of the field list.  ifieldRefCount specifies
     * the number of reference fields.
     */
    int             ifieldCount;
    int             ifieldRefCount; // number of fields that are object refs
    InstField*      ifields;

    /* bitmap of offsets of ifields */
    u4 refOffsets;

    /* source file name, if known */
#ifdef FASTIVA
	union {
		const char*     genericSig;
	    const char*     sourceFile;
	};
#else
    const char*     sourceFile;
#endif

    /* static fields */
    int             sfieldCount;

#ifdef FASTIVA
	union {
		StaticField*    sfields;
		//JValue for_8byte_align_of_arm_gcc_mode;
	};
#ifdef FASTIVA_USE_PREIMPORT_CLASSES
	const fastiva_Class_p* preimportedClasses;
#endif
#else
    StaticField     sfields[0]; /* MUST be last item */
#endif

