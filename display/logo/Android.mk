LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

$(shell cp $(LOCAL_PATH)/splash.img $(PRODUCT_OUT))
