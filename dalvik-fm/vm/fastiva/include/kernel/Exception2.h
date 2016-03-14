/*
#ifndef	__KERNEL_EXCEPTION_H__
#define __KERNEL_EXCEPTION_H__


#include <Fastiva.h>


struct DLL_EXPORT fm::ExceptionTrapHandler {
	JPublic fastiva_ExceptionContext* pContext;
	JPublic void init(fastiva_ExceptionContext* pContext) {
		pContext->init();
		this->pContext = pContext;
	}
	JPublic ~fm::ExceptionTrapHandler() {
		fm::removeExceptionHandler(pContext);
	}
};




#define TRY_EX(id)														\
		fastiva_ExceptionContext exContext##id;						\
		java_lang_Throwable_p catched_ex$;								\
		if ((catched_ex$ = (java_lang_Throwable_p)						\
				setjmp2(exContext##id.m_jmp_buf)) == 0) {				\
			fm::ExceptionTrapHandler exClear;						\
			exClear.init((&exContext##id));

#define CATCH_EX(type)													\
		} else if (type::isInstance(catched_ex$)) 						\


#define CATCH_ANY_EX()													\
		} else 


#endif // __KERNEL_EXCEPTION_H__
  */