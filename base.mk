# Board platforms lists to be used for
# TARGET_BOARD_PLATFORM specific featurization
QCOM_BOARD_PLATFORMS += msm8916
QCOM_BOARD_PLATFORMS += msm8909
QCOM_BOARD_PLATFORMS += msm8952

#List of targets that use video hw
MSM_VIDC_TARGET_LIST := msm8916 msm8909 msm8952

# Below projects/packages with LOCAL_MODULEs will be used by
# PRODUCT_PACKAGES to build LOCAL_MODULEs that are tagged with
# optional tag, which will not be available on target unless
# explicitly list here. Where project corresponds to the vars here
# in CAPs.

AUDIO_HARDWARE += audio.primary.msm8916
AUDIO_HARDWARE += audio.primary.msm8909
AUDIO_HARDWARE += audio.primary.msm8952
#
AUDIO_POLICY += audio_policy.msm8916
AUDIO_POLICY += audio_policy.msm8909
AUDIO_POLICY += audio_policy.msm8952

#tinyalsa test apps
TINY_ALSA_TEST_APPS := tinyplay
TINY_ALSA_TEST_APPS += tinycap
TINY_ALSA_TEST_APPS += tinymix
TINY_ALSA_TEST_APPS += tinypcminfo
TINY_ALSA_TEST_APPS += cplay

#INIT
INIT := init.qcom.composition_type.sh
INIT += init.qcom.sensor.sh
INIT += init.target.rc
INIT += init.qcom.bt.sh
INIT += hsic.control.bt.sh
INIT += init.qcom.early_boot.sh
INIT += init.qcom.post_boot.sh
INIT += init.qcom.rc
INIT += init.qcom.sdio.sh
INIT += init.qcom.sh
INIT += init.qcom.class_core.sh
INIT += init.class_main.sh
INIT += init.qcom.wifi.sh
INIT += vold.fstab
INIT += init.qcom.ril.path.sh
INIT += init.qcom.usb.rc
INIT += init.qcom.usb.sh
INIT += usf_post_boot.sh
INIT += init.qcom.efs.sync.sh
INIT += ueventd.qcom.rc
INIT += init.ath3k.bt.sh
INIT += qca6234-service.sh
INIT += init.qcom.audio.sh
INIT += ssr_setup
INIT += enable_swap.sh
INIT += init.mdm.sh
INIT += init.qcom.uicc.sh
INIT += fstab.qcom
INIT += init.qcom.debug.sh

#LIBCAMERA
LIBCAMERA += camera.msm8916
LIBCAMERA += camera.msm8909
LIBCAMERA += camera.msm8952
LIBCAMERA += libcamera
LIBCAMERA += libmmcamera_interface
LIBCAMERA += libmmcamera_interface2
LIBCAMERA += libmmjpeg_interface
LIBCAMERA += libqomx_core
LIBCAMERA += mm-qcamera-app
LIBCAMERA += camera_test
LIBCAMERA += org.codeaurora.camera

LIBGRALLOC += gralloc.msm8909
LIBGRALLOC += gralloc.msm8916
LIBGRALLOC += gralloc.msm8952
LIBGRALLOC += libmemalloc

#LIBLIGHTS
LIBLIGHTS += lights.msm8909
LIBLIGHTS += lights.msm8916
LIBLIGHTS += lights.msm8952

#LIBHWCOMPOSER
LIBHWCOMPOSER += hwcomposer.msm8916
LIBHWCOMPOSER += hwcomposer.msm8952

#LIBAUDIOPARAM -- Exposing AudioParameter as dynamic library for SRS TruMedia to work
LIBAUDIOPARAM := libaudioparameter

#LIBAUDIORESAMPLER -- High-quality audio resampler
LIBAUDIORESAMPLER := libaudio-resampler

#LIBOPENCOREHW
LIBOPENCOREHW := libopencorehw

#LIBOVERLAY
LIBOVERLAY := liboverlay
LIBOVERLAY += overlay.default

#LIBGENLOCK
LIBGENLOCK := libgenlock

#LIBPERFLOCK
LIBPERFLOCK := org.codeaurora.Performance

#LIBQCOMUI
LIBQCOMUI := libQcomUI

#LIBQDUTILS
LIBQDUTILS := libqdutils

#LIBQDMETADATA
LIBQDMETADATA := libqdMetaData

#LIBPOWER
LIBPOWER := power.qcom

#LLVM for RenderScript
#use qcom LLVM
$(call inherit-product-if-exists, external/llvm/llvm-select.mk)

#MEDIA_PROFILES
MEDIA_PROFILES := media_profiles.xml

#MM_AUDIO
MM_AUDIO := libOmxAacDec
MM_AUDIO += libOmxAacEnc
MM_AUDIO += libOmxAmrEnc
MM_AUDIO += libOmxEvrcEnc
MM_AUDIO += libOmxMp3Dec
MM_AUDIO += libOmxQcelp13Enc
MM_AUDIO += libOmxAc3HwDec
MM_AUDIO += libstagefright_soft_flacdec

#MM_CORE
MM_CORE := libmm-omxcore
MM_CORE += libOmxCore

#MM_VIDEO
MM_VIDEO := mm-vdec-omx-test
MM_VIDEO += mm-venc-omx-test

#WLAN
WLAN := prima_wlan.ko

PRODUCT_PACKAGES := \
    Bluetooth \
    Camera \

PRODUCT_PACKAGES += $(AUDIO_HARDWARE)
PRODUCT_PACKAGES += $(AUDIO_POLICY)
PRODUCT_PACKAGES += $(INIT)
PRODUCT_PACKAGES += $(LIBCAMERA)
PRODUCT_PACKAGES += $(LIBGRALLOC)
PRODUCT_PACKAGES += $(MM_AUDIO)
PRODUCT_PACKAGES += $(MM_CORE)
PRODUCT_PACKAGES += $(MM_VIDEO)

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml

#intialise PRODUCT_PACKAGES_DEBUG list for debug modules
PRODUCT_PACKAGES_DEBUG := init.qcom.testscripts.sh

# include additional build utilities
-include device/qcom/common/utils.mk

