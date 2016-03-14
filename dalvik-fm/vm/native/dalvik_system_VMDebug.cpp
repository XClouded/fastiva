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
 * dalvik.system.VMDebug
 */
#include "Dalvik.h"
#include "alloc/HeapSource.h"
#include "native/InternalNativePriv.h"
#include "hprof/Hprof.h"

#include <string.h>
#include <unistd.h>

#ifdef FASTIVA
#include <java/io/FileDescriptor.h>
#endif

/*
 * Extracts the fd from a FileDescriptor object.
 *
 * If an error is encountered, or the extracted descriptor is numerically
 * invalid, this returns -1 with an exception raised.
 */
static int getFileDescriptor(Object* obj)
{
    assert(obj != NULL);
    assert(strcmp(obj->clazz->descriptor, "Ljava/io/FileDescriptor;") == 0);

    int fd = dvmGetFieldInt(obj, gDvm.offJavaIoFileDescriptor_descriptor);
    if (fd < 0) {
        dvmThrowRuntimeException("Invalid file descriptor");
        return -1;
    }

    return fd;
}

/*
 * static String[] getVmFeatureList()
 *
 * Return a set of strings describing available VM features (this is chiefly
 * of interest to DDMS).
 */
static void Dalvik_dalvik_system_VMDebug_getVmFeatureList(const u4* args, JValue* pResult) {
#ifdef FASTIVA
	java_lang_String_ap fastiva_Dalvik_dalvik_system_VMDebug_getVmFeatureList();
	*(java_lang_String_ap*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_getVmFeatureList();
}

java_lang_String_ap fastiva_Dalvik_dalvik_system_VMDebug_getVmFeatureList() {
#endif

    std::vector<std::string> features;
    features.push_back("method-trace-profiling");
    features.push_back("method-trace-profiling-streaming");
    features.push_back("method-sample-profiling");
    features.push_back("hprof-heap-dump");
    features.push_back("hprof-heap-dump-streaming");

    ArrayObject* result = dvmCreateStringArray(features);
    dvmReleaseTrackedAlloc((Object*) result, dvmThreadSelf());
    RETURN_PTR((java_lang_String_ap)result);
}

/* These must match the values in dalvik.system.VMDebug.
 */
enum {
    KIND_ALLOCATED_OBJECTS      = 1<<0,
    KIND_ALLOCATED_BYTES        = 1<<1,
    KIND_FREED_OBJECTS          = 1<<2,
    KIND_FREED_BYTES            = 1<<3,
    KIND_GC_INVOCATIONS         = 1<<4,
    KIND_CLASS_INIT_COUNT       = 1<<5,
    KIND_CLASS_INIT_TIME        = 1<<6,

    /* These values exist for backward compatibility. */
    KIND_EXT_ALLOCATED_OBJECTS = 1<<12,
    KIND_EXT_ALLOCATED_BYTES   = 1<<13,
    KIND_EXT_FREED_OBJECTS     = 1<<14,
    KIND_EXT_FREED_BYTES       = 1<<15,

    KIND_GLOBAL_ALLOCATED_OBJECTS   = KIND_ALLOCATED_OBJECTS,
    KIND_GLOBAL_ALLOCATED_BYTES     = KIND_ALLOCATED_BYTES,
    KIND_GLOBAL_FREED_OBJECTS       = KIND_FREED_OBJECTS,
    KIND_GLOBAL_FREED_BYTES         = KIND_FREED_BYTES,
    KIND_GLOBAL_GC_INVOCATIONS      = KIND_GC_INVOCATIONS,
    KIND_GLOBAL_CLASS_INIT_COUNT    = KIND_CLASS_INIT_COUNT,
    KIND_GLOBAL_CLASS_INIT_TIME     = KIND_CLASS_INIT_TIME,

    KIND_THREAD_ALLOCATED_OBJECTS   = KIND_ALLOCATED_OBJECTS << 16,
    KIND_THREAD_ALLOCATED_BYTES     = KIND_ALLOCATED_BYTES << 16,
    KIND_THREAD_FREED_OBJECTS       = KIND_FREED_OBJECTS << 16,
    KIND_THREAD_FREED_BYTES         = KIND_FREED_BYTES << 16,

    KIND_THREAD_GC_INVOCATIONS      = KIND_GC_INVOCATIONS << 16,

    // TODO: failedAllocCount, failedAllocSize
};

#define KIND_ALL_COUNTS 0xffffffff

/*
 * Zero out the specified fields.
 */
static void clearAllocProfStateFields(AllocProfState *allocProf,
    unsigned int kinds)
{
    if (kinds & KIND_ALLOCATED_OBJECTS) {
        allocProf->allocCount = 0;
    }
    if (kinds & KIND_ALLOCATED_BYTES) {
        allocProf->allocSize = 0;
    }
    if (kinds & KIND_FREED_OBJECTS) {
        allocProf->freeCount = 0;
    }
    if (kinds & KIND_FREED_BYTES) {
        allocProf->freeSize = 0;
    }
    if (kinds & KIND_GC_INVOCATIONS) {
        allocProf->gcCount = 0;
    }
    if (kinds & KIND_CLASS_INIT_COUNT) {
        allocProf->classInitCount = 0;
    }
    if (kinds & KIND_CLASS_INIT_TIME) {
        allocProf->classInitTime = 0;
    }
}

/*
 * static void startAllocCounting()
 *
 * Reset the counters and enable counting.
 *
 * TODO: this currently only resets the per-thread counters for the current
 * thread.  If we actually start using the per-thread counters we'll
 * probably want to fix this.
 */
static void Dalvik_dalvik_system_VMDebug_startAllocCounting(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_startAllocCounting();
	fastiva_Dalvik_dalvik_system_VMDebug_startAllocCounting();
}

void fastiva_Dalvik_dalvik_system_VMDebug_startAllocCounting() {
#endif

	clearAllocProfStateFields(&gDvm.allocProf, KIND_ALL_COUNTS);
    clearAllocProfStateFields(&dvmThreadSelf()->allocProf, KIND_ALL_COUNTS);
    dvmStartAllocCounting();
    RETURN_VOID();
}

/*
 * public static void stopAllocCounting()
 */
static void Dalvik_dalvik_system_VMDebug_stopAllocCounting(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_stopAllocCounting();
	fastiva_Dalvik_dalvik_system_VMDebug_stopAllocCounting();
}

void fastiva_Dalvik_dalvik_system_VMDebug_stopAllocCounting() {
#endif

	dvmStopAllocCounting();
    RETURN_VOID();
}

/*
 * private static int getAllocCount(int kind)
 */
static void Dalvik_dalvik_system_VMDebug_getAllocCount(const u4* args,
    JValue* pResult)
{
    unsigned int kind = args[0];

#ifdef FASTIVA
	jint fastiva_Dalvik_dalvik_system_VMDebug_getAllocCount(jint);
	*(jint*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_getAllocCount(kind);
}

jint fastiva_Dalvik_dalvik_system_VMDebug_getAllocCount(jint kind) {
	JValue res;
	JValue* pResult = &res;
#endif

    AllocProfState *allocProf;
    if (kind < (1<<16)) {
        allocProf = &gDvm.allocProf;
    } else {
        allocProf = &dvmThreadSelf()->allocProf;
        kind >>= 16;
    }
    switch (kind) {
    case KIND_ALLOCATED_OBJECTS:
        pResult->i = allocProf->allocCount;
        break;
    case KIND_ALLOCATED_BYTES:
        pResult->i = allocProf->allocSize;
        break;
    case KIND_FREED_OBJECTS:
        pResult->i = allocProf->freeCount;
        break;
    case KIND_FREED_BYTES:
        pResult->i = allocProf->freeSize;
        break;
    case KIND_GC_INVOCATIONS:
        pResult->i = allocProf->gcCount;
        break;
    case KIND_CLASS_INIT_COUNT:
        pResult->i = allocProf->classInitCount;
        break;
    case KIND_CLASS_INIT_TIME:
        /* convert nsec to usec, reduce to 32 bits */
        pResult->i = (int) (allocProf->classInitTime / 1000);
        break;
    case KIND_EXT_ALLOCATED_OBJECTS:
    case KIND_EXT_ALLOCATED_BYTES:
    case KIND_EXT_FREED_OBJECTS:
    case KIND_EXT_FREED_BYTES:
        pResult->i = 0;  /* backward compatibility */
        break;
    default:
        assert(false);
        pResult->i = -1;
    }
	RETURN_INT(pResult->i);
}

/*
 * public static void resetAllocCount(int kinds)
 */
static void Dalvik_dalvik_system_VMDebug_resetAllocCount(const u4* args,
    JValue* pResult)
{
    unsigned int kinds = args[0];
#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_resetAllocCount(jint);
	fastiva_Dalvik_dalvik_system_VMDebug_resetAllocCount(kinds);
}

void fastiva_Dalvik_dalvik_system_VMDebug_resetAllocCount(jint kinds_0) {
	unsigned int kinds = kinds_0;
#endif
    clearAllocProfStateFields(&gDvm.allocProf, kinds & 0xffff);
    clearAllocProfStateFields(&dvmThreadSelf()->allocProf, kinds >> 16);
    RETURN_VOID();
}

#if FASTIVA_TARGET_ANDROID_VERSION < 40400
/*
 * static void startMethodTracingNative(String traceFileName,
 *     FileDescriptor fd, int bufferSize, int flags)
 *
 * Start method trace profiling.
 *
 * If both "traceFileName" and "fd" are null, the result will be sent
 * directly to DDMS.  (The non-DDMS versions of the calls are expected
 * to enforce non-NULL filenames.)
 */
static void Dalvik_dalvik_system_VMDebug_startMethodTracingNative(const u4* args,
    JValue* pResult)
{
    StringObject* traceFileStr = (StringObject*) args[0];
    Object* traceFd = (Object*) args[1];
    int bufferSize = args[2];
    int flags = args[3];

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingNative(java_lang_String_p, java_io_FileDescriptor_p, jint, jint);
	fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingNative((java_lang_String_p)traceFileStr, (java_io_FileDescriptor_p)traceFd, bufferSize, flags);
}

void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingNative(java_lang_String_p traceFileStr, java_io_FileDescriptor_p traceFd, jint bufferSize, jint flags) {
#endif

	if (bufferSize == 0) {
        // Default to 8MB per the documentation.
        bufferSize = 8 * 1024 * 1024;
    }

    if (bufferSize < 1024) {
        dvmThrowIllegalArgumentException(NULL);
        THROW_VOID();
    }

    char* traceFileName = NULL;
    if (traceFileStr != NULL)
        traceFileName = dvmCreateCstrFromString(traceFileStr);

    int fd = -1;
    if (traceFd != NULL) {
        int origFd = getFileDescriptor(traceFd);
        if (origFd < 0)
            THROW_VOID();

        fd = dup(origFd);
        if (fd < 0) {
            dvmThrowExceptionFmt(gDvm.exRuntimeException,
                "dup(%d) failed: %s", origFd, strerror(errno));
            THROW_VOID();
        }
    }

#ifdef FASTIVA // patch for KitKat
    dvmMethodTraceStart(traceFileName != NULL ? traceFileName : "[DDMS]",
        fd, bufferSize, flags, (traceFileName == NULL && fd == -1), false, 0);
#else
    dvmMethodTraceStart(traceFileName != NULL ? traceFileName : "[DDMS]",
        fd, bufferSize, flags, (traceFileName == NULL && fd == -1));
#endif
    free(traceFileName);
    RETURN_VOID();
}

/*
 * static boolean isMethodTracingActive()
 *
 * Determine whether method tracing is currently active.
 */
static void Dalvik_dalvik_system_VMDebug_isMethodTracingActive(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jbool fastiva_Dalvik_dalvik_system_VMDebug_isMethodTracingActive();
	*(jbool*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_isMethodTracingActive();
}

jbool fastiva_Dalvik_dalvik_system_VMDebug_isMethodTracingActive() {
#endif

#ifdef FASTIVA // patch for KitKat
	RETURN_BOOLEAN(dvmGetMethodTracingMode() != TRACING_INACTIVE);
#else
	RETURN_BOOLEAN(dvmIsMethodTraceActive());
#endif

}
#else
/*
 * static void startMethodTracingDdmsImpl(int bufferSize, int flags,
 *     boolean samplingEnabled, int intervalUs)
 *
 * Start method trace profiling, sending results directly to DDMS.
 */
static void Dalvik_dalvik_system_VMDebug_startMethodTracingDdmsImpl(const u4* args,
    JValue* pResult)
{
    int bufferSize = args[0];
    int flags = args[1];
    bool samplingEnabled = args[2];
    int intervalUs = args[3];
#ifdef FASTIVA // (IIZI)V
	void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingDdmsImpl(int bufferSize, int flags, jbool samplingEnabled, int intervalUs);
	fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingDdmsImpl(bufferSize, flags, samplingEnabled, intervalUs);
}

void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingDdmsImpl(int bufferSize, int flags, jbool samplingEnabled, int intervalUs) {
#endif
    dvmMethodTraceStart("[DDMS]", -1, bufferSize, flags, true, samplingEnabled,
        intervalUs);
    RETURN_VOID();
}

/*
 * static void startMethodTracingFd(String traceFileName, FileDescriptor fd,
 *     int bufferSize, int flags)
 *
 * Start method trace profiling, sending results to a file descriptor.
 */
static void Dalvik_dalvik_system_VMDebug_startMethodTracingFd(const u4* args,
    JValue* pResult)
{
    StringObject* traceFileStr = (StringObject*) args[0];
    Object* traceFd = (Object*) args[1];
    int bufferSize = args[2];
    int flags = args[3];
#ifdef G_WATCH
	bool bArg = args[4];
	int  iArg = args[5];
#endif
#ifdef FASTIVA // (Ljava/lang/String;Ljava/io/FileDescriptor;II)V
#ifdef G_WATACH
	void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFd(java_lang_String_p traceFileStr, java_io_FileDescriptor_p traceFd, int bufferSize, int flags, bool bArg, int iArg);
	fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFd((java_lang_String_p)traceFileStr, (java_io_FileDescriptor_p)traceFd, bufferSize, flags, bArg, iArg);
}

void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFd(java_lang_String_p traceFileStr, java_io_FileDescriptor_p traceFd, int bufferSize, int flags, bool bArg, int iArg) {
#else
	void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFd(java_lang_String_p traceFileStr, java_io_FileDescriptor_p traceFd, int bufferSize, int flags);
	fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFd((java_lang_String_p)traceFileStr, (java_io_FileDescriptor_p)traceFd, bufferSize, flags);
}

void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFd(java_lang_String_p traceFileStr, java_io_FileDescriptor_p traceFd, int bufferSize, int flags) {
#endif
#endif

    int origFd = getFileDescriptor(traceFd);
    if (origFd < 0)
        RETURN_VOID();

    int fd = dup(origFd);
    if (fd < 0) {
        dvmThrowExceptionFmt(gDvm.exRuntimeException,
            "dup(%d) failed: %s", origFd, strerror(errno));
        RETURN_VOID();
    }

    char* traceFileName = dvmCreateCstrFromString(traceFileStr);
    if (traceFileName == NULL) {
        RETURN_VOID();
    }

    dvmMethodTraceStart(traceFileName, fd, bufferSize, flags, false, false, 0);
    free(traceFileName);
    RETURN_VOID();
}

/*
 * static void startMethodTracingFilename(String traceFileName, int bufferSize,
 *     int flags)
 *
 * Start method trace profiling, sending results to a file.
 */
static void Dalvik_dalvik_system_VMDebug_startMethodTracingFilename(const u4* args,
    JValue* pResult)
{
    StringObject* traceFileStr = (StringObject*) args[0];
    int bufferSize = args[1];
    int flags = args[2];

#ifdef FASTIVA // (Ljava/lang/String;II)V
	void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFilename(java_lang_String_p traceFileStr, int bufferSize, int flags);
	fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFilename((java_lang_String_p)traceFileStr, bufferSize, flags);
}

void fastiva_Dalvik_dalvik_system_VMDebug_startMethodTracingFilename(java_lang_String_p traceFileStr, int bufferSize, int flags) {
#endif

    char* traceFileName = dvmCreateCstrFromString(traceFileStr);
    if (traceFileName == NULL) {
        RETURN_VOID();
    }

    dvmMethodTraceStart(traceFileName, -1, bufferSize, flags, false, false, 0);
    free(traceFileName);
    RETURN_VOID();
}

/*
 * static int getMethodTracingMode()
 *
 * Determine whether method tracing is currently active and what type is active.
 */
static void Dalvik_dalvik_system_VMDebug_getMethodTracingMode(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);
#ifdef FASTIVA // ()I
	int fastiva_Dalvik_dalvik_system_VMDebug_getMethodTracingMode();
	*(jint*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_getMethodTracingMode();
}

int fastiva_Dalvik_dalvik_system_VMDebug_getMethodTracingMode() {
#endif

    RETURN_INT(dvmGetMethodTracingMode());
}

#endif
/*
 * static void stopMethodTracing()
 *
 * Stop method tracing.
 */
static void Dalvik_dalvik_system_VMDebug_stopMethodTracing(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_stopMethodTracing();
	fastiva_Dalvik_dalvik_system_VMDebug_stopMethodTracing();
}

void fastiva_Dalvik_dalvik_system_VMDebug_stopMethodTracing() {
#endif

	dvmMethodTraceStop();
    RETURN_VOID();
}

/*
 * static void startEmulatorTracing()
 *
 * Start sending method trace info to the emulator.
 */
static void Dalvik_dalvik_system_VMDebug_startEmulatorTracing(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_startEmulatorTracing();
	fastiva_Dalvik_dalvik_system_VMDebug_startEmulatorTracing();
}

void fastiva_Dalvik_dalvik_system_VMDebug_startEmulatorTracing() {
#endif

	dvmEmulatorTraceStart();
    RETURN_VOID();
}

/*
 * static void stopEmulatorTracing()
 *
 * Start sending method trace info to the emulator.
 */
static void Dalvik_dalvik_system_VMDebug_stopEmulatorTracing(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_stopEmulatorTracing();
	fastiva_Dalvik_dalvik_system_VMDebug_stopEmulatorTracing();
}

void fastiva_Dalvik_dalvik_system_VMDebug_stopEmulatorTracing() {
#endif

	dvmEmulatorTraceStop();
    RETURN_VOID();
}

/*
 * static boolean isDebuggerConnected()
 *
 * Returns "true" if a debugger is attached.
 */
static void Dalvik_dalvik_system_VMDebug_isDebuggerConnected(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jbool fastiva_Dalvik_dalvik_system_VMDebug_isDebuggerConnected();
	*(jbool*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_isDebuggerConnected();
}

jbool fastiva_Dalvik_dalvik_system_VMDebug_isDebuggerConnected() {
#endif

    RETURN_BOOLEAN(dvmDbgIsDebuggerConnected());
}

/*
 * static boolean isDebuggingEnabled()
 *
 * Returns "true" if debugging is enabled.
 */
static void Dalvik_dalvik_system_VMDebug_isDebuggingEnabled(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);
#ifdef FASTIVA
	jbool fastiva_Dalvik_dalvik_system_VMDebug_isDebuggingEnabled();
	*(jbool*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_isDebuggingEnabled();
}

jbool fastiva_Dalvik_dalvik_system_VMDebug_isDebuggingEnabled() {
#endif

    RETURN_BOOLEAN(gDvm.jdwpConfigured);
}

/*
 * static long lastDebuggerActivity()
 *
 * Returns the time, in msec, since we last had an interaction with the
 * debugger (send or receive).
 */
static void Dalvik_dalvik_system_VMDebug_lastDebuggerActivity(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_dalvik_system_VMDebug_lastDebuggerActivity();
	*(jlonglong*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_lastDebuggerActivity();
}

jlonglong fastiva_Dalvik_dalvik_system_VMDebug_lastDebuggerActivity() {
#endif

	RETURN_LONG(dvmDbgLastDebuggerActivity());
}

/*
 * static void startInstructionCounting()
 */
static void Dalvik_dalvik_system_VMDebug_startInstructionCounting(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_startInstructionCounting();
	fastiva_Dalvik_dalvik_system_VMDebug_startInstructionCounting();
}

void fastiva_Dalvik_dalvik_system_VMDebug_startInstructionCounting() {
#endif

	dvmStartInstructionCounting();
    RETURN_VOID();
}

/*
 * static void stopInstructionCounting()
 */
static void Dalvik_dalvik_system_VMDebug_stopInstructionCounting(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_stopInstructionCounting();
	fastiva_Dalvik_dalvik_system_VMDebug_stopInstructionCounting();
}

void fastiva_Dalvik_dalvik_system_VMDebug_stopInstructionCounting() {
#endif

	dvmStopInstructionCounting();
    RETURN_VOID();
}

/*
 * static boolean getInstructionCount(int[] counts)
 *
 * Grab a copy of the global instruction count array.
 *
 * Since the instruction counts aren't synchronized, we use sched_yield
 * to improve our chances of finishing without contention.  (Only makes
 * sense on a uniprocessor.)
 */
static void Dalvik_dalvik_system_VMDebug_getInstructionCount(const u4* args,
    JValue* pResult)
{
    ArrayObject* countArray = (ArrayObject*) args[0];

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_getInstructionCount(Int_ap);
	fastiva_Dalvik_dalvik_system_VMDebug_getInstructionCount((Int_ap)countArray);
}

void fastiva_Dalvik_dalvik_system_VMDebug_getInstructionCount(Int_ap arg0) {
    ArrayObject* countArray = (ArrayObject*)arg0;
#endif

    if (countArray != NULL) {
        int* storage = (int*)(void*)countArray->contents;
        u4 length = countArray->length;

        /*
         * Ensure that we copy at most kNumPackedOpcodes
         * elements, but no more than the length of the given array.
         */
        if (length > kNumPackedOpcodes) {
            length = kNumPackedOpcodes;
        }

        sched_yield();
        memcpy(storage, gDvm.executedInstrCounts, length * sizeof(int));
    }

    RETURN_VOID();
}

/*
 * static boolean resetInstructionCount()
 *
 * Reset the instruction count array.
 */
static void Dalvik_dalvik_system_VMDebug_resetInstructionCount(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_resetInstructionCount();
	fastiva_Dalvik_dalvik_system_VMDebug_resetInstructionCount();
}

void fastiva_Dalvik_dalvik_system_VMDebug_resetInstructionCount() {
#endif

	sched_yield();
    memset(gDvm.executedInstrCounts, 0, kNumPackedOpcodes * sizeof(int));
    RETURN_VOID();
}

/*
 * static void printLoadedClasses(int flags)
 *
 * Dump the list of loaded classes.
 */
static void Dalvik_dalvik_system_VMDebug_printLoadedClasses(const u4* args,
    JValue* pResult)
{
    int flags = args[0];

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_printLoadedClasses(jint);
	fastiva_Dalvik_dalvik_system_VMDebug_printLoadedClasses(flags);
}

void fastiva_Dalvik_dalvik_system_VMDebug_printLoadedClasses(jint flags) {
#endif

	dvmDumpAllClasses(flags);

    RETURN_VOID();
}

/*
 * static int getLoadedClassCount()
 *
 * Return the number of loaded classes
 */
static void Dalvik_dalvik_system_VMDebug_getLoadedClassCount(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jint fastiva_Dalvik_dalvik_system_VMDebug_getLoadedClassCount();
	*(jint*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_getLoadedClassCount();
}

jint fastiva_Dalvik_dalvik_system_VMDebug_getLoadedClassCount() {
#endif

    int count;

    count = dvmGetNumLoadedClasses();

    RETURN_INT(count);
}

/*
 * Returns the thread-specific CPU-time clock value for the current thread,
 * or -1 if the feature isn't supported.
 */
static void Dalvik_dalvik_system_VMDebug_threadCpuTimeNanos(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_dalvik_system_VMDebug_threadCpuTimeNanos();
	*(jlonglong*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_threadCpuTimeNanos();
}

jlonglong fastiva_Dalvik_dalvik_system_VMDebug_threadCpuTimeNanos() {
#endif

	jlong result;

#ifdef HAVE_POSIX_CLOCKS
    struct timespec now;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);
    result = (jlong) (now.tv_sec*1000000000LL + now.tv_nsec);
#else
    result = (jlong) -1;
#endif

    RETURN_LONG(result);
}

/*
 * static void dumpHprofData(String fileName, FileDescriptor fd)
 *
 * Cause "hprof" data to be dumped.  We can throw an IOException if an
 * error occurs during file handling.
 */
static void Dalvik_dalvik_system_VMDebug_dumpHprofData(const u4* args,
    JValue* pResult)
{
	StringObject* fileNameStr = (StringObject*) args[0];
    Object* fileDescriptor = (Object*) args[1];

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_dumpHprofData(java_lang_String_p, java_io_FileDescriptor_p);
	fastiva_Dalvik_dalvik_system_VMDebug_dumpHprofData((java_lang_String_p)fileNameStr, (java_io_FileDescriptor_p)fileDescriptor);
}

void fastiva_Dalvik_dalvik_system_VMDebug_dumpHprofData(java_lang_String_p fileNameStr, java_io_FileDescriptor_p fileDescriptor) {
#endif

#if !FASTIVA_DISABLE_HPROFILE
	char* fileName;
    int result;

    /*
     * Only one of these may be NULL.
     */
    if (fileNameStr == NULL && fileDescriptor == NULL) {
        dvmThrowNullPointerException("fileName == null && fd == null");
        THROW_VOID();
    }

    if (fileNameStr != NULL) {
        fileName = dvmCreateCstrFromString(fileNameStr);
        if (fileName == NULL) {
            /* unexpected -- malloc failure? */
            dvmThrowRuntimeException("malloc failure?");
            THROW_VOID();
        }
    } else {
        fileName = strdup("[fd]");
    }

    int fd = -1;
    if (fileDescriptor != NULL) {
        fd = getFileDescriptor(fileDescriptor);
        if (fd < 0) {
            free(fileName);
            THROW_VOID();
        }
    }

	result = hprofDumpHeap(fileName, fd, false);
    free(fileName);

    if (result != 0) {
        /* ideally we'd throw something more specific based on actual failure */
        dvmThrowRuntimeException(
            "Failure during heap dump; check log output for details");
        THROW_VOID();
    }

    RETURN_VOID();
#endif // !FASTIVA_DISABLE_HPROFILE
}

/*
 * static void dumpHprofDataDdms()
 *
 * Cause "hprof" data to be computed and sent directly to DDMS.
 */
static void Dalvik_dalvik_system_VMDebug_dumpHprofDataDdms(const u4* args,
    JValue* pResult)
{
#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_dumpHprofDataDdms();
	fastiva_Dalvik_dalvik_system_VMDebug_dumpHprofDataDdms();
}

void fastiva_Dalvik_dalvik_system_VMDebug_dumpHprofDataDdms() {
#endif

#if !FASTIVA_DISABLE_HPROFILE
    int result;

    result = hprofDumpHeap("[DDMS]", -1, true);

    if (result != 0) {
        /* ideally we'd throw something more specific based on actual failure */
        dvmThrowRuntimeException(
            "Failure during heap dump; check log output for details");
        THROW_VOID();
    }

    RETURN_VOID();
#endif // !FASTIVA_DISABLE_HPROFILE
}

/*
 * static boolean cacheRegisterMap(String classAndMethodDescr)
 *
 * If the specified class is loaded, and the named method exists, ensure
 * that the method's register map is ready for use.  If the class/method
 * cannot be found, nothing happens.
 *
 * This can improve the zygote's sharing of compressed register maps.  Do
 * this after class preloading.
 *
 * Returns true if the register map is cached and ready, either as a result
 * of this call or earlier activity.  Returns false if the class isn't loaded,
 * if the method couldn't be found, or if the method has no register map.
 *
 * (Uncomment logs in dvmGetExpandedRegisterMap0() to gather stats.)
 */
static void Dalvik_dalvik_system_VMDebug_cacheRegisterMap(const u4* args,
    JValue* pResult)
{
    StringObject* classAndMethodDescStr = (StringObject*) args[0];

#ifdef FASTIVA
	jbool fastiva_Dalvik_dalvik_system_VMDebug_cacheRegisterMap(java_lang_String_p);
	*(jbool*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_cacheRegisterMap((java_lang_String_p)classAndMethodDescStr);
}

jbool fastiva_Dalvik_dalvik_system_VMDebug_cacheRegisterMap(java_lang_String_p classAndMethodDescStr) {
#endif

    ClassObject* clazz;
    bool result = false;

    if (classAndMethodDescStr == NULL) {
        dvmThrowNullPointerException("classAndMethodDesc == null");
        THROW_V();
    }

    char* classAndMethodDesc = NULL;

    /*
     * Pick the string apart.  We have a local copy, so just modify it
     * in place.
     */
    classAndMethodDesc = dvmCreateCstrFromString(classAndMethodDescStr);

    char* methodName = strchr(classAndMethodDesc, '.');
    if (methodName == NULL) {
        dvmThrowRuntimeException("method name not found in string");
        THROW_V();
    }
    *methodName++ = '\0';

    char* methodDescr = strchr(methodName, ':');
    if (methodDescr == NULL) {
        dvmThrowRuntimeException("method descriptor not found in string");
        THROW_V();
    }
    *methodDescr++ = '\0';

    //ALOGD("GOT: %s %s %s", classAndMethodDesc, methodName, methodDescr);

    /*
     * Find the class, but only if it's already loaded.
     */
    clazz = dvmLookupClass(classAndMethodDesc, NULL, false);
    if (clazz == NULL) {
        ALOGD("Class %s not found in bootstrap loader", classAndMethodDesc);
        goto bail;
    }

    Method* method;

    /*
     * Find the method, which could be virtual or direct, defined directly
     * or inherited.
     */
    if (methodName[0] == '<') {
        /*
         * Constructor or class initializer.  Only need to examine the
         * "direct" list, and don't need to search up the class hierarchy.
         */
        method = dvmFindDirectMethodByDescriptor(clazz, methodName,
                    methodDescr);
    } else {
        /*
         * Try both lists, and scan up the tree.
         */
        method = dvmFindVirtualMethodHierByDescriptor(clazz, methodName,
                    methodDescr);
        if (method == NULL) {
            method = dvmFindDirectMethodHierByDescriptor(clazz, methodName,
                        methodDescr);
        }
    }

    if (method != NULL) {
        /*
         * Got it.  See if there's a register map here.
         */
        const RegisterMap* pMap;
        pMap = dvmGetExpandedRegisterMap(method);
        if (pMap == NULL) {
            ALOGV("No map for %s.%s %s",
                classAndMethodDesc, methodName, methodDescr);
        } else {
            ALOGV("Found map %s.%s %s",
                classAndMethodDesc, methodName, methodDescr);
            result = true;
        }
    } else {
        ALOGV("Unable to find %s.%s %s",
            classAndMethodDesc, methodName, methodDescr);
    }

bail:
    free(classAndMethodDesc);
    RETURN_BOOLEAN(result);
}

/*
 * static void dumpReferenceTables()
 */
static void Dalvik_dalvik_system_VMDebug_dumpReferenceTables(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);
    UNUSED_PARAMETER(pResult);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_dumpReferenceTables();
	fastiva_Dalvik_dalvik_system_VMDebug_dumpReferenceTables();
}

void fastiva_Dalvik_dalvik_system_VMDebug_dumpReferenceTables() {
#endif


    ALOGI("--- reference table dump ---");
    dvmDumpJniReferenceTables();
    // could dump thread's internalLocalRefTable, probably not useful
    // ditto for thread's jniMonitorRefTable
    ALOGI("---");
    RETURN_VOID();
}

/*
 * static void crash()
 *
 * Dump the current thread's interpreted stack and abort the VM.  Useful
 * for seeing both interpreted and native stack traces.
 *
 * (Might want to restrict this to debuggable processes as a security
 * measure, or check SecurityManager.checkExit().)
 */
static void Dalvik_dalvik_system_VMDebug_crash(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);
    UNUSED_PARAMETER(pResult);

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_crash();
	fastiva_Dalvik_dalvik_system_VMDebug_crash();
}

void fastiva_Dalvik_dalvik_system_VMDebug_crash() {
#endif


    ALOGW("Crashing VM on request");
    dvmDumpThread(dvmThreadSelf(), false);
    dvmAbort();
}

/*
 * static void infopoint(int id)
 *
 * Provide a hook for gdb to hang to so that the VM can be stopped when
 * user-tagged source locations are being executed.
 */
static void Dalvik_dalvik_system_VMDebug_infopoint(const u4* args,
    JValue* pResult)
{
    jint arg0 = args[0];

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_infopoint(jint);
	fastiva_Dalvik_dalvik_system_VMDebug_infopoint(arg0);
}

void fastiva_Dalvik_dalvik_system_VMDebug_infopoint(jint arg0) {
#endif

    gDvm.nativeDebuggerActive = true;

    ALOGD("VMDebug infopoint %d hit", arg0);

    gDvm.nativeDebuggerActive = false;
    RETURN_VOID();
}

static void Dalvik_dalvik_system_VMDebug_countInstancesOfClass(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*)args[0];
    bool countAssignable = args[1];

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_dalvik_system_VMDebug_countInstancesOfClass(java_lang_Class_p, jbool);
	*(jlonglong*)pResult = fastiva_Dalvik_dalvik_system_VMDebug_countInstancesOfClass((java_lang_Class_p)clazz, countAssignable);
}

jlonglong fastiva_Dalvik_dalvik_system_VMDebug_countInstancesOfClass(java_lang_Class_p clazz, jbool countAssignable) {
#endif

    if (clazz == NULL) {
        RETURN_LONG(0);
    }
    if (countAssignable) {
        size_t count = dvmCountAssignableInstancesOfClass(clazz);
        RETURN_LONG((long long)count);
    } else {
        size_t count = dvmCountInstancesOfClass(clazz);
        RETURN_LONG((long long)count);
    }
}

#if FASTIVA_TARGET_ANDROID_VERSION >= 40400
/*
 * public static native void getHeapSpaceStats(long[] data)
 */
static void Dalvik_dalvik_system_VMDebug_getHeapSpaceStats(const u4* args,
    JValue* pResult)
{
    ArrayObject* dataArray = (ArrayObject*) args[0];
    if (dataArray == NULL || dataArray->length < 6) {
      RETURN_VOID();
    }

#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_VMDebug_getHeapSpaceStats(Longlong_ap dataArray);
	fastiva_Dalvik_dalvik_system_VMDebug_getHeapSpaceStats((Longlong_ap)dataArray);
}

void fastiva_Dalvik_dalvik_system_VMDebug_getHeapSpaceStats(Longlong_ap dataArray) {
    if (dataArray == NULL || dataArray->length() < 6) {
      RETURN_VOID();
    }

    jlong* arr = (jlong*)(void*)dataArray->getBuffer_unsafe$();
#else
    jlong* arr = (jlong*)(void*)dataArray->contents;
#endif

    int j = 0;
    size_t per_heap_allocated[2];
    size_t per_heap_size[2];
    memset(per_heap_allocated, 0, sizeof(per_heap_allocated));
    memset(per_heap_size, 0, sizeof(per_heap_size));
    dvmHeapSourceGetValue(HS_BYTES_ALLOCATED, (size_t*) &per_heap_allocated, 2);
    dvmHeapSourceGetValue(HS_FOOTPRINT, (size_t*) &per_heap_size, 2);
    jlong heapSize = per_heap_size[0];
    jlong heapUsed = per_heap_allocated[0];
    jlong heapFree = heapSize - heapUsed;
    jlong zygoteSize = per_heap_size[1];
    jlong zygoteUsed = per_heap_allocated[1];
    jlong zygoteFree = zygoteSize - zygoteUsed;
    arr[j++] = heapSize;
    arr[j++] = heapUsed;
    arr[j++] = heapFree;
    arr[j++] = zygoteSize;
    arr[j++] = zygoteUsed;
    arr[j++] = zygoteFree;

    RETURN_VOID();
}
#endif

const DalvikNativeMethod dvm_dalvik_system_VMDebug[] = {
    { "getVmFeatureList",           "()[Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_getVmFeatureList) },
    { "getAllocCount",              "(I)I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_getAllocCount) },
#if FASTIVA_TARGET_ANDROID_VERSION >= 40400
    { "getHeapSpaceStats",          "([J)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_getHeapSpaceStats) },
#endif
    { "resetAllocCount",            "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_resetAllocCount) },
    { "startAllocCounting",         "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startAllocCounting) },
    { "stopAllocCounting",          "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_stopAllocCounting) },
#if FASTIVA_TARGET_ANDROID_VERSION >= 40400
    { "startMethodTracingDdmsImpl", "(IIZI)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startMethodTracingDdmsImpl) },
#ifdef G_WATCH
    { "startMethodTracingFd",       "(Ljava/lang/String;Ljava/io/FileDescriptor;IIZI)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startMethodTracingFd) },
    { "startMethodTracingFilename", "(Ljava/lang/String;IIZI)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startMethodTracingFilename) },
#else
    { "startMethodTracingFd",       "(Ljava/lang/String;Ljava/io/FileDescriptor;II)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startMethodTracingFd) },
    { "startMethodTracingFilename", "(Ljava/lang/String;II)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startMethodTracingFilename) },
#endif
    { "getMethodTracingMode",       "()I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_getMethodTracingMode) },
#else
    { "startMethodTracingNative",   "(Ljava/lang/String;Ljava/io/FileDescriptor;II)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startMethodTracingNative) },
    { "isMethodTracingActive",      "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_isMethodTracingActive) },
#endif
    { "stopMethodTracing",          "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_stopMethodTracing) },
    { "startEmulatorTracing",       "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startEmulatorTracing) },
    { "stopEmulatorTracing",        "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_stopEmulatorTracing) },
    { "startInstructionCounting",   "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_startInstructionCounting) },
    { "stopInstructionCounting",    "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_stopInstructionCounting) },
    { "resetInstructionCount",      "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_resetInstructionCount) },
    { "getInstructionCount",        "([I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_getInstructionCount) },
    { "isDebuggerConnected",        "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_isDebuggerConnected) },
    { "isDebuggingEnabled",         "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_isDebuggingEnabled) },
    { "lastDebuggerActivity",       "()J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_lastDebuggerActivity) },
    { "printLoadedClasses",         "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_printLoadedClasses) },
    { "getLoadedClassCount",        "()I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_getLoadedClassCount) },
    { "threadCpuTimeNanos",         "()J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_threadCpuTimeNanos) },
    { "dumpHprofData",              "(Ljava/lang/String;Ljava/io/FileDescriptor;)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_dumpHprofData) },
    { "dumpHprofDataDdms",          "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_dumpHprofDataDdms) },
    { "cacheRegisterMap",           "(Ljava/lang/String;)Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_cacheRegisterMap) },
    { "dumpReferenceTables",        "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_dumpReferenceTables) },
    { "crash",                      "()V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_crash) },
    { "infopoint",                 "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_infopoint) },
    { "countInstancesOfClass",     "(Ljava/lang/Class;Z)J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_VMDebug_countInstancesOfClass) },
    { NULL, NULL, NULL, NULL },
};

