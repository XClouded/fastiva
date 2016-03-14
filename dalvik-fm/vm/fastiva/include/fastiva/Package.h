#ifndef __FASTIVA_PACKAGE__H__
#define __FASTIVA_PACKAGE__H__


#define JPP_PACKAGE_INFO_PTR(PACKAGE)											\
	((const fastiva_Package*)&FASTIVA_PACKAGE_INFO_NAME(PACKAGE))

#define JPP_BEGIN_PACKAGE_INFO(PACKAGE)											\
	static const JNI_HashEntry PACKAGE##_contextSlots$[] = {

#define JPP_INIT_CONTEXT_SLOT(CLASS)											\
	{ CLASS##_G$::classHash$, (void*)FASTIVA_RAW_CLASS_CONTEXT_PTR(CLASS) },			


#define JPP_END_PACKAGE_INFO(PACKAGE, NAME, hashCode)							\
	};																			\
	const fastiva_PackageInfo FASTIVA_PACKAGE_INFO_NAME(PACKAGE) = {			\
		NAME, PACKAGE##_contextSlots$, 											\
		sizeof(PACKAGE##_contextSlots$) / sizeof(PACKAGE##_contextSlots$[0]),	\
		(fastiva_Module*)(void*)&FASTIVA_MODULE_NAME(ModuleInfo)				\
	};


extern FASTIVA_RUNTIME_EXPORT fastiva_Module FASTIVA_MODULE_NAME(ModuleInfo);

struct fastiva_PackageInfo {
	// TokenSlot 연동을 위해 반드시 이름으로 시작. (마지막 '/' 포함 안함)
	const char* m_pszName;
	const JNI_HashEntry* m_aContextSlot;
	int m_cntContext;
	fastiva_Module* m_pModule;
};


struct fastiva_PackageSlot {
	uint m_hashCode;
	const fastiva_Package* m_pPackage;
};



#endif // __FASTIVA_PACKAGE__H__
