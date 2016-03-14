#ifndef __FASTIVA_EXCEPTION__H__
#define __FASTIVA_EXCEPTION__H__

#define __FASTIVA_EXCEPTION_H__

#include <fastiva/Config.h>

struct fastiva_ExceptionContext {
public:
	int curr_try_id;

#ifndef FASTIVA_USE_CPP_EXCEPTION
	fastiva_ExceptionContext*	m_pParent;
    fastiva_Rewinder*	m_pTopRewinder;
	fox_jmp_buf					m_buf;

	fox_jmp_buf& init(int try_id) {
		curr_try_id = try_id;
		fastiva.pushExceptionHandler(this);
		return m_buf;
	}

	~fastiva_ExceptionContext() {
		fastiva.removeExceptionHandler(this);
	}
#endif
};

struct fastiva_ExceptionContextTable : public fastiva_ExceptionContext {
	fastiva_ExceptionContextTable() {
#ifdef FASTIVA_USE_CPP_EXCEPTION
		catched_ex = NULL;
#endif
	}
	java_lang_Throwable_p catched_ex;
union {
	int finally_ret_addr;
	jlonglong ret_value;
};
};


#ifndef FASTIVA_USE_CPP_EXCEPTION

	#define JPP_VOLATILE_STACK_VAR  volatile

	#define FASTIVA_TRY_EX$							// start_of_try

	#define FASTIVA_EXCEPTION_CONTEXT$										\
		fastiva_ExceptionContextTable fastiva_exContext;					\
		if ((fastiva_exContext.catched_ex = (java_lang_Throwable_p)		\
			(void*)FASTIVA_SETJMP(fastiva_exContext.init(-1))) != 0)		\
			fastiva_catch_internal_excetion$:								\
			switch (fastiva_exContext.curr_try_id) 

	#define FASTIVA_RETRY_CATCH$(parent_section_id)							\
		goto parent_section_id;

	#define FASTIVA_CATCH_EX$(ex_type, ex_handler)							\
		if (ex_type::isInstance$(fastiva_exContext.catched_ex)) goto ex_handler;  

	#define FASTIVA_CATCH_ANY$(ex_handler)									\
		goto ex_handler;

	#define FASTIVA_END_CATCH$()											\
		fastiva.rethrow(&fastiva_exContext, fastiva_exContext.catched_ex);

	#define FASTIVA_CALL_FINALLY_2$(finally_handler, ret_addr)				\
		fastiva_exContext.finally_ret_addr = ret_addr; goto finally_handler;
		


	#define THROW_EX_INTERNAL$(ex)											\
		fastiva_exContext.catched_ex = ex;									\
		goto fastiva_catch_internal_excetion$;							
 
	#define THROW_EX_EXIT$(ex)												\
		fastiva_exContext.catched_ex = ex;									\
		goto rethrow_current_ex$;							

	#define THROW_EX$(ex)													\
		fastiva.throwException(ex);

	#define THROW_EX_NEW$(CLASS, PARAM)	{									\
			CLASS##_p ex$$ = FASTIVA_ALLOC(CLASS);				\
			ex$$->init$ PARAM;												\
			THROW_EX$(ex$$);												\
	}

	#define FASTIVA_END_EXCEPTION_CONTEXT$()	\
		if (false) { \
		rethrow_current_ex$: \
			fastiva.throwException(fastiva_exContext.catched_ex); \
		}


#else

	#define JPP_VOLATILE_STACK_VAR // ignore

	#define FASTIVA_TRY_EX$	{ \
		fastiva_ExceptionContextTable fastiva_exContext; \
		fastiva_catch_internal_excetion$: \
	
	#define FASTIVA_EXCEPTION_CONTEXT$ \
		try { \
		if (fastiva_exContext.catched_ex != NULL) \
			switch (fastiva_exContext.curr_try_id) 

	#define FASTIVA_RETRY_CATCH$(parent_section_id) \
		goto parent_section_id;

	#define FASTIVA_CATCH_EX$(ex_type, ex_handler) \
		if (ex_type::isInstance$(fastiva_exContext.catched_ex)) goto ex_handler;  

	#define FASTIVA_CATCH_ANY$(ex_handler) \
		goto ex_handler;

	#define FASTIVA_END_CATCH$() \
		goto rethrow_current_ex$;


	#define THROW_EX_INTERNAL$(ex)											\
		fastiva_exContext.catched_ex = ex;									\
		goto fastiva_catch_internal_excetion$;							

	#define THROW_EX_EXIT$(ex)												\
		fastiva_exContext.catched_ex = ex;									\
		goto rethrow_current_ex$;							

	#define THROW_EX$(ex)													\
		fastiva.throwException(ex); 

	#define THROW_EX_NEW$(CLASS, PARAM)										\
		THROW_EX$(FASTIVA_NEW(CLASS)PARAM)

	#define FASTIVA_END_EXCEPTION_CONTEXT$()	\
		} catch (void* catched_ex$) { \
			fastiva_exContext.catched_ex = (java_lang_Throwable_p)catched_ex$; \
			goto fastiva_catch_internal_excetion$; \
		} \
		if (false) { \
		rethrow_current_ex$: \
			THROW_EX$(fastiva_exContext.catched_ex); \
		} } \
	//#define FASTIVA_END_OF_TRY_EX$()				}}


#endif  


#define JPP_CALL_FINALLY$(finally_handler)						\
	fastiva_exContext.catched_ex = NULL; goto finally_handler;
	
#define JPP_CALL_FINALLY_AND_THROW$(finally_handler, ex)				\
	fastiva_exContext.catched_ex = ex;									\
	goto finally_handler;

#define JPP_CALL_FINALLY_AND_RETURN$(ret_t, ret_v, finally_handler)		\
	fastiva_exContext.catched_ex = (java_lang_Throwable_p)(void*)-1;	\
	*(ret_t*)&fastiva_exContext.ret_value = ret_v;						\
	goto finally_handler;

#define JPP_RETURN_FINALLY$(ret_t, catched_ex, exit_finally)				\
	if (catched_ex == NULL) {											\
		goto exit_finally;												\
	}																	\
	if (catched_ex != (void*)-1) {										\
		THROW_EX$(catched_ex);											\
	}																	\
	return *(ret_t*)(void*)&fastiva_exContext.ret_value;				\

#define JPP_RETURN_FINALLY_VOID$(catched_ex, exit_finally)				\
	if (catched_ex == NULL) {											\
		goto exit_finally;												\
	}																	\
	if (catched_ex != (void*)-1) {										\
		THROW_EX$(catched_ex);											\
	}																	\
	return;																\





#if 0
#ifdef __cplusplus
extern "C" {
#endif	
#define JPP_EXIT_ON_OOM   0

#if !JPP_EXIT_ON_OOM
	void fastiva_throwOutOfMemoryError();
#else
	#define fastiva_throwOutOfMemoryError()	{ fox_printf("!!! OutOfMemoryError: %s <%s:%d>\n", __FUNCTION__, __FILE__, __LINE__); fox_exit(-1); }
#endif

	void fastiva_throwNullPointerException(); //
	void fastiva_throwClassNotFoundException(java_lang_String_p pClassName);
	void fastiva_throwInstantiationException();
	void fastiva_throwIllegalAccessException();
	void fastiva_throwIllegalArgumentException();
	void fastiva_throwIllegalMonitorStateException();
	void fastiva_throwInterruptedException();
	void fastiva_throwArrayStoreException();
	void fastiva_pureVirtualFuncCalled();
	void fastiva_throwDivideByZeroException();
	void fastiva_throwIOException();
#ifdef __cplusplus
}
#endif	
#endif


#endif // __FASTIVA_EXCEPTION__H__
