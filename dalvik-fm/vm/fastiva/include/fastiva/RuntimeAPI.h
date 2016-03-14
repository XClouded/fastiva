
#ifndef FASTIVA_DECL_VM_RUNTIME_API
#error Error!! FASTIVA_DECL_VM_RUNTIME_API must be defined.
#endif

#ifdef FASTIVA_DECL_VM_RUNTIME_CLASS_NAME
struct FASTIVA_DECL_VM_RUNTIME_CLASS_NAME {
	static FASTIVA_DECL_VM_RUNTIME_CLASS_NAME FASTIVA_RUNTIME_EXPORT g_instance;
#endif

	//======================================================//
	// Class Loader
	//======================================================//
	FASTIVA_DECL_VM_RUNTIME_API(void, initFastivaApp, 1,, (
		void* // appInfo
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(linkClass_$$, 2,, ( //
		void*, //pVTable 
		fastiva_Class_p
	))

	FASTIVA_DECL_VM_RUNTIME_API(void, initClass_$$, 1,, ( // 
		fastiva_Class_p
	))

	FASTIVA_DECL_VM_RUNTIME_API(java_lang_Class_p, getPrimitiveClass, 2,, ( //
		int, // primitiveType, 
		int  //arrayDimension
	)) 

	FASTIVA_DECL_VM_RUNTIME_API(int, loadExternalModule, 2,, ( //
		fastiva_Module*, // module, 
		java_lang_ClassLoader_p // classLoader
	)) 

	FASTIVA_DECL_VM_RUNTIME_API(java_lang_String_p, getImmortalString, 2,, (
		fastiva_Module*, // module, 
		int // id
	))

	FASTIVA_DECL_VM_RUNTIME_API(void*, pushCallerClass, 2,, (
	    fastiva_CallerClassStack*,
		fastiva_Class_p // classInfo
	))

	FASTIVA_DECL_VM_RUNTIME_API(void, popCallerClass, 1,, (
	    fastiva_CallerClassStack* // classInfo
	))

	//======================================================//
	// Type Checking
	//======================================================//

	FASTIVA_DECL_VM_RUNTIME_API(fastiva_Class_p, getArrayClass, 2,, ( //
		const fastiva_ClassContext*,
		int // dimension
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(checkInstanceOf, 2,, ( // 
		fastiva_Instance_p, 
		const fastiva_ClassContext*
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(checkInstanceOf_dbg, 2,, (
		fastiva_Instance_p, 
		const fastiva_ClassContext*
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(checkImplemented, 2,, ( // 
		fastiva_Instance_p, 
		const fastiva_ClassContext*
	))

	FASTIVA_DECL_VM_RUNTIME_API(void*, isInstanceOf, 2,, ( //
		fastiva_Instance_p, 
		const fastiva_ClassContext*
	))

	FASTIVA_DECL_VM_RUNTIME_API(const void**, isImplemented, 2,, ( //
		fastiva_Instance_p, 
		const fastiva_ClassContext*
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(checkArrayInstanceOf, 3,, ( //
		fastiva_Instance_p, 
		const fastiva_ClassContext*, 
		int // dimension
	))

	FASTIVA_DECL_VM_RUNTIME_API(void*, isArrayInstanceOf, 3,, ( //
		fastiva_Instance_p, 
		const fastiva_ClassContext*, 
		int // dimension
	))

	//======================================================//
	// Allocation
	//======================================================//


	FASTIVA_DECL_VM_RUNTIME_API(java_lang_Object_p, allocate, 1, FOX_RESTRICT_API, ( //
		const fastiva_ClassContext*
	))


	FASTIVA_DECL_VM_RUNTIME_API(fastiva_ArrayHeader*, allocatePrimitiveArray, 2, FOX_RESTRICT_API, ( // 
		fastiva_PrimitiveClass_p,
		int // length 
	))

	FASTIVA_DECL_VM_RUNTIME_API(fastiva_ArrayHeader*, allocateInitializedArray, 3, FOX_RESTRICT_API, (
		fastiva_PrimitiveClass_p,
		const void*, // data,
		int // length
	))

	FASTIVA_DECL_VM_RUNTIME_API(fastiva_ArrayHeader*, allocateMultiArray, 2, FOX_RESTRICT_API, ( // 
		fastiva_Class_p, // pArrayClass,
		const int* // lengths 
	))


	FASTIVA_DECL_VM_RUNTIME_API(fastiva_ArrayHeader*, allocatePointerArray, 2, FOX_RESTRICT_API, ( // 
		const fastiva_ClassContext*,
		int // length 
	))

	FASTIVA_DECL_VM_RUNTIME_API(fastiva_ArrayHeader*, allocateMultiArrayEx, 3, FOX_RESTRICT_API, ( //
		const fastiva_ClassContext*,
		int, // dimension,
		const int* // lengths
	))

	//======================================================//
	// Reference manipulation
	//======================================================//

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setJavaInstanceField, 3,, (
		fastiva_Instance_p, // obj
		fastiva_BytecodeProxy_p, // value, 
		void* // pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setInstanceField, 3,, ( // 
		fastiva_Instance_p, // obj
		fastiva_Instance_p, // value, 
		void* // pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setInstanceVolatileField, 3,, (
		fastiva_Instance_p, // obj
		fastiva_Instance_p, // value, 
		fastiva_Instance_p* // pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API(int, getVolatileFieldInt, 1,, (
		int*// pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API(jlonglong, getVolatileFieldLong, 1,, (
		jlonglong*// pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setVolatileFieldInt, 2,, (
	    int, //value
		int* // pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setVolatileFieldLong, 2,, (
	    jlonglong, // value
		jlonglong* // pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setStaticField, 3,, ( //
		fastiva_MetaClass_p, // pClass, 
		fastiva_Instance_p, // pValue, 
		void* // pFieldPos
	))


	FASTIVA_DECL_VM_RUNTIME_API_VOID(setArrayItem, 4,, ( //
		fastiva_ArrayHeader*, // pArray, 
		const fastiva_ClassContext*, // pBaseType,
		void*, // pItemSlot, 
		fastiva_Instance_p // pItem
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setAbstractArrayItem, 4,, ( //
		fastiva_ArrayHeader*, // pArray, 
		const fastiva_ClassContext*, // pBaseType,
		void*, // pItemSlot, 
		fastiva_Instance_p // pItem
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setCustomArrayItem, 3,, (
		fastiva_ArrayHeader*, // pArray, 
		void*, // pItemSlot, 
		fastiva_BytecodeProxy_p // pItem
	))

	//======================================================//
	// Synchronization
	//======================================================//


	FASTIVA_DECL_VM_RUNTIME_API_VOID(monitorEnter, 1,, ( //
		fastiva_Instance_p
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(monitorExit, 1,, ( // 
		fastiva_Instance_p
	))



	FASTIVA_DECL_VM_RUNTIME_API_VOID(beginSynchronized, 2,, ( //
		fastiva_Instance_p,
		fastiva_Synchronize*
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(endSynchronized, 2,, ( //
		fastiva_Instance_p,
		fastiva_Synchronize*
	))


	//======================================================//
	// Eception Handling
	//======================================================//


	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwArrayIndexOutOfBoundsException, 0, FOX_NO_RETURN, ( //
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwStringIndexOutOfBoundsException, 2, FOX_NO_RETURN, ( //
		int, // length
		int  // index
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwAbstractMethodError, 0, FOX_NO_RETURN, ( // 
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwClassCastException, 2, FOX_NO_RETURN, ( // 
		fastiva_Instance_p, 
		java_lang_Class_p
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwNoSuchMethodError, 2, FOX_NO_RETURN, ( // 
		fastiva_Class_p, 
		const char * // method_name
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwNoSuchFieldError, 2, FOX_NO_RETURN, ( // 
		fastiva_Class_p, 
		const char * // field_name
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwNoClassDefFoundError, 1, FOX_NO_RETURN, ( // 
		const char * // class_name
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(throwException, 1, FOX_NO_RETURN, ( //
		java_lang_Throwable_p
	))

#ifndef FASTIVA_USE_CPP_EXCEPTION

	FASTIVA_DECL_VM_RUNTIME_API(int, pushExceptionHandler, 1,, ( //
		fastiva_ExceptionContext*
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(removeExceptionHandler, 1,, ( //
		fastiva_ExceptionContext*
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(rethrow, 2, FOX_NO_RETURN, ( //
		fastiva_ExceptionContext*, 
		void* // pException
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(pushRewinder, 2,, (
		fastiva_Task*, // pCurrTask,
		fastiva_Rewinder* // pRewinder
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(rewind, 1,, ( //
		fastiva_Rewinder* // pRewinder
	))
#endif

	//======================================================//
	// Floating Point
	//======================================================//

	FASTIVA_DECL_VM_RUNTIME_API(double, IEEEremainder, 2,, (
		double, // v1
		double  // v2
	))

	FASTIVA_DECL_VM_RUNTIME_API(float, IEEEremainderF, 2,, (
		float, // v1
		float  // v2
	))


	//======================================================//
	// JNI integration
	//======================================================//

	FASTIVA_DECL_VM_RUNTIME_API(void*, beginJniCall, 3,, ( //
		fastiva_Class_p, 
		int, // method_index, 
		fastiva_JniCallInfo* // callInfo
	))

	FASTIVA_DECL_VM_RUNTIME_API(java_lang_Object_p, endJniCall, 2,, ( //
		void*, // jniResult, 
		fastiva_JniCallInfo* // callInfo
	))

	FASTIVA_DECL_VM_RUNTIME_API(void*, beginFastJniCall, 3,, ( //
		fastiva_Class_p, 
		int, // method_index, 
		fastiva_JniCallInfo* // callInfo
	))

	FASTIVA_DECL_VM_RUNTIME_API(java_lang_Object_p, endFastJniCall, 2,, ( //
		void*, // jniResult, 
		fastiva_JniCallInfo* // callInfo
	))

	FASTIVA_DECL_VM_RUNTIME_API(void*, addLocalReference, 2,, ( //
		fastiva_Task*,
		java_lang_Object_p
	))

	//======================================================//
	// Debugging
	//======================================================//

	FASTIVA_DECL_VM_RUNTIME_API_VOID(debugBreak, 2,, ( // 
		const char*, // message
		int // forceSleep
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(pushCallStack, 1,, ( // 
		fastiva_CallStack* // pMethod
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(popCallStack, 1,, ( // 
		fastiva_CallStack* // pMethod
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(checkFieldAccess, 3,, ( // 
		fastiva_Instance*, 
		void*, // field, 
		int // size
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(ensureArray, 2,, ( // 
		const void*, // array_, 
		int // itemSize
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(preimportClass, 1,, ( // 
		fastiva_Class_p
	))


	int (*log)(int level, const char* tag, const char *fmt, ...); 


#if FASTIVA_SUPPORTS_JAVASCRIPT
	FASTIVA_DECL_VM_RUNTIME_API_VOID(setGenericField, 3,, (
		fastiva_Instance_p, 
		fastiva_lang_Generic_p, // pValue, 
		void* // pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setStaticField, 3,, (
		fastiva_Class_p, 
		fastiva_lang_Generic_p // pValue, 
		void* // pFieldPos
	))

	FASTIVA_DECL_VM_RUNTIME_API_VOID(setGenericArrayItem, 3,, (
		fastiva_ArrayHeader*, 
		void*, //pItemSlot, 
		fastiva_lang_Generic_p // pItem
	))
#endif


#ifdef FASTIVA_DECL_VM_RUNTIME_CLASS_NAME
};
#endif


#undef FASTIVA_DECL_VM_RUNTIME_CLASS_NAME
#undef FASTIVA_DECL_VM_RUNTIME_API
#undef FASTIVA_DECL_VM_RUNTIME_API_VOID




