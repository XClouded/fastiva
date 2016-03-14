
	#include <fastiva/lang/Generic.hpp>

//	FASTIVA_METHOD_STATIC(public, 1, ABSTRACT$(ACC_FINAL$),
//		VAL(void*), __asInstance__, (RawContext_p))

	FASTIVA_METHOD_STATIC(public, 0, ABSTRACT$(ACC_FINAL$),
		VAL(ulonglong), __identifier__, ())

	FASTIVA_METHOD_STATIC(public, 0, ABSTRACT$(ACC_FINAL$),
		VAL(ulonglong), __toPrimitive__, ())

	FASTIVA_METHOD_STATIC(public, 0, ABSTRACT$(ACC_FINAL$),
		VAL(ulonglong), __toNumber__, ())


	FASTIVA_METHOD_STATIC(public, 2, ABSTRACT$(0),
		void, __modifyAttr__, (
			jint,
			JsBinOperation_p
		))

	FASTIVA_METHOD_STATIC(public, 2, ABSTRACT$(0),
		void, __modifyItem__, (
			jint,
			JsBinOperation_p
		))

