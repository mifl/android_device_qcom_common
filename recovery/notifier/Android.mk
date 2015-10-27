ifeq ($(TARGET_RECOVERY_NOTIFIER_LIB),librecovery_notifier_qcom)
ifneq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_ARCH),arm)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libnotifier_qti

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += bootable/recovery bootable/recovery/notifier

LOCAL_SRC_FILES += leds.c

include $(BUILD_STATIC_LIBRARY)
endif   # TARGET_ARCH == arm
endif   # !TARGET_SIMULATOR
endif
