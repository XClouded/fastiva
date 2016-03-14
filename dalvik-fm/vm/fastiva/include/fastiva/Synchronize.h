#ifndef __FASTIVA_SYNCHRONIZE__H__
#define __FASTIVA_SYNCHRONIZE__H__

//=========================================================================//
// Synchronization
//=========================================================================//

#define SYNCHRONIZED_STATIC$()												\
	fastiva_Synchronize class_lock_$$$((fastiva_Instance_p)STATIC$::getRawStatic$());

#define SYNCHRONIZED_SELF$()												\
	fastiva_Synchronize instance_lock_$$$(self);

#define SYNCHRONIZED$(pObj) 												\
	fastiva_Synchronize fastiva_sync$(pObj->getInstance$());


#define FASTIVA_MONITOR_ENTER(ptr)	fastiva.monitorEnter(ptr)

#define FASTIVA_MONITOR_EXIT(ptr)	fastiva.monitorExit(ptr)

#define JPP_MONITOR_LINK(pObj)	fastiva_SynchronizedLink fastiva_SynchronizedLink$##pObj(pObj)


class fastiva_Rewinder {
	friend struct fastiva_Runtime;

FASTIVA_RUNTIME_CRITICAL(private):
	fastiva_Task* m_pTask;

#ifndef FASTIVA_USE_CPP_EXCEPTION
	fastiva_Rewinder* m_pPrev;

	fastiva_Rewinder() {
		this->m_pTask = 0;
	}

	void pushRewinder(fastiva_Task* pTask) {
#ifndef FASTIVA_USE_CPP_EXCEPTION
		 fastiva.pushRewinder(pTask, this);
#endif
	}

FASTIVA_RUNTIME_CRITICAL(protected):	
	 virtual ~fastiva_Rewinder() {
		 fastiva.rewind(this);
	 }
#endif
};


struct fastiva_CallerClassStack  : public fastiva_Rewinder {
FASTIVA_RUNTIME_CRITICAL(private):
	fastiva_Class_p m_pClass2;
	fastiva_Class_p m_pAppClass2;

public:
	inline fastiva_CallerClassStack(fastiva_Class_p pClass) {
		fastiva.pushCallerClass(this, pClass);
	}
	inline ~fastiva_CallerClassStack() {
		fastiva.popCallerClass(this);
	}
};


class fastiva_Synchronize : public fastiva_Rewinder {
FASTIVA_RUNTIME_CRITICAL(private):
	fastiva_Instance_p m_pObj;

public:
	inline fastiva_Synchronize(fastiva_Instance_p pObj) {
		fastiva.beginSynchronized(pObj, this);
	}
	inline ~fastiva_Synchronize() {
		if (m_pTask != 0) {
			fastiva.endSynchronized(this->m_pObj, this);
		}
	}
};

#define JPP_MONITOR_SYNCHRONIZED(obj)							\
		fastiva_Synchronize monitor_lock_$$$(obj->getInstance$());


struct fastiva_synchronized_Section {
	int curr_try_id;
	java_lang_Throwable_p catched_ex;
	int finally_ret_addr;
};

#define FASTIVA_SYNCHRONIZED_SECTION									\
	fastiva_synchronized_Section	 fastiva_exContext;					\


#endif // __FASTIVA_SYNCHRONIZE__H__
