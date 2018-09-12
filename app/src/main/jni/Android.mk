LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libjni_MadMagCalibrate

LOCAL_CFLAGS    += -Wall -Wno-unused-parameter -Wno-missing-field-initializers
LOCAL_CPPFLAGS  += -Wno-write-strings
LOCAL_LDFLAGS   := -lm
LOCAL_LDLIBS    := -llog

LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE)
    
LOCAL_SRC_FILES := MadMagCalibrate.cpp Calibration.cpp

include $(BUILD_SHARED_LIBRARY)
