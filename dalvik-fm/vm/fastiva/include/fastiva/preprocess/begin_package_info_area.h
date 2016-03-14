#if JPP_LIBCORE_MODULE_TYPE == JPP_MODULE_TYPE_LIBRARY

#ifdef _WIN32
	#ifdef EMBEDDED_RUNTIME
		#pragma data_seg("FXPKGS$5")
	#else 
		#pragma const_seg("FXPKGS$5")
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
