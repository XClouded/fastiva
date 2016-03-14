/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * java.lang.Object
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"


/*
 * private Object internalClone()
 *
 * Implements most of Object.clone().
 */
static void Dalvik_java_lang_Object_internalClone(const u4* args,
    JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];


#ifdef FASTIVA
	java_lang_Object_p fastiva_Dalvik_java_lang_Object_internalClone(java_lang_Object_p self, java_lang_Cloneable_p);
	*(java_lang_Object_p*)pResult = fastiva_Dalvik_java_lang_Object_internalClone((java_lang_Object_p)thisPtr, (java_lang_Cloneable_p)NULL);
}

java_lang_Object_p fastiva_Dalvik_java_lang_Object_internalClone(java_lang_Object_p thisPtr, java_lang_Cloneable_p unused) {
#endif
    Object* clone = dvmCloneObject(thisPtr, ALLOC_DONT_TRACK);

    RETURN_PTR(clone);
}

/*
 * public int hashCode()
 */
static void Dalvik_java_lang_Object_hashCode(const u4* args, JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];

#ifdef FASTIVA
	jint fastiva_Dalvik_java_lang_Object_hashCode(java_lang_Object_p self);
	*(jint*)pResult = fastiva_Dalvik_java_lang_Object_hashCode((java_lang_Object_p)thisPtr);
}

jint fastiva_Dalvik_java_lang_Object_hashCode(java_lang_Object_p thisPtr) {
#endif

	RETURN_INT(dvmIdentityHashCode(thisPtr));
}

/*
 * public Class getClass()
 */
static void Dalvik_java_lang_Object_getClass(const u4* args, JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_Object_getClass(java_lang_Object_p self);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_Object_getClass((java_lang_Object_p)thisPtr);
}

java_lang_Class_p fastiva_Dalvik_java_lang_Object_getClass(java_lang_Object_p thisPtr) {
#endif

    RETURN_PTR(thisPtr->clazz);
}

/*
 * public void notify()
 *
 * NOTE: we declare this as a full DalvikBridgeFunc, rather than a
 * DalvikNativeFunc, because we really want to avoid the "self" lookup.
 */
static void Dalvik_java_lang_Object_notify(const u4* args, JValue* pResult,
    const Method* method, Thread* self)
{
    Object* thisPtr = (Object*) args[0];

#ifdef FASTIVA
	void fastiva_Dalvik_java_lang_Object_notify(java_lang_Object_p self);
	fastiva_Dalvik_java_lang_Object_notify((java_lang_Object_p)thisPtr);
}

void fastiva_Dalvik_java_lang_Object_notify(java_lang_Object_p thisPtr) {
	Thread* self = dvmThreadSelf();
#endif

    dvmObjectNotify(self, thisPtr);
    MAY_THROW_VOID();
}

/*
 * public void notifyAll()
 */
static void Dalvik_java_lang_Object_notifyAll(const u4* args, JValue* pResult,
    const Method* method, Thread* self)
{
    Object* thisPtr = (Object*) args[0];

#ifdef FASTIVA
	void fastiva_Dalvik_java_lang_Object_notifyAll(java_lang_Object_p self);
	fastiva_Dalvik_java_lang_Object_notifyAll((java_lang_Object_p)thisPtr);
}

void fastiva_Dalvik_java_lang_Object_notifyAll(java_lang_Object_p thisPtr) {
	Thread* self = dvmThreadSelf();
#endif

    dvmObjectNotifyAll(self, thisPtr);
    MAY_THROW_VOID();
}

/*
 * public void wait(long ms, int ns) throws InterruptedException
 */
static void Dalvik_java_lang_Object_wait(const u4* args, JValue* pResult,
    const Method* method, Thread* self)
{
    Object* thisPtr = (Object*) args[0];
	jlonglong millis = GET_ARG_LONG(args,1);
	jint nanos = (s4)args[3];

#ifdef FASTIVA
	void fastiva_Dalvik_java_lang_Object_wait(java_lang_Object_p self, jlonglong, jint);
	fastiva_Dalvik_java_lang_Object_wait(thisPtr, millis, nanos);
}

void fastiva_Dalvik_java_lang_Object_wait(java_lang_Object_p thisPtr, jlonglong millis, jint nanos) {
	Thread* self = dvmThreadSelf();
#endif

    dvmObjectWait(self, thisPtr, millis, nanos, true);
    MAY_THROW_VOID();
}

const DalvikNativeMethod dvm_java_lang_Object[] = {
    { "internalClone",  "(Ljava/lang/Cloneable;)Ljava/lang/Object;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Object_internalClone) },
    { "hashCode",       "()I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Object_hashCode) },
    { "notify",         "()V",
        (DalvikNativeFunc) FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Object_notify) },
    { "notifyAll",      "()V",
        (DalvikNativeFunc) FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Object_notifyAll) },
    { "wait",           "(JI)V",
        (DalvikNativeFunc) FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Object_wait) },
    { "getClass",       "()Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Object_getClass) },
    { NULL, NULL, NULL, NULL },
};
