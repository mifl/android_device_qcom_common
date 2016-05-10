$(call inherit-product, device/qcom/common/base.mk)

# For PRODUCT_COPY_FILES, the first instance takes precedence.
# Since we want use QC specific files, we should inherit
# device-vendor.mk first to make sure QC specific files gets installed.
$(call inherit-product-if-exists, $(QCPATH)/common/config/device-vendor.mk)

PRODUCT_BRAND := qcom

ifndef PRODUCT_MANUFACTURER
PRODUCT_MANUFACTURER := QUALCOMM
endif

PRODUCT_PRIVATE_KEY := device/qcom/common/qcom.key
