#ifndef __FASTIVA_FNI_FASTCALL_H_
#define __FASTIVA_FNI_FASTCALL_H_

#include "fni.h"


struct fastiva_fniExceptionContext {
	int reserved[2];
	jmp_buf buf;
	JNIEnv* env;

	fastiva_fniExceptionContext(JNIEnv* env) {
		this->env = env;
		env->PushExceptionHandler(this); 
	}

	~fastiva_fniExceptionContext() {
		env->RemoveExceptionHandler(this); 
	}
};


#define FASTIVA_TRY(env) { \
	fastiva_fniExceptionContext fastiva_exHandler$(env); \
	jthrowable CATCHED_FASTIVA_EXCEPTION; \
	if ((CATCHED_FASTIVA_EXCEPTION = setjmp(fastiva_exHandler$.jmp_buf)) == 0) 

#define FASTIVA_CATCH_ANY else 


union FNI_METHOD {
	jobject	 (*mObject)(...);
	jboolean (*mBoolen)(...);
	jbyte	 (*mByte)(...);
	jchar	 (*mChar)(...);
	jshort	 (*mShort)(...); 
	jint	 (*mInt)(...); 
	jlong	 (*mLong)(...);
	jfloat	 (*mFloat)(...); 
	jdouble	 (*mDouble)(...);
	void     (*mVoid)(...);
};

inline int fastiva_isFastivaMethod(jmethodID method) {
	return method->accessFlags & __ACC_FASTIVA_CPP_NO_FLOAT_ARGS;
}

inline int fastiva_isFastivaMethod(jmethodID method, fastiva_fniExceptionContext* exContext) {
	return method->accessFlags & __ACC_FASTIVA_CPP_NO_FLOAT_ARGS;
}

#define fastiva_ctype_Object	jobject
#define fastiva_ctype_Boolen	jboolean
#define fastiva_ctype_Byte		jbyte
#define fastiva_ctype_Char		jchar
#define fastiva_ctype_Short		jshort
#define fastiva_ctype_Int		jint
#define fastiva_ctype_Long		jlong
#define fastiva_ctype_Float		jfloat
#define fastiva_ctype_Double	jdouble

#if 1// def __GNUC__

#define FASTIVA_INVOKE_VIRTUAL(jtype, env, obj, method, ...) \
	(fastiva_isFastivaMethod(method) \
		? ({ fastiva_ctype_##jtype result; \
			FASTIVA_TRY { \
				result = ((FNI_METHOD)obj->vtable[m->methodIndex]).m##jtype(obj, ## __VA_ARGS__); \
            } FASTIVA_CATCH_ANY { \
				env->Throw(CATCHED_FASTIVA_EXCEPTION); \
			}} result }) \
		: env->Call##jtype##Method(obj, method, ## __VA_ARGS__)) 

#define FASTIVA_INVOKE_DIRECT(jtype, env, obj, cls, method, ...) \
	(fastiva_isFastivaMethod(method) \
		? ({ fastiva_ctype_##jtype result; \
			FASTIVA_TRY { \
				result = ((FNI_METHOD)cls->vtable[method->methodIndex]).m##jtype(obj, ## __VA_ARGS__); \
            } FASTIVA_CATCH_ANY { \
				env->Throw(CATCHED_FASTIVA_EXCEPTION); \
			}} result }) \
		: env->CallNonvirtual##jtype##Method(obj, cls, method, ## __VA_ARGS__)) 

#define FASTIVA_INVOKE_STATIC(jtype, env, cls, method, ...) \
	(fastiva_isFastivaMethod(method) \
		? ({ fastiva_ctype_##jtype result; \
			FASTIVA_TRY { \
				result = ((FNI_METHOD)method->fastivaMethod).m##jtype(obj, ## __VA_ARGS__); \
            } FASTIVA_CATCH_ANY { \
				env->Throw(CATCHED_FASTIVA_EXCEPTION); \
			}} result }) \
		: env->CallStatic##jtype##Method(cls, method, ## __VA_ARGS__)) 


#define FASTIVA_INVOKE_VIRTUAL_Void(env, obj, method, ...) \
	if (fastiva_isFastivaMethod(method)) { \
			FASTIVA_TRY { \
				((FNI_METHOD)obj->vtable[m->methodIndex]).mVoid(obj, ## __VA_ARGS__); \
			} FASTIVA_CATCH_ANY { \
				env->Throw(CATCHED_FASTIVA_EXCEPTION); \
			}} \
	} else { \
		env->CallVoidMethod(obj, method, ## __VA_ARGS__)) \
	}

#define FASTIVA_INVOKE_DIRECT_Void(env, obj, method, ...) \
	if (fastiva_isFastivaMethod(method)) { \
			FASTIVA_TRY { \
				((FNI_METHOD)cls->vtable[m->methodIndex]).mVoid(obj, ## __VA_ARGS__); \
			} FASTIVA_CATCH_ANY { \
				env->Throw(CATCHED_FASTIVA_EXCEPTION); \
			}} \
	} else { \
		env->CallNonvirtualVoidMethod(obj, method, ## __VA_ARGS__)) \
	}

#define FASTIVA_INVOKE_STATIC_Void(jtype, env, obj, method, ...) \
	if (fastiva_isFastivaMethod(method)) { \
			FASTIVA_TRY { \
				((FNI_METHOD)method->fastivaMethod).mVoid(obj, ## __VA_ARGS__); \
			} FASTIVA_CATCH_ANY { \
				env->Throw(CATCHED_FASTIVA_EXCEPTION); \
			}} \
	} else { \
		env->CallStaticVoidMethod(obj, method, ## __VA_ARGS__)) \
	}

#define fastcallObjectMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Object, env, obj, methodID, __VA_ARGS__)
#define fastcallBooleanMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Boolean, env, obj, methodID, __VA_ARGS__)
#define fastcallByteMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Byte, env, obj, methodID, __VA_ARGS__)
#define fastcallCharMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Char, env, obj, methodID, __VA_ARGS__)
#define fastcallShortMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Short, env, obj, methodID, __VA_ARGS__)
#define fastcallIntMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Int, env, obj, methodID, __VA_ARGS__)
#define fastcallLongMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Long, env, obj, methodID, __VA_ARGS__)
#define fastcallFloatMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Float, env, obj, methodID, __VA_ARGS__)
#define fastcallDoubleMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL(Double, env, obj, methodID, __VA_ARGS__)
#define fastcallVoidMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_Void(env, obj, methodID, __VA_ARGS__)


#define fastcallNonvirtualObjectMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Object, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualBooleanMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Boolean, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualByteMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Byte, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualCharMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Char, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualShortMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Short, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualIntMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Int, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualLongMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Long, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualFloatMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Float, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualDoubleMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT(Double, env, obj, cls, methodID, __VA_ARGS__)
#define fastcallNonvirtualVoidMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_Void(env, obj, cls, methodID, __VA_ARGS__)


#define fastcallStaticObjectMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Object, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticBooleanMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Boolen, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticByteMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Byte, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticCharMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Char, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticShortMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Short, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticIntMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Int, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticLongMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Long, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticFloatMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Float, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticDoubleMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC(Double, env, cls, methodID, __VA_ARGS__)
#define fastcallStaticVoidMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_Void, env, cls, methodID, __VA_ARGS__)

#endif  /* __GNUC__ */

#define FASTIVA_INVOKE_VIRTUAL_direct(jtype, env, obj, method, ...) \
	(fastiva_isFastivaMethod(method, &fastiva_exHandler$) \
		? ((FNI_METHOD)obj->vtable[m->methodIndex]).m##jtype(obj, ## __VA_ARGS__) \
		: env->Call##jtype##Method(obj, method, ## __VA_ARGS__)) 

#define FASTIVA_INVOKE_DIRECT_direct(jtype, env, obj, cls, method, ...) \
	(fastiva_isFastivaMethod(method, &fastiva_exHandler$) \
		? ((FNI_METHOD)cls->vtable[method->methodIndex]).m##jtype(obj, ## __VA_ARGS__) \
		: env->CallNonvirtual##jtype##Method(obj, cls, method, ## __VA_ARGS__)) 

#define FASTIVA_INVOKE_STATIC_direct(jtype, env, cls, method, ...) \
	(fastiva_isFastivaMethod(method, &fastiva_exHandler$) \
		? (((FNI_METHOD)method->fastivaMethod).m##jtype(obj, ## __VA_ARGS__) \
		: env->CallStatic##jtype##Method(cls, method, ## __VA_ARGS__)) 


#define FASTIVA_INVOKE_VIRTUAL_Void_direct(jtype, env, obj, method, ...) \
	(fastiva_isFastivaMethod(method, &fastiva_exHandler$) \
		((FNI_METHOD)obj->vtable[m->methodIndex]).mVoid(obj, ## __VA_ARGS__); \
	} else { \
		: env->CallVoidMethod(obj, method, ## __VA_ARGS__)); \
	}

#define FASTIVA_INVOKE_DIRECT_Void_direct(jtype, env, obj, cls, method, ...) \
	(fastiva_isFastivaMethod(method, &fastiva_exHandler$) \
		((FNI_METHOD)cls->vtable[method->methodIndex]).mVoid(obj, ## __VA_ARGS__); \
	} else { \
		env->CallNonvirtualVoidMethod(obj, cls, method, ## __VA_ARGS__); \
	}

#define FASTIVA_INVOKE_STATIC_Void_direct(jtype, env, cls, method, ...) \
	(fastiva_isFastivaMethod(method, &fastiva_exHandler$) \
		(((FNI_METHOD)method->fastivaMethod).mVoid(obj, ## __VA_ARGS__); \
	} else { \
		env->CallNonvirtualVoidMethod(obj, cls, method, ## __VA_ARGS__); \
	}

#define directCallObjectMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Object, env, obj, methodID, __VA_ARGS__)
#define directCallBooleanMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Boolean, env, obj, methodID, __VA_ARGS__)
#define directCallByteMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Byte, env, obj, methodID, __VA_ARGS__)
#define directCallCharMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Char, env, obj, methodID, __VA_ARGS__)
#define directCallShortMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Short, env, obj, methodID, __VA_ARGS__)
#define directCallIntMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Int, env, obj, methodID, __VA_ARGS__)
#define directCallLongMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Long, env, obj, methodID, __VA_ARGS__)
#define directCallFloatMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Float, env, obj, methodID, __VA_ARGS__)
#define directCallDoubleMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_direct(Double, env, obj, methodID, __VA_ARGS__)
#define directCallVoidMethod(env, obj, methodID, ...) \
	FASTIVA_INVOKE_VIRTUAL_Void_direct(env, obj, methodID, __VA_ARGS__)


#define directCallNonvirtualObjectMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Object, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualBooleanMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Boolean, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualByteMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Byte, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualCharMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Char, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualShortMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Short, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualIntMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Int, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualLongMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Long, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualFloatMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Float, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualDoubleMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_direct(Double, env, obj, cls, methodID, __VA_ARGS__)
#define directCallNonvirtualVoidMethod(env, obj, cls, methodID, ...) \
	FASTIVA_INVOKE_DIRECT_Void_direct(env, obj, cls, methodID, __VA_ARGS__)


#define directCallStaticObjectMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Object, env, cls, methodID, __VA_ARGS__)
#define directCallStaticBooleanMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Boolen, env, cls, methodID, __VA_ARGS__)
#define directCallStaticByteMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Byte, env, cls, methodID, __VA_ARGS__)
#define directCallStaticCharMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Char, env, cls, methodID, __VA_ARGS__)
#define directCallStaticShortMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Short, env, cls, methodID, __VA_ARGS__)
#define directCallStaticIntMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Int, env, cls, methodID, __VA_ARGS__)
#define directCallStaticLongMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Long, env, cls, methodID, __VA_ARGS__)
#define directCallStaticFloatMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Float, env, cls, methodID, __VA_ARGS__)
#define directCallStaticDoubleMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_direct((Double, env, cls, methodID, __VA_ARGS__)
#define directCallStaticVoidMethod(env, cls, methodID, ...) \
	FASTIVA_INVOKE_STATIC_Void_direct, env, cls, methodID, __VA_ARGS__)
