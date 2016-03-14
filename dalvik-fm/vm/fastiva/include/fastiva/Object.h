#ifndef __FASTIVA_OBJECT_H__
#define __FASTIVA_OBJECT_H__

#include <fastiva/preprocess/preprocess.h>
#include <fastiva/class_def.h>
#include <fastiva/ClassContext.h>

#include <fastiva/class_def.h>

#ifdef EMBEDDED_RUNTIME 
	#define FASTIVA_RUNTIME_CRITICAL(scope)  public
#else 
	#define FASTIVA_RUNTIME_CRITICAL(scope)  scope
#endif

#define FASTIVA_NEW(CLASS)		(CLASS*)((CLASS*)fastiva.allocate(CLASS::getRawContext$()))->init$
#define FASTIVA_ALLOC(CLASS)	CLASS::allocate$()

#define m_pClass$  clazz

struct dalvik_Object {
FASTIVA_RUNTIME_CRITICAL(private):
	virtual ~dalvik_Object() {}

	/* ptr to class object */
    fastiva_Class_p    clazz;

    /*
     * A word containing either a "thin" lock or a "fat" monitor.  See
     * the comments in Sync.c for a description of its layout.
     */
    uint            lock;
	uint			flags;

};


FASTIVA_CLASS fastiva_Instance : public dalvik_Object {

public:

	fastiva_Instance() {}

public:
	struct VTABLE$;

	enum {
		dimension$ = 0,
	};

	java_lang_Object_p getInstance$() const {
		return (java_lang_Object_p)this;
	}

	const int* getInstanceVTable$() {
		return *(int**)(void*)this;
	}

	java_lang_Object_p as__java_lang_Object$() {
		return (java_lang_Object_p)this;
	}

	const int* getIVTable$(int IVTableID) FASTIVA_PURE_API;

	const int* getInterfaceTable$(
		const fastiva_ClassContext* pInterfaceContext
	);

	void releaseJavaProxy$(void* pEnv0) {}
	void callJavaProxyScan(void* pEnv, void* pField, FASTIVA_JPROXY_SCAN_METHOD method) {}


	fastiva_Class_p getClass$() const { 
		return (fastiva_Class_p)clazz; 
	}

	bool isMonitorLocked();
// 

	void setField_$$(fastiva_BytecodeProxy_p pValue, fastiva_Instance_p* pField) {
		fastiva.setJavaInstanceField(this, pValue, pField);
	}


	void setField_$$(fastiva_Instance_p pValue, fastiva_Instance_p* pField) {
		fastiva.setInstanceField(this, pValue, pField);
	}

	void setVolatileField_$$(fastiva_Instance_p pValue, fastiva_Instance_p* pField) {
		fastiva.setInstanceVolatileField(this, pValue, pField);
	}

	void* getVolatileField_$$(fastiva_Instance_p* pField) {
		return (void*)fastiva.getVolatileFieldInt((jint*)(void*)pField);
	}

// for Voaltile Access. All voltile fields must be aligned in 32bit boundary

	jbool getVolatileField_$$(jbool* pField) {
		return (jbool)fastiva.getVolatileFieldInt((jint*)(void*)pField);
	}

	jbyte getVolatileField_$$(jbyte* pField) {
		return (jbyte)fastiva.getVolatileFieldInt((jint*)(void*)pField);
	}

	jshort getVolatileField_$$(jshort* pField) {
		return (jshort)fastiva.getVolatileFieldInt((jint*)(void*)pField);
	}

	unicod getVolatileField_$$(unicod* pField) {
		return (unicod)fastiva.getVolatileFieldInt((jint*)(void*)pField);
	}

	jfloat getVolatileField_$$(jfloat* pField) {
		int v = fastiva.getVolatileFieldInt((jint*)(void*)pField);
		return *(jfloat*)(void*)&v;
	}


	jint getVolatileField_$$(jint* pField) {
		return (jint)fastiva.getVolatileFieldInt((jint*)(void*)pField);
	}

	jlonglong getVolatileField_$$(jlonglong* pField) {
		return fastiva.getVolatileFieldLong((jlonglong*)(void*)pField);
	}

	jdouble getVolatileField_$$(jdouble* pField) {
		jlonglong res = fastiva.getVolatileFieldLong((jlonglong*)(void*)pField);
		return *(jdouble*)(void*)&res;
	}


	void setVolatileField_$$(jbool value, jbool* pField) {
		fastiva.setVolatileFieldInt(value, (jint*)(void*)pField);
	}

	void setVolatileField_$$(jbyte value, jbyte* pField) {
		fastiva.setVolatileFieldInt(value, (jint*)(void*)pField);
	}

	void setVolatileField_$$(jshort value, jshort* pField) {
		fastiva.setVolatileFieldInt(value, (jint*)(void*)pField);
	}

	void setVolatileField_$$(unicod value, unicod* pField) {
		fastiva.setVolatileFieldInt(value, (jint*)(void*)pField);
	}

	void setVolatileField_$$(jint value, jint* pField) {
		fastiva.setVolatileFieldInt(value, (jint*)(void*)pField);
	}

	void setVolatileField_$$(jfloat value, jfloat* pField) {
		fastiva.setVolatileFieldInt(*(jint*)(void*)&value, (jint*)(void*)pField);
	}

	void setVolatileField_$$(jlonglong value, jlonglong* pField) {
		fastiva.setVolatileFieldLong(value, (jlonglong*)(void*)pField);
	}

	void setVolatileField_$$(jdouble value, jdouble* pField) {
		fastiva.setVolatileFieldLong(*(jlonglong*)(void*)&value, (jlonglong*)(void*)pField);
	}


private:
	void* operator new (size_t);
public:
	void* operator new (size_t, void* ptr) { return ptr; }
	void operator delete (void*);

};

struct fastiva_Instance_G$ {
	static void* aIVT$[1];
	enum { 
		inheritDepth$ = -1,
		isInterface$ = 0,
		g_vtable$ = 0
	};
};

class fastiva_Instance_I$	{ 
#ifndef ANDROID
	void* scanInstance_$$;
#if FASTIVA_SUPPORT_JNI_STUB
	void* scanJavaProxyFields_$$;
#endif
#endif

public:
	static void scanInstance$(fastiva_Instance_p self, FASTIVA_SCAN_METHOD method, fastiva_Scanner* scanner) {}

};

struct fastiva_Instance::VTABLE$ : public fastiva_Instance_I$ {
};


#endif // __FASTIVA_OBJECT_H__
