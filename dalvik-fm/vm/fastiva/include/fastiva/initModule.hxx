//#include <JniInternal.h>

fastiva_RuntimeProxy fastiva_RuntimeProxy::g_instance;
void* fastiva_Instance_G$::aIVT$[1] = { NULL };
fastiva_Module FASTIVA_MODULE_NAME(ModuleInfo);
#ifdef _WIN32
extern fastiva_AnnotationItem fastiva_AnnotationsData$[];
#else
extern const fastiva_AnnotationItem fastiva_AnnotationsData$[];
#endif

FASTIVA_RUNTIME_EXPORT void FASTIVA_MODULE_NAME(initModuleInfo)() {
	FASTIVA_MODULE_NAME(ModuleInfo).init(
		s_all_classes$, FASTIVA_ARRAY_ITEM_COUNT(s_all_classes$),
		NULL/*s_refNames$*/, 0/*, FASTIVA_ARRAY_ITEM_COUNT(s_refNames$)*/,
		0, 0,
		fastiva_AnnotationsData$, fastiva_AnnotationCache$,
		0/*fastiva_initClassIDs*/,
		(const unicod* const*)FASTIVA_MODULE_NAME(constStrings$),
		FASTIVA_STRING_POOL);
}

#if 0
java_lang_String_p fastiva_RuntimeProxy::getImmortalString(
	int stringId
) {
	java_lang_String_p pStr;
	//ifassert(stringId >= FASTIVA_EXTERNAL_STRING_INDEX_START);
	stringId &= (FASTIVA_EXTERNAL_STRING_INDEX_START - 1);
	fastiva_Module * pModule = &FASTIVA_MODULE_NAME(ModuleInfo);
	pStr = pModule->m_stringPool[stringId];
	if (pStr == NULL) {
		pStr = fastiva.getImmortalString_$$(pModule->m_aConstString[stringId]);
		pModule->m_stringPool[stringId] = pStr;
	}
	return pStr;
}
#endif

#if (FASTIVA_BUILD_TARGET == FASTIVA_BUILD_TARGET_EXTERNAL_COMPONENT) 
#include <jni.h>

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* fastiva_init_args) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        //ALOGE("JavaVM::GetEnv() failed");
        return JNI_ERR;
    }
	fastiva_InitArgs* args = (fastiva_InitArgs*)fastiva_init_args;
	fastiva_RuntimeProxy::g_instance = *args->m_pProxy;//((JavaVMExt*)vm)->fastiva_runtime;

	FASTIVA_MODULE_NAME(initModuleInfo)();

	//strncpy(mi->m_szFileName, args->fileName, sizeof(mi->m_szFileName)-1);
	//mi->m_szFileName[sizeof(mi->m_szFileName)-1] = 0;

	fastiva.loadExternalModule(&FASTIVA_MODULE_NAME(ModuleInfo), args->m_pLoader);

	return JNI_VERSION_1_6;
}

int main_obsolete(int argc, const char** argv) {
	return 0;
}


#endif