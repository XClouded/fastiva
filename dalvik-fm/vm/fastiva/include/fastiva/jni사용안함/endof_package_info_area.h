
#ifdef _WIN32
	#ifdef EMBEDDED_RUNTIME
		#pragma data_seg()
	#else 
		#pragma const_seg()
	#endif
#elif defined(FASTIVA_TARGET_OS_LINUX)
	#ifdef EMBEDDED_RUNTIME
		/*
		gcc에서는 Win32와 틀리게 const_seg()처럼 __attribute__((section()))를 하지 않아도 된다.
		만약 위와 같이 할경우 error가 나온다.
		*/
	#else 
	#endif
#else
	#error unknown target
#endif

