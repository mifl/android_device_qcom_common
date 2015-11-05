DEVICE_PACKAGE_OVERLAYS := device/qcom/POLARIS/overlay

PRODUCT_COPY_FILES += \
   device/qcom/POLARIS/WCNSS_qcom_wlan_nv.bin:persist/WCNSS_qcom_wlan_nv.bin

PRODUCT_COPY_FILES += \
   device/qcom/POLARIS/mixer_paths_wcd9326.xml:system/etc/mixer_paths_wcd9326.xml

PRODUCT_COPY_FILES += \
    device/qcom/POLARIS/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml

include device/qcom/msm8952_64/msm8952_64.mk


PRODUCT_NAME := POLARIS
PRODUCT_DEVICE := POLARIS
PRODUCT_MODEL := POLARIS
PRODUCT_CHARACTERISTICS := PHONE
