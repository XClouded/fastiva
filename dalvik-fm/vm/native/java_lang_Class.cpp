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
 * java.lang.Class
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"
#include "ScopedPthreadMutexLock.h"

#ifdef FASTIVA
#include <dalvik_Kernel.h>
#include <java/lang/reflect/Member.inl>
#include <java/lang/StringBuilder.inl>
#include <java/lang/ClassLoader.inl>
#endif
/*
 * native public boolean desiredAssertionStatus()
 *
 * Determine the class-init-time assertion status of a class.  This is
 * called from <clinit> in javac-generated classes that use the Java
 * programming language "assert" keyword.
 */
static void Dalvik_java_lang_Class_desiredAssertionStatus(const u4* args,
    JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];


#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_Class_desiredAssertionStatus(java_lang_Class_p self);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_Class_desiredAssertionStatus((java_lang_Class_p)thisPtr);
}

jbool fastiva_Dalvik_java_lang_Class_desiredAssertionStatus(java_lang_Class_p thisPtr) {
#endif

	char* className = dvmDescriptorToName(thisPtr->descriptor);
    int i;
    bool enable = false;

    /*
     * Run through the list of arguments specified on the command line.  The
     * last matching argument takes precedence.
     */
    for (i = 0; i < gDvm.assertionCtrlCount; i++) {
        const AssertionControl* pCtrl = &gDvm.assertionCtrl[i];

        if (pCtrl->isPackage) {
            /*
             * Given "dalvik/system/Debug" or "MyStuff", compute the
             * length of the package portion of the class name string.
             *
             * Unlike most package operations, we allow matching on
             * "sub-packages", so "dalvik..." will match "dalvik.Foo"
             * and "dalvik.system.Foo".
             *
             * The pkgOrClass string looks like "dalvik/system/", i.e. it still
             * has the terminating slash, so we can be sure we're comparing
             * against full package component names.
             */
            const char* lastSlash;
            int pkgLen;

            lastSlash = strrchr(className, '/');
            if (lastSlash == NULL) {
                pkgLen = 0;
            } else {
                pkgLen = lastSlash - className +1;
            }

            if (pCtrl->pkgOrClassLen > pkgLen ||
                memcmp(pCtrl->pkgOrClass, className, pCtrl->pkgOrClassLen) != 0)
            {
                ALOGV("ASRT: pkg no match: '%s'(%d) vs '%s'",
                    className, pkgLen, pCtrl->pkgOrClass);
            } else {
                ALOGV("ASRT: pkg match: '%s'(%d) vs '%s' --> %d",
                    className, pkgLen, pCtrl->pkgOrClass, pCtrl->enable);
                enable = pCtrl->enable;
            }
        } else {
            /*
             * "pkgOrClass" holds a fully-qualified class name, converted from
             * dot-form to slash-form.  An empty string means all classes.
             */
            if (pCtrl->pkgOrClass == NULL) {
                /* -esa/-dsa; see if class is a "system" class */
                if (strncmp(className, "java/", 5) != 0) {
                    ALOGV("ASRT: sys no match: '%s'", className);
                } else {
                    ALOGV("ASRT: sys match: '%s' --> %d",
                        className, pCtrl->enable);
                    enable = pCtrl->enable;
                }
            } else if (*pCtrl->pkgOrClass == '\0') {
                ALOGV("ASRT: class all: '%s' --> %d",
                    className, pCtrl->enable);
                enable = pCtrl->enable;
            } else {
                if (strcmp(pCtrl->pkgOrClass, className) != 0) {
                    ALOGV("ASRT: cls no match: '%s' vs '%s'",
                        className, pCtrl->pkgOrClass);
                } else {
                    ALOGV("ASRT: cls match: '%s' vs '%s' --> %d",
                        className, pCtrl->pkgOrClass, pCtrl->enable);
                    enable = pCtrl->enable;
                }
            }
        }
    }

    free(className);
    RETURN_INT(enable);
}

/*
 * static public Class<?> classForName(String name, boolean initialize,
 *     ClassLoader loader)
 *
 * Return the Class object associated with the class or interface with
 * the specified name.
 *
 * "name" is in "binary name" format, e.g. "dalvik.system.Debug$1".
 */
static void Dalvik_java_lang_Class_classForName(const u4* args, JValue* pResult)
{
    StringObject* nameObj = (StringObject*) args[0];
    bool initialize = (args[1] != 0);
    Object* loader = (Object*) args[2];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_Class_classForName(java_lang_String_p, jbool, java_lang_ClassLoader_p);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_Class_classForName((java_lang_String_p)nameObj, initialize, (java_lang_ClassLoader_p)loader);
}

java_lang_Class_p fastiva_Dalvik_java_lang_Class_classForName(java_lang_String_p nameObj, jbool initialize, java_lang_ClassLoader_p loader) {
#endif
	//if (loader == NULL && kernelData.g_appModule != NULL) {
	//	loader = kernelData.g_appModule->m_pClassLoader;
	//}
    RETURN_PTR((java_lang_Class_p)dvmFindClassByName(nameObj, loader, initialize));
}

/*
 * static private ClassLoader getClassLoader(Class clazz)
 *
 * Return the class' defining class loader.
 */
static void Dalvik_java_lang_Class_getClassLoader(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];


#ifdef FASTIVA
	java_lang_ClassLoader_p fastiva_Dalvik_java_lang_Class_getClassLoader(java_lang_Class_p);
	*(java_lang_ClassLoader_p*)pResult = fastiva_Dalvik_java_lang_Class_getClassLoader((java_lang_Class_p)clazz);
}

java_lang_ClassLoader_p fastiva_Dalvik_java_lang_Class_getClassLoader(java_lang_Class_p clazz) {
#endif

	RETURN_PTR((java_lang_ClassLoader_p)clazz->classLoader);
}

/*
 * public Class<?> getComponentType()
 *
 * If this is an array type, return the class of the elements; otherwise
 * return NULL.
 */
static void Dalvik_java_lang_Class_getComponentType(const u4* args,
    JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_Class_getComponentType(java_lang_Class_p self);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_Class_getComponentType((java_lang_Class_p) thisPtr);
}

java_lang_Class_p fastiva_Dalvik_java_lang_Class_getComponentType(java_lang_Class_p thisPtr) {
#endif

    if (!dvmIsArrayClass(thisPtr))
        RETURN_PTR(NULL);

    /*
     * We can't just return thisPtr->elementClass, because that gives
     * us the base type (e.g. X[][][] returns X).  If this is a multi-
     * dimensional array, we have to do the lookup by name.
     */
    if (thisPtr->descriptor[1] == '[') {
        RETURN_PTR((java_lang_Class_p)dvmFindArrayClass(&thisPtr->descriptor[1],
                   thisPtr->classLoader));
	}
    else
        RETURN_PTR((java_lang_Class_p)thisPtr->elementClass);
}

/*
 * private static Class<?>[] getDeclaredClasses(Class<?> clazz,
 *     boolean publicOnly)
 *
 * Return an array with the classes that are declared by the specified class.
 * If "publicOnly" is set, we strip out any classes that don't have "public"
 * access.
 */
static void Dalvik_java_lang_Class_getDeclaredClasses(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    bool publicOnly = (args[1] != 0);

#ifdef FASTIVA
	java_lang_Class_ap fastiva_Dalvik_java_lang_Class_getDeclaredClasses(java_lang_Class_p, jbool);
	*(java_lang_Class_ap*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredClasses((java_lang_Class_p) clazz, publicOnly);
}

java_lang_Class_ap fastiva_Dalvik_java_lang_Class_getDeclaredClasses(java_lang_Class_p clazz, jbool publicOnly) {
#endif

	ArrayObject* classes;

    classes = dvmGetDeclaredClasses(clazz);
    if (classes == NULL) {
        if (!dvmCheckException(dvmThreadSelf())) {
            /* empty list, so create a zero-length array */
            classes = dvmAllocArrayByClass(gDvm.classJavaLangClassArray,
                        0, ALLOC_DEFAULT);
        }
    } else if (publicOnly) {
        u4 count, newIdx, publicCount = 0;
        ClassObject** pSource = (ClassObject**)(void*)classes->contents;
        u4 length = classes->length;

        /* count up public classes */
        for (count = 0; count < length; count++) {
            if (dvmIsPublicClass(pSource[count]))
                publicCount++;
        }

        /* create a new array to hold them */
        ArrayObject* newClasses;
        newClasses = dvmAllocArrayByClass(gDvm.classJavaLangClassArray,
                        publicCount, ALLOC_DEFAULT);

        /* copy them over */
        for (count = newIdx = 0; count < length; count++) {
            if (dvmIsPublicClass(pSource[count])) {
                dvmSetObjectArrayElement(newClasses, newIdx,
                                         (Object *)pSource[count]);
                newIdx++;
            }
        }
        assert(newIdx == publicCount);
        dvmReleaseTrackedAlloc((Object*) classes, NULL);
        classes = newClasses;
    }

    dvmReleaseTrackedAlloc((Object*) classes, NULL);
    RETURN_PTR((java_lang_Class_ap)classes);
}

/*
 * static Constructor[] getDeclaredConstructors(Class clazz, boolean publicOnly)
 *     throws SecurityException
 */
static void Dalvik_java_lang_Class_getDeclaredConstructors(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    bool publicOnly = (args[1] != 0);

#ifdef FASTIVA
	java_lang_reflect_Constructor_ap fastiva_Dalvik_java_lang_Class_getDeclaredConstructors(java_lang_Class_p, jbool publicOnly);
	*(java_lang_reflect_Constructor_ap*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredConstructors((java_lang_Class_p)clazz, publicOnly);
}

java_lang_reflect_Constructor_ap fastiva_Dalvik_java_lang_Class_getDeclaredConstructors(java_lang_Class_p clazz, jbool publicOnly) {
#endif

	ArrayObject* constructors;

    constructors = dvmGetDeclaredConstructors(clazz, publicOnly);
    dvmReleaseTrackedAlloc((Object*) constructors, NULL);

    RETURN_PTR((java_lang_reflect_Constructor_ap)constructors);
}

/*
 * static Field[] getDeclaredFields(Class klass, boolean publicOnly)
 */
static void Dalvik_java_lang_Class_getDeclaredFields(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    bool publicOnly = (args[1] != 0);

#ifdef FASTIVA
	java_lang_reflect_Field_ap fastiva_Dalvik_java_lang_Class_getDeclaredFields(java_lang_Class_p, jbool);
	*(java_lang_reflect_Field_ap*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredFields((java_lang_Class_p)clazz, publicOnly);
}

java_lang_reflect_Field_ap fastiva_Dalvik_java_lang_Class_getDeclaredFields(java_lang_Class_p clazz, jbool publicOnly) {
#endif

	ArrayObject* fields;

    fields = dvmGetDeclaredFields(clazz, publicOnly);
    dvmReleaseTrackedAlloc((Object*) fields, NULL);

    RETURN_PTR((java_lang_reflect_Field_ap)fields);
}

/*
 * static Field getDeclaredField(Class klass, String name)
 */
static void Dalvik_java_lang_Class_getDeclaredField(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    StringObject* nameObj = (StringObject*) args[1];

#ifdef FASTIVA
	java_lang_reflect_Field_p fastiva_Dalvik_java_lang_Class_getDeclaredField(java_lang_Class_p, java_lang_String_p);
	*(java_lang_reflect_Field_p*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredField((java_lang_Class_p)clazz, (java_lang_String_p)nameObj);
}

java_lang_reflect_Field_p fastiva_Dalvik_java_lang_Class_getDeclaredField(java_lang_Class_p clazz, java_lang_String_p nameObj) {
#endif

	Object* fieldObj = dvmGetDeclaredField(clazz, nameObj);
    dvmReleaseTrackedAlloc((Object*) fieldObj, NULL);
    RETURN_PTR((java_lang_reflect_Field_p)fieldObj);
}

/*
 * static Method[] getDeclaredMethods(Class clazz, boolean publicOnly)
 *     throws SecurityException
 */
static void Dalvik_java_lang_Class_getDeclaredMethods(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    bool publicOnly = (args[1] != 0);

#ifdef FASTIVA
	java_lang_reflect_Method_ap fastiva_Dalvik_java_lang_Class_getDeclaredMethods(java_lang_Class_p, jbool);
	*(java_lang_reflect_Method_ap*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredMethods((java_lang_Class_p)clazz, publicOnly);
}

java_lang_reflect_Method_ap fastiva_Dalvik_java_lang_Class_getDeclaredMethods(java_lang_Class_p clazz, jbool publicOnly) {
#endif

	ArrayObject* methods;

    methods = dvmGetDeclaredMethods(clazz, publicOnly);
    dvmReleaseTrackedAlloc((Object*) methods, NULL);

    RETURN_PTR((java_lang_reflect_Method_ap)methods);
}

/*
 * static native Member getDeclaredConstructorOrMethod(
 *     Class clazz, String name, Class[] args);
 */
static void Dalvik_java_lang_Class_getDeclaredConstructorOrMethod(
    const u4* args, JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    StringObject* nameObj = (StringObject*) args[1];
    ArrayObject* methodArgs = (ArrayObject*) args[2];


#ifdef FASTIVA
	java_lang_reflect_Member_p fastiva_Dalvik_java_lang_Class_getDeclaredConstructorOrMethod(java_lang_Class_p, java_lang_String_p, java_lang_Class_ap);
	*(java_lang_reflect_Member_p*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredConstructorOrMethod((java_lang_Class_p)clazz, (java_lang_String_p)nameObj, (java_lang_Class_ap)methodArgs);
}

java_lang_reflect_Member_p fastiva_Dalvik_java_lang_Class_getDeclaredConstructorOrMethod(java_lang_Class_p clazz, java_lang_String_p nameObj, java_lang_Class_ap arg2) {
    ArrayObject* methodArgs = (ArrayObject*) arg2;
#endif

	Object* methodObj;

    methodObj = dvmGetDeclaredConstructorOrMethod(clazz, nameObj, methodArgs);
    dvmReleaseTrackedAlloc(methodObj, NULL);

    RETURN_PTR((java_lang_reflect_Member_p)methodObj);
}

/*
 * Class[] getInterfaces()
 */
static void Dalvik_java_lang_Class_getInterfaces(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_Class_ap fastiva_Dalvik_java_lang_Class_getInterfaces(java_lang_Class_p self);
	*(java_lang_Class_ap*)pResult = fastiva_Dalvik_java_lang_Class_getInterfaces((java_lang_Class_p)clazz);
}

java_lang_Class_ap fastiva_Dalvik_java_lang_Class_getInterfaces(java_lang_Class_p clazz) {
#endif

	ArrayObject* interfaces;

    interfaces = dvmGetInterfaces(clazz);
    dvmReleaseTrackedAlloc((Object*) interfaces, NULL);

    RETURN_PTR((java_lang_Class_ap)interfaces);
}

/*
 * private static int getModifiers(Class klass, boolean
 *     ignoreInnerClassesAttrib)
 *
 * Return the class' modifier flags.  If "ignoreInnerClassesAttrib" is false,
 * and this is an inner class, we return the access flags from the inner class
 * attribute.
 */
static void Dalvik_java_lang_Class_getModifiers(const u4* args, JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    bool ignoreInner = args[1];

#ifdef FASTIVA
	jint fastiva_Dalvik_java_lang_Class_getModifiers(java_lang_Class_p, jbool);
	*(jint*)pResult = fastiva_Dalvik_java_lang_Class_getModifiers((java_lang_Class_p)clazz, ignoreInner);
}

jint fastiva_Dalvik_java_lang_Class_getModifiers(java_lang_Class_p clazz, jbool ignoreInner) {
#endif
    u4 accessFlags;

    accessFlags = clazz->accessFlags & JAVA_FLAGS_MASK;

#ifdef FASTIVA
	if (d2f_isFastivaClass(clazz)) {
	    RETURN_INT(accessFlags);
	}
#endif
	// 2014.07.31 아래가 왜 필요하지???

    if (!ignoreInner) {
        /* see if we have an InnerClass annotation with flags in it */
        StringObject* className = NULL;
        int innerFlags;

        if (dvmGetInnerClass(clazz, &className, &innerFlags))
            accessFlags = innerFlags & JAVA_FLAGS_MASK;

        dvmReleaseTrackedAlloc((Object*) className, NULL);
    }

    RETURN_INT(accessFlags);
}

/*
 * private native String getNameNative()
 *
 * Return the class' name. The exact format is bizarre, but it's the specified
 * behavior: keywords for primitive types, regular "[I" form for primitive
 * arrays (so "int" but "[I"), and arrays of reference types written
 * between "L" and ";" but with dots rather than slashes (so "java.lang.String"
 * but "[Ljava.lang.String;"). Madness.
 */
static void Dalvik_java_lang_Class_getNameNative(const u4* args, JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_java_lang_Class_getNameNative(java_lang_Class_p self);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_java_lang_Class_getNameNative((java_lang_Class_p)clazz);
}

java_lang_String_p fastiva_Dalvik_java_lang_Class_getNameNative(java_lang_Class_p clazz) {
#endif

	const char* descriptor = clazz->descriptor;
    StringObject* nameObj;

    if ((descriptor[0] != 'L') && (descriptor[0] != '[')) {
        /*
         * The descriptor indicates that this is the class for
         * a primitive type; special-case the return value.
         */
        const char* name;
        switch (descriptor[0]) {
            case 'Z': name = "boolean"; break;
            case 'B': name = "byte";    break;
            case 'C': name = "char";    break;
            case 'S': name = "short";   break;
            case 'I': name = "int";     break;
            case 'J': name = "long";    break;
            case 'F': name = "float";   break;
            case 'D': name = "double";  break;
            case 'V': name = "void";    break;
            default: {
                ALOGE("Unknown primitive type '%c'", descriptor[0]);
                assert(false);
                RETURN_PTR(NULL);
            }
        }

        nameObj = dvmCreateStringFromCstr(name);
    } else {
        /*
         * Convert the UTF-8 name to a java.lang.String. The
         * name must use '.' to separate package components.
         *
         * TODO: this could be more efficient. Consider a custom
         * conversion function here that walks the string once and
         * avoids the allocation for the common case (name less than,
         * say, 128 bytes).
         */
        char* dotName = dvmDescriptorToDot(clazz->descriptor);
        nameObj = dvmCreateStringFromCstr(dotName);
        free(dotName);
    }

    dvmReleaseTrackedAlloc((Object*) nameObj, NULL);
    RETURN_PTR((java_lang_String_p)nameObj);
}

/*
 * Return the superclass for instances of this class.
 *
 * If the class represents a java/lang/Object, an interface, a primitive
 * type, or void (which *is* a primitive type??), return NULL.
 *
 * For an array, return the java/lang/Object ClassObject.
 */
static void Dalvik_java_lang_Class_getSuperclass(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_Class_getSuperclass(java_lang_Class_p self);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_Class_getSuperclass((java_lang_Class_p)clazz);
}

java_lang_Class_p fastiva_Dalvik_java_lang_Class_getSuperclass(java_lang_Class_p clazz) {
#endif

    if (dvmIsPrimitiveClass(clazz) || dvmIsInterfaceClass(clazz)) {
        RETURN_PTR(NULL);
	}
    else {
        RETURN_PTR((java_lang_Class_p)clazz->super);
	}
}

/*
 * public boolean isAssignableFrom(Class<?> cls)
 *
 * Determine if this class is either the same as, or is a superclass or
 * superinterface of, the class specified in the "cls" parameter.
 */
static void Dalvik_java_lang_Class_isAssignableFrom(const u4* args,
    JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];
    ClassObject* testClass = (ClassObject*) args[1];

#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_Class_isAssignableFrom(java_lang_Class_p self, java_lang_Class_p);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_Class_isAssignableFrom((java_lang_Class_p)thisPtr, (java_lang_Class_p)testClass);
}

jbool fastiva_Dalvik_java_lang_Class_isAssignableFrom(java_lang_Class_p thisPtr, java_lang_Class_p testClass) {
#endif

    if (testClass == NULL) {
        dvmThrowNullPointerException("cls == null");
        RETURN_INT(false);
    }
    RETURN_INT(dvmInstanceof(testClass, thisPtr));
}

/*
 * public boolean isInstance(Object o)
 *
 * Dynamic equivalent of Java programming language "instanceof".
 */
static void Dalvik_java_lang_Class_isInstance(const u4* args,
    JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];
    Object* testObj = (Object*) args[1];

#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_Class_isInstance(java_lang_Class_p self, java_lang_Object_p);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_Class_isInstance((java_lang_Class_p)thisPtr, (java_lang_Object_p)testObj);
}

jbool fastiva_Dalvik_java_lang_Class_isInstance(java_lang_Class_p thisPtr, java_lang_Object_p testObj) {
#endif

    if (testObj == NULL)
        RETURN_INT(false);
    RETURN_INT(dvmInstanceof(testObj->clazz, thisPtr));
}

/*
 * public boolean isInterface()
 */
static void Dalvik_java_lang_Class_isInterface(const u4* args,
    JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];

#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_Class_isInterface(java_lang_Class_p self);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_Class_isInterface((java_lang_Class_p)thisPtr);
}

jbool fastiva_Dalvik_java_lang_Class_isInterface(java_lang_Class_p thisPtr) {
#endif

    RETURN_INT(dvmIsInterfaceClass(thisPtr));
}

/*
 * public boolean isPrimitive()
 */
static void Dalvik_java_lang_Class_isPrimitive(const u4* args,
    JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];

#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_Class_isPrimitive(java_lang_Class_p self);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_Class_isPrimitive((java_lang_Class_p)thisPtr);
}

jbool fastiva_Dalvik_java_lang_Class_isPrimitive(java_lang_Class_p thisPtr) {
#endif

    RETURN_INT(dvmIsPrimitiveClass(thisPtr));
}

/*
 * public T newInstance() throws InstantiationException, IllegalAccessException
 *
 * Create a new instance of this class.
 */
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
#define Dalvik_java_lang_Class_newInstanceImpl   Dalvik_java_lang_Class_newInstance
#endif
static void Dalvik_java_lang_Class_newInstanceImpl(const u4* args, JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_Object_p fastiva_Dalvik_java_lang_Class_newInstanceImpl(java_lang_Class_p self);
	*(java_lang_Object_p*)pResult = fastiva_Dalvik_java_lang_Class_newInstanceImpl((java_lang_Class_p)clazz);
}

java_lang_Object_p fastiva_Dalvik_java_lang_Class_newInstanceImpl(java_lang_Class_p clazz) {
#endif

	Thread* self = dvmThreadSelf();
    Method* init;
    Object* newObj;

    /* can't instantiate these */
    if (dvmIsPrimitiveClass(clazz) || dvmIsInterfaceClass(clazz)
        || dvmIsArrayClass(clazz) || dvmIsAbstractClass(clazz))
    {
        ALOGD("newInstance failed: p%d i%d [%d a%d",
            dvmIsPrimitiveClass(clazz), dvmIsInterfaceClass(clazz),
            dvmIsArrayClass(clazz), dvmIsAbstractClass(clazz));
        dvmThrowInstantiationException(clazz, NULL);
        THROW_V();
    }

    /* initialize the class if it hasn't been already */
    if (!dvmIsClassInitialized(clazz)) {
        if (!dvmInitClass(clazz)) {
            ALOGW("Class init failed in newInstance call (%s)",
                clazz->descriptor);
            assert(dvmCheckException(self));
            THROW_V();
        }
    }

    /* find the "nullary" constructor */
    init = dvmFindDirectMethodByDescriptor(clazz, "<init>", "()V");
    if (init == NULL) {
        /* common cause: secret "this" arg on non-static inner class ctor */
        ALOGD("newInstance failed: no <init>()");
        dvmThrowInstantiationException(clazz, "no empty constructor");
        THROW_V();
    }

#ifdef _DEBUG
	//ALOGD("### newInstance() for %s", clazz->descriptor);
#endif
    /*
     * Verify access from the call site.
     *
     * First, make sure the method invoking Class.newInstance() has permission
     * to access the class.
     *
     * Second, make sure it has permission to invoke the constructor.  The
     * constructor must be public or, if the caller is in the same package,
     * have package scope.
     */
#ifdef FASTIVA
	ClassObject* d2f_getCallerClass(Thread* self);
    ClassObject* callerClass = d2f_getCallerClass(self);
#else
	// @zee newInstance() -> newInstanceImpl() -> current method
    ClassObject* callerClass = dvmGetCaller2Class(self->interpSave.curFrame);
#endif

    if (!dvmCheckClassAccess(callerClass, clazz)) {
        ALOGD("newInstance failed: %s not accessible to %s",
            clazz->descriptor, callerClass->descriptor);
        dvmThrowIllegalAccessException("access to class not allowed");
        THROW_V();
    }
    if (!dvmCheckMethodAccess(callerClass, init)) {
        ALOGD("newInstance failed: %s.<init>() not accessible to %s",
            clazz->descriptor, callerClass->descriptor);
        dvmThrowIllegalAccessException("access to constructor not allowed");
        THROW_V();
    }

    newObj = dvmAllocObject(clazz, ALLOC_DEFAULT);
    JValue unused;

    /* invoke constructor; unlike reflection calls, we don't wrap exceptions */
#ifdef FASTIVA
	if (d2f_isFastivaMethod(init)) {
		dvmReleaseTrackedAlloc(newObj, NULL);
		((void (*)(void*))init->fastivaMethod)(newObj);
	}
	else {
#endif
    dvmCallMethod(self, init, newObj, &unused);
    dvmReleaseTrackedAlloc(newObj, NULL);
#ifdef FASTIVA
	}
#endif

    RETURN_PTR(newObj);
}


#if FASTIVA_TARGET_ANDROID_VERSION < 40400
/*
 * private Object[] getSignatureAnnotation()
 *
 * Returns the signature annotation array.
 */
static void Dalvik_java_lang_Class_getSignatureAnnotation(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_Object_ap fastiva_Dalvik_java_lang_Class_getSignatureAnnotation(java_lang_Class_p self);
	*(java_lang_Object_ap*)pResult = fastiva_Dalvik_java_lang_Class_getSignatureAnnotation((java_lang_Class_p)clazz);
}

java_lang_Object_ap fastiva_Dalvik_java_lang_Class_getSignatureAnnotation(java_lang_Class_p clazz) {
#endif

	ArrayObject* arr = dvmGetClassSignatureAnnotation(clazz);

    dvmReleaseTrackedAlloc((Object*) arr, NULL);
    RETURN_PTR((java_lang_Object_ap)arr);
}
#endif

/*
 * public Class getDeclaringClass()
 *
 * Get the class that encloses this class (if any).
 */
static void Dalvik_java_lang_Class_getDeclaringClass(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_Class_getDeclaringClass(java_lang_Class_p self);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaringClass((java_lang_Class_p) clazz);
}

java_lang_Class_p fastiva_Dalvik_java_lang_Class_getDeclaringClass(java_lang_Class_p clazz) {
#endif

    ClassObject* enclosing = dvmGetDeclaringClass(clazz);
    dvmReleaseTrackedAlloc((Object*) enclosing, NULL);
    RETURN_PTR((java_lang_Class_p)enclosing);
}

/*
 * public Class getEnclosingClass()
 *
 * Get the class that encloses this class (if any).
 */
static void Dalvik_java_lang_Class_getEnclosingClass(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_Class_getEnclosingClass(java_lang_Class_p self);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_Class_getEnclosingClass((java_lang_Class_p) clazz);
}

java_lang_Class_p fastiva_Dalvik_java_lang_Class_getEnclosingClass(java_lang_Class_p clazz) {
#endif

    ClassObject* enclosing = dvmGetEnclosingClass(clazz);
    dvmReleaseTrackedAlloc((Object*) enclosing, NULL);
    RETURN_PTR((java_lang_Class_p)enclosing);
}

/*
 * public Constructor getEnclosingConstructor()
 *
 * Get the constructor that encloses this class (if any).
 */
static void Dalvik_java_lang_Class_getEnclosingConstructor(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_reflect_Constructor_p fastiva_Dalvik_java_lang_Class_getEnclosingConstructor(java_lang_Class_p clazz);
	*(java_lang_reflect_Constructor_p*)pResult = fastiva_Dalvik_java_lang_Class_getEnclosingConstructor((java_lang_Class_p) clazz);
}

java_lang_reflect_Constructor_p fastiva_Dalvik_java_lang_Class_getEnclosingConstructor(java_lang_Class_p clazz) {
#endif

    Object* enclosing = dvmGetEnclosingMethod(clazz);
    if (enclosing != NULL) {
        dvmReleaseTrackedAlloc(enclosing, NULL);
        if (enclosing->clazz == gDvm.classJavaLangReflectConstructor) {
            RETURN_PTR((java_lang_reflect_Constructor_p)enclosing);
        }
        assert(enclosing->clazz == gDvm.classJavaLangReflectMethod);
    }
    RETURN_PTR(NULL);
}

/*
 * public Method getEnclosingMethod()
 *
 * Get the method that encloses this class (if any).
 */
static void Dalvik_java_lang_Class_getEnclosingMethod(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_reflect_Method_p fastiva_Dalvik_java_lang_Class_getEnclosingMethod(java_lang_Class_p self);
	*(java_lang_reflect_Method_p*)pResult = fastiva_Dalvik_java_lang_Class_getEnclosingMethod((java_lang_Class_p) clazz);
}

java_lang_reflect_Method_p fastiva_Dalvik_java_lang_Class_getEnclosingMethod(java_lang_Class_p clazz) {
#endif

    Object* enclosing = dvmGetEnclosingMethod(clazz);
    if (enclosing != NULL) {
        dvmReleaseTrackedAlloc(enclosing, NULL);
        if (enclosing->clazz == gDvm.classJavaLangReflectMethod) {
            RETURN_PTR((java_lang_reflect_Method_p)enclosing);
        }
        assert(enclosing->clazz == gDvm.classJavaLangReflectConstructor);
    }
    RETURN_PTR(NULL);
}

#if 0 // 4.2에서도 사용되지 않음.
static void Dalvik_java_lang_Class_getGenericInterfaces(const u4* args,
    JValue* pResult)
{
    dvmThrowUnsupportedOperationException("native method not implemented");

    RETURN_PTR(NULL);
}

static void Dalvik_java_lang_Class_getGenericSuperclass(const u4* args,
    JValue* pResult)
{
    dvmThrowUnsupportedOperationException("native method not implemented");

    RETURN_PTR(NULL);
}

static void Dalvik_java_lang_Class_getTypeParameters(const u4* args,
    JValue* pResult)
{
    dvmThrowUnsupportedOperationException("native method not implemented");

    RETURN_PTR(NULL);
}
#endif

/*
 * public boolean isAnonymousClass()
 *
 * Returns true if this is an "anonymous" class.
 */
static void Dalvik_java_lang_Class_isAnonymousClass(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_Class_isAnonymousClass(java_lang_Class_p self);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_Class_isAnonymousClass((java_lang_Class_p)clazz);
}

jbool fastiva_Dalvik_java_lang_Class_isAnonymousClass(java_lang_Class_p clazz) {
#endif

	StringObject* className = NULL;
    int accessFlags;

#ifdef FASTIVA
	if (d2f_isFastivaClass(clazz)) {
		if (d2f_getEnclosingClass(clazz) == NULL) {
			return false;
		}
		className = d2f_getInnerClassName(clazz);
		RETURN_BOOLEAN(className == NULL);
	}
#endif
    /*
     * If this has an InnerClass annotation, pull it out.  Lack of the
     * annotation, or an annotation with a NULL class name, indicates
     * that this is an anonymous inner class.
     */
    if (!dvmGetInnerClass(clazz, &className, &accessFlags))
        RETURN_BOOLEAN(false);

    dvmReleaseTrackedAlloc((Object*) className, NULL);
    RETURN_BOOLEAN(className == NULL);
}

/*
 * private Annotation[] getDeclaredAnnotations()
 *
 * Return the annotations declared on this class.
 */
static void Dalvik_java_lang_Class_getDeclaredAnnotations(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_annotation_Annotation_ap fastiva_Dalvik_java_lang_Class_getDeclaredAnnotations(java_lang_Class_p self);
	*(java_lang_annotation_Annotation_ap*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredAnnotations((java_lang_Class_p) clazz);
}

java_lang_annotation_Annotation_ap fastiva_Dalvik_java_lang_Class_getDeclaredAnnotations(java_lang_Class_p clazz) {
#endif

    ArrayObject* annos = dvmGetClassAnnotations(clazz);
    dvmReleaseTrackedAlloc((Object*) annos, NULL);
    RETURN_PTR((java_lang_annotation_Annotation_ap)annos);
}

/*
 * private Annotation getDeclaredAnnotation(Class annotationClass)
 */
static void Dalvik_java_lang_Class_getDeclaredAnnotation(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    ClassObject* annotationClazz = (ClassObject*) args[1];


#ifdef FASTIVA
	java_lang_annotation_Annotation_p fastiva_Dalvik_java_lang_Class_getDeclaredAnnotation(java_lang_Class_p self, java_lang_Class_p);
	*(java_lang_annotation_Annotation_p*)pResult = fastiva_Dalvik_java_lang_Class_getDeclaredAnnotation((java_lang_Class_p) clazz, (java_lang_Class_p)annotationClazz);
}

java_lang_annotation_Annotation_p fastiva_Dalvik_java_lang_Class_getDeclaredAnnotation(java_lang_Class_p clazz, java_lang_Class_p annotationClazz) {
#endif
	
	RETURN_PTR((java_lang_annotation_Annotation_p)dvmGetClassAnnotation(clazz, annotationClazz));
}

/*
 * private boolean isDeclaredAnnotationPresent(Class annotationClass);
 */
static void Dalvik_java_lang_Class_isDeclaredAnnotationPresent(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    ClassObject* annotationClazz = (ClassObject*) args[1];

#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_Class_isDeclaredAnnotationPresent(java_lang_Class_p self, java_lang_Class_p);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_Class_isDeclaredAnnotationPresent((java_lang_Class_p)clazz, (java_lang_Class_p)annotationClazz);
}

jbool fastiva_Dalvik_java_lang_Class_isDeclaredAnnotationPresent(java_lang_Class_p clazz, java_lang_Class_p annotationClazz) {
#endif

    RETURN_BOOLEAN(dvmIsClassAnnotationPresent(clazz, annotationClazz));
}

/*
 * public String getInnerClassName()
 *
 * Returns the simple name of a member class or local class, or null otherwise.
 */
static void Dalvik_java_lang_Class_getInnerClassName(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_java_lang_Class_getInnerClassName(java_lang_Class_p self);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_java_lang_Class_getInnerClassName((java_lang_Class_p)clazz);
}

java_lang_String_p fastiva_Dalvik_java_lang_Class_getInnerClassName(java_lang_Class_p clazz) {
#endif

	StringObject* nameObj;
    int flags;

#ifdef FASTIVA
	if (d2f_isFastivaClass(clazz)) {
		nameObj = d2f_getInnerClassName(clazz);
		if (nameObj != NULL) {
			dvmAddTrackedAlloc((Object*)nameObj, NULL);
		}
        RETURN_PTR((java_lang_String_p)nameObj);
	}
#endif

	if (dvmGetInnerClass(clazz, &nameObj, &flags)) {
        dvmReleaseTrackedAlloc((Object*) nameObj, NULL);
        RETURN_PTR((java_lang_String_p)nameObj);
    } else {
        RETURN_PTR(NULL);
    }
}

JNIEXPORT jobject JNICALL Java_java_lang_Class_getDex(JNIEnv* env, jclass javaClass) {
    Thread* self = dvmThreadSelf();
    ClassObject* c = (ClassObject*) dvmDecodeIndirectRef(self, javaClass);

    DvmDex* dvm_dex = c->pDvmDex;
    if (dvm_dex == NULL) {
        return NULL;
    }
    // Already cached?
    if (dvm_dex->dex_object != NULL) {
        return dvm_dex->dex_object;
    }
    jobject byte_buffer = env->NewDirectByteBuffer(dvm_dex->memMap.addr, dvm_dex->memMap.length);
    if (byte_buffer == NULL) {
        return NULL;
    }

    jclass com_android_dex_Dex = env->FindClass("com/android/dex/Dex");
    if (com_android_dex_Dex == NULL) {
        return NULL;
    }

    jmethodID com_android_dex_Dex_create =
            env->GetStaticMethodID(com_android_dex_Dex,
                                   "create", "(Ljava/nio/ByteBuffer;)Lcom/android/dex/Dex;");
    if (com_android_dex_Dex_create == NULL) {
        return NULL;
    }

    jvalue args[1];
    args[0].l = byte_buffer;
    jobject local_ref = env->CallStaticObjectMethodA(com_android_dex_Dex,
                                                     com_android_dex_Dex_create,
                                                     args);
    if (local_ref == NULL) {
        return NULL;
    }

    // Check another thread didn't cache an object, if we've won install the object.
    ScopedPthreadMutexLock lock(&dvm_dex->modLock);

    if (dvm_dex->dex_object == NULL) {
        dvm_dex->dex_object = env->NewGlobalRef(local_ref);
    }
    return dvm_dex->dex_object;
}


const DalvikNativeMethod dvm_java_lang_Class[] = {
    { "desiredAssertionStatus", "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_desiredAssertionStatus) },
    { "classForName",           "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_classForName) },
    { "getClassLoader",         "(Ljava/lang/Class;)Ljava/lang/ClassLoader;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getClassLoader) },
    { "getComponentType",       "()Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getComponentType) },
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
    { "getSignatureAnnotation",  "()[Ljava/lang/Object;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getSignatureAnnotation) },
#endif
    { "getDeclaredClasses",     "(Ljava/lang/Class;Z)[Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredClasses) },
    { "getDeclaredConstructors", "(Ljava/lang/Class;Z)[Ljava/lang/reflect/Constructor;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredConstructors) },
    { "getDeclaredFields",      "(Ljava/lang/Class;Z)[Ljava/lang/reflect/Field;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredFields) },
    { "getDeclaredMethods",     "(Ljava/lang/Class;Z)[Ljava/lang/reflect/Method;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredMethods) },
    { "getDeclaredField",      "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/reflect/Field;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredField) },
    { "getDeclaredConstructorOrMethod", "(Ljava/lang/Class;Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Member;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredConstructorOrMethod) },
    { "getInterfaces",          "()[Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getInterfaces) },
    { "getModifiers",           "(Ljava/lang/Class;Z)I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getModifiers) },
    { "getNameNative",                "()Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getNameNative) },
    { "getSuperclass",          "()Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getSuperclass) },
    { "isAssignableFrom",       "(Ljava/lang/Class;)Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_isAssignableFrom) },
    { "isInstance",             "(Ljava/lang/Object;)Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_isInstance) },
    { "isInterface",            "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_isInterface) },
    { "isPrimitive",            "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_isPrimitive) },
    { "newInstanceImpl",        "()Ljava/lang/Object;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_newInstanceImpl) },
    { "getDeclaringClass",      "()Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaringClass) },
    { "getEnclosingClass",      "()Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getEnclosingClass) },
    { "getEnclosingConstructor", "()Ljava/lang/reflect/Constructor;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getEnclosingConstructor) },
    { "getEnclosingMethod",     "()Ljava/lang/reflect/Method;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getEnclosingMethod) },
#if 0
    { "getGenericInterfaces",   "()[Ljava/lang/reflect/Type;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getGenericInterfaces) },
    { "getGenericSuperclass",   "()Ljava/lang/reflect/Type;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getGenericSuperclass) },
    { "getTypeParameters",      "()Ljava/lang/reflect/TypeVariable;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getTypeParameters) },
#endif
    { "isAnonymousClass",       "()Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_isAnonymousClass) },
    { "getDeclaredAnnotations", "()[Ljava/lang/annotation/Annotation;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredAnnotations) },
    { "getDeclaredAnnotation", "(Ljava/lang/Class;)Ljava/lang/annotation/Annotation;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getDeclaredAnnotation) },
    { "isDeclaredAnnotationPresent", "(Ljava/lang/Class;)Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_isDeclaredAnnotationPresent) },
    { "getInnerClassName",       "()Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_Class_getInnerClassName) },
    { NULL, NULL, NULL, NULL },
};

#ifdef FASTIVA

static void Dalvik_libcore_reflect_AnnotationAccess_getSignature(const u4* args,
    JValue* pResult)
{
    java_lang_reflect_AnnotatedElement_p element = (java_lang_reflect_AnnotatedElement_p) args[0];

	java_lang_String_p fastiva_Dalvik_libcore_reflect_AnnotationAccess_getSignature(java_lang_reflect_AnnotatedElement_p element);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_libcore_reflect_AnnotationAccess_getSignature(element);
}

static java_lang_String_p emptyString = NULL;
java_lang_String_p fastiva_Dalvik_libcore_reflect_AnnotationAccess_getSignature(java_lang_reflect_AnnotatedElement_p element) {
	ClassObject* ci = NULL;

	if (element->getClass_() == java_lang_Class_C$::getRawStatic$()) {
		ci = (ClassObject*)(void*)element;
	}
	else {
#ifdef _DEBUG
		assert(java_lang_reflect_Member::isInstance$(element) != NULL);
#endif
		ci = java_lang_reflect_Member::ptr_cast$(element)->getDeclaringClass_();
	}
	java_lang_String_ap values = (java_lang_String_ap)dvmGetClassSignatureAnnotation(ci);
	if (values == NULL || values->length() == 0) {
		if (emptyString == NULL) {
			emptyString = (java_lang_String_p)dvmLookupImmortalInternedString(fm::createAsciiString(""));
		}
		return emptyString;
	}
	else if (values->length() == 1) {
		return values->get$(0);
	}
	else {
		java_lang_StringBuilder_p sb = FASTIVA_NEW(java_lang_StringBuilder)();
		for (int i = 0; i < values->length(); i ++) {
			java_lang_String_p str = values->get$(i);
			sb->append_(str);
		}
		return sb->toString_();
	}
}

extern const DalvikNativeMethod dvm_libcore_reflect_AnnotationAccess[] = {
    { "getSignature", "(Ljava/lang/reflect/AnnotatedElement;)Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_libcore_reflect_AnnotationAccess_getSignature) },
    { NULL, NULL, NULL, NULL },
};
#endif