#ifndef __FASTIVA_INLINE_INL__
#define __FASTIVA_INLINE_INL__

struct fastiva_Package : public fastiva_PackageInfo {

	const fastiva_ClassContext* findContextSlot(
		uint hashCode, 
		const char* pszBaseName,
		int* pInsertIndex = 0
	) const;
	
	bool loadModule(java_lang_ClassLoader_p pLoader = ADDR_ZERO);

	bool isMuttable() const;

private:
	void loadModule0(java_lang_ClassLoader_p pLoader);
};


#ifndef ANDROID


struct fastiva_ClassContext : public fastiva_ClassInfo {

	enum {
		NOT_LOADED = -1,
		IN_LOADING = -2,
		LOAD_FAIL = 0x7FFF,
	};

	typedef bool (*FieldVisitor)(const fastiva_Field* pField, void* userParam);

	typedef bool (*MethodVisitor)(const fastiva_Method* pMethod, void* userParam);

	jint getOwnInterfaceCount() const {
		// 최대한 자바 Reflection 과 동일한 결과가 나오도록 하였다.
		return this->m_cntOwnIFC;
	}

	const fastiva_ClassContext* getSuperClassContext() const;

	const fastiva_InstanceContext* getSuperInstanceContext() const;
	
	int getFieldCount() const;

	int getMethodCount() const;

	jint getInheritDepth() const {
		return m_inheritDepth;
	}

	jbool hasStaticInitializer() const {
		return (m_accessFlags & ACC_STATIC$) != 0;
	}

	int getVirtualMethodCount() const {
		return this->m_cntVirtualMethod;
	}

	const char* getPackageName() const {
		return m_pPackage->m_pszName;
	}

	const fastiva_Package* getPackage() const {
		return m_pPackage;
	}

	const int* getVirtualTable() const {
		return getClass()->obj.vtable$;
	}

	fastiva_Class* getClass() const {
#ifndef FASTIVA_PRELOAD_STATIC_INSTANCE
		return m_pClass0[0];
#else
		return m_pClass0;
#endif
	}

	jbool isRawContext() const {												
		return (short)this->m_accessFlags < 0; 
	}

	JNI_RawContext* toRawContext() const {
		KASSERT(isRawContext());
		return (JNI_RawContext*)(void*)this;
	}

	int markInLoading() const { 
		int s = (int)m_accessFlags; 
		if (s == NOT_LOADED) {
			*(ushort*)&this->m_accessFlags = (ushort)IN_LOADING; 
		}
		return s;
	}

	void markLoadFail() { 
		KASSERT(m_accessFlags == IN_LOADING);
		*(ushort*)&this->m_accessFlags = (short)LOAD_FAIL;
	}

	bool isLoadFailed() const {
		return this->m_accessFlags == (short)LOAD_FAIL;
	}


	jbool isPrimitive() const {												
		KASSERT(!isRawContext());											
		return this->m_pPackage == ADDR_ZERO;//(short)m_inheritDepth < 0;
	}

	// interface 여부를 판별할 경우에는 isInterface()를 사용하라.
	jbool isInstance() const {												
		KASSERT(!isRawContext());											
		KASSERT(!isPrimitive());											
		return (m_accessFlags & ACC_INTERFACE$) == 0;
	}

	jbool isInterface() const {												
		KASSERT(!isRawContext());											
		return (m_accessFlags & ACC_INTERFACE$) != 0;
	}

	const fastiva_ClassContext* toInterfaceContext() const {			
		KASSERT(!isRawContext());											
		KASSERT(!isInstance() && !isPrimitive());											
		return (const fastiva_ClassContext*)(void*)this;				
	}

	const fastiva_InstanceContext* toInstanceContext() const {				
		KASSERT(!isRawContext());											
		KASSERT(isInstance() && !isPrimitive());								
		return (const fastiva_InstanceContext*)(void*)this;					
	}																		
	
	jbool FOX_FASTCALL(isAssignableFrom)(java_lang_Class_p srcClass) const;

	jbool FOX_FASTCALL(isAssignableFrom)(const fastiva_ClassContext* pContext) const;

	jbool FOX_FASTCALL(isAssignableFrom)(const fastiva_InstanceContext* pContext) const;

	jbool FOX_FASTCALL(isAssignableFrom)(const fastiva_ClassContext* pContext) const;

	// pObj가 Array인 경우에도 dimension을 무시하고 비교한다.
	jbool FOX_FASTCALL(isInstance)(const fastiva_Instance_p pObj) const;

#if (JPP_JNI_EXPORT_LEVEL > 0)
	const fastiva_Field* FOX_FASTCALL(getDeclaredField)(
		JNI_FindField*
	) const;

	const fastiva_Field* FOX_FASTCALL(getField)(
		JNI_FindField*
	) const;

	const fastiva_Method* FOX_FASTCALL(getDeclaredMethod)(
		JNI_FindMethod*
	) const;

	const fastiva_Method* FOX_FASTCALL(getMethod)(
		JNI_FindMethod*
	) const;

#endif
};


struct fastiva_InterfaceContext : public fastiva_ClassContext {
	int ifc.itableId$;
	const fastiva_ClassContext** m_ppIFC;

	jint getInheritDepth() const {
		return m_inheritDepth;
	}
	
	jbool FOX_FASTCALL(isAssignableFrom)(
		const fastiva_ClassContext* pContext
	) const;
	
	jbool FOX_FASTCALL(isAssignableFrom)(
		const fastiva_ClassContext* pContext
	) const;
	
	const fastiva_ImplementInfo* FOX_FASTCALL(isAssignableFrom)(
		const fastiva_InstanceContext* pContext
	) const;

#if (JPP_JNI_EXPORT_LEVEL > 0)
	const fastiva_Field* FOX_FASTCALL(getField)(
		JNI_FindField*
	) const;

	const fastiva_Method* FOX_FASTCALL(getMethod)(
		JNI_FindMethod*
	) const;
#endif
	//const fastiva_Method* FOX_FASTCALL(getDeclaredMethod)(
	//	JNI_FindMethod*
	//) const;

};


struct fastiva_InstanceContext : public fastiva_ClassContext {
	
	const fastiva_ImplementInfo* m_aImplemented;

	ushort m_sizInstance;
	const  void*  m_pfnInit;

	const fastiva_InstanceContext* m_pSuperContext; // InterfaceContextInfo.m_ppIFC[0] 와 같은 위치에 있어야 한다.
	//const fastiva_IVTable** m_ppIVT;
	//const unsigned short* m_aScanOffset;
	//int   main$;
	//void (FASTIVA_STATICCALL(*main$))(java_lang_String_ap);

	jint getInheritDepth() const {
		return m_inheritDepth;
	}

	jbool FOX_FASTCALL(isAssignableFrom)(
		const fastiva_InstanceContext* pContext
	) const;	
	
	jbool FOX_FASTCALL(isAssignableFrom)(
		const fastiva_ClassContext* pContext
	) const;	

	jbool FOX_FASTCALL(isAssignableFrom)(
		const fastiva_ClassContext* pContext
	) const;	

#if (JPP_JNI_EXPORT_LEVEL > 0)
	const fastiva_Field* FOX_FASTCALL(getField)(
		JNI_FindField*
	) const;

	const fastiva_Method* FOX_FASTCALL(getMethod)(
		JNI_FindMethod*
	) const;
#endif

};


inline const fastiva_InstanceContext* fastiva_ClassContext::getSuperInstanceContext() const {
	return this->toInstanceContext()->m_pSuperContext;
}																		

inline const fastiva_ClassContext* fastiva_ClassContext::getSuperClassContext() const {
	return (fastiva_ClassContext*)(void*)this->toInstanceContext()->m_pSuperContext;
}																		


inline jbool fastiva_ClassContext::isAssignableFrom(const fastiva_ClassContext* srcContext) const {
	KASSERT ((void*)srcContext != (void*)this);

	if (this->isInstance()) {
		return this->toInstanceContext()->isAssignableFrom(srcContext);
	}
	else {
		return this->toInterfaceContext()->isAssignableFrom(srcContext);
	}
}

inline jbool fastiva_ClassContext::isAssignableFrom(const fastiva_InstanceContext* srcContext) const {
	KASSERT ((void*)srcContext != (void*)this);

	if (this->isInstance()) {
		return this->toInstanceContext()->isAssignableFrom(srcContext);
	}
	else {
		return this->toInterfaceContext()->isAssignableFrom(srcContext) != ADDR_ZERO;
	}
}

inline jbool fastiva_ClassContext::isAssignableFrom(const fastiva_ClassContext* srcContext) const {
	KASSERT ((void*)srcContext != (void*)this);

	if (this->isInstance()) {
		return this->toInstanceContext()->isAssignableFrom(srcContext);
	}
	else {
		return this->toInterfaceContext()->isAssignableFrom(srcContext);
	}
}



inline jbool fastiva_InterfaceContext::isAssignableFrom(const fastiva_ClassContext* srcContext) const {
	KASSERT ((void*)srcContext != (void*)this);

	if (srcContext->isInstance()) {
		return this->isAssignableFrom(srcContext->toInstanceContext()) != ADDR_ZERO;
	}
	else {
		return this->isAssignableFrom(srcContext->toInterfaceContext());
	}
}

inline jbool fastiva_InstanceContext::isAssignableFrom(const fastiva_ClassContext* srcContext) const {
	KASSERT ((void*)srcContext != (void*)this);

	if (srcContext->isInstance()) {
		return this->isAssignableFrom(srcContext->toInstanceContext());
	}
	else {
		return this->isAssignableFrom(srcContext->toInterfaceContext());
	}
}


inline jbool fastiva_InstanceContext::isAssignableFrom(const fastiva_ClassContext* srcContext) const {
	KASSERT ((void*)srcContext != (void*)this);
	return this == java_lang_Object::getRawContext$();
}



/*
inline jbool fastiva_Interface::equals(java_lang_Object_p pObject1) {
	return getInstance$()->equals(pObject1);
}

inline java_lang_Class_p fastiva_Interface::getClass() {
	return getInstance$()->getClass();
}

inline jint fastiva_Interface::hashCode() {
	return getInstance$()->hashCode();
}

inline void fastiva_Interface::notify() {
	getInstance$()->notify();
}

inline void fastiva_Interface::notifyAll() {
	getInstance$()->notifyAll();
}

inline java_lang_String_p fastiva_Interface::toString() {
	return getInstance$()->toString();
}

inline void fastiva_Interface::wait() {
	return getInstance$()->wait();
}

inline void fastiva_Interface::wait(jlonglong longlong1) {
	return getInstance$()->wait(longlong1);
}

inline void fastiva_Interface::wait(jlonglong longlong1, jint int3) {
	return getInstance$()->wait(longlong1, int3);
}

inline const void* fastiva_Interface::getInterfaceSlot$(const fastiva_ClassContext* pIFC, int ifcOffset) {
	void* pfn = (void*)getIVTable()[ifcOffset/4];
	return pfn;
}

*/
#endif

inline const int* fastiva_Instance::getIVTable$(int id) {
	return m_pClass$->obj.itables$[id];
}



#endif // __FASTIVA_INLINE_INL__

