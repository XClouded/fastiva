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
 * java.lang.String
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"

static void String_charAt(const u4* args, JValue* pResult)
{
    MAKE_INTRINSIC_TRAMPOLINE(javaLangString_charAt);
}

static void String_compareTo(const u4* args, JValue* pResult)
{
    MAKE_INTRINSIC_TRAMPOLINE(javaLangString_compareTo);
}

#ifndef G_WATCH
static void String_equals(const u4* args, JValue* pResult)
{
    MAKE_INTRINSIC_TRAMPOLINE(javaLangString_equals);
}
#endif

static void String_fastIndexOf(const u4* args, JValue* pResult)
{
    MAKE_INTRINSIC_TRAMPOLINE(javaLangString_fastIndexOf_II);
}

static void String_intern(const u4* args, JValue* pResult)
{
    StringObject* str = (StringObject*) args[0];
    StringObject* interned = dvmLookupInternedString(str);
    DALVIK_RETURN_PTR((java_lang_String_p)interned);
}

#ifdef FASTIVA
java_lang_String_p fastiva_String_intern(java_lang_String_p str) {
    StringObject* interned = dvmLookupInternedString((StringObject*)str);
	RETURN_PTR((java_lang_String_p)interned);
}
#endif

static void String_isEmpty(const u4* args, JValue* pResult)
{
    MAKE_INTRINSIC_TRAMPOLINE(javaLangString_isEmpty);
}


static void String_length(const u4* args, JValue* pResult)
{
    MAKE_INTRINSIC_TRAMPOLINE(javaLangString_length);
}


const DalvikNativeMethod dvm_java_lang_String[] = {
    { "charAt",      "(I)C",                  String_charAt, NULL },
    { "compareTo",   "(Ljava/lang/String;)I", String_compareTo, NULL },
#ifndef G_WATCH
    { "equals",      "(Ljava/lang/Object;)Z", String_equals, NULL },
#endif
    { "fastIndexOf", "(II)I",                 String_fastIndexOf, NULL },
    { "intern",      "()Ljava/lang/String;",  FASTIVA_INTERNAL_METHOD_PAIR(String_intern) },
    { "isEmpty",     "()Z",                   String_isEmpty, NULL },
    { "length",      "()I",                   String_length, NULL },
    { NULL, NULL, NULL, NULL },
};

