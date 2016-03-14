#if JPP_LIBCORE_MODULE_TYPE == JPP_MODULE_TYPE_LIBRARY

#include <fastiva/preprocess/util.h>
#ifdef _WIN32
	//#ifdef EMBEDDED_RUNTIME
		//#pragma section(JPP_CURRENT_PACKAGE_SEGMENT,read)
		#pragma const_seg(JPP_CURRENT_PACKAGE_SEGMENT)

		// extern const int FASTIVA_MERGE_TOKEN(zzzzzz_, JPP_CURRENT_PACKAGE);
		// 상수가 참조되지 않으면 segment 에 포함되지 않을 수 있으므로 zzzzz_ 를 참조.  
		//__declspec(allocate(JPP_CURRENT_PACKAGE_SEGMENT)) 
		static const int FASTIVA_MERGE_TOKEN(A00000_, JPP_CURRENT_PACKAGE)[1] = {0};
			//(int)&FASTIVA_MERGE_TOKEN(zzzzzz_, JPP_CURRENT_PACKAGE);
		#pragma const_seg()
	//#else 
	//	#pragma const_seg(FASTIVA_MAKE_STR(JPP_CURRENT_PACKET_SEG_NAME) "$5")
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

