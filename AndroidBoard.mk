include device/qcom/msm8952_64/AndroidBoard.mk
#----------------------------------------------------------------------

# Compile Linux Kernel

#----------------------------------------------------------------------

ifeq ($(TARGET_BUILD_VARIANT),user)
   KERNEL_DEFCONFIG := polaris-perf_defconfig
else
   KERNEL_DEFCONFIG := polaris_defconfig
endif
