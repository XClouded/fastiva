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
 * dalvik.system.VMStack
 */
#include "Dalvik.h"
#include "UniquePtr.h"
#include "native/InternalNativePriv.h"

#ifdef FASTIVA
#include <java/lang/Thread.h>
#include <dalvik_Kernel.h>
#define FASTIVA_ENABLE_CALLER_APP_CLASS_STACK 1
#endif


/*
 * public static ClassLoader getCallingClassLoader()
 *
 * Return the defining class loader of the caller's caller.
 */
static void Dalvik_dalvik_system_VMStack_getCallingClassLoader(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	java_lang_ClassLoader_p fastiva_Dalvik_dalvik_system_VMStack_getCallingClassLoader();
	*(java_lang_ClassLoader_p*)pResult = fastiva_Dalvik_dalvik_system_VMStack_getCallingClassLoader();
}

java_lang_ClassLoader_p fastiva_Dalvik_dalvik_system_VMStack_getCallingClassLoader() {
	ClassObject* d2f_getCallerClass(Thread* self);
	ClassObject* clazz = d2f_getCallerClass(dvmThreadSelf());
	if (clazz == NULL) {
	    RETURN_PTR(NULL);
	}
	
	if (0 && fastiva_isSystemClass((fastiva_Class_p)clazz) && kernelData.g_appModule != NULL) {
		return kernelData.g_appModule->m_pClassLoader;
	}
	else {
	    RETURN_PTR((java_lang_ClassLoader_p)clazz->classLoader);
	}
	//assert(kernelData.g_appModule == NULL ||
	//	kernelData.g_appModule->m_pClassLoader == NULL);
#else
	// @zee A caller Method -> A Privileged method -> this method
	ClassObject* clazz =
        dvmGetCaller2Class(dvmThreadSelf()->interpSave.curFrame);

    if (clazz == NULL)
        RETURN_PTR(NULL);
    RETURN_PTR((java_lang_ClassLoader_p)clazz->classLoader);
#endif
}

/*
 * public static Class<?> getStackClass2()
 *
 * Returns the class of the caller's caller's caller.
 */
static void Dalvik_dalvik_system_VMStack_getStackClass2(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_dalvik_system_VMStack_getStackClass2();
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_dalvik_system_VMStack_getStackClass2();
}

java_lang_Class_p fastiva_Dalvik_dalvik_system_VMStack_getStackClass2() {
	ClassObject* d2f_getCallerClass(Thread* self);
	ClassObject* clazz = d2f_getCallerClass(dvmThreadSelf());
#else
	ClassObject* clazz =
        dvmGetCaller3Class(dvmThreadSelf()->interpSave.curFrame);
#endif
    RETURN_PTR((java_lang_Class_p)clazz);
}

/*
 * public static Class<?>[] getClasses(int maxDepth)
 *
 * Create an array of classes for the methods on the stack, skipping the
 * first two and all reflection methods.  If "stopAtPrivileged" is set,
 * stop shortly after we encounter a privileged class.
 */
static void Dalvik_dalvik_system_VMStack_getClasses(const u4* args,
    JValue* pResult)
{
    /* note "maxSize" is unsigned, so -1 turns into a very large value */
    size_t maxSize = args[0];

#ifdef FASTIVA
	java_lang_Class_ap fastiva_Dalvik_dalvik_system_VMStack_getClasses(jint);
	*(java_lang_Class_ap*)pResult = fastiva_Dalvik_dalvik_system_VMStack_getClasses(maxSize);
}

java_lang_Class_ap fastiva_Dalvik_dalvik_system_VMStack_getClasses(jint arg0) {
	size_t maxSize = arg0;
    const size_t kSkip = 0;
#else
    const size_t kSkip = 2;
#endif

	size_t size = 0;

    /*
     * Get an array with the stack trace in it.
     */
    void *fp = dvmThreadSelf()->interpSave.curFrame;
    size_t depth = dvmComputeExactFrameDepth(fp);
	FASTIVA_ASSERT(depth < 1024);
    UniquePtr<const Method*[]> methods(new const Method*[depth]);
    dvmFillStackTraceArray(fp, methods.get(), depth);

    /*
     * Run through the array and count up how many elements there are.
     */
    for (size_t i = kSkip; i < depth && size < maxSize; ++i) {
        const Method* meth = methods[i];

		if (dvmIsReflectionMethod(meth))
            continue;

        size++;
    }

#ifdef FASTIVA
	Thread* self = dvmThreadSelf();
	fastiva_Class_p pCaller = self->m_pCallerClass;
#ifdef FASTIVA_ENABLE_CALLER_APP_CLASS_STACK
	fastiva_Class_p pAppCaller = NULL;
	if (kernelData.g_appModule != NULL) {
		pAppCaller = kernelData.g_appModule->m_classes.m_aSlot->m_pClass;
		size ++;
	}
	if (pCaller != NULL) {
		size ++;
	}
#endif
#endif
    /*
     * Create an array object to hold the classes.
     * TODO: can use gDvm.classJavaLangClassArray here?
     */
    ClassObject* classArrayClass = dvmFindArrayClass("[Ljava/lang/Class;",
                                                     NULL);
    if (classArrayClass == NULL) {
        ALOGW("Unable to find java.lang.Class array class");
        THROW_V();
    }
    ArrayObject* classes = dvmAllocArrayByClass(classArrayClass,
                                                size,
                                                ALLOC_DEFAULT);
    if (classes == NULL) {
        ALOGW("Unable to allocate class array of %zd elements", size);
        THROW_V();
    }

#ifdef _DEBUG
#ifdef FASTIVA_ENABLE_CALLER_APP_CLASS_STACK
	ALOGE("## Fastiva Call Stack %s, %s, %d", pAppCaller != NULL ? pAppCaller->descriptor : "Nothing", 
									pCaller != NULL ? pCaller->descriptor : "Nothing", 
									pCaller != NULL ? fastiva_isSystemClass(pCaller) : 0);
#endif
#endif
    /*
     * Fill in the array.
     */
    size_t objCount = 0;
#ifdef FASTIVA
#ifdef FASTIVA_ENABLE_CALLER_APP_CLASS_STACK
	if (pAppCaller != NULL) {
        dvmSetObjectArrayElement(classes, objCount++, pAppCaller);
	}
#endif
	if (pCaller != NULL) {
        dvmSetObjectArrayElement(classes, objCount++, pCaller);
	}
#endif
    for (size_t i = kSkip; i < depth; ++i) {
        if (dvmIsReflectionMethod(methods[i])) {
            continue;
        }
        Object* klass = (Object *)methods[i]->clazz;
#ifdef _DEBUG
		if (pCaller == NULL) {
			ALOGE("## In Call Stack %s", ((ClassObject*)klass)->descriptor);
		}
#endif
        dvmSetObjectArrayElement(classes, objCount, klass);
        objCount++;
    }
#ifdef FASTIVA_ENABLE_CALLER_APP_CLASS_STACK
#ifdef FASTIVA
	//if (self->m_pCallerAppClass != NULL) {
    //    dvmSetObjectArrayElement(classes, objCount++, self->m_pCallerAppClass);
	//}
#endif
#endif
    assert(objCount == classes->length);

    dvmReleaseTrackedAlloc((Object*)classes, NULL);
    RETURN_PTR((java_lang_Class_ap)classes);
}

/*
 * Return a trace buffer for the specified thread or NULL if the
 * thread is not still alive. *depth is set to the length of a
 * non-NULL trace buffer. Caller is responsible for freeing the trace
 * buffer.
 */
static int* getTraceBuf(Object* targetThreadObj, size_t* pStackDepth)
{
    Thread* self = dvmThreadSelf();
    Thread* thread;
    int* traceBuf;

    assert(targetThreadObj != NULL);

    dvmLockThreadList(self);

    /*
     * Make sure the thread is still alive and in the list.
     */
    for (thread = gDvm.threadList; thread != NULL; thread = thread->next) {
        if (thread->threadObj == targetThreadObj)
            break;
    }
    if (thread == NULL) {
        ALOGI("VMStack.getTraceBuf: threadObj %p not active",
            targetThreadObj);
        dvmUnlockThreadList();
        return NULL;
    }

    /*
     * Suspend the thread, pull out the stack trace, then resume the thread
     * and release the thread list lock.  If we're being asked to examine
     * our own stack trace, skip the suspend/resume.
     */
    if (thread != self)
        dvmSuspendThread(thread);
    traceBuf = dvmFillInStackTraceRaw(thread, pStackDepth);
    if (thread != self)
        dvmResumeThread(thread);
    dvmUnlockThreadList();

    return traceBuf;
}

/*
 * public static StackTraceElement[] getThreadStackTrace(Thread t)
 *
 * Retrieve the stack trace of the specified thread and return it as an
 * array of StackTraceElement.  Returns NULL on failure.
 */
static void Dalvik_dalvik_system_VMStack_getThreadStackTrace(const u4* args,
    JValue* pResult)
{
    Object* targetThreadObj = (Object*) args[0];

#ifdef FASTIVA
	java_lang_StackTraceElement_ap fastiva_Dalvik_dalvik_system_VMStack_getThreadStackTrace(java_lang_Thread_p);
	*(java_lang_StackTraceElement_ap*)pResult = fastiva_Dalvik_dalvik_system_VMStack_getThreadStackTrace((java_lang_Thread_p)targetThreadObj);
}

java_lang_StackTraceElement_ap fastiva_Dalvik_dalvik_system_VMStack_getThreadStackTrace(java_lang_Thread_p targetThreadObj) {
#endif

	size_t stackDepth;
    int* traceBuf = getTraceBuf(targetThreadObj, &stackDepth);

    if (traceBuf == NULL)
        RETURN_PTR(NULL);

    /*
     * Convert the raw buffer into an array of StackTraceElement.
     */
    ArrayObject* trace = dvmGetStackTraceRaw(traceBuf, stackDepth);
    free(traceBuf);
    RETURN_PTR((java_lang_StackTraceElement_ap)trace);
}

/*
 * public static int fillStackTraceElements(Thread t, StackTraceElement[] stackTraceElements)
 *
 * Retrieve a partial stack trace of the specified thread and return
 * the number of frames filled.  Returns 0 on failure.
 */
static void Dalvik_dalvik_system_VMStack_fillStackTraceElements(const u4* args,
    JValue* pResult)
{
    Object* targetThreadObj = (Object*) args[0];
    ArrayObject* steArray = (ArrayObject*) args[1];

#ifdef FASTIVA
	jint fastiva_Dalvik_dalvik_system_VMStack_fillStackTraceElements(java_lang_Thread_p, java_lang_StackTraceElement_ap);
	*(jint*)pResult = fastiva_Dalvik_dalvik_system_VMStack_fillStackTraceElements((java_lang_Thread_p)targetThreadObj, (java_lang_StackTraceElement_ap)steArray);
}

jint fastiva_Dalvik_dalvik_system_VMStack_fillStackTraceElements(java_lang_Thread_p targetThreadObj, java_lang_StackTraceElement_ap arg1) {
    ArrayObject* steArray = (ArrayObject*) arg1;
#endif

	size_t stackDepth;
    int* traceBuf = getTraceBuf(targetThreadObj, &stackDepth);

    if (traceBuf == NULL)
        RETURN_PTR((jint)NULL);

    /*
     * Set the raw buffer into an array of StackTraceElement.
     */
    if (stackDepth > steArray->length) {
        stackDepth = steArray->length;
    }
    dvmFillStackTraceElements(traceBuf, stackDepth, steArray);
    free(traceBuf);
    RETURN_INT(stackDepth);
}


#if defined FASTIVA && FASTIVA_TARGET_ANDROID_VERSION >= 40400

static void Dalvik_dalvik_system_VMStack_getClosestUserClassLoader(const u4* args,
    JValue* pResult)
{
    java_lang_ClassLoader_p bootstrap = (java_lang_ClassLoader_p) args[0];
	java_lang_ClassLoader_p system = (java_lang_ClassLoader_p) args[1];

	java_lang_ClassLoader_p fastiva_Dalvik_dalvik_system_VMStack_getClosestUserClassLoader(java_lang_ClassLoader_p bootstrap, java_lang_ClassLoader_p system);
	*(java_lang_ClassLoader_p*)pResult = fastiva_Dalvik_dalvik_system_VMStack_getClosestUserClassLoader(bootstrap, system);
}


java_lang_ClassLoader_p fastiva_Dalvik_dalvik_system_VMStack_getClosestUserClassLoader(java_lang_ClassLoader_p bootstrap, java_lang_ClassLoader_p system) {
	java_lang_Class_ap classes = fastiva_Dalvik_dalvik_system_VMStack_getClasses(-1);
	for (int i = 0; i < classes->length(); i ++) {
		java_lang_ClassLoader_p pLoader = (java_lang_ClassLoader_p)classes->get$(i)->classLoader;
		if (pLoader != NULL && pLoader != bootstrap && pLoader != system) {
			return pLoader;
		}
	}
	Thread* self = dvmThreadSelf();
	java_lang_Thread_p pThread = (java_lang_Thread_p)self->threadObj;
	return pThread->get__contextClassLoader();
}

#endif

const DalvikNativeMethod dvm_dalvik_system_VMStack[] = {
    { "getCallingClassLoader",  "()Ljava/lang/ClassLoader;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMStack_getCallingClassLoader) },
    { "getStackClass2",         "()Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMStack_getStackClass2) },
    { "getClasses",             "(I)[Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMStack_getClasses) },
    { "getThreadStackTrace",    "(Ljava/lang/Thread;)[Ljava/lang/StackTraceElement;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMStack_getThreadStackTrace) },
    { "fillStackTraceElements", "(Ljava/lang/Thread;[Ljava/lang/StackTraceElement;)I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMStack_fillStackTraceElements) },
#if defined FASTIVA && FASTIVA_TARGET_ANDROID_VERSION >= 40400
    { "getClosestUserClassLoader", "(Ljava/lang/ClassLoader;Ljava/lang/ClassLoader;)Ljava/lang/ClassLoader;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMStack_getClosestUserClassLoader) },
#endif
    { NULL, NULL, NULL, NULL },
};

