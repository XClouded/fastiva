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
 * org.apache.harmony.dalvik.ddmc.DdmVmInternal
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"


/*
 * public static void threadNotify(boolean enable)
 *
 * Enable DDM thread notifications.
 */
static void Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_threadNotify(
    const u4* args, JValue* pResult)
{
    bool enable = (args[0] != 0);

#ifdef FASTIVA
	void fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_threadNotify(jbool);
	fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_threadNotify(enable);
}

void fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_threadNotify(jbool enable) {
#endif

    //ALOGI("ddmThreadNotification: %d", enable);
    dvmDdmSetThreadNotification(enable);
    RETURN_VOID();
}

/*
 * public static byte[] getThreadStats()
 *
 * Get a buffer full of thread info.
 */
static void Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getThreadStats(
    const u4* args, JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	Byte_ap fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getThreadStats();
	*(Byte_ap*)pResult = fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getThreadStats();
}

Byte_ap fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getThreadStats() {
#endif

    ArrayObject* result = dvmDdmGenerateThreadStats();
    dvmReleaseTrackedAlloc((Object*) result, NULL);
    RETURN_PTR((Byte_ap)result);
}

/*
 * public static int heapInfoNotify(int what)
 *
 * Enable DDM heap notifications.
 */
static void Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapInfoNotify(
    const u4* args, JValue* pResult)
{
    int when = args[0];

#ifdef FASTIVA
	jbool fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapInfoNotify(jint);
	*(jbool*)pResult = fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapInfoNotify(when);
}

jbool fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapInfoNotify(jint when) {
#endif

	bool ret;

    ret = dvmDdmHandleHpifChunk(when);
    RETURN_BOOLEAN(ret);
}

/*
 * public static boolean heapSegmentNotify(int when, int what, bool native)
 *
 * Enable DDM heap notifications.
 */
static void
    Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapSegmentNotify(
    const u4* args, JValue* pResult)
{
    int  when   = args[0];        // 0=never (off), 1=during GC
    int  what   = args[1];        // 0=merged objects, 1=distinct objects
    bool native = (args[2] != 0); // false=virtual heap, true=native heap

#ifdef FASTIVA
	jbool fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapSegmentNotify(jint, jint, jbool);
	*(jbool*)pResult = fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapSegmentNotify(when, what, native);
}

jbool fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapSegmentNotify(jint when, jint what, jbool native) {
#endif

    bool ret;

    ret = dvmDdmHandleHpsgNhsgChunk(when, what, native);
    RETURN_BOOLEAN(ret);
}

/*
 * public static StackTraceElement[] getStackTraceById(int threadId)
 *
 * Get a stack trace as an array of StackTraceElement objects.  Returns
 * NULL on failure, e.g. if the threadId couldn't be found.
 */
static void
    Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getStackTraceById(
    const u4* args, JValue* pResult)
{
    u4 threadId = args[0];

#ifdef FASTIVA
	java_lang_StackTraceElement_ap fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getStackTraceById(jint);
	*(java_lang_StackTraceElement_ap*)pResult = fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getStackTraceById(threadId);
}

java_lang_StackTraceElement_ap fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getStackTraceById(jint threadId) {
#endif

    ArrayObject* trace;

    trace = dvmDdmGetStackTraceById(threadId);
    RETURN_PTR((java_lang_StackTraceElement_ap)trace);
}

/*
 * public static void enableRecentAllocations(boolean enable)
 *
 * Enable or disable recent allocation tracking.
 */
static void
    Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_enableRecentAllocations(
    const u4* args, JValue* pResult)
{
    bool enable = (args[0] != 0);

#ifdef FASTIVA
	void fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_enableRecentAllocations(jbool);
	fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_enableRecentAllocations(enable);
}

void fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_enableRecentAllocations(jbool enable) {
#endif

    if (enable)
        (void) dvmEnableAllocTracker();
    else
        (void) dvmDisableAllocTracker();
    RETURN_VOID();
}

/*
 * public static boolean getRecentAllocationStatus()
 *
 * Returns "true" if allocation tracking is enabled.
 */
static void
    Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocationStatus(
    const u4* args, JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jbool fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocationStatus();
	*(jbool*)pResult = fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocationStatus();
}

jbool fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocationStatus() {
#endif

    RETURN_BOOLEAN(gDvm.allocRecords != NULL);
}

/*
 * public static byte[] getRecentAllocations()
 *
 * Fill a buffer with data on recent heap allocations.
 */
static void
    Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocations(
    const u4* args, JValue* pResult)
{

#ifdef FASTIVA
	Byte_ap fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocations();
	*(Byte_ap*)pResult = fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocations();
}

Byte_ap fastiva_Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocations() {
#endif

    ArrayObject* data;

    data = dvmDdmGetRecentAllocations();
    dvmReleaseTrackedAlloc((Object*) data, NULL);
    RETURN_PTR((Byte_ap)data);
}

const DalvikNativeMethod dvm_org_apache_harmony_dalvik_ddmc_DdmVmInternal[] = {
    { "threadNotify",       "(Z)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_threadNotify) },
    { "getThreadStats",     "()[B",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getThreadStats) },
    { "heapInfoNotify",     "(I)Z",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapInfoNotify) },
    { "heapSegmentNotify",  "(IIZ)Z",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_heapSegmentNotify) },
    { "getStackTraceById",  "(I)[Ljava/lang/StackTraceElement;",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getStackTraceById) },
    { "enableRecentAllocations", "(Z)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_enableRecentAllocations) },
    { "getRecentAllocationStatus", "()Z",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocationStatus) },
    { "getRecentAllocations", "()[B",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_org_apache_harmony_dalvik_ddmc_DdmVmInternal_getRecentAllocations) },
    { NULL, NULL, NULL, NULL },
};
