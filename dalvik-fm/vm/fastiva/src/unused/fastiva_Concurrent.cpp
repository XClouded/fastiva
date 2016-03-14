#include <precompiled_libcore.h>

#include <java/util/concurrent/atomic/AtomicReferenceFieldUpdater.h>
#include <java/util/concurrent/atomic/AtomicReferenceFieldUpdater.inl>


#include <kernel/Kernel.h>
#include <kernel/Runnable.h>
#include <fox/Task.h>
#include <fox/kernel/Sys.h>

JPP_RETURNS(VAL$(jbool))
java_util_concurrent_atomic_AtomicReferenceFieldUpdater::compareAndSet(
	java_lang_Object_p&  pObject1,
	jint  int2,
	java_lang_Object_p  pObject3,
	java_lang_Object_p  pObject4
) {
#if defined(FASTIVA_SUPPORT_JNI) && !defined(FASTIVA_NO_JNI_PACKAGE_java_util_concurrent_atomic)
	typedef JPP_JNI_RETURNS(VAL$(jbool)) (FOX_FASTCALL(*METHOD_T))(void*, void*, java_lang_Object_p,jint,java_lang_Object_p,java_lang_Object_p);
	static const char* mname[] = {
		"Java_java_util_concurrent_atomic_AtomicReferenceFieldUpdater",
		"compareAndSet@24",
	};
	JPP_JNI_RETURNS(VAL$(jbool)) res = JPP_INVOKE_JNI_METHOD(METHOD_T, (this, mname,  pObject1,  int2,  pObject3,  pObject4));
#else
	jbool res = java_util_concurrent_atomic_AtomicReferenceFieldUpdater__compareAndSet(this,  pObject1,  int2,  pObject3,  pObject4);
#endif

	JPP_JNI_RETURN_RESULT(VAL$(jbool), res);
}

