
#ifdef _WIN32
	#ifdef EMBEDDED_RUNTIME
		#pragma data_seg()
	#else 
		#pragma const_seg()
	#endif
#elif defined(FASTIVA_TARGET_OS_LINUX)
	#ifdef EMBEDDED_RUNTIME
		/*
		gcc������ Win32�� Ʋ���� const_seg()ó�� __attribute__((section()))�� ���� �ʾƵ� �ȴ�.
		���� ���� ���� �Ұ�� error�� ���´�.
		*/
	#else 
	#endif
#else
	#error unknown target
#endif

