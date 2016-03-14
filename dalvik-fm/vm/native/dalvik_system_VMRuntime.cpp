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
 * dalvik.system.VMRuntime
 */
#include "Dalvik.h"
#include "ScopedPthreadMutexLock.h"
#include "UniquePtr.h"
#include "alloc/HeapSource.h"
#include "alloc/Visit.h"
#include "libdex/DexClass.h"
#include "native/InternalNativePriv.h"

#if 0 // FASTIVA_TARGET_ANDROID_VERSION < 40400
#include <cutils/array.h>
#endif
#include <limits.h>

#include <map>

#ifdef FASTIVA
#include <dalvik/system/VMRuntime.inl>
#endif

/*
 * public native float getTargetHeapUtilization()
 *
 * Gets the current ideal heap utilization, represented as a number
 * between zero and one.
 */
static void Dalvik_dalvik_system_VMRuntime_getTargetHeapUtilization(
    const u4* args, JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jfloat fastiva_Dalvik_dalvik_system_VMRuntime_getTargetHeapUtilization(dalvik_system_VMRuntime_p self);
	*(jfloat*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_getTargetHeapUtilization((dalvik_system_VMRuntime_p)args[0]);
}

jfloat fastiva_Dalvik_dalvik_system_VMRuntime_getTargetHeapUtilization(dalvik_system_VMRuntime_p self) {
#endif

    RETURN_FLOAT(dvmGetTargetHeapUtilization());
}

/*
 * native float nativeSetTargetHeapUtilization()
 *
 * Sets the current ideal heap utilization, represented as a number
 * between zero and one.  Returns the old utilization.
 *
 * Note that this is NOT static.
 */
static void Dalvik_dalvik_system_VMRuntime_nativeSetTargetHeapUtilization(
    const u4* args, JValue* pResult)
{
	jfloat v = dvmU4ToFloat(args[1]);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_nativeSetTargetHeapUtilization(dalvik_system_VMRuntime_p self, jfloat);
	fastiva_Dalvik_dalvik_system_VMRuntime_nativeSetTargetHeapUtilization((dalvik_system_VMRuntime_p)args[0], v);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_nativeSetTargetHeapUtilization(dalvik_system_VMRuntime_p self, jfloat v) {
#endif

	dvmSetTargetHeapUtilization(v);

    RETURN_VOID();
}

/*
 * public native void startJitCompilation()
 *
 * Callback function from the framework to indicate that an app has gone
 * through the startup phase and it is time to enable the JIT compiler.
 */
static void Dalvik_dalvik_system_VMRuntime_startJitCompilation(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_startJitCompilation(dalvik_system_VMRuntime_p self);
	fastiva_Dalvik_dalvik_system_VMRuntime_startJitCompilation((dalvik_system_VMRuntime_p)args[0]);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_startJitCompilation(dalvik_system_VMRuntime_p self) {
#endif

#if defined(WITH_JIT)
    if (gDvm.executionMode == kExecutionModeJit && gDvmJit.disableJit == false) {
        ScopedPthreadMutexLock lock(&gDvmJit.compilerLock);
        gDvmJit.alreadyEnabledViaFramework = true;
        pthread_cond_signal(&gDvmJit.compilerQueueActivity);
    }
#endif
    RETURN_VOID();
}

/*
 * public native void disableJitCompilation()
 *
 * Callback function from the framework to indicate that a VM instance wants to
 * permanently disable the JIT compiler. Currently only the system server uses
 * this interface when it detects system-wide safe mode is enabled.
 */
static void Dalvik_dalvik_system_VMRuntime_disableJitCompilation(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_disableJitCompilation(dalvik_system_VMRuntime_p self);
	fastiva_Dalvik_dalvik_system_VMRuntime_disableJitCompilation((dalvik_system_VMRuntime_p)args[0]);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_disableJitCompilation(dalvik_system_VMRuntime_p self) {
#endif

#if defined(WITH_JIT)
    if (gDvm.executionMode == kExecutionModeJit) {
        gDvmJit.disableJit = true;
    }
#endif
    RETURN_VOID();
}

static void Dalvik_dalvik_system_VMRuntime_newNonMovableArray(const u4* args,
    JValue* pResult)
{
    ClassObject* elementClass = (ClassObject*) args[1];
    int length = args[2];


#ifdef FASTIVA
	java_lang_Object_p fastiva_Dalvik_dalvik_system_VMRuntime_newNonMovableArray(dalvik_system_VMRuntime_p self, java_lang_Class_p, jint);
	*(java_lang_Object_p*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_newNonMovableArray((dalvik_system_VMRuntime_p)args[0], (java_lang_Class_p)elementClass, length);
}

java_lang_Object_p fastiva_Dalvik_dalvik_system_VMRuntime_newNonMovableArray(dalvik_system_VMRuntime_p self, java_lang_Class_p elementClass, jint length) {
#endif

	if (elementClass == NULL) {
        dvmThrowNullPointerException("elementClass == null");
        THROW_V();
    }
    if (length < 0) {
        dvmThrowNegativeArraySizeException(length);
        THROW_V();
    }

    // TODO: right now, we don't have a copying collector, so there's no need
    // to do anything special here, but we ought to pass the non-movability
    // through to the allocator.
    ClassObject* arrayClass = dvmFindArrayClassForElement(elementClass);
    ArrayObject* newArray = dvmAllocArrayByClass(arrayClass,
                                                 length,
                                                 ALLOC_NON_MOVING);
    if (newArray == NULL) {
        assert(dvmCheckException(dvmThreadSelf()));
        THROW_V();
    }
    dvmReleaseTrackedAlloc((Object*) newArray, NULL);

    RETURN_PTR(newArray);
}

static void Dalvik_dalvik_system_VMRuntime_addressOf(const u4* args,
    JValue* pResult)
{
    ArrayObject* array = (ArrayObject*) args[1];

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_dalvik_system_VMRuntime_addressOf(dalvik_system_VMRuntime_p, java_lang_Object_p);
	*(jlonglong*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_addressOf((dalvik_system_VMRuntime_p)args[0], (java_lang_Object_p)array);
}

jlonglong fastiva_Dalvik_dalvik_system_VMRuntime_addressOf(dalvik_system_VMRuntime_p self, java_lang_Object_p arg1) {
    ArrayObject* array = (ArrayObject*) arg1;
#endif

	if (!dvmIsArray(array)) {
        dvmThrowIllegalArgumentException(NULL);
        THROW_V();
    }
    // TODO: we should also check that this is a non-movable array.
    s8 result = (uintptr_t) array->contents;
    RETURN_LONG(result);
}

static void Dalvik_dalvik_system_VMRuntime_clearGrowthLimit(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_clearGrowthLimit(dalvik_system_VMRuntime_p self);
	fastiva_Dalvik_dalvik_system_VMRuntime_clearGrowthLimit((dalvik_system_VMRuntime_p)args[0]);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_clearGrowthLimit(dalvik_system_VMRuntime_p self) {
#endif

	dvmClearGrowthLimit();
    RETURN_VOID();
}

static void Dalvik_dalvik_system_VMRuntime_isDebuggerActive(
    const u4* args, JValue* pResult)
{

#ifdef FASTIVA
	jbool fastiva_Dalvik_dalvik_system_VMRuntime_isDebuggerActive(dalvik_system_VMRuntime_p self);
	*(jbool*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_isDebuggerActive((dalvik_system_VMRuntime_p)args[0]);
}

jbool fastiva_Dalvik_dalvik_system_VMRuntime_isDebuggerActive(dalvik_system_VMRuntime_p self) {
#endif

	RETURN_BOOLEAN(gDvm.debuggerActive || gDvm.nativeDebuggerActive);
}

static void Dalvik_dalvik_system_VMRuntime_properties(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	java_lang_String_ap fastiva_Dalvik_dalvik_system_VMRuntime_properties(dalvik_system_VMRuntime_p self);
	*(java_lang_String_ap*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_properties((dalvik_system_VMRuntime_p)args[0]);
}

java_lang_String_ap fastiva_Dalvik_dalvik_system_VMRuntime_properties(dalvik_system_VMRuntime_p self) {
#endif

    ArrayObject* result = dvmCreateStringArray(*gDvm.properties);
    dvmReleaseTrackedAlloc((Object*) result, dvmThreadSelf());
    RETURN_PTR((java_lang_String_ap)result);
}

#ifdef FASTIVA
#define returnCString(pResult, s) return fastiva_makeString(s)
java_lang_String_p fastiva_makeString(const char* s)
{
    Object* result = (Object*) dvmCreateStringFromCstr(s);
    dvmReleaseTrackedAlloc(result, dvmThreadSelf());
    RETURN_PTR((java_lang_String_p)result);
}
#else
static void returnCString(JValue* pResult, const char* s)
{
    Object* result = (Object*) dvmCreateStringFromCstr(s);
    dvmReleaseTrackedAlloc(result, dvmThreadSelf());
    DALVIK_RETURN_PTR(result);
}
#endif

static void Dalvik_dalvik_system_VMRuntime_bootClassPath(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_bootClassPath(dalvik_system_VMRuntime_p self);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_bootClassPath((dalvik_system_VMRuntime_p)args[0]);
}

java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_bootClassPath(dalvik_system_VMRuntime_p self) {
#endif

	returnCString(pResult, gDvm.bootClassPathStr);
}

static void Dalvik_dalvik_system_VMRuntime_classPath(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_classPath(dalvik_system_VMRuntime_p self);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_classPath((dalvik_system_VMRuntime_p)args[0]);
}

java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_classPath(dalvik_system_VMRuntime_p self) {
#endif

	returnCString(pResult, gDvm.classPathStr);
}

static void Dalvik_dalvik_system_VMRuntime_vmVersion(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_vmVersion(dalvik_system_VMRuntime_p self);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_vmVersion((dalvik_system_VMRuntime_p)args[0]);
}

java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_vmVersion(dalvik_system_VMRuntime_p self) {
#endif

	char buf[64];
    sprintf(buf, "%d.%d.%d",
            DALVIK_MAJOR_VERSION, DALVIK_MINOR_VERSION, DALVIK_BUG_VERSION);
    returnCString(pResult, buf);
}

#if FASTIVA_TARGET_ANDROID_VERSION >= 40400
static void Dalvik_dalvik_system_VMRuntime_vmLibrary(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_vmLibrary(dalvik_system_VMRuntime_p self);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_dalvik_system_VMRuntime_vmLibrary((dalvik_system_VMRuntime_p)args[0]);
}

java_lang_String_p fastiva_Dalvik_dalvik_system_VMRuntime_vmLibrary(dalvik_system_VMRuntime_p self) {
#endif

    returnCString(pResult, "libdvm.so");
}
#endif

#ifdef G_WATCH
static void Dalvik_dalvik_system_VMRuntime_updateProcessState(const u4* args,
    JValue* pResult)
{
    // This is the target SDK version of the app we're about to run.
    // Note that this value may be CUR_DEVELOPMENT (10000).
    // Note that this value may be 0, meaning "current".
    int iArg = args[1];
#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_updateProcessState(jint);
	fastiva_Dalvik_dalvik_system_VMRuntime_updateProcessState(iArg);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_updateProcessState(jint) {
#endif
	ALOGD("Fastiva VMRuntime_updateProcessState not implemented");
}
#endif

static void Dalvik_dalvik_system_VMRuntime_setTargetSdkVersion(const u4* args,
    JValue* pResult)
{
    // This is the target SDK version of the app we're about to run.
    // Note that this value may be CUR_DEVELOPMENT (10000).
    // Note that this value may be 0, meaning "current".
    int targetSdkVersion = args[1];

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_setTargetSdkVersion(dalvik_system_VMRuntime_p self, jint);
	fastiva_Dalvik_dalvik_system_VMRuntime_setTargetSdkVersion((dalvik_system_VMRuntime_p)args[0], targetSdkVersion);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_setTargetSdkVersion(dalvik_system_VMRuntime_p self, jint targetSdkVersion) {
#endif

    if (targetSdkVersion > 0 && targetSdkVersion <= 13 /* honeycomb-mr2 */) {
        if (gDvmJni.useCheckJni) {
            ALOGI("CheckJNI enabled: not enabling JNI app bug workarounds.");
        } else {
            ALOGI("Enabling JNI app bug workarounds for target SDK version %i...",
                  targetSdkVersion);
            gDvmJni.workAroundAppJniBugs = true;
        }
    }
    RETURN_VOID();
}

#if FASTIVA_TARGET_ANDROID_VERSION >= 40400
static void Dalvik_dalvik_system_VMRuntime_registerNativeAllocation(const u4* args,
                                                                    JValue* pResult)
{
  int bytes = args[1];

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_registerNativeAllocation(dalvik_system_VMRuntime_p self, jint);
	fastiva_Dalvik_dalvik_system_VMRuntime_registerNativeAllocation((dalvik_system_VMRuntime_p)args[0], bytes);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_registerNativeAllocation(dalvik_system_VMRuntime_p self, jint bytes) {
#endif  
  if (bytes < 0) {
    dvmThrowRuntimeException("allocation size negative");
  } else {
    dvmHeapSourceRegisterNativeAllocation(bytes);
  }
  RETURN_VOID();
}

static void Dalvik_dalvik_system_VMRuntime_registerNativeFree(const u4* args,
                                                              JValue* pResult)
{
  int bytes = args[1];
#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_registerNativeFree(dalvik_system_VMRuntime_p self, jint);
	fastiva_Dalvik_dalvik_system_VMRuntime_registerNativeFree((dalvik_system_VMRuntime_p)args[0], bytes);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_registerNativeFree(dalvik_system_VMRuntime_p self, jint bytes) {
#endif  
  if (bytes < 0) {
    dvmThrowRuntimeException("allocation size negative");
  } else {
    dvmHeapSourceRegisterNativeFree(bytes);
  }
  RETURN_VOID();
}

static DvmDex* getDvmDexFromClassPathEntry(ClassPathEntry* cpe) {
#ifdef FASTIVA
	assert("getDvmDexFromClassPathEntry not supported" == NULL);
#endif
    if (cpe->kind == kCpeDex) {
        return ((RawDexFile*) cpe->ptr)->pDvmDex;
    }
    if (cpe->kind == kCpeJar) {
        return ((JarFile*) cpe->ptr)->pDvmDex;
    }
    LOG_ALWAYS_FATAL("Unknown cpe->kind=%d", cpe->kind);
}

typedef std::map<std::string, StringObject*> StringTable;

static void preloadDexCachesStringsVisitor(void* addr, u4 threadId, RootType type, void* arg) {
    StringTable& table = *(StringTable*) arg;
    StringObject* strObj = *(StringObject**) addr;
    LOG_FATAL_IF(strObj->clazz != gDvm.classJavaLangString, "Unknown class for supposed string");
    char* newStr = dvmCreateCstrFromString(strObj);
    // ALOGI("VMRuntime.preloadDexCaches interned=%s", newStr);
    table[newStr] = strObj;
    free(newStr);
}

// Based on dvmResolveString.
static void preloadDexCachesResolveString(DvmDex* pDvmDex,
                                          uint32_t stringIdx,
                                          StringTable& strings) {
    StringObject* string = dvmDexGetResolvedString(pDvmDex, stringIdx);
    if (string != NULL) {
        return;
    }
    const DexFile* pDexFile = pDvmDex->pDexFile;
    uint32_t utf16Size;
    const char* utf8 = dexStringAndSizeById(pDexFile, stringIdx, &utf16Size);
    string = strings[utf8];
    if (string == NULL) {
        return;
    }
    // ALOGI("VMRuntime.preloadDexCaches found string=%s", utf8);
    dvmDexSetResolvedString(pDvmDex, stringIdx, string);
}

// Based on dvmResolveClass.
static void preloadDexCachesResolveType(DvmDex* pDvmDex, uint32_t typeIdx) {
    ClassObject* clazz = dvmDexGetResolvedClass(pDvmDex, typeIdx);
    if (clazz != NULL) {
        return;
    }
    const DexFile* pDexFile = pDvmDex->pDexFile;
    const char* className = dexStringByTypeIdx(pDexFile, typeIdx);
    if (className[0] != '\0' && className[1] == '\0') {
        /* primitive type */
        clazz = dvmFindPrimitiveClass(className[0]);
    } else {
        clazz = dvmLookupClass(className, NULL, true);
    }
    if (clazz == NULL) {
        return;
    }
    // Skip uninitialized classes because filled cache entry implies it is initialized.
    if (!dvmIsClassInitialized(clazz)) {
        // ALOGI("VMRuntime.preloadDexCaches uninitialized clazz=%s", className);
        return;
    }
    // ALOGI("VMRuntime.preloadDexCaches found clazz=%s", className);
    dvmDexSetResolvedClass(pDvmDex, typeIdx, clazz);
}

// Based on dvmResolveInstField/dvmResolveStaticField.
static void preloadDexCachesResolveField(DvmDex* pDvmDex, uint32_t fieldIdx, bool instance) {
    Field* field = dvmDexGetResolvedField(pDvmDex, fieldIdx);
    if (field != NULL) {
        return;
    }
    const DexFile* pDexFile = pDvmDex->pDexFile;
    const DexFieldId* pFieldId = dexGetFieldId(pDexFile, fieldIdx);
    ClassObject* clazz = dvmDexGetResolvedClass(pDvmDex, pFieldId->classIdx);
    if (clazz == NULL) {
        return;
    }
    // Skip static fields for uninitialized classes because a filled
    // cache entry implies the class is initialized.
    if (!instance && !dvmIsClassInitialized(clazz)) {
        return;
    }
    const char* fieldName = dexStringById(pDexFile, pFieldId->nameIdx);
    const char* signature = dexStringByTypeIdx(pDexFile, pFieldId->typeIdx);
    if (instance) {
        field = dvmFindInstanceFieldHier(clazz, fieldName, signature);
    } else {
        field = dvmFindStaticFieldHier(clazz, fieldName, signature);
    }
    if (field == NULL) {
        return;
    }
    // ALOGI("VMRuntime.preloadDexCaches found field %s %s.%s",
    //       signature, clazz->descriptor, fieldName);
    dvmDexSetResolvedField(pDvmDex, fieldIdx, field);
}

// Based on dvmResolveMethod.
static void preloadDexCachesResolveMethod(DvmDex* pDvmDex,
                                          uint32_t methodIdx,
                                          MethodType methodType) {
    Method* method = dvmDexGetResolvedMethod(pDvmDex, methodIdx);
    if (method != NULL) {
        return;
    }
    const DexFile* pDexFile = pDvmDex->pDexFile;
    const DexMethodId* pMethodId = dexGetMethodId(pDexFile, methodIdx);
    ClassObject* clazz = dvmDexGetResolvedClass(pDvmDex, pMethodId->classIdx);
    if (clazz == NULL) {
        return;
    }
    // Skip static methods for uninitialized classes because a filled
    // cache entry implies the class is initialized.
    if ((methodType == METHOD_STATIC) && !dvmIsClassInitialized(clazz)) {
        return;
    }
    const char* methodName = dexStringById(pDexFile, pMethodId->nameIdx);
    DexProto proto;
    dexProtoSetFromMethodId(&proto, pDexFile, pMethodId);

    if (methodType == METHOD_DIRECT) {
        method = dvmFindDirectMethod(clazz, methodName, &proto);
    } else if (methodType == METHOD_STATIC) {
        method = dvmFindDirectMethodHier(clazz, methodName, &proto);
    } else {
        method = dvmFindVirtualMethodHier(clazz, methodName, &proto);
    }
    if (method == NULL) {
        return;
    }
    // ALOGI("VMRuntime.preloadDexCaches found method %s.%s",
    //        clazz->descriptor, methodName);
    dvmDexSetResolvedMethod(pDvmDex, methodIdx, method);
}

struct DexCacheStats {
    uint32_t numStrings;
    uint32_t numTypes;
    uint32_t numFields;
    uint32_t numMethods;
    DexCacheStats() : numStrings(0), numTypes(0), numFields(0), numMethods(0) {};
};


#ifdef FASTIVA
static const bool kPreloadDexCachesEnabled = false;
#else
static const bool kPreloadDexCachesEnabled = true;
#endif

// Disabled because it takes a long time (extra half second) but
// gives almost no benefit in terms of saving private dirty pages.
static const bool kPreloadDexCachesStrings = false;

static const bool kPreloadDexCachesTypes = true;
static const bool kPreloadDexCachesFieldsAndMethods = true;

#ifdef FASTIVA
static const bool kPreloadDexCachesCollectStats = false;
#else
static const bool kPreloadDexCachesCollectStats = false;
#endif

static void preloadDexCachesStatsTotal(DexCacheStats* total) {
    if (!kPreloadDexCachesCollectStats) {
        return;
    }

    for (ClassPathEntry* cpe = gDvm.bootClassPath; cpe->kind != kCpeLastEntry; cpe++) {
        DvmDex* pDvmDex = getDvmDexFromClassPathEntry(cpe);
        const DexHeader* pHeader = pDvmDex->pHeader;
        total->numStrings += pHeader->stringIdsSize;
        total->numFields += pHeader->fieldIdsSize;
        total->numMethods += pHeader->methodIdsSize;
        total->numTypes += pHeader->typeIdsSize;
    }
}

static void preloadDexCachesStatsFilled(DexCacheStats* filled) {
    if (!kPreloadDexCachesCollectStats) {
        return;
    }
    for (ClassPathEntry* cpe = gDvm.bootClassPath; cpe->kind != kCpeLastEntry; cpe++) {
        DvmDex* pDvmDex = getDvmDexFromClassPathEntry(cpe);
        const DexHeader* pHeader = pDvmDex->pHeader;
        for (size_t i = 0; i < pHeader->stringIdsSize; i++) {
            StringObject* string = dvmDexGetResolvedString(pDvmDex, i);
            if (string != NULL) {
                filled->numStrings++;
            }
        }
        for (size_t i = 0; i < pHeader->typeIdsSize; i++) {
            ClassObject* clazz = dvmDexGetResolvedClass(pDvmDex, i);
            if (clazz != NULL) {
                filled->numTypes++;
            }
        }
        for (size_t i = 0; i < pHeader->fieldIdsSize; i++) {
            Field* field = dvmDexGetResolvedField(pDvmDex, i);
            if (field != NULL) {
                filled->numFields++;
            }
        }
        for (size_t i = 0; i < pHeader->methodIdsSize; i++) {
            Method* method = dvmDexGetResolvedMethod(pDvmDex, i);
            if (method != NULL) {
                filled->numMethods++;
            }
        }
    }
}

static void Dalvik_dalvik_system_VMRuntime_preloadDexCaches(const u4* args, JValue* pResult)
{
#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMRuntime_preloadDexCaches(dalvik_system_VMRuntime_p self);
	fastiva_Dalvik_dalvik_system_VMRuntime_preloadDexCaches((dalvik_system_VMRuntime_p)args[0]);
}

void fastiva_Dalvik_dalvik_system_VMRuntime_preloadDexCaches(dalvik_system_VMRuntime_p self) {
#endif  
    if (!kPreloadDexCachesEnabled) {
        return;
    }

    DexCacheStats total;
    DexCacheStats before;
    if (kPreloadDexCachesCollectStats) {
        ALOGI("VMRuntime.preloadDexCaches starting");
        preloadDexCachesStatsTotal(&total);
        preloadDexCachesStatsFilled(&before);
    }
#ifdef FASTIVA
	return;
#endif

    // We use a std::map to avoid heap allocating StringObjects to lookup in gDvm.literalStrings
    StringTable strings;
    if (kPreloadDexCachesStrings) {
        dvmLockMutex(&gDvm.internLock);
        dvmHashTableLock(gDvm.literalStrings);
        for (int i = 0; i < gDvm.literalStrings->tableSize; ++i) {
            HashEntry *entry = &gDvm.literalStrings->pEntries[i];
            if (entry->data != NULL && entry->data != HASH_TOMBSTONE) {
                preloadDexCachesStringsVisitor(&entry->data, 0, ROOT_INTERNED_STRING, &strings);
            }
        }
        dvmHashTableUnlock(gDvm.literalStrings);
        dvmUnlockMutex(&gDvm.internLock);
    }

    for (ClassPathEntry* cpe = gDvm.bootClassPath; cpe->kind != kCpeLastEntry; cpe++) {
        DvmDex* pDvmDex = getDvmDexFromClassPathEntry(cpe);
        const DexHeader* pHeader = pDvmDex->pHeader;
        const DexFile* pDexFile = pDvmDex->pDexFile;

        if (kPreloadDexCachesStrings) {
            for (size_t i = 0; i < pHeader->stringIdsSize; i++) {
                preloadDexCachesResolveString(pDvmDex, i, strings);
            }
        }

        if (kPreloadDexCachesTypes) {
            for (size_t i = 0; i < pHeader->typeIdsSize; i++) {
                preloadDexCachesResolveType(pDvmDex, i);
            }
        }

        if (kPreloadDexCachesFieldsAndMethods) {
            for (size_t classDefIndex = 0;
                 classDefIndex < pHeader->classDefsSize;
                 classDefIndex++) {
                const DexClassDef* pClassDef = dexGetClassDef(pDexFile, classDefIndex);
                const u1* pEncodedData = dexGetClassData(pDexFile, pClassDef);
                UniquePtr<DexClassData> pClassData(dexReadAndVerifyClassData(&pEncodedData, NULL));
                if (pClassData.get() == NULL) {
                    continue;
                }
                for (uint32_t fieldIndex = 0;
                     fieldIndex < pClassData->header.staticFieldsSize;
                     fieldIndex++) {
                    const DexField* pField = &pClassData->staticFields[fieldIndex];
                    preloadDexCachesResolveField(pDvmDex, pField->fieldIdx, false);
                }
                for (uint32_t fieldIndex = 0;
                     fieldIndex < pClassData->header.instanceFieldsSize;
                     fieldIndex++) {
                    const DexField* pField = &pClassData->instanceFields[fieldIndex];
                    preloadDexCachesResolveField(pDvmDex, pField->fieldIdx, true);
                }
                for (uint32_t methodIndex = 0;
                     methodIndex < pClassData->header.directMethodsSize;
                     methodIndex++) {
                    const DexMethod* pDexMethod = &pClassData->directMethods[methodIndex];
                    MethodType methodType = (((pDexMethod->accessFlags & ACC_STATIC) != 0) ?
                                             METHOD_STATIC :
                                             METHOD_DIRECT);
                    preloadDexCachesResolveMethod(pDvmDex, pDexMethod->methodIdx, methodType);
                }
                for (uint32_t methodIndex = 0;
                     methodIndex < pClassData->header.virtualMethodsSize;
                     methodIndex++) {
                    const DexMethod* pDexMethod = &pClassData->virtualMethods[methodIndex];
                    preloadDexCachesResolveMethod(pDvmDex, pDexMethod->methodIdx, METHOD_VIRTUAL);
                }
            }
        }
    }

    if (kPreloadDexCachesCollectStats) {
        DexCacheStats after;
        preloadDexCachesStatsFilled(&after);
        ALOGI("VMRuntime.preloadDexCaches strings total=%d before=%d after=%d",
              total.numStrings, before.numStrings, after.numStrings);
        ALOGI("VMRuntime.preloadDexCaches types total=%d before=%d after=%d",
              total.numTypes, before.numTypes, after.numTypes);
        ALOGI("VMRuntime.preloadDexCaches fields total=%d before=%d after=%d",
              total.numFields, before.numFields, after.numFields);
        ALOGI("VMRuntime.preloadDexCaches methods total=%d before=%d after=%d",
              total.numMethods, before.numMethods, after.numMethods);
        ALOGI("VMRuntime.preloadDexCaches finished");
    }

    RETURN_VOID();
}
#endif

const DalvikNativeMethod dvm_dalvik_system_VMRuntime[] = {
    { "addressOf", "(Ljava/lang/Object;)J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_addressOf) },
    { "bootClassPath", "()Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_bootClassPath) },
    { "classPath", "()Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_classPath) },
    { "clearGrowthLimit", "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_clearGrowthLimit) },
    { "disableJitCompilation", "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_disableJitCompilation) },
    { "isDebuggerActive", "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_isDebuggerActive) },
    { "getTargetHeapUtilization", "()F",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_getTargetHeapUtilization) },
    { "nativeSetTargetHeapUtilization", "(F)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_nativeSetTargetHeapUtilization) },
    { "newNonMovableArray", "(Ljava/lang/Class;I)Ljava/lang/Object;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_newNonMovableArray) },
    { "properties", "()[Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_properties) },
#ifdef G_WATCH
    { "setTargetSdkVersionNative", "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_setTargetSdkVersion) },
#else
    { "setTargetSdkVersion", "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_setTargetSdkVersion) },
#endif
    { "startJitCompilation", "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_startJitCompilation) },
    { "vmVersion", "()Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_vmVersion) },
#if FASTIVA_TARGET_ANDROID_VERSION >= 40400
    { "vmLibrary", "()Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_vmLibrary )},
    { "registerNativeAllocation", "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_registerNativeAllocation) },
    { "registerNativeFree", "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_registerNativeFree) },
    { "preloadDexCaches", "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_preloadDexCaches) },
#endif
#ifdef G_WATCH
    { "updateProcessState", "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMRuntime_updateProcessState) },
#endif
    { NULL, NULL, NULL, NULL },
};

