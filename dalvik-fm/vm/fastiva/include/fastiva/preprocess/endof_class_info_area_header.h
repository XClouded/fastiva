#if JPP_LIBCORE_MODULE_TYPE == JPP_MODULE_TYPE_LIBRARY

#ifdef _WIN32
	// ���� linker option�� /align:16(16�� �ּ�)�� �����Ǿ�� �Ѵ�.
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

#endif
