
#ifdef _WIN32
	#ifndef FASTIVA_DECL_PACKAGE_INFO
		#define FASTIVA_INIT_PACKAGE_INFO(CLASS)							\
			extern "C" FASTIVA_EXPORT Fastiva::Package CLASS##_Package$ = 
	#endif
	#ifdef EMBEDDED_RUNTIME
		#pragma data_seg("FXPKGS")
	#else 
		#pragma const_seg("FXPKGS")
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

