#!/system/bin/sh
# Copyright (c) 2016, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of The Linux Foundation nor
#       the names of its contributors may be used to endorse or promote
#       products derived from this software without specific prior written
#       permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

target=`getprop ro.board.platform`
if [ -f /sys/devices/soc0/soc_id ]; then
    platformid=`cat /sys/devices/soc0/soc_id`
else
    platformid=`cat /sys/devices/system/soc/soc0/id`
fi

start_msm_irqbalance_8939()
{
        if [ -f /system/bin/msm_irqbalance ]; then
                case "$platformid" in
                    "239" | "241" | "263" | "264" | "268" | "269" | "270" | "271")
                        start msm_irqbalance;;
                esac
        fi
}

start_msm_irqbalance_8952()
{
        if [ -f /system/bin/msm_irqbalance ]; then
                case "$platformid" in
                    "239" | "241" | "263" | "264" | "268" | "269" | "270" | "271")
                        start msm_irqbalance;;
                esac
                case "$platformid" in
                        "266" | "274" | "277" | "278")
                        start msm_irqbal_lb;;
                esac
        fi
}

start_msm_irqbalance()
{
        if [ -f /system/bin/msm_irqbalance ]; then
                start msm_irqbalance
        fi
}

case "$target" in

"msm8952")
	start_msm_irqbalance_8952
        if [ -f /sys/devices/soc0/soc_id ]; then
            soc_id=`cat /sys/devices/soc0/soc_id`
        else
            soc_id=`cat /sys/devices/system/soc/soc0/id`
        fi

        if [ -f /sys/devices/soc0/platform_subtype_id ]; then
             platform_subtype_id=`cat /sys/devices/soc0/platform_subtype_id`
        fi
        if [ -f /sys/devices/soc0/hw_platform ]; then
             hw_platform=`cat /sys/devices/soc0/hw_platform`
        fi
        case "$soc_id" in
             "264")
                  case "$hw_platform" in
                       "Surf")
                            case "$platform_subtype_id" in
                                 "1" | "2")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "MTP")
                            case "$platform_subtype_id" in
                                 "3")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "QRD")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                  esac
                  ;;
             "278")
                  case "$hw_platform" in
                       "Surf")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                       "MTP")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "QRD")
                            case "$platform_subtype_id" in
                                 "0" | "64")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "RCM")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                  esac
                  ;;
             "266")
                  case "$hw_platform" in
                       "Surf")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                       "MTP")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "QRD")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "RCM")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                  esac
                  ;;
             "277")
                  case "$hw_platform" in
                       "Surf")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                       "MTP")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "QRD")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "RCM")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                  esac
                  ;;
             "274")
                  case "$hw_platform" in
                       "Surf")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                       "MTP")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "QRD")
                            case "$platform_subtype_id" in
                                 "0")
                                      setprop qemu.hw.mainkeys 0
                                      ;;
                            esac
                            ;;
                       "RCM")
                            case "$platform_subtype_id" in
                                 "0")
                                    if [ $panel_xres -eq 1440 ]; then
                                         setprop qemu.hw.mainkeys 0
                                    fi
                                    ;;
                            esac
                            ;;
                  esac
                  ;;
        esac
        ;;
    "msm8994")
        start_msm_irqbalance
        ;;
    "msm8909")
        ;;
esac
