#include <precompiled_libcore.h>

#include <jni.h>
extern JNIEnv_* fastiva_jniEnv;



#if 0
void fastiva_wrapJavaPrimitiveArray(void* javaArray0) {
	jarray javaArray = (jarray)fastiva_jniEnv->NewGlobalRef((jobject)javaArray0);
	int len = fastiva_jniEnv->GetArrayLength(javaArray);
	//void* items = fastiva_jniEnv->GetArrayElements(javaArray, NULL);

}

class String {
	int javaHandle$;
	unicod* m_pText$;
	
	
	finalize() {
		if (javaHandle$ != null) {
		    ReleaseGlobalRef(javaHandle);
		}
	}
	
	initNativeString(unicod* pText, int len) {
	    m_pText = pText;
	    m_length = len;
	}
	
	intWithJavaObject(int javaObject, int length) {
	    if (length <= 0) {
	        length = g_pJniEnv->GetStringLength();
	    }
	    this->javaHandle$ = g_lJniEnv->NewGlobalRef(javaObject);
	    this->m_pText$ = g_pJniEnv->GetStringCritical(javaObject);
	    this->m_length = length;
	}
	
	toJavaObject() {
	    if (javaHandle$ != null) {
	        return javaHandle$;
	    }
	    else {
	        unicod* pChar = fm::getUnsfaeStringBuffer(this);        
	        jobject javaStr = g_pFastivaJNI->CreateString(pChar, m_length);
	        initWithJavaObject(javaStr);
	    }
	}
	
}



class Array {
	int javaHandle$;
	ITEM* m_aItem;
	
	
	finalize() {
		if (javaHandle$ != null) {
		    ReleaseGlobalRef(javaHandle);
		}
	}
	
	initNative(ITEM* pText, int len) {
	    m_pText = pText;
	    m_length = len;
	}
	
	intWithJavaObject(int javaObject, int length) {
	    if (length <= 0) {
	        length = g_pJniEnv->GetStringLength();
	    }
	    this->javaHandle$ = g_lJniEnv->NewGlobalRef(javaObject);
	    this->m_pText$ = g_pJniEnv->GetStringCritical(javaObject);
	    this->m_length = length;
	}
	
	toJavaObject() {
	    if (javaHandle$ != null) {
	        return javaHandle$;
	    }
	    else {
	        unicod* pChar = fm::getCreateJavaArray(this);        
	        g_pFastivaJNI->CreateString(pChar, m_length);
	        
	    }
	}
	
}


#endif