LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libjni_MadMagCalibrate
LOCAL_CFLAGS    += -Wall -Wno-unused-parameter -Wno-missing-field-initializers
LOCAL_CPPFLAGS  += -Wno-write-strings
LOCAL_LDFLAGS   := -lm
LOCAL_LDLIBS    := -llog

LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE)

My_All_Files := $(shell find $(LOCAL_PATH))
My_All_Files := $(My_All_Files:$(LOCAL_PATH)/./%=$(LOCAL_PATH)%)
MY_CPP_LIST  := $(filter %.cpp %.c,$(My_All_Files)) 
MY_CPP_LIST  := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)
 
LOCAL_SRC_FILES := $(MY_CPP_LIST)

include $(BUILD_SHARED_LIBRARY)
