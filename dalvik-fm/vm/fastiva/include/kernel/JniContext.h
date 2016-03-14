#ifndef __FASTIVA_JNI_CONTEXT_H__
#define __FASTIVA_JNI_CONTEXT_H__

#if (JPP_JNI_EXPORT_LEVEL > 0)

#include <fastiva/ClassContext.h>
#include <kernel/JniConstant.h>
#include <stdarg.h>


struct JavaCodeAttr;
class ClassFileParser;

// JniInfo -> ClassContext.h 로 이동.

struct fastiva_Field : public fastiva_FieldInfo { // exported only


	bool isInternal() const {
		return (m_accessFlags & (ACC_PUBLIC$ | ACC_PROTECTED$ | ACC_PRIVATE$)) == 0;
	}

	int getValueSize() const {
		if (!isPrimitive()) {
			return sizeof(fastiva_Instance_p);
		}
		return 1 << (m_type & 3);
	}
	
	jbool isPrimitive() const {
		return m_type <= fastiva_Primitives::jvoid;
	}
	
	jbool isStatic() const {
		return (m_accessFlags & ACC_STATIC$) != 0;
	}
	
	int getDimension() const {
		return (ushort)m_accessFlags >> 13;
	}
	
	jlonglong FOX_FASTCALL(getValue)(
		java_lang_Object_p pOwner
	) const;

	void FOX_FASTCALL(setValue)(
		java_lang_Object_p pOwner, 
		jlonglong* pValue
	) const;

	java_lang_Object_p FOX_FASTCALL(getReflectionValue)(
		java_lang_Object_p pOwner,
		java_lang_Class_p pOwnerClass
	) const;

	void FOX_FASTCALL(setReflectionValue)(
		java_lang_Object_p pOwner, 
		java_lang_Class_p pOwnerClass,
		java_lang_Object_p pValue
	) const;

//	java_lang_Class_p FOX_FASTCALL(getClass)() const;

	//const fastiva_ClassContext* FOX_FASTCALL(getContext)() const;
	
//	setField를 위하여 m_pContext 필요.(VM)
//  VM 연동을 위하여 m_accessFlags 반드시 필요. (interface 여부도 표시).

	jlonglong FOX_FASTCALL(getPrimitiveValue)(
		java_lang_Object_p pOwner, 
		java_lang_Class_p pOwnerClass,
		const fastiva_ClassMoniker* pMoniker
	) const;

	void FOX_FASTCALL(setPrimitiveValue)(
		java_lang_Object_p pOwner,
		java_lang_Class_p pOwnerClass,
		const fastiva_ClassMoniker* pMoniker,
		jlonglong value
	) const;

	static jlonglong FOX_FASTCALL(getPrimitiveValue)(
		jint field_type, 
		java_lang_Object_p pReflectValue
	);

	static jint FOX_FASTCALL(getPrimitiveType)(
		java_lang_Class_p pReflectType
	);

	static java_lang_Object_p FOX_FASTCALL(createReflectionValue)(
		int field_type,
		jlonglong* pSlot
	);

	void* fastiva_Field::getSlot_$$(
		java_lang_Object_p pOwner
	) const;
};






#include <java/lang/Class.ptr>

struct fastiva_Method : public fastiva_MethodInfo { // exported only

	void* getUnlinkedAddress(
		const fastiva_ClassContext* pContext
	) const;
	
	void* getMethodAddr(
		java_lang_Object_p pObj, 
		java_lang_Class_p pClass
	) const;

	jlonglong invoke(
		java_lang_Object_p pObj, 
		java_lang_Class_p pType, 
		va_list args) const;

	jlonglong invoke(
		java_lang_Object_p pObj, 
		java_lang_Class_p pType, 
		jlonglong* args) const;

	//JsValueEx invoke(
	//	java_lang_Object_p pObj, 
	//	//java_lang_Class_p pClass, 
	//	JsVariant* aArg,
	//	int cntArg) const;

	/* ZEE
	java_lang_Class_p getDeclaringClass() const {
		KASSERT((this->m_accessFlags & ACC_NATIVE$) == 0);
		return ((fastiva_ClassContext*)m_pDeclared)->m_pClass;
	}
	*/

	int getParamSize() const {
		return this->m_argSize;//(char)(m_paramFlags);
	}

	const fastiva_ClassContext* getDeclaredContext() const {
		return ((fastiva_ClassContext*)this->m_internal);
	}

	const fastiva_ClassContext* getInterfaceContext() const {
		KASSERT((this->m_accessFlags & ACC_NATIVE$) == 0);
		KASSERT((this->getDeclaredContext()->m_accessFlags & ACC_INTERFACE$) != 0);
		return ((fastiva_ClassContext*)this->m_internal)->toInterfaceContext();
	}

	const fastiva_InstanceContext* getInstanceContext() const {
		KASSERT((this->m_accessFlags & ACC_NATIVE$) == 0);
		KASSERT(!isBytecodes());
		return ((fastiva_ClassContext*)this->m_internal)->toInstanceContext();
	}


	const JavaCodeAttr* getCodeAttr() const {
		KASSERT((this->m_accessFlags & ACC_NATIVE$) == 0);
		KASSERT(isBytecodes());
		return (JavaCodeAttr*)this->m_code;
	}

	bool isBytecodes() const {
		return (this->m_accessFlags & ACC_BYTECODES$) != 0;
	}

	bool isInternal() const {
		return (m_accessFlags & (ACC_PUBLIC$ | ACC_PROTECTED$ | ACC_PRIVATE$)) == 0;
	}
	// last item is a return value_type, 
	java_lang_Class_ap getParamTypes() const;
	
	java_lang_Class_p getReturnType() const;

	java_lang_Class_ap getExceptionTypes() const;
};


#define FASTIVA_PACKAGE_SLOT_PTR(package)	(g_aBootstrapPackage + jpp_component_PackageNameID::package##$) && OBSOLETE &&
#define FASTIVA_PACKAGE_SLOT_IFC(package)	(g_aBootstrapPackage + jpp_component_PackageNameID::package##$) && OBSOLETE &&

#endif

#endif // __FASTIVA_JNI_CONTEXT_H__
