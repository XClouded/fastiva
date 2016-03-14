#ifndef	__JPP_EXCEPTION_H__
#define __JPP_EXCEPTION_H__

#include <fastiva/Config.h>

extern "C" void * _exception_info(void);

#if defined(_WIN32) 
	extern int fastiva_dispatchNativeException(void* pExInfo);
    #define FASTIVA_BEGIN_MANAGED_SECTION(pJniEnv)								\
	__try {																		\
		fastiva_ManagedSection fastiva_ManagedSection_$$(pJniEnv);


	#define FASTIVA_END_MANAGED_SECTION()										\
	}																			\
	__except (fastiva_dispatchNativeException(_exception_info())) {				\
         FASTIVA_DBREAK();														\
    }
#else
    #define FASTIVA_BEGIN_MANAGED_SECTION(pJniEnv)								\
	{ fastiva_ManagedSection fastiva_ManagedSection_$$(pJniEnv);  \

    #define FASTIVA_END_MANAGED_SECTION()										\
	}
#endif 



#define FASTIVA_ENTER_NATIVE_SECTION()										\
	{ fastiva_NativeSectionContext fastiva_NativeSectionContext_$$;			\

#define FASTIVA_LEAVE_NATIVE_SECTION()										\
	}


struct fastiva_ManagedSection {
	fastiva_ManagedSection* m_pPrev;
	int   m_depth;
	void* m_pThread;
	void* m_pStackTop;

	fastiva_ManagedSection(void* unused);

	~fastiva_ManagedSection();

	inline void* getTop() {
		return m_pStackTop;
	}
	
	inline void* getBottom() { 
		return this; 
	}
	
	inline bool isClosed() {
		return m_pStackTop != ADDR_ZERO;
	}
};



//========================================================================//
// 
//  Exception Handler  
//
//========================================================================//

#ifdef FASTIVA_USE_CPP_EXCEPTION
	#define TRY$ 																\
		try 
	#define CATCH_ANY$															\
		catch (void* catched_ex$)  
#else
	#define TRY$																\
		fastiva_ExceptionContext exContext$;									\
		java_lang_Throwable_p catched_ex$;									\
		if ((catched_ex$ = (java_lang_Throwable_p)(void*)						\
				FASTIVA_SETJMP(exContext$.init(0))) == 0) 						\

	#define CATCH$(type)														\
		else if (type::isInstance$(catched_ex$)) 

	#define CATCH_ANY$															\
		else 
#endif


#endif // __JPP_EXCEPTION_H__

/**====================== end ========================**/
