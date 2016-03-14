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
 * java.lang.reflect.Proxy
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"
#ifdef FASTIVA
#  include <java/lang/ClassLoader.inl>
#endif

/*
 * static Class generateProxy(String name, Class[] interfaces,
 *      ClassLoader loader)
 *
 * Generate a proxy class with the specified characteristics.  Throws an
 * exception on error.
 */
static void Dalvik_java_lang_reflect_Proxy_generateProxy(const u4* args,
    JValue* pResult)
{
    StringObject* str = (StringObject*) args[0];
    ArrayObject* interfaces = (ArrayObject*) args[1];
    Object* loader = (Object*) args[2];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_java_lang_reflect_Proxy_generateProxy(java_lang_String_p, java_lang_Class_ap, java_lang_ClassLoader_p);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_java_lang_reflect_Proxy_generateProxy((java_lang_String_p)str, (java_lang_Class_ap)interfaces, (java_lang_ClassLoader_p)loader);
}

java_lang_Class_p fastiva_Dalvik_java_lang_reflect_Proxy_generateProxy(java_lang_String_p str, java_lang_Class_ap interfaces, java_lang_ClassLoader_p loader) {
#endif

    ClassObject* result;

    result = dvmGenerateProxyClass(str, (ArrayObject*)interfaces, loader);
    RETURN_PTR((java_lang_Class_p)result);
}

const DalvikNativeMethod dvm_java_lang_reflect_Proxy[] = {
    { "generateProxy", "(Ljava/lang/String;[Ljava/lang/Class;Ljava/lang/ClassLoader;)Ljava/lang/Class;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_reflect_Proxy_generateProxy) },
    { NULL, NULL, NULL, NULL },
};

