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
 * dalvik.system.DexFile
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"
#ifdef FASTIVA
#include <fcntl.h>
#include <JNIHelp.h>
#include <dalvik_Kernel.h>
#include <libdex/OptInvocation.h>
#include <java/lang/ClassLoader.inl>
#endif

/*
 * Return true if the given name ends with ".dex".
 */
static bool hasDexExtension(const char* name) {
    size_t len = strlen(name);

    return (len >= 5)
        && (name[len - 5] != '/')
        && (strcmp(&name[len - 4], ".dex") == 0);
}

#ifdef FASTIVA
static const int JAR_FILE				= 0;
static const int DEX_FILE				= 1;
static const int FASTIVA_NOT_LOADED		= 2;
static const int FASTIVA_LOADED			= 3;
static const char* kDexInJarName = "classes.dex";

char* fastiva_hasCompiled(const char* sourceName, const char* name) {
	char* cachedName = NULL;
	//if (name == NULL) {
		int fastiva_openAlternateSuffix(const char *fileName, const char *suffix,
										int flags, char **pCachedName);

		int fd = fastiva_openAlternateSuffix(sourceName, "dex.so", O_RDONLY, &cachedName);
		if (fd >= 0) {
			close(fd);
			return cachedName;
		}


		cachedName = dexOptGenerateCacheFileName(sourceName,
									kDexInJarName);
		name = cachedName;
	//}
    size_t len = strlen(name);
	char* fileName = (char*)malloc(len + 4);
	memcpy(fileName, name, len);
	memcpy(fileName + len, ".so", 4);
	if (cachedName != NULL) {
		free(cachedName);
	}
    int rc = TEMP_FAILURE_RETRY(access(fileName, F_OK));
	if (rc == 0) {
		return fileName;
	}
	free(fileName);
	return NULL;
}
#endif
/*
 * Internal struct for managing DexFile.
 */
struct DexOrJar {
    char*       fileName;
#ifdef FASTIVA
    char*       fastivaFileName;
    u1			fileType;
#else
	bool		isDex;
#endif
    bool        okayToFree;
    RawDexFile* pRawDexFile;
    JarFile*    pJarFile;
    u1*         pDexMemory; // malloc()ed memory, if any
};

/*
 * (This is a dvmHashTableFree callback.)
 */
void dvmFreeDexOrJar(void* vptr)
{
    DexOrJar* pDexOrJar = (DexOrJar*) vptr;

    ALOGV("Freeing DexOrJar '%s'", pDexOrJar->fileName);

#ifdef FASTIVA
	switch (pDexOrJar->fileType) {
	case DEX_FILE:
        dvmRawDexFileFree(pDexOrJar->pRawDexFile);
		break;
	case JAR_FILE:
        dvmJarFileFree(pDexOrJar->pJarFile);
		break;
	case FASTIVA_NOT_LOADED:
	case FASTIVA_LOADED:
		break;
	default:
		ALOGE("Unknown File Type %d", pDexOrJar->fileType);
	}
#else
	if (pDexOrJar->isDex)
        dvmRawDexFileFree(pDexOrJar->pRawDexFile);
    else
        dvmJarFileFree(pDexOrJar->pJarFile);
#endif
    free(pDexOrJar->fileName);
#ifdef FASTIVA
	free(pDexOrJar->fastivaFileName);
#endif
    free(pDexOrJar->pDexMemory);
    free(pDexOrJar);
}

/*
 * (This is a dvmHashTableLookup compare func.)
 *
 * Args are DexOrJar*.
 */
static int hashcmpDexOrJar(const void* tableVal, const void* newVal)
{
    return (int) newVal - (int) tableVal;
}

/*
 * Verify that the "cookie" is a DEX file we opened.
 *
 * Expects that the hash table will be *unlocked* here.
 *
 * If the cookie is invalid, we throw an exception and return "false".
 */
static bool validateCookie(int cookie)
{
    DexOrJar* pDexOrJar = (DexOrJar*) cookie;

    LOGVV("+++ dex verifying cookie %p", pDexOrJar);

    if (pDexOrJar == NULL)
        return false;

    u4 hash = cookie;
    dvmHashTableLock(gDvm.userDexFiles);
    void* result = dvmHashTableLookup(gDvm.userDexFiles, hash, pDexOrJar,
                hashcmpDexOrJar, false);
    dvmHashTableUnlock(gDvm.userDexFiles);
    if (result == NULL) {
        dvmThrowRuntimeException("invalid DexFile cookie");
        return false;
    }

    return true;
}


/*
 * Add given DexOrJar to the hash table of user-loaded dex files.
 */
static void addToDexFileTable(DexOrJar* pDexOrJar) {
    /*
     * Later on, we will receive this pointer as an argument and need
     * to find it in the hash table without knowing if it's valid or
     * not, which means we can't compute a hash value from anything
     * inside DexOrJar. We don't share DexOrJar structs when the same
     * file is opened multiple times, so we can just use the low 32
     * bits of the pointer as the hash.
     */
    u4 hash = (u4) pDexOrJar;
    void* result;

    dvmHashTableLock(gDvm.userDexFiles);
    result = dvmHashTableLookup(gDvm.userDexFiles, hash, pDexOrJar,
            hashcmpDexOrJar, true);
    dvmHashTableUnlock(gDvm.userDexFiles);

    if (result != pDexOrJar) {
        ALOGE("Pointer has already been added?");
        dvmAbort();
    }

    pDexOrJar->okayToFree = true;
}

/*
 * private static int openDexFile(String sourceName, String outputName,
 *     int flags) throws IOException
 *
 * Open a DEX file, returning a pointer to our internal data structure.
 *
 * "sourceName" should point to the "source" jar or DEX file.
 *
 * If "outputName" is NULL, the DEX code will automatically find the
 * "optimized" version in the cache directory, creating it if necessary.
 * If it's non-NULL, the specified file will be used instead.
 *
 * TODO: at present we will happily open the same file more than once.
 * To optimize this away we could search for existing entries in the hash
 * table and refCount them.  Requires atomic ops or adding "synchronized"
 * to the non-native code that calls here.
 *
 * TODO: should be using "long" for a pointer.
 */
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
#define Dalvik_dalvik_system_DexFile_openDexFileNative Dalvik_dalvik_system_DexFile_openDexFile
#endif

static void Dalvik_dalvik_system_DexFile_openDexFileNative(const u4* args,
    JValue* pResult)
{
    StringObject* sourceNameObj = (StringObject*) args[0];
    StringObject* outputNameObj = (StringObject*) args[1];

#ifdef FASTIVA
	jint fastiva_Dalvik_dalvik_system_DexFile_openDexFileNative(java_lang_String_p, java_lang_String_p, jint);
	*(jint*)pResult = fastiva_Dalvik_dalvik_system_DexFile_openDexFileNative((java_lang_String_p)sourceNameObj, (java_lang_String_p)outputNameObj, 0);
}

jint fastiva_Dalvik_dalvik_system_DexFile_openDexFileNative(java_lang_String_p sourceNameObj, java_lang_String_p outputNameObj, jint unused$) {
#endif

    DexOrJar* pDexOrJar = NULL;
    JarFile* pJarFile;
    RawDexFile* pRawDexFile;
    char* sourceName;
    char* outputName;

    if (sourceNameObj == NULL) {
        dvmThrowNullPointerException("sourceName == null");
        THROW_V();
    }

    sourceName = dvmCreateCstrFromString(sourceNameObj);
    if (outputNameObj != NULL)
        outputName = dvmCreateCstrFromString(outputNameObj);
    else
        outputName = NULL;

    /*
     * We have to deal with the possibility that somebody might try to
     * open one of our bootstrap class DEX files.  The set of dependencies
     * will be different, and hence the results of optimization might be
     * different, which means we'd actually need to have two versions of
     * the optimized DEX: one that only knows about part of the boot class
     * path, and one that knows about everything in it.  The latter might
     * optimize field/method accesses based on a class that appeared later
     * in the class path.
     *
     * We can't let the user-defined class loader open it and start using
     * the classes, since the optimized form of the code skips some of
     * the method and field resolution that we would ordinarily do, and
     * we'd have the wrong semantics.
     *
     * We have to reject attempts to manually open a DEX file from the boot
     * class path.  The easiest way to do this is by filename, which works
     * out because variations in name (e.g. "/system/framework/./ext.jar")
     * result in us hitting a different dalvik-cache entry.  It's also fine
     * if the caller specifies their own output file.
     */
    if (dvmClassPathContains(gDvm.bootClassPath, sourceName)) {
        ALOGW("Refusing to reopen boot DEX '%s'", sourceName);
        dvmThrowIOException(
            "Re-opening BOOTCLASSPATH DEX files is not allowed");
        free(sourceName);
        free(outputName);
        THROW_V();
    }

    /*
     * Try to open it directly as a DEX if the name ends with ".dex".
     * If that fails (or isn't tried in the first place), try it as a
     * Zip with a "classes.dex" inside.
     */
#ifdef FASTIVA
	char* fastiva_so_name;
	if ((fastiva_so_name = fastiva_hasCompiled(sourceName, outputName))) {

        ALOGD("Opening compiled Fastiva file '%s'", fastiva_so_name);

        pDexOrJar = (DexOrJar*) malloc(sizeof(DexOrJar));
        pDexOrJar->fileType = FASTIVA_NOT_LOADED;
		pDexOrJar->fastivaFileName = fastiva_so_name;
        pDexOrJar->pRawDexFile = NULL;
        pDexOrJar->pDexMemory = NULL;
		pDexOrJar->pJarFile = NULL;
	}
	else
#endif
    if (hasDexExtension(sourceName)
            && dvmRawDexFileOpen(sourceName, outputName, &pRawDexFile, false) == 0) {
        ALOGD("Opening DEX file '%s' (DEX)", sourceName);

        pDexOrJar = (DexOrJar*) malloc(sizeof(DexOrJar));
#ifdef FASTIVA
		pDexOrJar->fastivaFileName = NULL;
		pDexOrJar->fileType = DEX_FILE;
#else
        pDexOrJar->isDex = true;
#endif
        pDexOrJar->pRawDexFile = pRawDexFile;
        pDexOrJar->pDexMemory = NULL;
    } else if (dvmJarFileOpen(sourceName, outputName, &pJarFile, false) == 0) {
        ALOGD("Opening DEX file '%s' (Jar)", sourceName);

        pDexOrJar = (DexOrJar*) malloc(sizeof(DexOrJar));

#ifdef FASTIVA
		pDexOrJar->fastivaFileName = NULL;
		if (FASTIVA_IS_PRECOMPILED(sourceName)) {
	        pDexOrJar->fileType = FASTIVA_LOADED;
		}
		else {
	        pDexOrJar->fileType = JAR_FILE;
		}
#else
        pDexOrJar->isDex = false;
#endif
        pDexOrJar->pJarFile = pJarFile;
        pDexOrJar->pDexMemory = NULL;
    } else {
        ALOGV("Unable to open DEX file '%s'", sourceName);
        dvmThrowIOException("unable to open DEX file");
    }

    if (pDexOrJar != NULL) {
        pDexOrJar->fileName = sourceName;
        addToDexFileTable(pDexOrJar);
    } else {
        free(sourceName);
    }
#ifdef _DEBUG
	void fastiva_init_debug();
	fastiva_init_debug();
#endif

    free(outputName);
    RETURN_PTR((jint)pDexOrJar);
}

/*
 * private static int openDexFile(byte[] fileContents) throws IOException
 *
 * Open a DEX file represented in a byte[], returning a pointer to our
 * internal data structure.
 *
 * The system will only perform "essential" optimizations on the given file.
 *
 * TODO: should be using "long" for a pointer.
 */
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
static void Dalvik_dalvik_system_DexFile_openDexFile_bytearray(const u4* args,
    JValue* pResult)
{
    ArrayObject* fileContentsObj = (ArrayObject*) args[0];

#ifdef FASTIVA
	jint fastiva_Dalvik_dalvik_system_DexFile_openDexFile_bytearray(Byte_ap);
	*(jint*)pResult = fastiva_Dalvik_dalvik_system_DexFile_openDexFile_bytearray((Byte_ap)fileContentsObj);
}

jint fastiva_Dalvik_dalvik_system_DexFile_openDexFile_bytearray(Byte_ap arg0) {
    ArrayObject* fileContentsObj = (ArrayObject*)arg0;
#endif


	u4 length;
    u1* pBytes;
    RawDexFile* pRawDexFile;
    DexOrJar* pDexOrJar = NULL;

    if (fileContentsObj == NULL) {
        dvmThrowNullPointerException("fileContents == null");
        THROW_V();
    }

    /* TODO: Avoid making a copy of the array. (note array *is* modified) */
    length = fileContentsObj->length;
    pBytes = (u1*) malloc(length);

    if (pBytes == NULL) {
        dvmThrowRuntimeException("unable to allocate DEX memory");
        THROW_V();
    }

    memcpy(pBytes, fileContentsObj->contents, length);

    if (dvmRawDexFileOpenArray(pBytes, length, &pRawDexFile) != 0) {
        ALOGV("Unable to open in-memory DEX file");
        free(pBytes);
        dvmThrowRuntimeException("unable to open in-memory DEX file");
        THROW_V();
    }

    ALOGV("Opening in-memory DEX");
    pDexOrJar = (DexOrJar*) malloc(sizeof(DexOrJar));
#ifdef FASTIVA
    pDexOrJar->fileType = DEX_FILE;
#else
    pDexOrJar->isDex = true;
#endif
    pDexOrJar->pRawDexFile = pRawDexFile;
    pDexOrJar->pDexMemory = pBytes;
    pDexOrJar->fileName = strdup("<memory>"); // Needs to be free()able.
    addToDexFileTable(pDexOrJar);

    RETURN_PTR((jint)pDexOrJar);
}
#endif

/*
 * private static void closeDexFile(int cookie)
 *
 * Release resources associated with a user-loaded DEX file.
 */
static void Dalvik_dalvik_system_DexFile_closeDexFile(const u4* args,
    JValue* pResult)
{
    int cookie = args[0];
#ifdef FASTIVA
	void fastiva_Dalvik_dalvik_system_DexFile_closeDexFile(jint);
	fastiva_Dalvik_dalvik_system_DexFile_closeDexFile(cookie);
}

void fastiva_Dalvik_dalvik_system_DexFile_closeDexFile(jint cookie) {
#endif
    DexOrJar* pDexOrJar = (DexOrJar*) cookie;

    if (pDexOrJar == NULL)
        RETURN_VOID();
    if (!validateCookie(cookie))
        THROW_VOID();

    ALOGV("Closing DEX file %p (%s)", pDexOrJar, pDexOrJar->fileName);

    /*
     * We can't just free arbitrary DEX files because they have bits and
     * pieces of loaded classes.  The only exception to this rule is if
     * they were never used to load classes.
     *
     * If we can't free them here, dvmInternalNativeShutdown() will free
     * them when the VM shuts down.
     */
    if (pDexOrJar->okayToFree) {
        u4 hash = (u4) pDexOrJar;
        dvmHashTableLock(gDvm.userDexFiles);
        if (!dvmHashTableRemove(gDvm.userDexFiles, hash, pDexOrJar)) {
            ALOGW("WARNING: could not remove '%s' from DEX hash table",
                pDexOrJar->fileName);
        }
        dvmHashTableUnlock(gDvm.userDexFiles);
        ALOGV("+++ freeing DexFile '%s' resources", pDexOrJar->fileName);
        dvmFreeDexOrJar(pDexOrJar);
    } else {
        ALOGV("+++ NOT freeing DexFile '%s' resources", pDexOrJar->fileName);
    }

    RETURN_VOID();
}

/*
 * private static Class defineClassNative(String name, ClassLoader loader,
 *      int cookie)
 *
 * Load a class from a DEX file.  This is roughly equivalent to defineClass()
 * in a regular VM -- it's invoked by the class loader to cause the
 * creation of a specific class.  The difference is that the search for and
 * reading of the bytes is done within the VM.
 *
 * The class name is a "binary name", e.g. "java.lang.String".
 *
 * Returns a null pointer with no exception if the class was not found.
 * Throws an exception on other failures.
 */
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
#define fastiva_Dalvik_dalvik_system_DexFile_defineClassNative fastiva_Dalvik_dalvik_system_DexFile_defineClass
#endif

static void Dalvik_dalvik_system_DexFile_defineClassNative(const u4* args,
    JValue* pResult)
{
    StringObject* nameObj = (StringObject*) args[0];
    Object* loader = (Object*) args[1];
    int cookie = args[2];

#ifdef FASTIVA
	java_lang_Class_p fastiva_Dalvik_dalvik_system_DexFile_defineClassNative(java_lang_String_p, java_lang_ClassLoader_p, jint);
	*(java_lang_Class_p*)pResult = fastiva_Dalvik_dalvik_system_DexFile_defineClassNative((java_lang_String_p)nameObj, (java_lang_ClassLoader_p)loader, cookie);
}

java_lang_Class_p fastiva_Dalvik_dalvik_system_DexFile_defineClassNative(java_lang_String_p nameObj, java_lang_ClassLoader_p loader, jint cookie) {
#endif

    ClassObject* clazz = NULL;
    DexOrJar* pDexOrJar = (DexOrJar*) cookie;
    DvmDex* pDvmDex;
    char* name;
    char* descriptor;


    name = dvmCreateCstrFromString(nameObj);
    descriptor = dvmDotToDescriptor(name);
#ifdef _DEBUG
	//assert(strcmp(descriptor, "Lorg/keyczar/KeyMetadata;") != 0);
#endif

    ALOGV("--- Explicit class load '%s' l=%p c=0x%08x",
        descriptor, loader, cookie);
    free(name);

	if (!validateCookie(cookie))
        THROW_V();

#ifdef FASTIVA
    Thread* self = dvmThreadSelf();
	switch (pDexOrJar->fileType) {
	case FASTIVA_NOT_LOADED: 
		{
			pDvmDex = NULL;
			char* fileName = pDexOrJar->fastivaFileName;
			assert(pDexOrJar->pJarFile == NULL);
			pDexOrJar->fileType = FASTIVA_LOADED;
			char* reason;
			bool success = dvmLoadNativeCode(fileName, loader, &reason);
			free(reason);
			ALOGD("fastiva.so loaded on %s", fileName);
			//if (!success) {
			//	const char* msg = (reason != NULL) ? reason : "unknown failure";
			//	result = dvmCreateStringFromCstr(msg);
			//	dvmReleaseTrackedAlloc((Object*) result, NULL);
			//}

		}
		break;
	case FASTIVA_LOADED:
		if (pDexOrJar->pJarFile != NULL) {
			assert(FASTIVA_IS_PRECOMPILED(pDexOrJar->fileName));
	        pDvmDex = dvmGetJarFileDex(pDexOrJar->pJarFile);
		}
		else {
			pDvmDex = NULL;
			ALOGD("Multiple fastiva load on %s", pDexOrJar->fileName);
		}
		break;
	case DEX_FILE:
        pDvmDex = dvmGetRawDexFileDex(pDexOrJar->pRawDexFile);
		break;
	case JAR_FILE:
        pDvmDex = dvmGetJarFileDex(pDexOrJar->pJarFile);
	}
#else
    if (pDexOrJar->isDex)
        pDvmDex = dvmGetRawDexFileDex(pDexOrJar->pRawDexFile);
    else
        pDvmDex = dvmGetJarFileDex(pDexOrJar->pJarFile);
#endif

    /* once we load something, we can't unmap the storage */
    pDexOrJar->okayToFree = false;


    clazz = dvmDefineClass(pDvmDex, descriptor, loader);
#ifndef FASTIVA
    Thread* self = dvmThreadSelf();
#endif
    if (dvmCheckException(self)) {
        /*
         * If we threw a "class not found" exception, stifle it, since the
         * contract in the higher method says we simply return null if
         * the class is not found.
         */
        Object* excep = dvmGetException(self);
        if (strcmp(excep->clazz->descriptor,
                   "Ljava/lang/ClassNotFoundException;") == 0 ||
            strcmp(excep->clazz->descriptor,
                   "Ljava/lang/NoClassDefFoundError;") == 0)
        {
            dvmClearException(self);
        }
        clazz = NULL;
    }


    free(descriptor);
    RETURN_PTR((java_lang_Class_p)clazz);
}

/*
 * private static String[] getClassNameList(int cookie)
 *
 * Returns a String array that holds the names of all classes in the
 * specified DEX file.
 */
static void Dalvik_dalvik_system_DexFile_getClassNameList(const u4* args,
    JValue* pResult)
{
    int cookie = args[0];
#ifdef FASTIVA
	java_lang_String_ap fastiva_Dalvik_dalvik_system_DexFile_getClassNameList(jint);
	*(java_lang_String_ap*)pResult = fastiva_Dalvik_dalvik_system_DexFile_getClassNameList(cookie);
}

java_lang_String_ap fastiva_Dalvik_dalvik_system_DexFile_getClassNameList(jint cookie) {
#endif

    DexOrJar* pDexOrJar = (DexOrJar*) cookie;
    Thread* self = dvmThreadSelf();

    if (!validateCookie(cookie))
        THROW_V();

    DvmDex* pDvmDex;
#ifdef FASTIVA
	switch (pDexOrJar->fileType) {
	case FASTIVA_NOT_LOADED:
	case FASTIVA_LOADED:
		ALOGD("##### getClassNameList called on Fastiva");
		return java_lang_String_A::create$(0);
	case DEX_FILE:
        pDvmDex = dvmGetRawDexFileDex(pDexOrJar->pRawDexFile);
		break;
	case JAR_FILE:
        pDvmDex = dvmGetJarFileDex(pDexOrJar->pJarFile);
	}
#else
    if (pDexOrJar->isDex)
        pDvmDex = dvmGetRawDexFileDex(pDexOrJar->pRawDexFile);
    else
        pDvmDex = dvmGetJarFileDex(pDexOrJar->pJarFile);
#endif
    assert(pDvmDex != NULL);
    DexFile* pDexFile = pDvmDex->pDexFile;

    int count = pDexFile->pHeader->classDefsSize;
    ClassObject* arrayClass =
        dvmFindArrayClassForElement(gDvm.classJavaLangString);
    ArrayObject* stringArray =
        dvmAllocArrayByClass(arrayClass, count, ALLOC_DEFAULT);
    if (stringArray == NULL) {
        /* probably OOM */
        ALOGD("Failed allocating array of %d strings", count);
        assert(dvmCheckException(self));
        THROW_V();
    }

    int i;
    for (i = 0; i < count; i++) {
        const DexClassDef* pClassDef = dexGetClassDef(pDexFile, i);
        const char* descriptor =
            dexStringByTypeIdx(pDexFile, pClassDef->classIdx);

        char* className = dvmDescriptorToDot(descriptor);
        StringObject* str = dvmCreateStringFromCstr(className);
        dvmSetObjectArrayElement(stringArray, i, (Object *)str);
        dvmReleaseTrackedAlloc((Object *)str, self);
        free(className);
    }

    dvmReleaseTrackedAlloc((Object*)stringArray, self);
    RETURN_PTR((java_lang_String_ap)stringArray);
}

/*
 * public static boolean isDexOptNeeded(String fileName)
 *         throws FileNotFoundException, IOException
 *
 * Returns true if the VM believes that the apk/jar file is out of date
 * and should be passed through "dexopt" again.
 *
 * @param fileName the absolute path to the apk/jar file to examine.
 * @return true if dexopt should be called on the file, false otherwise.
 * @throws java.io.FileNotFoundException if fileName is not readable,
 *         not a file, or not present.
 * @throws java.io.IOException if fileName is not a valid apk/jar file or
 *         if problems occur while parsing it.
 * @throws java.lang.NullPointerException if fileName is null.
 * @throws dalvik.system.StaleDexCacheError if the optimized dex file
 *         is stale but exists on a read-only partition.
 */
static void Dalvik_dalvik_system_DexFile_isDexOptNeeded(const u4* args,
    JValue* pResult)
{
    StringObject* nameObj = (StringObject*) args[0];
 
#ifdef FASTIVA
	jbool fastiva_Dalvik_dalvik_system_DexFile_isDexOptNeeded(java_lang_String_p);
	*(jbool*)pResult = fastiva_Dalvik_dalvik_system_DexFile_isDexOptNeeded((java_lang_String_p)nameObj);
}

jbool fastiva_Dalvik_dalvik_system_DexFile_isDexOptNeeded(java_lang_String_p nameObj) {
#endif

   char* name;
    DexCacheStatus status;
    int result;

    name = dvmCreateCstrFromString(nameObj);
    if (name == NULL) {
        dvmThrowNullPointerException("fileName == null");
        THROW_V();
    }
    if (access(name, R_OK) != 0) {
        dvmThrowFileNotFoundException(name);
        free(name);
        THROW_V();
    }
    status = dvmDexCacheStatus(name);
    ALOGV("dvmDexCacheStatus(%s) returned %d", name, status);

    result = true;
    switch (status) {
    default: //FALLTHROUGH
    case DEX_CACHE_BAD_ARCHIVE:
        dvmThrowIOException(name);
        result = -1;
        break;
    case DEX_CACHE_OK:
        result = false;
        break;
    case DEX_CACHE_STALE:
        result = true;
        break;
    case DEX_CACHE_STALE_ODEX:
        dvmThrowStaleDexCacheError(name);
        result = -1;
        break;
    }
    free(name);

    if (result >= 0) {
        RETURN_BOOLEAN(result);
    } else {
        THROW_V();
    }
}


const DalvikNativeMethod dvm_dalvik_system_DexFile[] = {
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
    { "openDexFile",		"(Ljava/lang/String;Ljava/lang/String;I)I",
#else
    { "openDexFileNative",  "(Ljava/lang/String;Ljava/lang/String;I)I",
#endif
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_DexFile_openDexFileNative) },
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
    { "openDexFile",        "([B)I",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_DexFile_openDexFile_bytearray) },
#endif
    { "closeDexFile",       "(I)V",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_DexFile_closeDexFile) },
#if FASTIVA_TARGET_ANDROID_VERSION < 40400
    { "defineClass",        "(Ljava/lang/String;Ljava/lang/ClassLoader;I)Ljava/lang/Class;",
#else
    { "defineClassNative",  "(Ljava/lang/String;Ljava/lang/ClassLoader;I)Ljava/lang/Class;",
#endif
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_DexFile_defineClassNative) },
    { "getClassNameList",   "(I)[Ljava/lang/String;",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_DexFile_getClassNameList) },
    { "isDexOptNeeded",     "(Ljava/lang/String;)Z",
        FASTIVA_INTERNAL_METHOD_PAIR(Dalvik_dalvik_system_DexFile_isDexOptNeeded) },
    { NULL, NULL, NULL, NULL },
};


