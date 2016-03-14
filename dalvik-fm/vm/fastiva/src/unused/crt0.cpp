#ifdef EMBEDDED_RUNTIME
	#pragma data_seg("FXPKGS")
	extern "C" int FASTIVA_PACKAGE_LIST_HEADER[] = { 0 };
	#pragma data_seg("FXPKGS$")
	extern "C" int FASTIVA_PACKAGE_LIST_FOOTER[] = { 0 };
	#pragma data_seg()  /* reset */
#else
	#pragma const_seg("FXPKGS")
	extern "C" int FASTIVA_PACKAGE_LIST_HEADER[] = { 0 };
	#pragma const_seg("FXPKGS$")
	extern "C" int FASTIVA_PACKAGE_LIST_FOOTER[] = { 0 };
	#pragma const_seg()  /* reset */
#endif