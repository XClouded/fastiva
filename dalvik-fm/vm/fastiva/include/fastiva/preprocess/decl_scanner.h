#include <fastiva/preprocess/prolog.h>

#ifdef FASTIVA_SHOW_PREPROCESS_MESSAGE
	#pragma message ("decl_scanner.h")
#endif

#define JPP_PREPROCESS_INSTANCE_DATA

#define FASTIVA_END_OF_PREPROCESS											\
	SUPER$::scanInstance$(self, method, scanner);	}


#define FASTIVA_FIELD(acc, name, type, declrared_cls, jni_acc)				\
	FASTIVA_IMPL_SCAN_FIELD_##type(m_##name)


