#ifndef __FASTIVA_CLASS_H__
#define __FASTIVA_CLASS_H__

FASTIVA_CLASS fastiva_Class : public java_lang_Class {

public:
	fastiva_Class() {}
#if FASTIVA_TARGET_ANDROID_VERSION >= 40400 || defined (FASTIVA_USE_PREIMPORT_CLASSES)
	u4 just_for_8_byte_align;
#endif

public:
	const fastiva_ClassContext* getContext$() {
#ifdef ANDROID
		return (ClassObject*)this;
#else
		return m_pContext$;
#endif
	}

private: /*prevented to be called*/
	static java_lang_Class_p getRawStatic$() { return NULL; }
	static java_lang_Class_p importClass$() { return NULL; }
};

class fastiva_InterfaceClass : public fastiva_Class {
	// vtable 이 없는 interface 등의 처리를 위한 guard 함수.
public: 
	static void initVTable$(fastiva_Class* pClass) { }
};

class fastiva_PrimitiveClass : public fastiva_Class {
FASTIVA_RUNTIME_CRITICAL(private):
	fastiva_PrimitiveClass() {}
};

class fastiva_EnumClass : public fastiva_Class {
FASTIVA_RUNTIME_CRITICAL(private):

FASTIVA_DECL_STATIC_FIELD(protected, java_lang_Enum_ap, m_$VALUES)
	FASTIVA_DECL_POINTER_FIELD(protected, 0, java_lang_Enum_ap, m_$VALUES, get__$VALUES, set__$VALUES)
public:
	java_lang_Enum_p getEnumValue$(int idx) {
		return (java_lang_Enum_p)get__$VALUES()->get$(idx);
	}
};


#define fastiva_Thread_G$  java_lang_Object_G$

FASTIVA_CLASS fastiva_Thread : public java_lang_Object {
FASTIVA_RUNTIME_CRITICAL(private):
	FOX_HTASK m_hTask$;
public:
	public: jbool isDaemon()	{ return false; }
	public: static void init$(fastiva_Thread_p self);
};

// 삭제...
typedef java_lang_Object fastiva_Throwable;

FASTIVA_CLASS fastiva_ClassLoader : public java_lang_Object {
FASTIVA_RUNTIME_CRITICAL(private):
	fastiva_Module* m_pModule;
};

struct fastiva_RawClass {
	// dalvik Object
	void* cpp_vtable$;
    fastiva_Class_p    clazz;
    uint            lock;
	uint			flags;

	// dalvik ClassObject
	#include "dalvik_class_fields.hxx"

	char	filler[sizeof(fastiva_Class)-sizeof(ClassObject)];
};

class fastiva_Interface	: public java_lang_Object { 
public:
	struct VTABLE$ {};
};



#endif // __FASTIVA_CLASS_H__

/**====================== end ========================**/
