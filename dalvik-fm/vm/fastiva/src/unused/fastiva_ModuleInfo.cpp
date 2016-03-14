#include <precompiled_libcore.h>

#include <fastiva/Module.h>
//fastiva_ModuleInfo FASTIVA_CURRENT_MODULE_INFO;

//extern FASTIVA_ALIGN_SEGMENT(fm::package_0$) fastiva_PackageSlot fm::Package_0$;
//extern FASTIVA_ALIGN_SEGMENT(fm::package_9$) fastiva_PackageSlot fm::Package_9$;
typedef unicod const * WCS;
WCS pWCS;
unicod const* stringPool2[1] = {};
unicod const* const* stringPoolp = stringPool2;

void fastiva_ModuleInfo::init(
		const fastiva_PackageSlot* packageSlots, 
		int cntPackage, 
		const JNI_HashEntry* pRefNames, 
		int cntRefName,
		const JNI_HashEntry* pArgLists,
		int cntArgList,
		void (*initClassIDs)(java_lang_Class_p* classes),
		unicod const* const* aConstString,
		java_lang_String_p* stringPool
) {
#ifdef _WIN32
	class abstract_Object$ {
		virtual void abstract_fn() = 0;
	};
	jbyte c2[sizeof(java_lang_Object)];
	((abstract_Object$*)c2)->abstract_Object$::abstract_Object$();
	// abstract_method에 대한 주소값을 취한다.
	this->m_imageOffset = (*(int**)&c2)[0];
#else
	this->m_imageOffset = 0;
#endif
	this->m_aPackageSlot = packageSlots;
	//this->m_aPackage = fm::Package_0$ + 1;//fm::aPackageSlot$;
	this->m_cntPackage = cntPackage;//fm::Package_9$ - this->m_aPackage;//FASTIVA_ARRAY_ITEM_COUNT(fm::aPackageSlot$);
	this->m_pRuntime = &fastiva;
	this->m_aRefName = pRefNames;
	this->m_aArgList = pArgLists;
	this->m_cntRefName = cntRefName;
	this->m_cntArgList = cntArgList;
	this->m_stringPool = stringPool;
	this->m_aConstString = aConstString;
	this->m_fnInitClassIDs = initClassIDs;
}

