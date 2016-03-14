#ifndef __FASTIVA_NATIVE_RUNTIME_H__
#define __FASTIVA_NATIVE_RUNTIME_H__




#if 0
FASTIVA_SUPPORTS_REFLECTION: {
	java/lang/reflect/;
	java/lang/Class {
		getDeclaredAnnotations
		getDeclaredClasses
		getDeclaredConstructors
		getDeclaredFields
		getDeclaredMethods
		getDeclaringClass
		getEnclosingClass
		getEnclosingConstructor
		getEnclosingMethod
		getInnerClassName
		getInterfaces
		getModifiers
		getSignatureAnnotation
	};
}

FASTIVA_SUPPORTS_BYTECODE_INTERPRETER: {
	java/lang/Class {
		defineClass
	};
}
#endif


#endif // __FASTIVA_NATIVE_RUNTIME_H__
