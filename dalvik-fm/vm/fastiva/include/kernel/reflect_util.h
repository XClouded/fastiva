#ifndef REFLECT_UTIL
#define REFLECT_UTIL

#include <Fastiva.h>
#include <kernel/Kernel.h>


class Reflect_Util {
public:
	static const fastiva_Field* getFieldInfo(
		java_lang_reflect_Field_p pRefField
	);
	
	static java_lang_Class_p getReturnType(
		const char *pSig
	);
	
	static java_lang_Class_ap getParamTypes(
		const char *pSigBuf
	);
	
	static int typesToSig(
		java_lang_Class_ap types, 
		char *pSigBuf
	);

	static int typeToSig(
		java_lang_Class_p types, 
		char *pSigBuf
	);

	static jlonglong invoke(
		const fastiva_Method* pMethod,
		java_lang_Object_p selfObj, 
		java_lang_Class_p pClass,
		java_lang_Class_ap aParamType, 
		java_lang_Object_ap aParam
	);

	static java_lang_Object_p createReflectedMethod(
		java_lang_Class_p pClass, 
		const fastiva_Method* pMethodInfo
	);

	static java_lang_reflect_Constructor_p createConstructor(
		java_lang_Class_p pClass, 
		const fastiva_Method* pMethodInfo,
		java_lang_Class_ap aNewParamTypes = (java_lang_Class_ap)ADDR_ZERO,
		java_lang_Class_ap aExceptionTypes = (java_lang_Class_ap)ADDR_ZERO
	);

	static java_lang_reflect_Method_p createMethod(
		java_lang_Class_p pClass, 
		const fastiva_Method* pMethodInfo,
		java_lang_Class_ap aNewParamTypes = (java_lang_Class_ap)ADDR_ZERO,
		java_lang_Class_ap aExceptionTypes = (java_lang_Class_ap)ADDR_ZERO
	);


	static void* getMethod(
		java_lang_Class_p pClass,
		java_lang_String_p  pName,
		java_lang_Class_ap  parameterTypes,
		jint  which,
		jbool isConstructor
	);
	
	static java_lang_reflect_Method_ap getDeclaredMethods(
		java_lang_Class_p pClass, 
		jint  which
	);

	static java_lang_reflect_Field_ap getFields(
		java_lang_Class_p pClass, 
		jint  which
	);

	static java_lang_Class_ap getOwnInterfaces(
		java_lang_Class_p pClass
	);
							
	static java_lang_reflect_Constructor_ap getConstructors(
		java_lang_Class_p pClass,
		jint which
	);

	//static void *getFieldAddr(java_lang_reflect_Field_p pField, java_lang_Object_p pObj);
	//static java_lang_Object_p wrapPrimitive(java_lang_Class_p pClass, void *pFieldAddr);
	//static char getPrimitiveSig(java_lang_Class_p pClass);
	//static int fillPrimParam(java_lang_Class_p pClass, java_lang_Object_p pObj, jvalue* pValue);
	//static int fillParameters(java_lang_Class_ap types, java_lang_Object_ap aParams, jvalue *aValue);

//	static const fastiva_InstanceContext* getInstanceContextOf(java_lang_Class_p pClass) {
//		return fm::getInstanceContext(pClass);
//	}

//	static const fastiva_ClassContext *getClassContextOf(java_lang_Class_p pClass) {
//		return fm::getClassContext(pClass);
//		//return pClass->m_pContext0;
//	}

		//static java_lang_Class_ap getParamTypes(const char *pSigBuf);

	//static java_lang_Class_p sigToType(const char *pSigBuf);
	//static java_lang_Class_p getReturnType(const char *pSig);

	//static jbool isObjectSig(const char *pSig);

};

#endif
