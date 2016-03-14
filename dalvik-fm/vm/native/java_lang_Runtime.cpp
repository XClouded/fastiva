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
 * java.lang.Runtime
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif
#include <limits.h>
#include <unistd.h>

#ifdef FASTIVA
#include <java/lang/Runtime.inl>
#include <java/lang/ClassLoader.inl>
#include <dalvik_Kernel.h>
#endif
/*
 * public void gc()
 *
 * Initiate a gc.
 */
static void Dalvik_java_lang_Runtime_gc(const u4* args, JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	void fastiva_Dalvik_java_lang_Runtime_gc(java_lang_Runtime_p self);
	fastiva_Dalvik_java_lang_Runtime_gc(NULL);
}

void fastiva_Dalvik_java_lang_Runtime_gc(java_lang_Runtime_p pRuntime) {
#endif

    dvmCollectGarbage();
    RETURN_VOID();
}

/*
 * private static void nativeExit(int code)
 *
 * Runtime.exit() calls this after doing shutdown processing.  Runtime.halt()
 * uses this as well.
 */
static void Dalvik_java_lang_Runtime_nativeExit(const u4* args,
    JValue* pResult)
{
    int status = args[0];

#ifdef FASTIVA
	void fastiva_Dalvik_java_lang_Runtime_nativeExit(jint);
	fastiva_Dalvik_java_lang_Runtime_nativeExit(status);
}

void fastiva_Dalvik_java_lang_Runtime_nativeExit(jint status) {
#endif

    assert(dvmThreadSelf()->status == THREAD_NATIVE);
    if (gDvm.exitHook != NULL) {
        (*gDvm.exitHook)(status);     // not expected to return
        ALOGW("JNI exit hook returned");
    }
#if defined(WITH_JIT) && defined(WITH_JIT_TUNING)
    dvmCompilerDumpStats();
#endif
    ALOGD("Calling exit(%d)", status);
    exit(status);
}

/*
 * static String nativeLoad(String filename, ClassLoader loader, String ldLibraryPath)
 *
 * Load the specified full path as a dynamic library filled with
 * JNI-compatible methods. Returns null on success, or a failure
 * message on failure.
 */
static java_lang_String_p fastiva_Dalvik_java_lang_Runtime_nativeLoad_impl(java_lang_String_p fileNameObj, java_lang_ClassLoader_p classLoader, java_lang_String_p ldLibraryPathObj);

static void Dalvik_java_lang_Runtime_nativeLoad(const u4* args,
    JValue* pResult)
{
    StringObject* fileNameObj = (StringObject*) args[0];
    Object* classLoader = (Object*) args[1];
    StringObject* ldLibraryPathObj = (StringObject*) args[2];

#ifdef FASTIVA
	//java_lang_String_p fastiva_Dalvik_java_lang_Runtime_nativeLoad(java_lang_String_p, java_lang_ClassLoader_p);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_java_lang_Runtime_nativeLoad_impl((java_lang_String_p)fileNameObj, (java_lang_ClassLoader_p)classLoader, (java_lang_String_p)ldLibraryPathObj);
}

static Method* method_nativeLoad = NULL;

#if FASTIVA_TARGET_ANDROID_VERSION < 40400
java_lang_String_p fastiva_Dalvik_java_lang_Runtime_nativeLoad(java_lang_String_p fileNameObj, java_lang_ClassLoader_p classLoader) {
	java_lang_String_p ldLibraryPathObj = NULL;
	const char* nativeLoad_sig = "(Ljava/lang/String;Ljava/lang/ClassLoader;)Ljava/lang/String;";
#else 
java_lang_String_p fastiva_Dalvik_java_lang_Runtime_nativeLoad(java_lang_String_p fileNameObj, java_lang_ClassLoader_p classLoader, java_lang_String_p ldLibraryPathObj) {
	const char* nativeLoad_sig = "(Ljava/lang/String;Ljava/lang/ClassLoader;Ljava/lang/String;)Ljava/lang/String;";
#endif
	if (method_nativeLoad == NULL) {
		// CheckJni option 이 켜진 경우, dvmGetCurrentJNIMethod() 함수를 이용하여
		// 호출이 발생한 Native 함수를 검사한다.
		// 즉, Native 함수를 초훌하기 직전에는 반드시 native-method frame call-stack 에 존재해야 한다.
		method_nativeLoad = dvmFindDirectMethodByDescriptor(java_lang_Runtime_C$::getRawStatic$(), 
			"nativeLoad",         nativeLoad_sig);
                assert(method_nativeLoad != NULL);
	}
	Thread* self = dvmThreadSelf();
	dvmPushJNIFrame(self, method_nativeLoad);
	java_lang_String_p res = fastiva_Dalvik_java_lang_Runtime_nativeLoad_impl((java_lang_String_p)fileNameObj, (java_lang_ClassLoader_p)classLoader, ldLibraryPathObj);
	if (self->exception != NULL) {
		fastiva_dvmPopFrame(self);
		fastiva_popDalvikException(self);
	}
	fastiva_dvmPopFrame(self);
	return res;
}


java_lang_String_p fastiva_Dalvik_java_lang_Runtime_nativeLoad_impl(java_lang_String_p fileNameObj, java_lang_ClassLoader_p classLoader, java_lang_String_p ldLibraryPathObj) {

#endif

    assert(fileNameObj != NULL);
    char* fileName = dvmCreateCstrFromString(fileNameObj);

    if (ldLibraryPathObj != NULL) {
#ifdef _WIN32
		FASTIVA_DBREAK();
#else
        char* ldLibraryPath = dvmCreateCstrFromString(ldLibraryPathObj);
        void* sym = dlsym(RTLD_DEFAULT, "android_update_LD_LIBRARY_PATH");
        if (sym != NULL) {
            typedef void (*Fn)(const char*);
            Fn android_update_LD_LIBRARY_PATH = reinterpret_cast<Fn>(sym);
            (*android_update_LD_LIBRARY_PATH)(ldLibraryPath);
        } else {
            ALOGE("android_update_LD_LIBRARY_PATH not found; .so dependencies will not work!");
        }
        free(ldLibraryPath);
#endif
    }

    StringObject* result = NULL;
    char* reason = NULL;
    bool success; 

    success = dvmLoadNativeCode(fileName, classLoader, &reason);
    if (!success) {
        const char* msg = (reason != NULL) ? reason : "unknown failure";
        result = dvmCreateStringFromCstr(msg);
        dvmReleaseTrackedAlloc((Object*) result, NULL);
    }

    free(reason);
    free(fileName);

    RETURN_PTR((java_lang_String_p)result);
}

/*
 * public long maxMemory()
 *
 * Returns GC heap max memory in bytes.
 */
static void Dalvik_java_lang_Runtime_maxMemory(const u4* args, JValue* pResult)
{

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_java_lang_Runtime_maxMemory(java_lang_Runtime_p self);
	*(jlonglong*)pResult = fastiva_Dalvik_java_lang_Runtime_maxMemory(NULL);
}

jlonglong fastiva_Dalvik_java_lang_Runtime_maxMemory(java_lang_Runtime_p self) {
#endif

    RETURN_LONG(dvmGetHeapDebugInfo(kVirtualHeapMaximumSize));
}

/*
 * public long totalMemory()
 *
 * Returns GC heap total memory in bytes.
 */
static void Dalvik_java_lang_Runtime_totalMemory(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_java_lang_Runtime_totalMemory(java_lang_Runtime_p self);
	*(jlonglong*)pResult = fastiva_Dalvik_java_lang_Runtime_totalMemory(NULL);
}

jlonglong fastiva_Dalvik_java_lang_Runtime_totalMemory(java_lang_Runtime_p self) {
#endif

	RETURN_LONG(dvmGetHeapDebugInfo(kVirtualHeapSize));
}

/*
 * public long freeMemory()
 *
 * Returns GC heap free memory in bytes.
 */
static void Dalvik_java_lang_Runtime_freeMemory(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_java_lang_Runtime_freeMemory(java_lang_Runtime_p self);
	*(jlonglong*)pResult = fastiva_Dalvik_java_lang_Runtime_freeMemory(NULL);
}

jlonglong fastiva_Dalvik_java_lang_Runtime_freeMemory(java_lang_Runtime_p self) {
#endif

	size_t size = dvmGetHeapDebugInfo(kVirtualHeapSize);
    size_t allocated = dvmGetHeapDebugInfo(kVirtualHeapAllocated);
    long long result = size - allocated;
    if (result < 0) {
        result = 0;
    }
    RETURN_LONG(result);
}

const DalvikNativeMethod dvm_java_lang_Runtime[] = {
    { "freeMemory",          "()J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Runtime_freeMemory) },
    { "gc",                 "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Runtime_gc) },
    { "maxMemory",          "()J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Runtime_maxMemory) },
    { "nativeExit",         "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Runtime_nativeExit) },
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
    { "nativeLoad",         "(Ljava/lang/String;Ljava/lang/ClassLoader;)Ljava/lang/String;",
#else
    { "nativeLoad",         "(Ljava/lang/String;Ljava/lang/ClassLoader;Ljava/lang/String;)Ljava/lang/String;",
#endif
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Runtime_nativeLoad) },
    { "totalMemory",          "()J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Runtime_totalMemory) },
    { NULL, NULL, NULL, NULL },
};
