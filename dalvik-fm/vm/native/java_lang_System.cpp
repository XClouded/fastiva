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
 * java.lang.Class native methods
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"

/*
 * The VM makes guarantees about the atomicity of accesses to primitive
 * variables.  These guarantees also apply to elements of arrays.
 * In particular, 8-bit, 16-bit, and 32-bit accesses must be atomic and
 * must not cause "word tearing".  Accesses to 64-bit array elements must
 * either be atomic or treated as two 32-bit operations.  References are
 * always read and written atomically, regardless of the number of bits
 * used to represent them.
 *
 * We can't rely on standard libc functions like memcpy() and memmove()
 * in our implementation of System.arraycopy(), because they may copy
 * byte-by-byte (either for the full run or for "unaligned" parts at the
 * start or end).  We need to use functions that guarantee 16-bit or 32-bit
 * atomicity as appropriate.
 *
 * System.arraycopy() is heavily used, so having an efficient implementation
 * is important.  The bionic libc provides a platform-optimized memory move
 * function that should be used when possible.  If it's not available,
 * the trivial "reference implementation" versions below can be used until
 * a proper version can be written.
 *
 * For these functions, The caller must guarantee that dest/src are aligned
 * appropriately for the element type, and that n is a multiple of the
 * element size.
 */

#pragma GCC diagnostic ignored "-Wunused-function"


/*
 * Works like memmove(), except:
 * - if all arguments are at least 32-bit aligned, we guarantee that we
 *   will use operations that preserve atomicity of 32-bit values
 * - if not, we guarantee atomicity of 16-bit values
 *
 * If all three arguments are not at least 16-bit aligned, the behavior
 * of this function is undefined.  (We could remove this restriction by
 * testing for unaligned values and punting to memmove(), but that's
 * not currently useful.)
 *
 * TODO: add loop for 64-bit alignment
 * TODO: use __builtin_prefetch
 * TODO: write an ARM-optimized version
 */
static void memmove_words(void* dest, const void* src, size_t n) {
    assert((((uintptr_t) dest | (uintptr_t) src | n) & 0x01) == 0);

    char* d = (char*) dest;
    const char* s = (const char*) src;
    size_t copyCount;

    /*
     * If the source and destination pointers are the same, this is
     * an expensive no-op.  Testing for an empty move now allows us
     * to skip a check later.
     */
    if (n == 0 || d == s)
        return;

    /*
     * Determine if the source and destination buffers will overlap if
     * we copy data forward (i.e. *dest++ = *src++).
     *
     * It's okay if the destination buffer starts before the source and
     * there is some overlap, because the reader is always ahead of the
     * writer.
     */
    if (__builtin_expect((d < s) || ((size_t)(d - s) >= n), 1)) {
        /*
         * Copy forward.  We prefer 32-bit loads and stores even for 16-bit
         * data, so sort that out.
         */
        if ((((uintptr_t) d | (uintptr_t) s) & 0x03) != 0) {
            /*
             * Not 32-bit aligned.  Two possibilities:
             * (1) Congruent, we can align to 32-bit by copying one 16-bit val
             * (2) Non-congruent, we can do one of:
             *   a. copy whole buffer as a series of 16-bit values
             *   b. load/store 32 bits, using shifts to ensure alignment
             *   c. just copy the as 32-bit values and assume the CPU
             *      will do a reasonable job
             *
             * We're currently using (a), which is suboptimal.
             */
            if ((((uintptr_t) d ^ (uintptr_t) s) & 0x03) != 0) {
                copyCount = n;
    } else {
                copyCount = 2;
        }
            n -= copyCount;
            copyCount /= sizeof(uint16_t);

            while (copyCount--) {
                *(uint16_t*)d = *(uint16_t*)s;
                d += sizeof(uint16_t);
                s += sizeof(uint16_t);
    }
}

        /*
         * Copy 32-bit aligned words.
         */
        copyCount = n / sizeof(uint32_t);
        while (copyCount--) {
            *(uint32_t*)d = *(uint32_t*)s;
            d += sizeof(uint32_t);
            s += sizeof(uint32_t);
        }

        /*
         * Check for leftovers.  Either we finished exactly, or we have
         * one remaining 16-bit chunk.
         */
        if ((n & 0x02) != 0) {
            *(uint16_t*)d = *(uint16_t*)s;
        }
    } else {
        /*
         * Copy backward, starting at the end.
         */
        d += n;
        s += n;

        if ((((uintptr_t) d | (uintptr_t) s) & 0x03) != 0) {
            /* try for 32-bit alignment */
            if ((((uintptr_t) d ^ (uintptr_t) s) & 0x03) != 0) {
                copyCount = n;
            } else {
                copyCount = 2;
            }
            n -= copyCount;
            copyCount /= sizeof(uint16_t);

            while (copyCount--) {
                d -= sizeof(uint16_t);
                s -= sizeof(uint16_t);
                *(uint16_t*)d = *(uint16_t*)s;
            }
        }

        /* copy 32-bit aligned words */
        copyCount = n / sizeof(uint32_t);
        while (copyCount--) {
            d -= sizeof(uint32_t);
            s -= sizeof(uint32_t);
            *(uint32_t*)d = *(uint32_t*)s;
        }

        /* copy leftovers */
        if ((n & 0x02) != 0) {
            d -= sizeof(uint16_t);
            s -= sizeof(uint16_t);
            *(uint16_t*)d = *(uint16_t*)s;
        }
    }
}

#define move16 memmove_words
#define move32 memmove_words

/*
 * public static void arraycopy(Object src, int srcPos, Object dest,
 *      int destPos, int length)
 *
 * The description of this function is long, and describes a multitude
 * of checks and exceptions.
 */
static void Dalvik_java_lang_System_arraycopy(const u4* args, JValue* pResult)
{
    ArrayObject* srcArray = (ArrayObject*) args[0];
    int srcPos = args[1];
    ArrayObject* dstArray = (ArrayObject*) args[2];
    int dstPos = args[3];
    int length = args[4];

#ifdef FASTIVA
	void fastiva_Dalvik_java_lang_System_arraycopy(java_lang_Object_p, jint, java_lang_Object_p, jint, jint);
	fastiva_Dalvik_java_lang_System_arraycopy(srcArray, srcPos, dstArray, dstPos, length);
}

void fastiva_Dalvik_java_lang_System_arraycopy(java_lang_Object_p arg0, jint srcPos, java_lang_Object_p arg2, jint dstPos, jint length) {
    ArrayObject* srcArray = (ArrayObject*) arg0;
    ArrayObject* dstArray = (ArrayObject*) arg2;
#endif

    /* Check for null pointers. */
    if (srcArray == NULL) {
        dvmThrowNullPointerException("src == null");
        THROW_VOID();
    }
    if (dstArray == NULL) {
        dvmThrowNullPointerException("dst == null");
        THROW_VOID();
    }

    /* Make sure source and destination are arrays. */
    if (!dvmIsArray(srcArray)) {
        dvmThrowArrayStoreExceptionNotArray(((Object*)srcArray)->clazz, "source");
        THROW_VOID();
    }
    if (!dvmIsArray(dstArray)) {
        dvmThrowArrayStoreExceptionNotArray(((Object*)dstArray)->clazz, "destination");
        THROW_VOID();
    }

    /* avoid int overflow */
    if (srcPos < 0 || dstPos < 0 || length < 0 ||
        srcPos > (int) srcArray->length - length ||
        dstPos > (int) dstArray->length - length)
    {
        dvmThrowExceptionFmt(gDvm.exArrayIndexOutOfBoundsException,
            "src.length=%d srcPos=%d dst.length=%d dstPos=%d length=%d",
            srcArray->length, srcPos, dstArray->length, dstPos, length);
        THROW_VOID();
    }

    ClassObject* srcClass = srcArray->clazz;
    ClassObject* dstClass = dstArray->clazz;
    char srcType = srcClass->descriptor[1];
    char dstType = dstClass->descriptor[1];

    /*
     * If one of the arrays holds a primitive type, the other array must
     * hold the same type.
     */
    bool srcPrim = (srcType != '[' && srcType != 'L');
    bool dstPrim = (dstType != '[' && dstType != 'L');
    if (srcPrim || dstPrim) {
        if (srcPrim != dstPrim || srcType != dstType) {
            dvmThrowArrayStoreExceptionIncompatibleArrays(srcClass, dstClass);
            THROW_VOID();
        }

        if (false) ALOGD("arraycopy prim[%c] dst=%p %d src=%p %d len=%d",
            srcType, dstArray->contents, dstPos,
            srcArray->contents, srcPos, length);

        switch (srcType) {
        case 'B':
        case 'Z':
            /* 1 byte per element */
            memmove((u1*) dstArray->contents + dstPos,
                (const u1*) srcArray->contents + srcPos,
                length);
            break;
        case 'C':
        case 'S':
            /* 2 bytes per element */
            move16((u1*) dstArray->contents + dstPos * 2,
                (const u1*) srcArray->contents + srcPos * 2,
                length * 2);
            break;
        case 'F':
        case 'I':
            /* 4 bytes per element */
            move32((u1*) dstArray->contents + dstPos * 4,
                (const u1*) srcArray->contents + srcPos * 4,
                length * 4);
            break;
        case 'D':
        case 'J':
            /*
             * 8 bytes per element.  We don't need to guarantee atomicity
             * of the entire 64-bit word, so we can use the 32-bit copier.
             */
            move32((u1*) dstArray->contents + dstPos * 8,
                (const u1*) srcArray->contents + srcPos * 8,
                length * 8);
            break;
        default:        /* illegal array type */
            ALOGE("Weird array type '%s'", srcClass->descriptor);
            dvmAbort();
        }
    } else {
        /*
         * Neither class is primitive.  See if elements in "src" are instances
         * of elements in "dst" (e.g. copy String to String or String to
         * Object).
         */
        const int width = sizeof(Object*);

        if (srcClass->arrayDim == dstClass->arrayDim &&
            dvmInstanceof(srcClass, dstClass))
        {
            /*
             * "dst" can hold "src"; copy the whole thing.
             */
            if (false) ALOGD("arraycopy ref dst=%p %d src=%p %d len=%d",
                dstArray->contents, dstPos * width,
                srcArray->contents, srcPos * width,
                length * width);
            move32((u1*)dstArray->contents + dstPos * width,
                (const u1*)srcArray->contents + srcPos * width,
                length * width);
            dvmWriteBarrierArray(dstArray, dstPos, dstPos+length);
        } else {
            /*
             * The arrays are not fundamentally compatible.  However, we
             * may still be able to do this if the destination object is
             * compatible (e.g. copy Object[] to String[], but the Object
             * being copied is actually a String).  We need to copy elements
             * one by one until something goes wrong.
             *
             * Because of overlapping moves, what we really want to do
             * is compare the types and count up how many we can move,
             * then call move32() to shift the actual data.  If we just
             * start from the front we could do a smear rather than a move.
             */
            Object** srcObj;
            int copyCount;
            ClassObject*   clazz = NULL;

            srcObj = ((Object**)(void*)srcArray->contents) + srcPos;

            if (length > 0 && srcObj[0] != NULL)
            {
                clazz = srcObj[0]->clazz;
                if (!dvmCanPutArrayElement(clazz, dstClass))
                    clazz = NULL;
            }

            for (copyCount = 0; copyCount < length; copyCount++)
            {
                if (srcObj[copyCount] != NULL &&
                    srcObj[copyCount]->clazz != clazz &&
                    !dvmCanPutArrayElement(srcObj[copyCount]->clazz, dstClass))
                {
                    /* can't put this element into the array */
                    break;
                }
            }

            if (false) ALOGD("arraycopy iref dst=%p %d src=%p %d count=%d of %d",
                dstArray->contents, dstPos * width,
                srcArray->contents, srcPos * width,
                copyCount, length);
            move32((u1*)dstArray->contents + dstPos * width,
                (const u1*)srcArray->contents + srcPos * width,
                copyCount * width);
            dvmWriteBarrierArray(dstArray, 0, copyCount);
            if (copyCount != length) {
                dvmThrowArrayStoreExceptionIncompatibleArrayElement(srcPos + copyCount,
                        srcObj[copyCount]->clazz, dstClass);
                THROW_VOID();
            }
        }
    }

    RETURN_VOID();
}

/*
 * static long currentTimeMillis()
 *
 * Current time, in miliseconds.  This doesn't need to be internal to the
 * VM, but we're already handling java.lang.System here.
 */
static void Dalvik_java_lang_System_currentTimeMillis(const u4* args,
    JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_java_lang_System_currentTimeMillis();
	*(jlonglong*)pResult = fastiva_Dalvik_java_lang_System_currentTimeMillis();
}

jlonglong fastiva_Dalvik_java_lang_System_currentTimeMillis() {
#endif

	struct timeval tv;

    gettimeofday(&tv, (struct timezone *) NULL);
    long long when = tv.tv_sec * 1000LL + tv.tv_usec / 1000;

    RETURN_LONG(when);
}

/*
 * static long nanoTime()
 *
 * Current monotonically-increasing time, in nanoseconds.  This doesn't
 * need to be internal to the VM, but we're already handling
 * java.lang.System here.
 */
static void Dalvik_java_lang_System_nanoTime(const u4* args, JValue* pResult)
{
    UNUSED_PARAMETER(args);

#ifdef FASTIVA
	jlonglong fastiva_Dalvik_java_lang_System_nanoTime();
	*(jlonglong*)pResult = fastiva_Dalvik_java_lang_System_nanoTime();
}

jlonglong fastiva_Dalvik_java_lang_System_nanoTime() {
#endif

    u8 when = dvmGetRelativeTimeNsec();
    RETURN_LONG(when);
}

/*
 * static int identityHashCode(Object x)
 *
 * Returns that hash code that the default hashCode()
 * method would return for "x", even if "x"s class
 * overrides hashCode().
 */
#ifdef FASTIVA
jint fastiva_Dalvik_java_lang_System_identityHashCode(java_lang_Object_p);
#endif 

static void Dalvik_java_lang_System_identityHashCode(const u4* args,
    JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    *(jint*)pResult = (dvmIdentityHashCode(thisPtr));
}

static void Dalvik_java_lang_System_mapLibraryName(const u4* args,
    JValue* pResult)
{
    StringObject* nameObj = (StringObject*) args[0];

#ifdef FASTIVA
	java_lang_String_p fastiva_Dalvik_java_lang_System_mapLibraryName(java_lang_String_p);
	*(java_lang_String_p*)pResult = fastiva_Dalvik_java_lang_System_mapLibraryName((java_lang_String_p)nameObj);
}

java_lang_String_p fastiva_Dalvik_java_lang_System_mapLibraryName(java_lang_String_p nameObj) {
#endif

	StringObject* result = NULL;
    char* name;
    char* mappedName;

    if (nameObj == NULL) {
        dvmThrowNullPointerException("userLibName == null");
        THROW_V();
    }

    name = dvmCreateCstrFromString(nameObj);

#ifdef _DEBUG
	assert(strstr(name, "cfb") == NULL);
#endif


    mappedName = dvmCreateSystemLibraryName(name);
    if (mappedName != NULL) {
        result = dvmCreateStringFromCstr(mappedName);
        dvmReleaseTrackedAlloc((Object*) result, NULL);
    }

    free(name);
    free(mappedName);
    RETURN_PTR((java_lang_String_p)result);
}

#ifdef G_WATCH
static void Dalvik_java_lang_System_arraycopyCharUnchecked(const u4* args,
    JValue* pResult)
{
    Unicod_ap srcArray = (Unicod_ap) args[0];
    int srcPos = args[1];
    Unicod_ap dstArray = (Unicod_ap) args[2];
    int dstPos = args[3];
    int length = args[4];

#ifdef FASTIVA
	void fastiva_Dalvik_java_lang_System_arraycopyCharUnchecked(Unicod_ap, jint, Unicod_ap, jint, jint);
	fastiva_Dalvik_java_lang_System_arraycopyCharUnchecked(srcArray, srcPos, dstArray, dstPos, length);
}

void fastiva_Dalvik_java_lang_System_arraycopyCharUnchecked(Unicod_ap srcArray, jint srcPos, Unicod_ap dstArray, jint dstPos, jint length) {
	assert(srcArray != NULL && dstArray != NULL);
	assert(srcPos >= 0 && dstPos >= 0 && length >= 0);
	assert(srcArray->length() >= srcPos + length);
	assert(dstArray->length() >= dstPos + length);

	memcpy(dstArray->getBuffer_unsafe$() + dstPos, srcArray->getBuffer_unsafe$() + srcPos, length * sizeof(jchar));
#endif
}

#endif

const DalvikNativeMethod dvm_java_lang_System[] = {
    { "arraycopy",          "(Ljava/lang/Object;ILjava/lang/Object;II)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_System_arraycopy) },
#ifdef G_WATCH
    { "arraycopyCharUnchecked",          "([CI[CII)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_System_arraycopyCharUnchecked) },
#endif
    { "currentTimeMillis",  "()J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_System_currentTimeMillis) },
    { "identityHashCode",  "(Ljava/lang/Object;)I",
        Dalvik_java_lang_System_identityHashCode, NULL },
    { "mapLibraryName",     "(Ljava/lang/String;)Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_System_mapLibraryName) },
    { "nanoTime",  "()J",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_java_lang_System_nanoTime) },
    { NULL, NULL, NULL, NULL },
};
