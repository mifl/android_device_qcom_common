# This makefile is used to generate extra images for QCOM targets
# persist, device tree & NAND images required for different QCOM targets.

# These variables are required to make sure that the required
# files/targets are available before generating NAND images.
# This file is included from device/qcom/<TARGET>/AndroidBoard.mk
# and gets parsed before build/core/Makefile, which has these
# variables defined. build/core/Makefile will overwrite these
# variables again.
ifneq ($(strip $(TARGET_NO_KERNEL)),true)
INSTALLED_BOOTIMAGE_TARGET := $(PRODUCT_OUT)/boot.img
INSTALLED_RAMDISK_TARGET := $(PRODUCT_OUT)/ramdisk.img
INSTALLED_SYSTEMIMAGE := $(PRODUCT_OUT)/system.img
INSTALLED_USERDATAIMAGE_TARGET := $(PRODUCT_OUT)/userdata.img
endif

#----------------------------------------------------------------------
# Generate secure boot image
#----------------------------------------------------------------------
ifeq ($(TARGET_BOOTIMG_SIGNED),true)
INSTALLED_SEC_BOOTIMAGE_TARGET := $(PRODUCT_OUT)/boot.img.secure

ifndef TARGET_SHA_TYPE
  TARGET_SHA_TYPE := sha256
endif

define build-boot-image
	$(hide) mv -f $(1) $(1).nonsecure
	$(hide) openssl dgst -$(TARGET_SHA_TYPE) -binary $(1).nonsecure > $(1).$(TARGET_SHA_TYPE)
	$(hide) openssl rsautl -sign -in $(1).$(TARGET_SHA_TYPE) -inkey $(PRODUCT_PRIVATE_KEY) -out $(1).sig
	$(hide) dd if=/dev/zero of=$(1).sig.padded bs=$(BOARD_KERNEL_PAGESIZE) count=1
	$(hide) dd if=$(1).sig of=$(1).sig.padded conv=notrunc
	$(hide) cat $(1).nonsecure $(1).sig.padded > $(1).secure
	$(hide) rm -rf $(1).$(TARGET_SHA_TYPE) $(1).sig $(1).sig.padded
	$(hide) mv -f $(1).secure $(1)
endef

$(INSTALLED_SEC_BOOTIMAGE_TARGET): $(INSTALLED_BOOTIMAGE_TARGET)
	$(hide) $(call build-boot-image,$(INSTALLED_BOOTIMAGE_TARGET),$(INTERNAL_BOOTIMAGE_ARGS))

ALL_DEFAULT_INSTALLED_MODULES += $(INSTALLED_SEC_BOOTIMAGE_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(INSTALLED_SEC_BOOTIMAGE_TARGET)
endif

#----------------------------------------------------------------------
# Compile (L)ittle (K)ernel bootloader and the nandwrite utility
#----------------------------------------------------------------------
ifneq ($(strip $(TARGET_NO_BOOTLOADER)),true)

# Compile
include hardware/bsp/qcom/bootable/bootloader/lk/AndroidBoot.mk

$(INSTALLED_BOOTLOADER_MODULE): $(TARGET_EMMC_BOOTLOADER) | $(ACP)
	$(transform-prebuilt-to-target)
$(BUILT_TARGET_FILES_PACKAGE): $(INSTALLED_BOOTLOADER_MODULE)

droidcore: $(INSTALLED_BOOTLOADER_MODULE)
endif

#----------------------------------------------------------------------
# Generate secure boot image
#----------------------------------------------------------------------
ifeq ($(TARGET_BOOTIMG_SIGNED),true)
.PHONY: bootimage
bootimage: $(INSTALLED_BOOTIMAGE_TARGET) $(INSTALLED_SEC_BOOTIMAGE_TARGET)
endif

###################################################################################################

.PHONY: aboot
ifeq ($(USESECIMAGETOOL), true)
aboot: gensecimage_target
else
aboot: $(INSTALLED_BOOTLOADER_MODULE)
endif

