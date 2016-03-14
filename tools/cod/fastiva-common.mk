LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc \
		$(JPP_OUT_DIR)/jni/inc \
		$(JPP_DIR)/dalvik-fm/vm/fastiva/include \
		$(LOCAL_C_INCLUDES)

LOCAL_CFLAGS += -DFASTIVA \
		-fPIE -fno-short-enums -fno-builtin-sin -fno-strict-volatile-bitfields \
		-fgcse-after-reload -frerun-cse-after-loop -frename-registers \
		-fno-strict-aliasing -Wstrict-aliasing=2 \
		-finline-functions -g -O2 
#		-faggressive-loop-optimizations 

ifeq ($(FASTIVA_EXCEPTION_TYPE),C++)
	LOCAL_CFLAGS += -DFASTIVA_USE_CPP_EXCEPTION -fexceptions -fnon-call-exceptions -lgnustl_shared
endif

ifneq (_$(ARCH),_) 
	ifeq ($(ARCH),x86)
	    LOCAL_CFLAGS += -Dx86
	else
	    LOCAL_CFLAGS += -mfpu=neon -mthumb-interwork 
	endif
endif