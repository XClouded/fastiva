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
 * sun.misc.Unsafe
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"

#ifdef FASTIVA
#include <sun/misc/Unsafe.inl>
#  include <java/lang/reflect/Field.inl>
#endif
/*
 * private static native long objectFieldOffset0(Field field);
 */
static void Dalvik_sun_misc_Unsafe_objectFieldOffset0(const u4* args,
    JValue* pResult)
{
    Object* fieldObject = (Object*) args[0];

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_sun_misc_Unsafe_objectFieldOffset0(java_lang_reflect_Field_p);
	*(jlonglong*)pResult = fastiva_Dalvik_sun_misc_Unsafe_objectFieldOffset0((java_lang_reflect_Field_p)fieldObject);
}

jlonglong fastiva_Dalvik_sun_misc_Unsafe_objectFieldOffset0(java_lang_reflect_Field_p fieldObject) {
#endif

    InstField* field = (InstField*) dvmGetFieldFromReflectObj(fieldObject);
    s8 result = ((s8) field->byteOffset);

    RETURN_LONG(result);
}

/*
 * private static native int arrayBaseOffset0(Class clazz);
 */
static void Dalvik_sun_misc_Unsafe_arrayBaseOffset0(const u4* args,
    JValue* pResult)
{
    // The base offset is not type-dependent in this vm.
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jint fastiva_Dalvik_sun_misc_Unsafe_arrayBaseOffset0(java_lang_Class_p);
	*(jint*)pResult = fastiva_Dalvik_sun_misc_Unsafe_arrayBaseOffset0((java_lang_Class_p)args[0]);
}

jint fastiva_Dalvik_sun_misc_Unsafe_arrayBaseOffset0(java_lang_Class_p unused) {
#endif

    RETURN_INT(OFFSETOF_MEMBER(ArrayObject, contents));
}

/*
 * private static native int arrayIndexScale0(Class clazz);
 */
static void Dalvik_sun_misc_Unsafe_arrayIndexScale0(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	jint fastiva_Dalvik_sun_misc_Unsafe_arrayIndexScale0(java_lang_Class_p);
	*(jint*)pResult = fastiva_Dalvik_sun_misc_Unsafe_arrayIndexScale0((java_lang_Class_p)clazz);
}

jint fastiva_Dalvik_sun_misc_Unsafe_arrayIndexScale0(java_lang_Class_p clazz) {
#endif

    RETURN_INT(dvmArrayClassElementWidth(clazz));
}

/*
 * public native boolean compareAndSwapInt(Object obj, long offset,
 *         int expectedValue, int newValue);
 */
static void Dalvik_sun_misc_Unsafe_compareAndSwapInt(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4 expectedValue = args[4];
    s4 newValue = args[5];

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	jbool fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapInt(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jint, jint);
	*(jbool*)pResult = fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapInt((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, expectedValue, newValue);
}

jbool fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapInt(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jint expectedValue, jint newValue) {
#endif

    volatile int32_t* address = (volatile int32_t*) (((u1*) obj) + offset);

    // Note: android_atomic_release_cas() returns 0 on success, not failure.
    int result = android_atomic_release_cas(expectedValue, newValue, address);

    RETURN_BOOLEAN(result == 0);
}

/*
 * public native boolean compareAndSwapLong(Object obj, long offset,
 *         long expectedValue, long newValue);
 */
static void Dalvik_sun_misc_Unsafe_compareAndSwapLong(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8 expectedValue = GET_ARG_LONG(args, 4);
    s8 newValue = GET_ARG_LONG(args, 6);

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset+4);
#endif
	jbool fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapLong(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jlonglong, jlonglong);
	*(jbool*)pResult = fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapLong((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, expectedValue, newValue);
}

jbool fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapLong(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jlonglong expectedValue, jlonglong newValue) {
#endif
    volatile int64_t* address = (volatile int64_t*) (((u1*) obj) + offset);
    // Note: android_atomic_cmpxchg() returns 0 on success, not failure.
    int result =
        dvmQuasiAtomicCas64(expectedValue, newValue, address);

    RETURN_BOOLEAN(result == 0);
}

/*
 * public native boolean compareAndSwapObject(Object obj, long offset,
 *         Object expectedValue, Object newValue);
 */
static void Dalvik_sun_misc_Unsafe_compareAndSwapObject(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object* expectedValue = (Object*) args[4];
    Object* newValue = (Object*) args[5];

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	jbool fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapObject(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, java_lang_Object_p, java_lang_Object_p);
	*(jbool*)pResult = fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapObject((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, expectedValue, newValue);
}

jbool fastiva_Dalvik_sun_misc_Unsafe_compareAndSwapObject(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, java_lang_Object_p expectedValue, java_lang_Object_p newValue) {
#endif

    int32_t* address = (int32_t*) (((u1*) obj) + offset);

	// Note: android_atomic_cmpxchg() returns 0 on success, not failure.
    int result = android_atomic_release_cas((int32_t) expectedValue,
            (int32_t) newValue, address);
    dvmWriteBarrierField(obj, address);
    RETURN_BOOLEAN(result == 0);
}

/*
 * public native int getIntVolatile(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getIntVolatile(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);

#ifdef FASTIVA
	jint fastiva_Dalvik_sun_misc_Unsafe_getIntVolatile(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong);
	*(jint*)pResult = fastiva_Dalvik_sun_misc_Unsafe_getIntVolatile((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset);
}

jint fastiva_Dalvik_sun_misc_Unsafe_getIntVolatile(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset) {
#endif

	volatile int32_t* address = (volatile int32_t*) (((u1*) obj) + offset);

    int32_t value = android_atomic_acquire_load(address);
    RETURN_INT(value);
}

/*
 * public native void putIntVolatile(Object obj, long offset, int newValue);
 */
static void Dalvik_sun_misc_Unsafe_putIntVolatile(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4 value = (s4) args[4];


#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putIntVolatile(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jint);
	fastiva_Dalvik_sun_misc_Unsafe_putIntVolatile((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putIntVolatile(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jint value) {
#endif

	volatile int32_t* address = (volatile int32_t*) (((u1*) obj) + offset);

    android_atomic_release_store(value, address);
    RETURN_VOID();
}

/*
 * public native long getLongVolatile(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getLongVolatile(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_sun_misc_Unsafe_getLongVolatile(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong);
	*(jlonglong*)pResult = fastiva_Dalvik_sun_misc_Unsafe_getLongVolatile((sun_misc_Unsafe_p)args[0], obj, offset);
}

jlonglong fastiva_Dalvik_sun_misc_Unsafe_getLongVolatile(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset) {
#endif

    volatile int64_t* address = (volatile int64_t*) (((u1*) obj) + offset);

    assert((offset & 7) == 0);
    RETURN_LONG(dvmQuasiAtomicRead64(address));
}

/*
 * public native void putLongVolatile(Object obj, long offset, long newValue);
 */
static void Dalvik_sun_misc_Unsafe_putLongVolatile(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8 value = GET_ARG_LONG(args, 4);

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset+4);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putLongVolatile(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jlonglong);
	fastiva_Dalvik_sun_misc_Unsafe_putLongVolatile((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}
void fastiva_Dalvik_sun_misc_Unsafe_putLongVolatile(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jlonglong value) {
#endif

    volatile int64_t* address = (volatile int64_t*) (((u1*) obj) + offset);

    assert((offset & 7) == 0);
    dvmQuasiAtomicSwap64(value, address);
    RETURN_VOID();
}

/*
 * public native Object getObjectVolatile(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getObjectVolatile(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	java_lang_Object_p fastiva_Dalvik_sun_misc_Unsafe_getObjectVolatile(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong);
	*(java_lang_Object_p*)pResult = fastiva_Dalvik_sun_misc_Unsafe_getObjectVolatile((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset);
}

java_lang_Object_p fastiva_Dalvik_sun_misc_Unsafe_getObjectVolatile(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset) {
#endif

	volatile int32_t* address = (volatile int32_t*) (((u1*) obj) + offset);

    RETURN_PTR((Object*) android_atomic_acquire_load(address));
}

/*
 * public native void putObjectVolatile(Object obj, long offset,
 *         Object newValue);
 */
static void Dalvik_sun_misc_Unsafe_putObjectVolatile(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object* value = (Object*) args[4];

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putObjectVolatile(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, java_lang_Object_p);
	fastiva_Dalvik_sun_misc_Unsafe_putObjectVolatile((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putObjectVolatile(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, java_lang_Object_p value) {
#endif

    volatile int32_t* address = (volatile int32_t*) (((u1*) obj) + offset);

    android_atomic_release_store((int32_t)value, address);
    dvmWriteBarrierField(obj, (void *)address);
    RETURN_VOID();
}

/*
 * public native int getInt(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getInt(const u4* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);

#ifdef FASTIVA
	jint fastiva_Dalvik_sun_misc_Unsafe_getInt(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong);
	*(jint*)pResult = fastiva_Dalvik_sun_misc_Unsafe_getInt((sun_misc_Unsafe_p)args[0], obj, offset);
}

jint fastiva_Dalvik_sun_misc_Unsafe_getInt(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset) {
#endif

    s4* address = (s4*) (((u1*) obj) + offset);

    RETURN_INT(*address);
}

/*
 * public native void putInt(Object obj, long offset, int newValue);
 */
static void Dalvik_sun_misc_Unsafe_putInt(const u4* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4 value = (s4) args[4];

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putInt(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jint);
	fastiva_Dalvik_sun_misc_Unsafe_putInt((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putInt(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jint value) {
#endif

    s4* address = (s4*) (((u1*) obj) + offset);

    *address = value;
    RETURN_VOID();
}

/*
 * public native void putOrderedInt(Object obj, long offset, int newValue);
 */
static void Dalvik_sun_misc_Unsafe_putOrderedInt(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4 value = (s4) args[4];

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putOrderedInt(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jint);
	fastiva_Dalvik_sun_misc_Unsafe_putOrderedInt((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putOrderedInt(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jint value) {
#endif

    s4* address = (s4*) (((u1*) obj) + offset);

    ANDROID_MEMBAR_STORE();
    *address = value;
    RETURN_VOID();
}

/*
 * public native long getLong(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getLong(const u4* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_sun_misc_Unsafe_getLong(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong);
	*(jlonglong*)pResult = fastiva_Dalvik_sun_misc_Unsafe_getLong((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset);
}

jlonglong fastiva_Dalvik_sun_misc_Unsafe_getLong(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset) {
#endif

    s8* address = (s8*) (((u1*) obj) + offset);

    RETURN_LONG(*address);
}

/*
 * public native void putLong(Object obj, long offset, long newValue);
 */
static void Dalvik_sun_misc_Unsafe_putLong(const u4* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8 value = GET_ARG_LONG(args, 4);

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset+4);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putLong(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jlonglong);
	fastiva_Dalvik_sun_misc_Unsafe_putLong((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putLong(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jlonglong value) {
#endif

    s8* address = (s8*) (((u1*) obj) + offset);

    *address = value;
    RETURN_VOID();
}

/*
 * public native void putOrderedLong(Object obj, long offset, long newValue);
 */
static void Dalvik_sun_misc_Unsafe_putOrderedLong(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8 value = GET_ARG_LONG(args, 4);

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset+4);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putOrderedLong(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, jlonglong);
	fastiva_Dalvik_sun_misc_Unsafe_putOrderedLong((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putOrderedLong(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, jlonglong value) {
#endif

    s8* address = (s8*) (((u1*) obj) + offset);

    ANDROID_MEMBAR_STORE();
    *address = value;
    RETURN_VOID();
}

/*
 * public native Object getObject(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getObject(const u4* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);

#ifdef FASTIVA
	java_lang_Object_p fastiva_Dalvik_sun_misc_Unsafe_getObject(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong);
	*(java_lang_Object_p*)pResult = fastiva_Dalvik_sun_misc_Unsafe_getObject((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset);
}

java_lang_Object_p fastiva_Dalvik_sun_misc_Unsafe_getObject(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset) {
#endif

    Object** address = (Object**) (((u1*) obj) + offset);

    RETURN_PTR(*address);
}

/*
 * public native void putObject(Object obj, long offset, Object newValue);
 */
static void Dalvik_sun_misc_Unsafe_putObject(const u4* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object* value = (Object*) args[4];

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putObject(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, java_lang_Object_p);
	fastiva_Dalvik_sun_misc_Unsafe_putObject((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putObject(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, java_lang_Object_p value) {
#endif

    Object** address = (Object**) (((u1*) obj) + offset);

    *address = value;
    dvmWriteBarrierField(obj, address);
    RETURN_VOID();
}

/*
 * public native void putOrderedObject(Object obj, long offset,
 *      Object newValue);
 */
static void Dalvik_sun_misc_Unsafe_putOrderedObject(const u4* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object* value = (Object*) args[4];

#ifdef FASTIVA
#ifdef FASTIVA_GC_DEBUG
	fastiva_getFieldOffset((u1*)obj, (int)offset);
#endif
	void fastiva_Dalvik_sun_misc_Unsafe_putOrderedObject(sun_misc_Unsafe_p self, java_lang_Object_p, jlonglong, java_lang_Object_p);
	fastiva_Dalvik_sun_misc_Unsafe_putOrderedObject((sun_misc_Unsafe_p)args[0], (java_lang_Object_p)obj, offset, value);
}

void fastiva_Dalvik_sun_misc_Unsafe_putOrderedObject(sun_misc_Unsafe_p self, java_lang_Object_p obj, jlonglong offset, java_lang_Object_p value) {
#endif

    Object** address = (Object**) (((u1*) obj) + offset);

    ANDROID_MEMBAR_STORE();
    *address = value;
    dvmWriteBarrierField(obj, address);
    RETURN_VOID();
}

#ifdef FASTIVA
// 원래 libcore 에 jni 함수로 구현되어 있슴.
java_lang_Object_p fastiva_Dalvik_sun_misc_Unsafe_allocateInstance(sun_misc_Unsafe_p self, java_lang_Class_p pClass) {
  return fastiva.allocate(pClass);
}
#endif

const DalvikNativeMethod dvm_sun_misc_Unsafe[] = {
    { "objectFieldOffset0", "(Ljava/lang/reflect/Field;)J",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_objectFieldOffset0) },
    { "objectFieldOffset0", "(Ljava/lang/reflect/Field;)J",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_objectFieldOffset0) },
    { "arrayBaseOffset0", "(Ljava/lang/Class;)I",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_arrayBaseOffset0) },
    { "arrayIndexScale0", "(Ljava/lang/Class;)I",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_arrayIndexScale0) },
    { "compareAndSwapInt", "(Ljava/lang/Object;JII)Z",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_compareAndSwapInt) },
    { "compareAndSwapLong", "(Ljava/lang/Object;JJJ)Z",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_compareAndSwapLong) },
    { "compareAndSwapObject",
      "(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_compareAndSwapObject) },
    { "getIntVolatile", "(Ljava/lang/Object;J)I",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_getIntVolatile) },
    { "putIntVolatile", "(Ljava/lang/Object;JI)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putIntVolatile) },
    { "getLongVolatile", "(Ljava/lang/Object;J)J",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_getLongVolatile) },
    { "putLongVolatile", "(Ljava/lang/Object;JJ)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putLongVolatile) },
    { "getObjectVolatile", "(Ljava/lang/Object;J)Ljava/lang/Object;",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_getObjectVolatile) },
    { "putObjectVolatile", "(Ljava/lang/Object;JLjava/lang/Object;)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putObjectVolatile) },
    { "getInt", "(Ljava/lang/Object;J)I",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_getInt) },
    { "putInt", "(Ljava/lang/Object;JI)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putInt) },
    { "putOrderedInt", "(Ljava/lang/Object;JI)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putOrderedInt) },
    { "getLong", "(Ljava/lang/Object;J)J",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_getLong) },
    { "putLong", "(Ljava/lang/Object;JJ)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putLong) },
    { "putOrderedLong", "(Ljava/lang/Object;JJ)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putOrderedLong) },
    { "getObject", "(Ljava/lang/Object;J)Ljava/lang/Object;",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_getObject) },
    { "putObject", "(Ljava/lang/Object;JLjava/lang/Object;)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putObject) },
    { "putOrderedObject", "(Ljava/lang/Object;JLjava/lang/Object;)V",
      FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_sun_misc_Unsafe_putOrderedObject) },
    { NULL, NULL, NULL, NULL },
};
