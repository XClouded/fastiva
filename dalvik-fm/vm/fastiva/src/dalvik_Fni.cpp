#include <Common.h>

struct C {
	int a;
	int b;
};

struct D : C {
	//enum {bb = (int)&((C*)0)->b };
};


#if 0// def _WIN32
#define FASTIVA_JNI_ENV
#define FASTIVA_CONTANTS_VERIFY
#include <fni.h>
#include "mterp/common/asm-constants.h"

void check_fni() {

	assert(OFFSETOF_MEMBER(_jstring, count) == STRING_FIELDOFF_COUNT);
	assert(OFFSETOF_MEMBER(_jstring, value) == STRING_FIELDOFF_VALUE);
	assert(OFFSETOF_MEMBER(_jstring, offset) == STRING_FIELDOFF_OFFSET);

	assert(OFFSETOF_MEMBER(_jarray, length) == offArrayObject_length);
	assert(OFFSETOF_MEMBER(_jarray, items) == offArrayObject_contents);

	assert(OFFSETOF_MEMBER(_jclass, vtable) == offClassObject_cpp_vtable);

	//assert(ACC_FASTIVA_CPP_NO_FLOAT_ARGS == __ACC_FASTIVA_CPP_NO_FLOAT_ARGS);
	//assert(ACC_VOLATILE == __ACC_VOLATILE);

}
#endif



