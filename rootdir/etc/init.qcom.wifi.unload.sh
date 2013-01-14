#!/system/bin/sh
# Copyright (c) 2012, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided
#      with the distribution.
#    * Neither the name of The Linux Foundation nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

target=$1
wlanchip=$2

case $1 in
    "msm8960")
        case "$2" in
            "AR6004-USB")
                retry_limit=10
                retry=0

                while [ $retry -lt $retry_limit ]
                do
                    device_ids=`/system/bin/ls /sys/bus/usb/devices/`
                    for id in $device_ids; do
                        if [ -f /sys/bus/usb/devices/$id/idVendor ]; then
                            vendor=`cat /sys/bus/usb/devices/$id/idVendor`
                            if [ $vendor = "0cf3" ]; then
                                if [ -f /sys/bus/usb/devices/$id/idProduct ]; then
                                    product=`cat /sys/bus/usb/devices/$id/idProduct`
                	            if [ $product = "9374" ] || [ $product = "9372" ]; then
                                        echo "auto" > /sys/bus/usb/devices/$id/power/control
                                        exit 0
                                    fi
                                fi
                            fi
                        fi
                    done

                    retry=$(($retry+1))
                    /system/bin/sleep 1
                done

                ;; #AR6004-USB
            *)
                ;;
        esac
        ;;  #msm8960

    *)
        ;;
esac
