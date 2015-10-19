/* Copyright (c) 2015, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <linux/input.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <cutils/properties.h>
#include "leds.h"

char const*const SYSFS_LED_GREEN_ENGINE_MODE
        = "/sys/class/leds/green/device/engine2_mode";

char const*const SYSFS_LED_GREEN_ENGINE_LOAD
        = "/sys/class/leds/green/device/engine2_load";

char const*const SYSFS_LED_BLUE_ENGINE_MODE
        = "/sys/class/leds/green/device/engine3_mode";

char const*const SYSFS_LED_BLUE_ENGINE_LOAD
        = "/sys/class/leds/green/device/engine3_load";

char const*const SYSFS_LED_LP5523_ENGINE_MODE
        = "/sys/class/leds/lp5523:channel1/device/engine1_mode";

char const*const SYSFS_LED_LP5523_ENGINE_LOAD
        = "/sys/class/leds/lp5523:channel1/device/engine1_load";


unsigned int
is_subtype_match(unsigned int subtype, int* ptr_value)
{
    int fd;
    int var;
    char buffer[2];
    ssize_t n;
    unsigned int ret = 0;

    fd = open(SYSFS_SOC_PLATFORM_SUPTYPE,O_RDONLY);

    memset(buffer, 0, sizeof(buffer));
    if (fd >= 0) {
        n = read(fd, buffer, 1);
        if(n >= 0) {
            var = atoi(buffer);
            *ptr_value = var;
            printf("platform (%s) subtype (%d) \n", TARGET_HW_PLATFORM, var);
            if(var == subtype)
                ret = 1;
        } else {
            printf("Cant read %s (errno %s)\n", SYSFS_SOC_PLATFORM_SUPTYPE, strerror(errno));
        }
        close(fd);
    } else {
        printf("Cant open %s (errno %s)\n", SYSFS_SOC_PLATFORM_SUPTYPE, strerror(errno));
    }

    return(ret);
}


unsigned int
is_platform_match(char* platform)
{
    int fd;
    char buffer[10];
    ssize_t n;
    unsigned int ret = 0;

    fd = open(SYSFS_SOC_HW_PLATFORM,O_RDONLY);

    memset(buffer, 0, sizeof(buffer));
    if (fd >= 0) {
        n = read(fd, buffer, (sizeof(platform)-1));

        if(n >= 0) {
            if(strncmp(buffer, platform, (sizeof(platform)-1)) == 0)
                ret = 1;
        } else {
            printf("Cant read %s (errno %s)\n", SYSFS_SOC_HW_PLATFORM, strerror(errno));
        }
        close(fd);
    } else {
        printf("Cant open %s (errno %s)\n", SYSFS_SOC_HW_PLATFORM, strerror(errno));
    }
    return(ret);
}

static void load_leds_program(char const* mode_path, char const* load_path, char const* program)
{
    int mode_fd, load_fd;

    mode_fd = open(mode_path, O_WRONLY);

    if (mode_fd >= 0) {
        write(mode_fd, "load", 5);
        close(mode_fd);

        load_fd = open(load_path, O_WRONLY);
        if (load_fd >= 0) {
            printf("loading led1 program (%s)\n", program);
            write(load_fd, program, strlen(program));
            close(load_fd);

            mode_fd = open(mode_path, O_WRONLY);
            if (mode_fd >= 0) {
                write(mode_fd, "run", 4);
                close(mode_fd);
            }
            else {
                printf("Cannot open (%s)\n", mode_path);
            }
        }
        else {
            printf("Cannot open (%s)\n", load_path);
        }
    }
    else {
        printf("Cannot open (%s)\n", mode_path);
    }
}

static void unload_leds_program(char const* mode_path)
{
    int mode_fd, load_fd;

    mode_fd = open(mode_path, O_WRONLY);

    if (mode_fd >= 0) {
        write(mode_fd, "disabled", 9);
        close(mode_fd);
    }
    else {
        printf("Cannot open (%s)\n", mode_path);
    }
}

/*
 * LEDs will only be turned on if target and platform sub type are matched
 * The logic to turn on the LEDs is:
 * Check if the target HW platform is QM8626, then check the sub type
 * If the subtype is 1, it means it supports LP 5521 LEDs, so we turn those on
 * If the subtype is between 2 to 4, the client should have setup that which leds they will use
 * between LP5521 and LP55231.
 * Therefore we check the preference, if nothing is set, no LEDs will be turn on
 *
 * Following the requirements, the LEDs will be turned on and off in 500ms
 * Green and Blue LEDs will alternate
 */
bool leds_entry()
{
    int rc = false, ret=0, andled1_value=0,andled2_value=0, subtype=0;
    char value[PROPERTY_VALUE_MAX];

    printf("leds_entry \n");
    if(is_platform_match((char*)TARGET_HW_PLATFORM)){
        printf("notifier leds platform match (%s)\n", TARGET_HW_PLATFORM);

        if(is_subtype_match(1, &subtype)) {
            rc = true;
            load_leds_program(SYSFS_LED_BLUE_ENGINE_MODE, SYSFS_LED_BLUE_ENGINE_LOAD, "E10040FF60004000E004A020");
            load_leds_program(SYSFS_LED_GREEN_ENGINE_MODE, SYSFS_LED_GREEN_ENGINE_LOAD, "40FF60004000E208A010");

        }
        else if ((subtype >=2) && (subtype <= 4)){
            memset(value, 0, sizeof(value));
            ret = property_get("persist.qm8626.andled_1.enable", value, NULL);
            andled1_value = atoi(value);

            if(andled1_value) {
                rc = true;
                load_leds_program(SYSFS_LED_BLUE_ENGINE_MODE, SYSFS_LED_BLUE_ENGINE_LOAD, "E10040FF60004000E004A020");
                load_leds_program(SYSFS_LED_GREEN_ENGINE_MODE, SYSFS_LED_GREEN_ENGINE_LOAD, "40FF60004000E208A010");
            } else {
                memset(value, 0, sizeof(value));
                ret = property_get("persist.qm8626.andled_2.enable", value, NULL);
                andled2_value = atoi(value);
                printf("int led1 (%d) int led2 (%d)\n", andled1_value, andled2_value);

                if(andled2_value) {
                    rc = true;
                    load_leds_program(SYSFS_LED_LP5523_ENGINE_MODE, SYSFS_LED_LP5523_ENGINE_LOAD, "9D0240FF7E00420040009D0340FF7E0042004000A000");
                }
                else {
                    printf("error no int leds enabled subtype (%d)\n", subtype);
                }
            }
        }
        else {
            printf("error subtype mismatch (%d)\n", subtype);
        }
    }

    return rc;
}


void leds_exit()
{
    printf("leds_exit \n");
    unload_leds_program(SYSFS_LED_BLUE_ENGINE_MODE);
    unload_leds_program(SYSFS_LED_GREEN_ENGINE_MODE);
    unload_leds_program(SYSFS_LED_LP5523_ENGINE_MODE);
}
