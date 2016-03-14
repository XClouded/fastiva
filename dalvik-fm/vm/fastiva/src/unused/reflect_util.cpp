#include <precompiled_libcore.h>

#include "kernel/kernel.h"

#include <00_java_lang_reflect.inl>
#include <java/lang/Class.inl>
#include <java/lang/Object.inl>
#include <java/lang/Boolean.inl>
#include <java/lang/Byte.inl>
#include <java/lang/Character.inl>
#include <java/lang/Short.inl>
#include <java/lang/Integer.inl>
#include <java/lang/Long.inl>
#include <java/lang/Float.inl>
#include <java/lang/Double.inl>
#include <java/lang/reflect/Field.h>
#include <java/lang/NoSuchMethodException.inl>
#include <java/util/Vector.inl>
#include <java/util/Enumeration.inl>
#include <fastiva/JppException.h>

#include "kernel/reflect_util.h"
//int fm::getPrimitiveType(java_lang_Object_p pValue);
//jlonglong fm::getPrimitiveValue(int field_type, java_lang_Object_p pValue);

#include <string.h>

const fastiva_Field * Reflect_Util::getFieldInfo(java_lang_reflect_Field_p pRefField) {
	// java_lang_Class_p pClass = pRefField->m_clazz;
	// const fastiva_InstanceContext* pIC = pClass->m_pContext->toInstanceContext();
	// java_lang_reflect_Field의 slot을 실질적인 Field의 fastiva_Field를 나타냄.
	// java_lang_Class->getField0에서 만들어 놓음.
	KASSERT(pRefField->m_slot != ADDR_ZERO);
	return (fastiva_Field *)pRefField->m_slot;
}

/*
*/

inline int Reflect_Util::typesToSig(java_lang_Class_ap types, char *pSigBuf) {
	int len = types->length();
	
	for (int i=0; i<len; i++) {
		java_lang_Class_p pClass = types->get$(i);
		int dimension = fm::getArrayDimension(pClass);
		jbool isPrimitive = pClass->isPrimitive();
		const fastiva_ClassContext* pContext = pClass->m_pContext$;

		if (dimension > 0) {
			for (int i = dimension; i -- > 0; ) {
				strcat(pSigBuf, "[");
			}
		}
		else if (dimension == 0 && !isPrimitive) {
			strcat(pSigBuf, "L");
		}
		const char *pClassName = pContext->m_pBaseName;
		const char *pPackageName = NULL;
		//PrimitiveType은 package가 없음.
		if (pContext->m_pPackage != NULL) {
			pPackageName = pContext->m_pPackage->m_pszName;
			if (pPackageName[0] != 0) {
				strcat(pSigBuf, pPackageName);
				strcat(pSigBuf, "/");
			}
		}
		strcat(pSigBuf, pClassName);
		if (dimension == 0 && !isPrimitive) {
			strcat(pSigBuf, ";");
		}
	}
	return strlen(pSigBuf);
}

static void __fillParameters(java_lang_Class_ap aType, java_lang_Object_ap aParam, jlonglong *pValue) {
	int cntType = aType->length();
	int cntParam = aParam->length();
	KASSERT (cntParam > 0);

	if (cntType != cntParam) {
		fastiva_throwIllegalArgumentException();
	}
	int idxParam = 0;
	
	for (int i=0; i<cntParam; i++) {
		java_lang_Class_p type = aType->get$(i);
		const fastiva_ClassContext* pContext = type->m_pContext$;
		if (!type->isPrimitive()) {
			java_lang_Object_p pObj = aParam->get$(i);
			if (!type->isAssignableFrom(pObj->getClass())) {
				fastiva_throwIllegalArgumentException();
			}
			*pValue++ = (jint)(void *)pObj;
			idxParam++;
		}
		else {
			int field_type = fastiva_Field::getPrimitiveType(type);
			*pValue ++ = fastiva_Field::getPrimitiveValue(field_type, aParam->get$(i));
			idxParam ++;
		}
		
	}
}


jlonglong Reflect_Util::invoke(
	const fastiva_Method* pMethod,
	java_lang_Object_p pSelf, 
	java_lang_Class_p pClass,
	java_lang_Class_ap aParamType, 
	java_lang_Object_ap aParam
) {
	TRY$ {	
		jlonglong aArg[26]; //Fastiva에서는 최대 26개의 Parameter만 지원한다.
		int cntParam = aParamType->length();
		if (cntParam > 0) {
			__fillParameters(aParamType, aParam, aArg);
		}
		else {
			KASSERT(aParam == FASTIVA_NULL || aParam->length() == 0);
		}
		return pMethod->invoke(pSelf, pClass, aArg);
	}
	CATCH_ANY$ {
		THROW_EX_NEW$(java_lang_reflect_InvocationTargetException, (catched_ex$));
	}
}


/*
java_lang_Class_p Reflect_Util::getReturnType(const char *pSig) {
	char *pdes = strchr(pSig, ')');
	if (pdes == NULL) {
		return (java_lang_Class_p)FASTIVA_NULL;
	}
	++pdes;
	int len = strlen(pdes);
	return fm::findClass(FASTIVA_SIGNATURE_LOADER, pdes, len);
}

java_lang_Class_ap Reflect_Util::getParamTypes(const char *pSigBuf) {
	int count = 0;
	java_lang_Class_p types[26];

	while (pSigBuf) {
		switch (*pSigBuf) {
		case 'V':
		case 'B':
		case 'C':
		case 'S':
		case 'I':
		case 'Z':
		case 'F':
		case 'D':
		case 'J':
		{
			types[count] = (java_lang_Class_p)-1;//java_lang_Class::allocate$();
			types[count++] = (fm::findClass(FASTIVA_SIGNATURE_LOADER, pSigBuf, 1));
			pSigBuf++;
			break;
		}
		
		case 'L':
		{
			const char *pStart = pSigBuf;
			pSigBuf = strchr(pSigBuf, ';');
			const char *pEnd = pSigBuf;
			types[count] = (java_lang_Class_p)-1;//java_lang_Class::allocate$();java_lang_Class::allocate$();
			types[count++] = (fm::findClass(FASTIVA_SIGNATURE_LOADER, pStart, (pEnd - pStart+1)));
			pSigBuf++;
			break;
		}

		case '[':
		{
			const char *pStart = pSigBuf;
			const char *pEnd = ADDR_ZERO;
			const fastiva_ClassContext *pContext = ADDR_ZERO;
			pSigBuf++;
SIG_ARRAY:
			switch (*pSigBuf) {
			case 'L':
				pSigBuf = strchr(pSigBuf, ';');
				pEnd = pSigBuf;
				types[count] = (java_lang_Class_p)-1;//java_lang_Class::allocate$();java_lang_Class::allocate$();
				types[count++] = (fm::findClass(FASTIVA_SIGNATURE_LOADER, pStart, (pEnd - pStart + 1)));
				pSigBuf++;
				break;
			case '[':
				pSigBuf++;
				goto SIG_ARRAY;

			default:
				types[count] = (java_lang_Class_p)-1;//java_lang_Class::allocate$();java_lang_Class::allocate$();
				types[count++] = (fm::findClass(FASTIVA_SIGNATURE_LOADER, pStart, (pEnd - pStart)));
				pSigBuf++;
				break;

			}
			break;
		}
		
		case '(':
			pSigBuf++;
			break;
		case ')':
			goto PARSE_COMPLETED;
		}
	}
PARSE_COMPLETED:

	java_lang_Class_ap aTypes = java_lang_Class_A::create(count);
	for (int i=0; i<count; i++) {
		aTypes->set$(i, types[i]);
	}
	return aTypes;
}
*/

Byte_ap fm::charToByteArray(const unicod* pcode, int len);

void* Reflect_Util::getMethod(
	java_lang_Class_p pClass,
	java_lang_String_p  pName,
	java_lang_Class_ap  parameterTypes,
	jint  declaredOnly,
	jbool isConstructor
) {

	/*
	CDC에서는 java_lang_String::getBytes()를 사용하면 문제가 발생한다.
	String의 getBytes()를 호출하면 Reflection을 이용해서 sun_io_CharToByteISO8859_1 Class 같은 
	String의 Default Converter의 constructor를 호출한다.
	그러면 getMethod를 호출하는데 여기에서 다시 getBytes()를 호출하면 다시 getMethod를 호출해서
	Stack Overflow를 발생한다.
	참고로 CDC에서는 Class.newInstance를 호출하면 이것은 reflection을 이용한다.
	*/
	Unicod_A::Buffer unicodRegion(pName->m_value, pName->m_offset);
	Byte_ap aMethodNameBuff = fm::charToByteArray((const unicod *)unicodRegion, pName->length());

	Byte_A::Buffer byte_buf(aMethodNameBuff);
	const char *pMethodName = (const char *)(jbyte*)byte_buf;

	const fastiva_ClassContext* pContext;
	if (fm::getArrayDimension(pClass) == 0) {
		pContext = pClass->m_pContext$;
	}
	else if (isConstructor) {
		return ADDR_ZERO;
	}
	else {
		pContext = //(fastiva_ClassContext*)fm::aArrayContext$;// 
					FASTIVA_RAW_CLASS_CONTEXT_PTR(java_lang_Object);
	}

	char sig[256];
	memset(sig, 0, sizeof(sig));
	sig[0] = '(';
	int sigSize;
	if (parameterTypes != FASTIVA_NULL) {
		sigSize = typesToSig(parameterTypes, &sig[1]);
	}
	else {
		sigSize = 0;
	}
	sig[sigSize+1] = ')';
	sigSize += 2;
	
	
	const fastiva_Method* pFoundMethod;
	JNI_FindMethod jfm;
	jfm.init(pMethodName, sig);

	if (isConstructor) {
		//DECLARED일 때는 FINAL$에 해당하는 것을 
		//PUBLIC일때는 PUBLIC인 것만을 찾는다.
		jfm.m_accessFlags = (declaredOnly) ?
					ACC_FINAL$ : ACC_PUBLIC$;
		pFoundMethod = pContext->getDeclaredMethod(&jfm);
	}
	else if (declaredOnly) {
		jfm.m_accessFlags = ACC_STATIC$;
		pFoundMethod = pContext->getDeclaredMethod(&jfm); // ound ADDR_ZERO;

		if (pFoundMethod == ADDR_ZERO) {
			jfm.m_accessFlags = ACC_MEMBER$; && ACC_MEMBER$ IS OBSOLETE &&
			pFoundMethod = pContext->getMethod(&jfm);

			if (pFoundMethod != ADDR_ZERO 
			&&  (pFoundMethod->m_accessFlags & (ACC_STATIC$ | ACC_FINAL$)) == 0) {
				void *pThisMethod  = pFoundMethod->getMethodAddr(ADDR_ZERO, pClass);
				void *pSuperMethod = pFoundMethod->getMethodAddr(ADDR_ZERO, pClass->getSuperclass());
				if (pThisMethod == pSuperMethod) {
					pFoundMethod = ADDR_ZERO;
				}
			}
		}
	}
	else {
		//상위는 PROTECTED이고 하위는 PUBLIC으로 OVERRIDDEN된 METHOD를 찾기 위해서
		//임시적으로 아래와 같이 한다.
		jfm.m_accessFlags = ACC_PUBLIC$ | ACC_PROTECTED$;
		pFoundMethod = pContext->getMethod(&jfm); // ound ADDR_ZERO;
	}
	
	if (pFoundMethod == ADDR_ZERO) {
		return ADDR_ZERO;
	}
	
	return createReflectedMethod(pClass, pFoundMethod);
}

java_lang_reflect_Constructor_ap Reflect_Util::getConstructors(java_lang_Class_p pClass,
									jint filter) {

	jint cntMethods = 0;
	java_lang_reflect_Constructor_p aConstBuf[2048];
	
	if (!pClass->isPrimitive() 
	&&  !pClass->isInterface() 
	&& 	!pClass->isArray()) {
		const fastiva_JniInfo* pJNI = pClass->m_pContext$->m_pJNI;
		const fastiva_Method* pMethod = pJNI->m_aMethod;

		for (int i=pJNI->m_cntMethod; i-- > 0; pMethod++) {
			if (pMethod->m_name != JPP_REF_NAME_ID::init$$
			||  (pMethod->m_accessFlags & filter) == 0 ) {
				continue;
			}
			
			aConstBuf[cntMethods++] = createConstructor(pClass, pMethod);
		}
	}
	java_lang_reflect_Constructor_ap aConstructors = 
		java_lang_reflect_Constructor_A::create$(cntMethods);

	for (int idx=0; idx<cntMethods; idx++) {
		aConstructors->set$(idx, aConstBuf[idx]);
	}
	return aConstructors;
}

java_lang_reflect_Constructor_p Reflect_Util::createConstructor(
	java_lang_Class_p pClass, 
	const fastiva_Method* pMethodInfo,
	java_lang_Class_ap aNewParamTypes,
	java_lang_Class_ap aExceptionTypes
) {
	KASSERT(pMethodInfo->m_name == JPP_REF_NAME_ID::init$$);
	if (aNewParamTypes == (java_lang_Class_ap)ADDR_ZERO) {
		aNewParamTypes = pMethodInfo->getParamTypes();
	}
	if (aExceptionTypes == (java_lang_Class_ap)ADDR_ZERO) {
		aExceptionTypes = pMethodInfo->getExceptionTypes();
	}
	java_lang_reflect_Constructor_p pConstructor = java_lang_reflect_Constructor_T$::allocate$();
	pConstructor->set__declaringClass(pClass);
	pConstructor->set__slot((int)pMethodInfo);
	pConstructor->set__parameterTypes(aNewParamTypes);
	pConstructor->set__exceptionTypes(aExceptionTypes);
#if 0 //ndef _ANDROID
	pConstructor->set__modifiers(pMethodInfo->m_accessFlags & ACC_CONSTRUCTOR_FLAGS$);
#endif
	return pConstructor;
}

java_lang_reflect_Method_p Reflect_Util::createMethod(
	java_lang_Class_p pClass, 
	const fastiva_Method* pMethodInfo,
	java_lang_Class_ap aNewParamTypes,
	java_lang_Class_ap aExceptionTypes
) {
	KASSERT(pMethodInfo->m_name != JPP_REF_NAME_ID::init$$);
	if (aNewParamTypes == (java_lang_Class_ap)ADDR_ZERO) {
		aNewParamTypes = pMethodInfo->getParamTypes();
	}
	if (aExceptionTypes == (java_lang_Class_ap)ADDR_ZERO) {
		aExceptionTypes = pMethodInfo->getExceptionTypes();
	}
	java_lang_reflect_Method_p pMethod = java_lang_reflect_Method_T$::allocate$();
	pMethod->set__declaringClass(pClass);
	pMethod->set__slot((int)pMethodInfo);
	if (pMethodInfo->m_name == 0x2247) {
		int a = 3;
	}
	pMethod->set__name(fastiva.createAsciiString(JNI_FindMethod::getMethodName(pMethodInfo->m_name)));
	pMethod->set__returnType(pMethodInfo->getReturnType());
	pMethod->set__parameterTypes(aNewParamTypes);
	pMethod->set__exceptionTypes(aExceptionTypes);
#if 0 //ndef _ANDROID
	pMethod->set__modifiers(pMethodInfo->m_accessFlags & ACC_METHOD_TYPE$);
#endif
	return pMethod;
}

java_lang_Object_p Reflect_Util::createReflectedMethod(
	java_lang_Class_p pClass, 
	const fastiva_Method* pMethodInfo
) {
	java_lang_Class_ap aParamTypes = pMethodInfo->getParamTypes();
	if (pMethodInfo->m_name == JPP_REF_NAME_ID::init$$) {
		return createConstructor(pClass, pMethodInfo, aParamTypes);
	}
	else {
		return createMethod(pClass, pMethodInfo, aParamTypes);
	}
}

java_lang_reflect_Method_ap Reflect_Util::getDeclaredMethods(
	java_lang_Class_p pSelfClass, 
	jint  filter
) {

	int cntMethods = 0;
	java_lang_reflect_Method_p aMethodBuf[2048];
	java_lang_Class_p pSuperClass;

	if (pSelfClass->isPrimitive()) {
		goto done;
	}
	if (pSelfClass->isArray()) {
		goto done;
	}

	pSuperClass = pSelfClass->getSuperclass();
	const fastiva_ClassContext* pSelfContext;
	KASSERT(fm::getArrayDimension(pSelfClass) == 0);
	pSelfContext = pSelfClass->m_pContext$;
	{
		const fastiva_JniInfo* pJNI = pSelfContext->m_pJNI;
		if (pJNI != ADDR_ZERO) {
			const fastiva_Method* pMethod = pJNI->m_aMethod;
			for (int i = pJNI->m_cntMethod; i-- > 0; pMethod++) {
				if ((pMethod->m_accessFlags & filter) != 0 && pMethod->m_name != JPP_REF_NAME_ID::init$$) {
					// add method.
					aMethodBuf[cntMethods++] = createMethod(pSelfClass, pMethod);
				}
			}
		}
	}
	
	// Spec(java.lang.Class.getMethods API Help)에서는 Array이면 length 0으로 하는 java_lang_reflect_Method_A를 
	// 리턴하게 되어 있는데 소스상에서는 그렇게 되어 있지 않다.
done:	
	java_lang_reflect_Method_ap aMethods = java_lang_reflect_Method_A::create$(cntMethods);

	for (int i=0; i<cntMethods; i++) {
		 aMethods->set$(i, aMethodBuf[i]);
	}
	return aMethods;
}	

java_lang_reflect_Field_ap Reflect_Util::getFields(
	java_lang_Class_p pClass, 
	jint  filter
) {

	// 먼저 Count를 계산한 후에 Dynamic하게 메모리를 잡을 수 있으나
	// 일단 그냥 아래와 같이 한다.
	int cntField = 0;
	java_lang_reflect_Field_p aRefField[2048];
	const fastiva_ClassContext *pContext;
	java_lang_Class_p thisClass;

	if (pClass->isPrimitive() 
	||  pClass->isArray()) {
		goto done;
	}
	
	pContext = pClass->m_pContext$;
	thisClass = pClass;

	do {
		const fastiva_JniInfo* pJNI = pContext->m_pJNI;
		if (pJNI != ADDR_ZERO) {
			const fastiva_Field* pField = pJNI->m_aField;
			for (int i = pJNI->m_cntField; i-- > 0; pField++) {
				if ((filter & pField->m_accessFlags) != 0) {
					aRefField[cntField] = java_lang_reflect_Field_T$::allocate$();

					aRefField[cntField]->set__declaringClass(thisClass);
					aRefField[cntField]->set__slot((int)pField);
					aRefField[cntField]->set__name(fastiva.createAsciiString(JNI_FindField::getFieldName(pField->m_name)));
					aRefField[cntField]->set__type(JNI_FindClass::loadRawClass(pField->m_type));
#if 0 //ndef _ANDROID
					aRefField[cntField]->set__modifiers(pField->m_accessFlags & ACC_FIELD_TYPE_FLAGS$);
#endif
					cntField++;
				}
			}
		}
		if (pContext->isInterface()) {
			break;
		}
		pContext = pContext->getSuperClassContext();
		thisClass = pClass->getSuperclass();
	} while ((filter & ACC_INHERITED$) != 0 && pContext != ADDR_ZERO);

done:
	java_lang_reflect_Field_ap aFields = java_lang_reflect_Field_A::create$(cntField);
	
	for (int i=0; i<cntField; i++) {
		aFields->set$(i, aRefField[i]);
	}
	return aFields;
}

#if 0 // 사용안되는 코드
jbool isBelongSuper(java_lang_Class_p pClass, const fastiva_InterfaceContext *pInterContext) {
	java_lang_Class_p pSuperClass = pClass->getSuperclass();
	if (pSuperClass == ADDR_ZERO ||
		java_lang_Object_C$::getRawStatic$() == pSuperClass) {
		return false;
	}
	const fastiva_ImplementInfo* pImpl = pContext->m_aImplemented;
	const fastiva_InterfaceContext* pIFC;
	for (; (pFC = pImpl->m_pInterfaceContext) != NULL; pImpl++) {
		if (pInterContext == pIFC) {
			return true;
		}
	}
	return false;
}
#endif


java_lang_Class_ap Reflect_Util::getOwnInterfaces(java_lang_Class_p pClass) {

	java_lang_Class_ap aClasses;

	if (pClass->isPrimitive()) {
		aClasses = java_lang_Class_A::create$(0);
		return aClasses;
	}
	else if (pClass->isArray()) {
		aClasses = java_lang_Class_A::create$(2);
		aClasses->set$(0, java_lang_Cloneable_C$::importClass$());
		aClasses->set$(1, java_io_Serializable_C$::importClass$());
		return aClasses;
	}

	fastiva_ClassContext_cp pContext = pClass->m_pContext$;
	int cntImpl = pContext->getOwnInterfaceCount();
	aClasses = java_lang_Class_A::create$(cntImpl);
	
	// ObjectStreamClass의 serial UID를 생성하기 위해, implements 선언된 interface의 정확한 목록이 필요하다.
	if (!pContext->isInterface()) {
		const fastiva_ImplementInfo* pII = pContext->toInstanceContext()->m_aImplemented;
		for (int idx = 0; idx < cntImpl; idx ++, pII++) {
			const fastiva_InterfaceContext* pIFC = pII->m_pInterfaceContext;
			if (pIFC->isRawContext()) {
				*(const fastiva_InterfaceContext**)&pII->m_pInterfaceContext = pIFC = fm::validateContext(pContext)->toInterfaceContext();
			}
			aClasses->set$(idx, pIFC->m_pClass);
		}
	}
	else {
		const fastiva_InterfaceContext** ppIFC = pContext->toInterfaceContext()->m_ppIFC;
		const fastiva_InterfaceContext* pIFC;
		for (int idx = 0; idx < cntImpl; ppIFC++) {
			aClasses->set$(idx, ppIFC[0]->m_pClass);
		}
	}

	/*
	java_util_Enumeration_p enumer = table->elements();
	int idx = 0;
	while (enumer->hasMoreElements()) {
		aClasses->set$(idx, INSTANCE_PTR$(java_lang_Class, enumer->nextElement()));
		idx++;
	}
	*/
	return aClasses;
}