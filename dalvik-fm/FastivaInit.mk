include $(JPP_DIR)/tools/cod/fastiva-common.mk
include $(AOSP_PATH)/fastiva-version.mk
############### default fastiva-version.mk ####################
#LOCAL_CFLAGS += -DFASTIVA_TARGET_ANDROID_VERSION=40400 -DnoCUBIE_4_2_2
#HAVE_SELINUX := true
###############################################################


LOCAL_CFLAGS += -DEMBEDDED_RUNTIME -DFASTIVA_BUILD_TARGET=FASTIVA_BUILD_TARGET_RUNTIME_KERNEL 

LOCAL_C_INCLUDES += dalvik dalvik/vm dalvik/vm/fastiva/include $(JPP_OUT_DIR)/jni/inc 
#                    external/stlport/stlport bionic/ bionic/libstdc++/include
