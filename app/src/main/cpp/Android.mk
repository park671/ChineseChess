LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE:= ai
LOCAL_SRC_FILES:= ai.cpp
LOCAL_LDLIBS := -llog -landroid
include $(BUILD_SHARED_LIBRARY)