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
 * java.lang.VMClassLoader
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"
#ifdef FASTIVA
#include <java/lang/ClassLoader.inl>
#endif

#if FASTIVA_TARGET_ANDROID_VERSION < 40400
/*
 * static Class defineClass(ClassLoader cl, String name,
 *     byte[] data, int offset, int len)
 *     throws ClassFormatError
 *
 * Convert an array of bytes to a Class object.
 */
static void Dalvik_java_lang_VMClassLoader_defineClass(const u4* args,
    JValue* pResult)
{
    Object* loader = (Object*) args[0];
    StringObject* nameObj = (StringObject*) args[1];
    const u1* data = (const u1*) args[2];
    int offset = args[3];
    int len = args[4];


#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_defineClass(java_lang_ClassLoader_p, java_lang_String_p, Byte_ap, jint, jint);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_VMClassLoader_defineClass((java_lang_ClassLoader_p)loader, (java_lang_String_p)nameObj, (Byte_ap)data, offset, len);
}

java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_defineClass(java_lang_ClassLoader_p loader, java_lang_String_p nameObj, Byte_ap data, jint offset, jint len) {
#endif

    char* name = NULL;

    name = dvmCreateCstrFromString(nameObj);
    ALOGE("ERROR: defineClass(%p, %s, %p, %d, %d)",
        loader, name, data, offset, len);
    dvmThrowUnsupportedOperationException(
        "can't load this type of class file");

    free(name);
    THROW_V();
}

/*
 * static Class defineClass(ClassLoader cl, byte[] data, int offset,
 *     int len)
 *     throws ClassFormatError
 *
 * Convert an array of bytes to a Class object. Deprecated version of
 * previous method, lacks name parameter.
 */
static void Dalvik_java_lang_VMClassLoader_defineClass2(const u4* args,
    JValue* pResult)
{
    Object* loader = (Object*) args[0];
    const u1* data = (const u1*) args[1];
    int offset = args[2];
    int len = args[3];
#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_defineClass2(java_lang_ClassLoader_p, Byte_ap, jint, jint);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_VMClassLoader_defineClass2((java_lang_ClassLoader_p)loader, (Byte_ap)data, offset, len);
}

java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_defineClass2(java_lang_ClassLoader_p loader, Byte_ap data, jint offset, jint len) {
#endif

    ALOGE("ERROR: defineClass(%p, %p, %d, %d)",
        loader, data, offset, len);
    dvmThrowUnsupportedOperationException(
        "can't load this type of class file");

    THROW_V();
}
#endif

/*
 * static Class findLoadedClass(ClassLoader cl, String name)
 */
static void Dalvik_java_lang_VMClassLoader_findLoadedClass(const u4* args,
    JValue* pResult)
{
    Object* loader = (Object*) args[0];
    StringObject* nameObj = (StringObject*) args[1];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_findLoadedClass(java_lang_ClassLoader_p, java_lang_String_p);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_VMClassLoader_findLoadedClass((java_lang_ClassLoader_p) loader, (java_lang_String_p)nameObj);
}

java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_findLoadedClass(java_lang_ClassLoader_p loader, java_lang_String_p nameObj) {
#endif

    ClassObject* clazz = NULL;
    char* name = NULL;
    char* descriptor = NULL;

    if (nameObj == NULL) {
        dvmThrowNullPointerException("name == null");
        goto bail;
    }

    /*
     * Get a UTF-8 copy of the string, and convert dots to slashes.
     */
    name = dvmCreateCstrFromString(nameObj);
    if (name == NULL)
        goto bail;

    descriptor = dvmDotToDescriptor(name);
    if (descriptor == NULL)
        goto bail;

    clazz = dvmLookupClass(descriptor, loader, false);
    LOGVV("look: %s ldr=%p --> %p", descriptor, loader, clazz);

bail:
    free(name);
    free(descriptor);
    RETURN_PTR((java_lang_Class_p)clazz);
}

/*
 * private static int getBootClassPathSize()
 *
 * Get the number of entries in the boot class path.
 */
static void Dalvik_java_lang_VMClassLoader_getBootClassPathSize(const u4* args,
    JValue* pResult)
{

#ifdef FASTIVA
	jint fastiva_Dalvik_java_lang_VMClassLoader_getBootClassPathSize();
	*(jint*)pResult = fastiva_Dalvik_java_lang_VMClassLoader_getBootClassPathSize();
}

jint fastiva_Dalvik_java_lang_VMClassLoader_getBootClassPathSize() {
#endif

	int count = dvmGetBootPathSize();
    RETURN_INT(count);
}

/*
 * private static String getBootClassPathResource(String name, int index)
 *
 * Find a resource with a matching name in a boot class path entry.
 *
 * This mimics the previous VM interface, since we're sharing class libraries.
 */
static void Dalvik_java_lang_VMClassLoader_getBootClassPathResource(
    const u4* args, JValue* pResult)
{
    StringObject* nameObj = (StringObject*) args[0];
    int idx = args[1];

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_java_lang_VMClassLoader_getBootClassPathResource(java_lang_String_p, jint);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_java_lang_VMClassLoader_getBootClassPathResource((java_lang_String_p)nameObj, idx);
}

java_lang_String_p fastiva_Dalvik_java_lang_VMClassLoader_getBootClassPathResource(java_lang_String_p nameObj, jint idx) {
#endif

	StringObject* result;
    char* name;

    name = dvmCreateCstrFromString(nameObj);
    if (name == NULL)
        RETURN_PTR(NULL);

    result = dvmGetBootPathResource(name, idx);
    free(name);
    dvmReleaseTrackedAlloc((Object*)result, NULL);
    RETURN_PTR((java_lang_String_p)result);
}

/*
 * static final Class getPrimitiveClass(char prim_type)
 */
static void Dalvik_java_lang_VMClassLoader_getPrimitiveClass(const u4* args,
    JValue* pResult)
{
    int primType = args[0];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_getPrimitiveClass(unicod);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_VMClassLoader_getPrimitiveClass((unicod)primType);
}

java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_getPrimitiveClass(unicod primType) {
#endif

    Object* res = (Object*)dvmFindPrimitiveClass((char)primType);
	RETURN_PTR((java_lang_Class_p)res);
}

/*
 * static Class loadClass(String name, boolean resolve)
 *     throws ClassNotFoundException
 *
 * Load class using bootstrap class loader.
 *
 * Return the Class object associated with the class or interface with
 * the specified name.
 *
 * "name" is in "binary name" format, e.g. "dalvik.system.Debug$1".
 */
static void Dalvik_java_lang_VMClassLoader_loadClass(const u4* args,
    JValue* pResult)
{
    StringObject* nameObj = (StringObject*) args[0];
    bool resolve = (args[1] != 0);

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_loadClass(java_lang_String_p, jbool);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_VMClassLoader_loadClass((java_lang_String_p)nameObj, resolve);
}

java_lang_Class_p fastiva_Dalvik_java_lang_VMClassLoader_loadClass(java_lang_String_p nameObj, jbool resolve) {
#endif

	ClassObject* clazz;
    clazz = dvmFindClassByName(nameObj, NULL, resolve);

	if (clazz == NULL) {
		fastiva_popDalvikException();
	}

    assert(clazz == NULL || dvmIsClassLinked(clazz));
	RETURN_PTR((java_lang_Class_p)clazz);
}

const DalvikNativeMethod dvm_java_lang_VMClassLoader[] = {
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
    { "defineClass",        "(Ljava/lang/ClassLoader;Ljava/lang/String;[BII)Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_VMClassLoader_defineClass) },
    { "defineClass",        "(Ljava/lang/ClassLoader;[BII)Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_VMClassLoader_defineClass2) },
#endif
    { "findLoadedClass",    "(Ljava/lang/ClassLoader;Ljava/lang/String;)Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_VMClassLoader_findLoadedClass) },
    { "getBootClassPathSize", "()I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_VMClassLoader_getBootClassPathSize) },
    { "getBootClassPathResource", "(Ljava/lang/String;I)Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_VMClassLoader_getBootClassPathResource) },
    { "getPrimitiveClass",  "(C)Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_VMClassLoader_getPrimitiveClass) },
    { "loadClass",          "(Ljava/lang/String;Z)Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_VMClassLoader_loadClass) },
    { NULL, NULL, NULL, NULL },
};

