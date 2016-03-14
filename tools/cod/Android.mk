# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# dexopt, the DEX file optimizer.  This is fully integrated with the VM,
# so it must be linked against the full VM shared library.
#

LOCAL_PATH:= $(call my-dir)
JNI_PATH:= $(LOCAL_PATH)


include $(CLEAR_VARS)
include $(JPP_DIR)/tools/cod/fastiva-common.mk


LOCAL_CFLAGS += -fvisibility=hidden \
		-DFASTIVA_BUILD_TARGET=FASTIVA_BUILD_TARGET_EXTERNAL_COMPONENT \
		-DFASTIVA_no_USE_PRECOMPILED_HEADER


LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/, , $(wildcard $(LOCAL_PATH)/cpp/00_prj/0*.cpp))

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libfandroid
LOCAL_LDLIBS += $(JPP_OUT_DIR)/libs/armeabi-v7a/libfastiva.so

include $(BUILD_SHARED_LIBRARY)
