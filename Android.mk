LOCAL_PATH:= $(call my-dir)

NFC_LOCAL_PATH:= $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
        $(call all-java-files-under, app/src/main/java)

LOCAL_PACKAGE_NAME := MadMagCalibrate
LOCAL_CERTIFICATE := platform
LOCAL_SDK_VERSION := current

LOCAL_JNI_SHARED_LIBRARIES := libjni_MadMagCalibrate

include $(BUILD_PACKAGE)

include $(call all-makefiles-under, app/src/main/jni)

