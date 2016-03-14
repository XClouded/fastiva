&&&& OBSOELTE &&&&
fastiva_ModuleInfo FASTIVA_CURRENT_MODULE_INFO;
fastiva_ModuleInfo::fastiva_ModuleInfo() {
	class abstract_Object$ {
		virtual void abstract_fn() = 0;
	};
	jbyte c2[sizeof(java_lang_Object)];
	((abstract_Object$*)c2)->abstract_Object$::abstract_Object$();
	// abstract_method에 대한 주소값을 취한다.

	this->m_aPackageSlot = fastiva_aPackageSlot$;
	this->m_cntPackage = FASTIVA_ARRAY_ITEM_COUNT(fastiva_aPackageSlot$);
	this->m_pRuntime = &fastiva;
	this->m_imageOffset = (*(int**)&c2)[0];
#ifdef JPP_JNI_EXPORT_LEVEL > 0
	this->m_aRefName = g_aAppRefName$;
	this->m_aArgList = g_aAppArguments$;
	this->m_aContext = g_aAppArguments$;
#endif
}

