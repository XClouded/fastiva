#if JPP_LIBCORE_MODULE_TYPE == JPP_MODULE_TYPE_LIBRARY

#include <fastiva/preprocess/util.h>
#ifdef _WIN32
	// 주의 linker option에 /align:16(16이 최소)가 지정되어야 한다.
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

