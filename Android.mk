LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := vkcl
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/ \
	$(LOCAL_PATH)/inc

LOCAL_SRC_FILES := vkcl.c
LOCAL_LDLIBS := -lvulkan

include $(BUILD_EXECUTABLE)