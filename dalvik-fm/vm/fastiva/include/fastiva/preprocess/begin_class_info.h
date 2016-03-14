#if JPP_LIBCORE_MODULE_TYPE == JPP_MODULE_TYPE_LIBRARY

#include <fastiva/preprocess/util.h>
#ifdef _WIN32
	// ���� linker option�� /align:16(16�� �ּ�)�� �����Ǿ�� �Ѵ�.
	//#ifdef EMBEDDED_RUNTIME
	//	#pragma data_seg(JPP_CURRENT_PACKAGE_SEGMENT)
	//#else
		#pragma const_seg(JPP_CURRENT_PACKAGE_SEGMENT)
	//#endif

#elif defined(FASTIVA_TARGET_OS_LINUX)
	#ifndef FASTIVA_DECL_PACKAGE_INFO
		#define FASTIVA_INIT_PACKAGE_INFO(CLASS)							\
			extern "C" FASTIVA_EXPORT __attribute__((section("FXPKGS")))	\
				fastiva_Package CLASS##_Package$ = 
	#endif
#else
#error unknown target
#endif

#endif

