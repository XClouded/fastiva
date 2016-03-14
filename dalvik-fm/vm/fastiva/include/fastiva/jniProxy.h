#ifndef __FASTIVA_JNI_PROXY_H__
#define __FASTIVA_JNI_PROXY_H__

#include <fastiva/Synchronize.h>

FASTIVA_DECL_ARRAY_EX(fastiva_CustomArray, fastiva_BytecodeProxy, false);

struct fastiva_jni_LocalFrame : fastiva_Rewinder {
	void* m_res;
	void* m_pEnv;
	fastiva_jni_LocalFrame(void* pEnv);
	int setResult(void* res);
	~fastiva_jni_LocalFrame();
};

#define JPP_JNI_PUSH_LOCAL_FRAME(pEnv)	fastiva_jni_LocalFrame fastiva_jni_local_frame$(pEnv);

#define JPP_JNI_SET_LOCAL_FRAME_RESULT(res)	*(int*)(void*)&res = fastiva_jni_local_frame$.setResult(res);

#endif // __FASTIVA_JNI_PROXY_H__

