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
 * Functions for dealing with method prototypes
 */

#include "DexProto.h"

#ifdef NO_FASTIVA_PROTO
#undef FASTIVA
#endif


#ifdef FASTIVA
#include <fastiva/jni_constants.h> // to use precompiled header
#include <fastiva/jni_def.h> // to use precompiled header
#include <fastiva/ClassContext.h> // to use precompiled header
#include <kernel/Module.h> // to use precompiled header
#endif

#include <stdlib.h>
#include <string.h>

/*
 * ===========================================================================
 *      String Cache
 * ===========================================================================
 */

/*
 * Make sure that the given cache can hold a string of the given length,
 * including the final '\0' byte.
 */
void dexStringCacheAlloc(DexStringCache* pCache, size_t length) {
    if (pCache->allocatedSize != 0) {
        if (pCache->allocatedSize >= length) {
            return;
        }
        free((void*) pCache->value);
    }

    if (length <= sizeof(pCache->buffer)) {
        pCache->value = pCache->buffer;
        pCache->allocatedSize = 0;
    } else {
        pCache->value = (char*) malloc(length);
        pCache->allocatedSize = length;
    }
}

/*
 * Initialize the given DexStringCache. Use this function before passing
 * one into any other function.
 */
void dexStringCacheInit(DexStringCache* pCache) {
    pCache->value = pCache->buffer;
    pCache->allocatedSize = 0;
    pCache->buffer[0] = '\0';
}

/*
 * Release the allocated contents of the given DexStringCache, if any.
 * Use this function after your last use of a DexStringCache.
 */
void dexStringCacheRelease(DexStringCache* pCache) {
    if (pCache->allocatedSize != 0) {
        free((void*) pCache->value);
        pCache->value = pCache->buffer;
        pCache->allocatedSize = 0;
    }
}

/*
 * If the given DexStringCache doesn't already point at the given value,
 * make a copy of it into the cache. This always returns a writable
 * pointer to the contents (whether or not a copy had to be made). This
 * function is intended to be used after making a call that at least
 * sometimes doesn't populate a DexStringCache.
 */
char* dexStringCacheEnsureCopy(DexStringCache* pCache, const char* value) {
    if (value != pCache->value) {
        size_t length = strlen(value) + 1;
        dexStringCacheAlloc(pCache, length);
        memcpy(pCache->value, value, length);
    }

    return pCache->value;
}

/*
 * Abandon the given DexStringCache, and return a writable copy of the
 * given value (reusing the string cache's allocation if possible).
 * The return value must be free()d by the caller. Use this instead of
 * dexStringCacheRelease() if you want the buffer to survive past the
 * scope of the DexStringCache.
 */
char* dexStringCacheAbandon(DexStringCache* pCache, const char* value) {
    if ((value == pCache->value) && (pCache->allocatedSize != 0)) {
        char* result = pCache->value;
        pCache->allocatedSize = 0;
        pCache->value = pCache->buffer;
        return result;
    } else {
        return strdup(value);
    }
}


/*
 * ===========================================================================
 *      Method Prototypes
 * ===========================================================================
 */

/*
 * Return the DexProtoId from the given DexProto. The DexProto must
 * actually refer to a DexProtoId.
 */
static inline const DexProtoId* getProtoId(const DexProto* pProto) {
    return dexGetProtoId(pProto->dexFile, pProto->protoIdx);
}

/* (documented in header file) */
const char* dexProtoGetShorty(const DexProto* pProto) {
    const DexProtoId* protoId = getProtoId(pProto);

    return dexStringById(pProto->dexFile, protoId->shortyIdx);
}

/* (documented in header file) */
const char* dexProtoGetMethodDescriptor(const DexProto* pProto,
        DexStringCache* pCache) {
    const DexFile* dexFile = pProto->dexFile;
#ifdef FASTIVA
	if (FASTIVA_IS_FASTIVA_PROTO(pProto)) {
		//const fastiva_MethodInfo* pMethod = (const fastiva_MethodInfo*)pProto->protoIdx;
		int args = FASTIVA_ARG_LIST_ID(pProto);
		int ret_t = FASTIVA_RET_TTYPE_ID(pProto);
		char buff[1024];
		char* dst = buff;
		*dst ++  = '(';

		JNI_ArgIterator it;
		const char* sig;
		for (int i = it.init(args); --i >= -1; ) {
			if (i < 0) {
				*dst++ = ')';
				sig = d2f_getTypeDescriptor(ret_t);
			}
			else {
				sig = d2f_getTypeDescriptor(it.nextID());
			}
			int len = strlen(sig);
			memcpy(dst, sig, len);
			dst += len;
		}
		*dst++ = 0;
		int len = dst - buff;
		dexStringCacheAlloc(pCache, len);
		char *at = (char*) pCache->value;
		memcpy(at, buff, len);
		return pCache->value;
	}
#endif
    const DexProtoId* protoId = getProtoId(pProto);
    const DexTypeList* typeList = dexGetProtoParameters(dexFile, protoId);
    size_t length = 3; // parens and terminating '\0'
    u4 paramCount = (typeList == NULL) ? 0 : typeList->size;
    u4 i;

    for (i = 0; i < paramCount; i++) {
        u4 idx = dexTypeListGetIdx(typeList, i);
        length += strlen(dexStringByTypeIdx(dexFile, idx));
    }

    length += strlen(dexStringByTypeIdx(dexFile, protoId->returnTypeIdx));

    dexStringCacheAlloc(pCache, length);

    char *at = (char*) pCache->value;
    *(at++) = '(';

    for (i = 0; i < paramCount; i++) {
        u4 idx = dexTypeListGetIdx(typeList, i);
        const char* desc = dexStringByTypeIdx(dexFile, idx);
        strcpy(at, desc);
        at += strlen(desc);
    }

    *(at++) = ')';

    strcpy(at, dexStringByTypeIdx(dexFile, protoId->returnTypeIdx));
    return pCache->value;
}

/* (documented in header file) */
char* dexProtoCopyMethodDescriptor(const DexProto* pProto) {
    DexStringCache cache;

    dexStringCacheInit(&cache);
    return dexStringCacheAbandon(&cache,
            dexProtoGetMethodDescriptor(pProto, &cache));
}

/* (documented in header file) */
const char* dexProtoGetParameterDescriptors(const DexProto* pProto,
        DexStringCache* pCache) {
    DexParameterIterator iterator;
    size_t length = 1; /* +1 for the terminating '\0' */

    dexParameterIteratorInit(&iterator, pProto);

    for (;;) {
        const char* descriptor = dexParameterIteratorNextDescriptor(&iterator);
        if (descriptor == NULL) {
            break;
        }

        length += strlen(descriptor);
    }

    dexParameterIteratorInit(&iterator, pProto);

    dexStringCacheAlloc(pCache, length);
    char *at = (char*) pCache->value;

    for (;;) {
        const char* descriptor = dexParameterIteratorNextDescriptor(&iterator);
        if (descriptor == NULL) {
            break;
        }

        strcpy(at, descriptor);
        at += strlen(descriptor);
    }

    return pCache->value;
}

/* (documented in header file) */
const char* dexProtoGetReturnType(const DexProto* pProto) {
#ifdef FASTIVA
	if (FASTIVA_IS_FASTIVA_PROTO(pProto)) {
		return d2f_getTypeDescriptor(FASTIVA_RET_TTYPE_ID(pProto));
	}
#endif
    const DexProtoId* protoId = getProtoId(pProto);
    return dexStringByTypeIdx(pProto->dexFile, protoId->returnTypeIdx);
}

/* (documented in header file) */
size_t dexProtoGetParameterCount(const DexProto* pProto) {
#ifdef FASTIVA
	DexParameterIterator it;
	dexParameterIteratorInit(&it, pProto);
	return it.parameterCount;
#else
    const DexProtoId* protoId = getProtoId(pProto);
    const DexTypeList* typeList =
        dexGetProtoParameters(pProto->dexFile, protoId);
    return (typeList == NULL) ? 0 : typeList->size;
#endif
}

/* (documented in header file) */
int dexProtoComputeArgsSize(const DexProto* pProto) {
#ifdef FASTIVA
	if (FASTIVA_IS_FASTIVA_PROTO(pProto)) {
		JNI_ArgIterator pIterator[1];
		int argList_id = FASTIVA_ARG_LIST_ID(pProto);
		int cntArg = ((JNI_ArgIterator*)pIterator)->init(argList_id);
		return cntArg;
	}
#endif
    const char* shorty = dexProtoGetShorty(pProto);
    int count = 0;

    /* Skip the return type. */
    shorty++;

    for (;;) {
        switch (*(shorty++)) {
            case '\0': {
                return count;
            }
            case 'D':
            case 'J': {
                count += 2;
                break;
            }
            default: {
                count++;
                break;
            }
        }
    }
}

#ifdef FASTIVA
const char* fastiva_dexGetReturnTypeDescriptor(const DexProto* pProto) {
	if (FASTIVA_IS_FASTIVA_PROTO(pProto)) {
		int ret_t = FASTIVA_RET_TTYPE_ID(pProto);
		const char* desc = d2f_getTypeDescriptor(ret_t);
		return desc;
	}
	else {
        const DexProtoId* protoId = getProtoId(pProto);
		const char* desc = dexStringByTypeIdx(pProto->dexFile, protoId->returnTypeIdx);
		return desc;
	}
}
#endif

/*
 * Common implementation for dexProtoCompare() and dexProtoCompareParameters().
 */
static int protoCompare(const DexProto* pProto1, const DexProto* pProto2,
        bool compareReturnType) {

    if (pProto1 == pProto2) {
        // Easy out.
        return 0;
    } else {
#ifdef FASTIVA
		DexParameterIterator list1;
		dexParameterIteratorInit(&list1, pProto1);

        //const DexFile* dexFile1 = pProto1->dexFile;
        //const DexProtoId* protoId1 = getProtoId(pProto1);
        //const DexTypeList* typeList1 =
        //    dexGetProtoParameters(dexFile1, protoId1);
        int paramCount1 = list1.parameterCount;

		DexParameterIterator list2;
		dexParameterIteratorInit(&list2, pProto2);

		//const DexFile* dexFile2 = pProto2->dexFile;
        //const DexProtoId* protoId2 = getProtoId(pProto2);
        //const DexTypeList* typeList2 =
        //    dexGetProtoParameters(dexFile2, protoId2);
        int paramCount2 = list2.parameterCount;

        // Compare return types.

        if (compareReturnType) {
            int result =
                strcmp(fastiva_dexGetReturnTypeDescriptor(pProto1),
					   fastiva_dexGetReturnTypeDescriptor(pProto2));

            if (result != 0) {
                return result;
            }
        }


		if (list1.parameters == list2.parameters) {
            // @Zee must after return type comparing.
            return 0;
        }

        // Compare parameters.

        int minParam = (paramCount1 > paramCount2) ? paramCount2 : paramCount1;
        int i;

        for (i = 0; i < minParam; i++) {
            int result =
                strcmp(dexParameterIteratorNextDescriptor(&list1),
                       dexParameterIteratorNextDescriptor(&list2));

            if (result != 0) {
                return result;
            }
        }
#else
        const DexFile* dexFile1 = pProto1->dexFile;
        const DexProtoId* protoId1 = getProtoId(pProto1);
        const DexTypeList* typeList1 =
            dexGetProtoParameters(dexFile1, protoId1);
        int paramCount1 = (typeList1 == NULL) ? 0 : typeList1->size;

        const DexFile* dexFile2 = pProto2->dexFile;
        const DexProtoId* protoId2 = getProtoId(pProto2);
        const DexTypeList* typeList2 =
            dexGetProtoParameters(dexFile2, protoId2);
        int paramCount2 = (typeList2 == NULL) ? 0 : typeList2->size;

        if (protoId1 == protoId2) {
            // Another easy out.
            return 0;
        }

        // Compare return types.

        if (compareReturnType) {
            int result =
                strcmp(dexStringByTypeIdx(dexFile1, protoId1->returnTypeIdx),
                        dexStringByTypeIdx(dexFile2, protoId2->returnTypeIdx));

            if (result != 0) {
                return result;
            }
        }

        // Compare parameters.

        int minParam = (paramCount1 > paramCount2) ? paramCount2 : paramCount1;
        int i;

        for (i = 0; i < minParam; i++) {
            u4 idx1 = dexTypeListGetIdx(typeList1, i);
            u4 idx2 = dexTypeListGetIdx(typeList2, i);
            int result =
                strcmp(dexStringByTypeIdx(dexFile1, idx1),
                        dexStringByTypeIdx(dexFile2, idx2));

            if (result != 0) {
                return result;
            }
        }
#endif
        if (paramCount1 < paramCount2) {
            return -1;
        } else if (paramCount1 > paramCount2) {
            return 1;
        } else {
            return 0;
        }
    }
}

/* (documented in header file) */
int dexProtoCompare(const DexProto* pProto1, const DexProto* pProto2) {
    return protoCompare(pProto1, pProto2, true);
}

/* (documented in header file) */
int dexProtoCompareParameters(const DexProto* pProto1, const DexProto* pProto2){
    return protoCompare(pProto1, pProto2, false);
}


/*
 * Helper for dexProtoCompareToDescriptor(), which gets the return type
 * descriptor from a method descriptor string.
 */
static const char* methodDescriptorReturnType(const char* descriptor) {
    const char* result = strchr(descriptor, ')');

    if (result == NULL) {
        return NULL;
    }

    // The return type is the character just past the ')'.
    return result + 1;
}

/*
 * Helper for dexProtoCompareToDescriptor(), which indicates the end
 * of an embedded argument type descriptor, which is also the
 * beginning of the next argument type descriptor. Since this is for
 * argument types, it doesn't accept 'V' as a valid type descriptor.
 */
static const char* methodDescriptorNextType(const char* descriptor) {
    // Skip any array references.

    while (*descriptor == '[') {
        descriptor++;
    }

    switch (*descriptor) {
        case 'B': case 'C': case 'D': case 'F':
        case 'I': case 'J': case 'S': case 'Z': {
            return descriptor + 1;
        }
        case 'L': {
            const char* result = strchr(descriptor + 1, ';');
            if (result != NULL) {
                // The type ends just past the ';'.
                return result + 1;
            }
        }
    }

    return NULL;
}

/*
 * Common implementation for dexProtoCompareToDescriptor() and
 * dexProtoCompareToParameterDescriptors(). The descriptor argument
 * can be either a full method descriptor (with parens and a return
 * type) or an unadorned concatenation of types (e.g. a list of
 * argument types).
 */
static int protoCompareToParameterDescriptors(const DexProto* proto,
        const char* descriptor, bool expectParens) {
    char expectedEndChar = expectParens ? ')' : '\0';
    DexParameterIterator iterator;
    dexParameterIteratorInit(&iterator, proto);

    if (expectParens) {
        // Skip the '('.
        assert (*descriptor == '(');
        descriptor++;
    }

    for (;;) {
        const char* protoDesc = dexParameterIteratorNextDescriptor(&iterator);

        if (*descriptor == expectedEndChar) {
            // It's the end of the descriptor string.
            if (protoDesc == NULL) {
                // It's also the end of the prototype's arguments.
                return 0;
            } else {
                // The prototype still has more arguments.
                return 1;
            }
        }

        if (protoDesc == NULL) {
            /*
             * The prototype doesn't have arguments left, but the
             * descriptor string does.
             */
            return -1;
        }

        // Both prototype and descriptor have arguments. Compare them.

        const char* nextDesc = methodDescriptorNextType(descriptor);
        assert(nextDesc != NULL);

        for (;;) {
            char c1 = *(protoDesc++);
            char c2 = (descriptor < nextDesc) ? *(descriptor++) : '\0';

            if (c1 < c2) {
                // This includes the case where the proto is shorter.
                return -1;
            } else if (c1 > c2) {
                // This includes the case where the desc is shorter.
                return 1;
            } else if (c1 == '\0') {
                // The two types are equal in length. (c2 necessarily == '\0'.)
                break;
            }
        }

        /*
         * If we made it here, the two arguments matched, and
         * descriptor == nextDesc.
         */
    }
}

/* (documented in header file) */
int dexProtoCompareToDescriptor(const DexProto* proto,
        const char* descriptor) {
    // First compare the return types.

    const char *returnType = methodDescriptorReturnType(descriptor);
    assert(returnType != NULL);

    int result = strcmp(dexProtoGetReturnType(proto), returnType);

    if (result != 0) {
        return result;
    }

    // The return types match, so we have to check arguments.
    return protoCompareToParameterDescriptors(proto, descriptor, true);
}

/* (documented in header file) */
int dexProtoCompareToParameterDescriptors(const DexProto* proto,
        const char* descriptors) {
    return protoCompareToParameterDescriptors(proto, descriptors, false);
}






/*
 * ===========================================================================
 *      Parameter Iterators
 * ===========================================================================
 */

/*
 * Initialize the given DexParameterIterator to be at the start of the
 * parameters of the given prototype.
 */
void dexParameterIteratorInit(DexParameterIterator* pIterator,
        const DexProto* pProto) {
#ifdef FASTIVA
	if (FASTIVA_IS_FASTIVA_PROTO(pProto)) {
		pIterator->proto = NULL;
		int argList_id = FASTIVA_ARG_LIST_ID(pProto);
		pIterator->parameterCount = ((JNI_ArgIterator*)pIterator)->init(argList_id);
		return;
	}
#endif
    pIterator->proto = pProto;
    pIterator->cursor = 0;

    pIterator->parameters =
        dexGetProtoParameters(pProto->dexFile, getProtoId(pProto));
    pIterator->parameterCount = (pIterator->parameters == NULL) ? 0
        : pIterator->parameters->size;
}

/*
 * Get the type_id index for the next parameter, if any. This returns
 * kDexNoIndex if the last parameter has already been consumed.
 */
u4 dexParameterIteratorNextIndex(DexParameterIterator* pIterator) {
#ifdef FASTIVA
	if (pIterator->proto == NULL) {
		return ((JNI_ArgIterator*)pIterator)->nextID();
	}
#endif
    int cursor = pIterator->cursor;
    int parameterCount = pIterator->parameterCount;

    if (cursor >= parameterCount) {
        // The iteration is complete.
        return kDexNoIndex;
    } else {
        u4 idx = dexTypeListGetIdx(pIterator->parameters, cursor);
        pIterator->cursor++;
        return idx;
    }
}

/*
 * Get the type descriptor for the next parameter, if any. This returns
 * NULL if the last parameter has already been consumed.
 */
const char* dexParameterIteratorNextDescriptor(
        DexParameterIterator* pIterator) {
#ifdef FASTIVA
	if (pIterator->proto == NULL) {
		int id = ((JNI_ArgIterator*)pIterator)->nextID();
		return d2f_getTypeDescriptor(id);
	}
#endif
    u4 idx = dexParameterIteratorNextIndex(pIterator);

    if (idx == kDexNoIndex) {
        return NULL;
    }

    return dexStringByTypeIdx(pIterator->proto->dexFile, idx);
}

#ifdef FASTIVA

extern bool (*fastiva_isMultiArgList_fn)(u4 argsId) = 0;

int JNI_ArgIterator::init(uint argsId) {

	this->dexProto = 0;
	this->point = 0;// + JPP_LIBCORE_ARGLIST_ID_START;

	bool isArgList = fastiva_isMultiArgList_fn(argsId);

	if (!isArgList) {
		m_pArgType = &m_reserved;
		if (*(char*)argsId == 'V') {
			return (this->parameterCount = 0);
		}
		m_reserved = argsId;
		return (this->parameterCount = 1);
	}

	//int idx = argsId & FASTIVA_JNI_LAST_CLASS_ID;
	//idx -= 1;
/*
	if (argsId >= JPP_EXTERNAL_ARGLIST_ID_START) {
		argsId -= JPP_EXTERNAL_ARGLIST_ID_START;
		m_pArgType = (FASTIVA_TOKEN_T*)fastiva_component_ModuleInfo.m_aArgList[argsId].m_pszToken;
	}
	else 
*/
	{
		m_pArgType = (uint*)(argsId);
	}
	/*else {
		//argsId -= FASTIVA_JNI_LAST_CLASS_ID + 1;
		//m_pArgType = (FASTIVA_TOKEN_T*)kernelData.g_utfHashTable.getKeyword(argsId);
	}*/
	int cntArg = *m_pArgType ++;
	cntArg &= ~JPP_ARG_CNT_FLAG;
	return (this->parameterCount = cntArg);
}

const char* d2f_getTypeDescriptor(u4 typeId) {
	//const char* res = kernelData.g_typeSigs[typeID];
	if (typeId == (u4)-1) {
		return NULL;
	}

	const char* sig = (const char*)typeId;
	return sig;
#if 0
	int dim = (typeId >> FASTIVA_CLASS_ID_BITS) & 7;
	int class_id = (typeId & ~(-1 << FASTIVA_CLASS_ID_BITS));
	const int FASTIVA_CLASS_ID_BIT_MASK = 0xFFFF0FFF;
	const char* name;
	if (typeId >= ((u4)-1 << FASTIVA_PACKAGE_ID_SHIFT)) {
		PrimitiveType type;
		switch (typeId & FASTIVA_CLASS_ID_BIT_MASK) {
			case JPP_CLASS_ID::void$:
				type = PRIM_VOID;
				break;
			case JPP_CLASS_ID::jbool$:
				type = PRIM_BOOLEAN;
				break;
			case JPP_CLASS_ID::jbyte$:
				type = PRIM_BYTE;
				break;
			case JPP_CLASS_ID::jshort$:
				type = PRIM_SHORT;
				break;
			case JPP_CLASS_ID::unicod$:
				type = PRIM_CHAR;
				break;
			case JPP_CLASS_ID::jint$:
				type = PRIM_INT;
				break;
			case JPP_CLASS_ID::jlonglong$:
				type = PRIM_LONG;
				break;
			case JPP_CLASS_ID::jfloat$:
				type = PRIM_FLOAT;
				break;
			case JPP_CLASS_ID::jdouble$:
				type = PRIM_DOUBLE;
				break;
			default:
				assert(false == "Should not be here");
				type = PRIM_NOT;
		}

		name = dexGetPrimitiveTypeDescriptor(type);
		//return kernel.getPrimitiveSig((class_id >> 4), dim);
	}
	else {
		typeId &= ~FASTIVA_CLASS_ID_START;
		int packageId = typeId >> FASTIVA_PACKAGE_ID_SHIFT;
		fastiva_ModuleInfo* mi;
		if (packageId < FASTIVA_MAX_PACKAGE_CNT_PER_MODULE) {
			mi = &fastiva_libcore_ModuleInfo;
		}
		else {
			packageId -= FASTIVA_MAX_PACKAGE_CNT_PER_MODULE;
			mi = kernelData.g_appModule;
		}

		const fastiva_ClassContext* clazz = mi->m_aPackageSlot[packageId].m_pPackage->m_aContextSlot[class_id].m_pContext;
		name = clazz->descriptor;
	}
#ifdef _DEBUG
	for (int i = 1; i <= dim; i ++) {
		assert(name[-i] == '[');
	}
#endif
	return name - dim;
#endif
}


	//static const char* FASTIVA_SIG_CLASS_aaaaaaap = "[[[[[[[LCLASS_NAME;"; 
	//static const char FASTIVA_SIG_CLASS_aaaaaaap[] = "[[[[[[[LCLASS_NAME;"; 
	//static const char FASTIVA_SIG_CLASS_aaaaaaap[] = "[[[[[[[LCLASS_NAME;"; 
	//const char * FASTIVA_SIG_CLASS_p = FASTIVA_SIG_CLASS_aaaaaaap + 7 - 0; 


#endif
