#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_jproxy_scanner.h")
#endif

#define JPP_PREPROCESS_INSTANCE_DATA

#define FASTIVA_END_OF_PREPROCESS											\
	} //no support in dalvik SUPER$::scanJavaProxyFields$(self, pEnv0, method);	}


#define FASTIVA_FIELD(acc, name, type, declrared_cls, jni_acc)				\
	//no support in dalvik FASTIVA_IMPL_SCAN_JPROXY_FIELD_##type(name)





