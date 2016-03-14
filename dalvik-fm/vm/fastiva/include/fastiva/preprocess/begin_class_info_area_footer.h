#if JPP_LIBCORE_MODULE_TYPE == JPP_MODULE_TYPE_LIBRARY

#include <fastiva/preprocess/util.h>
#ifdef _WIN32
	#ifdef EMBEDDED_RUNTIME
		#pragma data_seg(FASTIVA_MAKE_STR(JPP_CURRENT_PACKET_SEG_NAME) "$9")
	#else 
		#pragma const_seg(FASTIVA_MAKE_STR(JPP_CURRENT_PACKET_SEG_NAME) "$9")
	#endif

#elif defined(FASTIVA_TARGET_OS_LINUX)
	#ifndef FASTIVA_DECL_PACKAGE_INFO
		#define FASTIVA_INIT_PACKAGE_INFO(CLASS)							\
			extern "C" FASTIVA_EXPORT __attribute__((section("FXPKGS")))	\
				Fastiva::Package CLASS##_Package$ = 
	#endif
#else
#error unknown target
#endif

#endif
