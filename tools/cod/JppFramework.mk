LOCAL_PATH:= $(call my-dir)
JNI_PATH:= $(LOCAL_PATH)

include $(CLEAR_VARS)
include $(JPP_DIR)/tools/cod/fastiva-common.mk


LOCAL_CFLAGS += -Wno-error=invalid-offsetof -Wno-error=comment -Wno-error=unused-variable \
		-Wno-error=unused-label -Wno-error=maybe-uninitialized -Wno-error=type-limits \
         -Wno-error=clobbered -Wno-error=trigraphs

LOCAL_CFLAGS += -flto -fvisibility=hidden \
		-DFASTIVA_BUILD_TARGET=FASTIVA_BUILD_TARGET_RUNTIME_LIBCORE \
		-DFASTIVA_no_USE_PRECOMPILED_HEADER

LOCAL_SRC_FILES := \
	$(JPP_DIR)/dalvik-fm/vm/fastiva/src/dalvik_java_lang_String.cpp \
	$(JPP_DIR)/dalvik-fm/vm/fastiva/src/dalvik_java_lang_Math.cpp \
	$(subst $(LOCAL_PATH)/, , $(wildcard $(LOCAL_PATH)/cpp/00_prj/0*.cpp))


LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libfastiva
include $(BUILD_SHARED_LIBRARY)
