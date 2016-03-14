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
 * java.lang.reflect.Method
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"

#ifdef FASTIVA
#include <dalvik_Kernel.h>
#endif 

/*
 * static int getMethodModifiers(Class decl_class, int slot)
 *
 * (Not sure why the access flags weren't stored in the class along with
 * everything else.  Not sure why this isn't static.)
 */
static void Dalvik_java_lang_reflect_Method_getMethodModifiers(const u4* args,
    JValue* pResult)
{
    ClassObject* declaringClass = (ClassObject*) args[0];
    int slot = args[1];

#ifdef FASTIVA
	jint fastiva_Dalvik_java_lang_reflect_Method_getMethodModifiers(java_lang_Class_p clazz, jint);
	*(jint*)pResult = fastiva_Dalvik_java_lang_reflect_Method_getMethodModifiers((java_lang_Class_p)declaringClass, slot);
}

jint fastiva_Dalvik_java_lang_reflect_Method_getMethodModifiers(java_lang_Class_p declaringClass, jint slot) {
#endif

	Method* meth;

    meth = dvmSlotToMethod(declaringClass, slot);
    RETURN_INT(dvmFixMethodFlags(meth->accessFlags));
}

/*
 * private Object invokeNative(Object obj, Object[] args, Class declaringClass,
 *   Class[] parameterTypes, Class returnType, int slot, boolean noAccessCheck)
 *
 * Invoke a static or virtual method via reflection.
 */
static void Dalvik_java_lang_reflect_Method_invokeNative(const u4* args,
    JValue* pResult)
{
    // ignore thisPtr in args[0]
    Object* methObj = (Object*) args[1];        // null for static methods
    ArrayObject* argList = (ArrayObject*) args[2];
    ClassObject* declaringClass = (ClassObject*) args[3];
    ArrayObject* params = (ArrayObject*) args[4];
    ClassObject* returnType = (ClassObject*) args[5];
    int slot = args[6];
    bool noAccessCheck = (args[7] != 0);

#ifdef FASTIVA
	java_lang_Object_p fastiva_Dalvik_java_lang_reflect_Method_invokeNative(java_lang_reflect_Method_p self, java_lang_Object_p, java_lang_Object_ap, java_lang_Class_p, java_lang_Class_ap, java_lang_Class_p, jint, jbool);
	*(java_lang_Object_p*)pResult = fastiva_Dalvik_java_lang_reflect_Method_invokeNative((java_lang_reflect_Method_p)args[0], (java_lang_Object_p)methObj, (java_lang_Object_ap)argList, (java_lang_Class_p)declaringClass, (java_lang_Class_ap)params, (java_lang_Class_p)returnType, slot, noAccessCheck);
}

java_lang_Object_p fastiva_Dalvik_java_lang_reflect_Method_invokeNative(java_lang_reflect_Method_p self, java_lang_Object_p methObj, java_lang_Object_ap argList, java_lang_Class_p declaringClass, java_lang_Class_ap params, java_lang_Class_p returnType, jint slot, jbool noAccessCheck) {
#endif


	const Method* meth;
    Object* result;

    /*
     * "If the underlying method is static, the class that declared the
     * method is initialized if it has not already been initialized."
     */
    meth = dvmSlotToMethod(declaringClass, slot);
    assert(meth != NULL);

    if (dvmIsStaticMethod(meth)) {
        if (!dvmIsClassInitialized(declaringClass)) {
            if (!dvmInitClass(declaringClass))
                goto init_failed;
        }
    } else {
        /* looks like interfaces need this too? */
        if (dvmIsInterfaceClass(declaringClass) &&
            !dvmIsClassInitialized(declaringClass))
        {
            if (!dvmInitClass(declaringClass))
                goto init_failed;
        }

        /* make sure the object is an instance of the expected class */
        if (!dvmVerifyObjectInClass(methObj, declaringClass)) {
            assert(dvmCheckException(dvmThreadSelf()));
            THROW_V();
        }

        /* do the virtual table lookup for the method */
        meth = dvmGetVirtualizedMethod(methObj->clazz, meth);
        if (meth == NULL) {
            assert(dvmCheckException(dvmThreadSelf()));
            THROW_V();
        }
    }

    /*
     * If the method has a return value, "result" will be an object or
     * a boxed primitive.
     */
    result = dvmInvokeMethod(methObj, meth, (ArrayObject*)argList, (ArrayObject*)params, returnType,
                noAccessCheck);

    RETURN_PTR(result);

init_failed:
    /*
     * If initialization failed, an exception will be raised.
     */
    ALOGD("Method.invoke() on bad class %s failed",
        declaringClass->descriptor);
    assert(dvmCheckException(dvmThreadSelf()));
    THROW_V();
}

/*
 * static Annotation[] getDeclaredAnnotations(Class declaringClass, int slot)
 *
 * Return the annotations declared for this method.
 */
static void Dalvik_java_lang_reflect_Method_getDeclaredAnnotations(
    const u4* args, JValue* pResult)
{
    ClassObject* declaringClass = (ClassObject*) args[0];
    int slot = args[1];

#ifdef FASTIVA
	java_lang_annotation_Annotation_ap fastiva_Dalvik_java_lang_reflect_Method_getDeclaredAnnotations(java_lang_Class_p clazz, jint);
	*(java_lang_annotation_Annotation_ap*)pResult = fastiva_Dalvik_java_lang_reflect_Method_getDeclaredAnnotations((java_lang_Class_p)declaringClass, slot);
}

java_lang_annotation_Annotation_ap fastiva_Dalvik_java_lang_reflect_Method_getDeclaredAnnotations(java_lang_Class_p declaringClass, jint slot) {
#endif

	Method* meth;

    meth = dvmSlotToMethod(declaringClass, slot);
    assert(meth != NULL);

    ArrayObject* annos = dvmGetMethodAnnotations(meth);
    dvmReleaseTrackedAlloc((Object*)annos, NULL);
    RETURN_PTR((java_lang_annotation_Annotation_ap)annos);
}

/*
 * static Annotation getAnnotation(
 *         Class declaringClass, int slot, Class annotationType);
 */
static void Dalvik_java_lang_reflect_Method_getAnnotation(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    int slot = args[1];
    ClassObject* annotationClazz = (ClassObject*) args[2];

#ifdef FASTIVA
	java_lang_annotation_Annotation_p fastiva_Dalvik_java_lang_reflect_Method_getAnnotation(java_lang_Class_p clazz, jint, java_lang_Class_p);
	*(java_lang_annotation_Annotation_p*)pResult = fastiva_Dalvik_java_lang_reflect_Method_getAnnotation((java_lang_Class_p)clazz, slot, (java_lang_Class_p)annotationClazz);
}

java_lang_annotation_Annotation_p fastiva_Dalvik_java_lang_reflect_Method_getAnnotation(java_lang_Class_p clazz, jint slot, java_lang_Class_p annotationClazz) {
#endif


    Method* meth = dvmSlotToMethod(clazz, slot);
    RETURN_PTR((java_lang_annotation_Annotation_p)dvmGetMethodAnnotation(clazz, meth, annotationClazz));
}

/*
 * static boolean isAnnotationPresent(
 *         Class declaringClass, int slot, Class annotationType);
 */
static void Dalvik_java_lang_reflect_Method_isAnnotationPresent(const u4* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    int slot = args[1];
    ClassObject* annotationClazz = (ClassObject*) args[2];

#ifdef FASTIVA
	jbool fastiva_Dalvik_java_lang_reflect_Method_isAnnotationPresent(java_lang_Class_p clazz, jint, java_lang_Class_p);
	*(jbool*)pResult = fastiva_Dalvik_java_lang_reflect_Method_isAnnotationPresent((java_lang_Class_p)clazz, slot, (java_lang_Class_p)annotationClazz);
}

jbool fastiva_Dalvik_java_lang_reflect_Method_isAnnotationPresent(java_lang_Class_p clazz, jint slot, java_lang_Class_p annotationClazz) {
#endif

    Method* meth = dvmSlotToMethod(clazz, slot);
    RETURN_BOOLEAN(dvmIsMethodAnnotationPresent(clazz, meth, annotationClazz));
}

/*
 * static Annotation[][] getParameterAnnotations(Class declaringClass, int slot)
 *
 * Return the annotations declared for this method's parameters.
 */
static void Dalvik_java_lang_reflect_Method_getParameterAnnotations(
    const u4* args, JValue* pResult)
{
    ClassObject* declaringClass = (ClassObject*) args[0];
    int slot = args[1];

#ifdef FASTIVA
	java_lang_annotation_Annotation_aap fastiva_Dalvik_java_lang_reflect_Method_getParameterAnnotations(java_lang_Class_p clazz, jint);
	*(java_lang_annotation_Annotation_aap*)pResult = fastiva_Dalvik_java_lang_reflect_Method_getParameterAnnotations((java_lang_Class_p)declaringClass, slot);
}

java_lang_annotation_Annotation_aap fastiva_Dalvik_java_lang_reflect_Method_getParameterAnnotations(java_lang_Class_p declaringClass, jint slot) {
#endif

    Method* meth;

    meth = dvmSlotToMethod(declaringClass, slot);
    assert(meth != NULL);

    ArrayObject* annos = dvmGetParameterAnnotations(meth);
    dvmReleaseTrackedAlloc((Object*)annos, NULL);
    RETURN_PTR((java_lang_annotation_Annotation_aap)annos);
}

/*
 * private Object getDefaultValue(Class declaringClass, int slot)
 *
 * Return the default value for the annotation member represented by
 * this Method instance.  Returns NULL if none is defined.
 */
static void Dalvik_java_lang_reflect_Method_getDefaultValue(const u4* args,
    JValue* pResult)
{
    // ignore thisPtr in args[0]
    ClassObject* declaringClass = (ClassObject*) args[1];
    int slot = args[2];

#ifdef FASTIVA
	java_lang_Object_p fastiva_Dalvik_java_lang_reflect_Method_getDefaultValue(java_lang_reflect_Method_p self, java_lang_Class_p clazz, jint);
	*(java_lang_Object_p*)pResult = fastiva_Dalvik_java_lang_reflect_Method_getDefaultValue((java_lang_reflect_Method_p)args[0], (java_lang_Class_p)declaringClass, slot);
}

java_lang_Object_p fastiva_Dalvik_java_lang_reflect_Method_getDefaultValue(java_lang_reflect_Method_p self, java_lang_Class_p declaringClass, jint slot) {
#endif

    Method* meth;

    /* make sure this is an annotation class member */
    if (!dvmIsAnnotationClass(declaringClass))
        RETURN_PTR(NULL);
#ifdef FASTIVA
	if (d2f_isFastivaClass(declaringClass) != 0) {
		FASTIVA_ASSERT(dvmSlotToMethod(declaringClass, slot)->clazz == declaringClass);
	    meth = dvmSlotToMethod(declaringClass, slot);
		return d2f_getDefaultAnnotationValue(meth, declaringClass);
	}
#endif
    meth = dvmSlotToMethod(declaringClass, slot);
    assert(meth != NULL);

    Object* def = dvmGetAnnotationDefaultValue(meth);
    dvmReleaseTrackedAlloc(def, NULL);
    RETURN_PTR(def);
}

/*
 * static Object[] getSignatureAnnotation()
 *
 * Returns the signature annotation.
 */
static void Dalvik_java_lang_reflect_Method_getSignatureAnnotation(
    const u4* args, JValue* pResult)
{
    ClassObject* declaringClass = (ClassObject*) args[0];
    int slot = args[1];

#ifdef FASTIVA
	java_lang_Object_ap fastiva_Dalvik_java_lang_reflect_Method_getSignatureAnnotation(java_lang_Class_p clazz, jint);
	*(java_lang_Object_ap*)pResult = fastiva_Dalvik_java_lang_reflect_Method_getSignatureAnnotation((java_lang_Class_p)declaringClass, slot);
}

java_lang_Object_ap fastiva_Dalvik_java_lang_reflect_Method_getSignatureAnnotation(java_lang_Class_p declaringClass, jint slot) {
#endif

    Method* meth;

    meth = dvmSlotToMethod(declaringClass, slot);
    assert(meth != NULL);

    ArrayObject* arr = dvmGetMethodSignatureAnnotation(meth);
    dvmReleaseTrackedAlloc((Object*) arr, NULL);
    RETURN_PTR((java_lang_Object_ap)arr);
}

const DalvikNativeMethod dvm_java_lang_reflect_Method[] = {
    { "getMethodModifiers", "(Ljava/lang/Class;I)I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_getMethodModifiers) },
    { "invokeNative",       "(Ljava/lang/Object;[Ljava/lang/Object;Ljava/lang/Class;[Ljava/lang/Class;Ljava/lang/Class;IZ)Ljava/lang/Object;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_invokeNative) },
    { "getDeclaredAnnotations", "(Ljava/lang/Class;I)[Ljava/lang/annotation/Annotation;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_getDeclaredAnnotations) },
    { "getAnnotation", "(Ljava/lang/Class;ILjava/lang/Class;)Ljava/lang/annotation/Annotation;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_getAnnotation) },
    { "isAnnotationPresent", "(Ljava/lang/Class;ILjava/lang/Class;)Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_isAnnotationPresent) },
    { "getParameterAnnotations", "(Ljava/lang/Class;I)[[Ljava/lang/annotation/Annotation;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_getParameterAnnotations) },
    { "getDefaultValue",    "(Ljava/lang/Class;I)Ljava/lang/Object;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_getDefaultValue) },
    { "getSignatureAnnotation",  "(Ljava/lang/Class;I)[Ljava/lang/Object;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Method_getSignatureAnnotation) },
    { NULL, NULL, NULL, NULL },
};
