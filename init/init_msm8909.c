/*
   Copyright (c) 2014, The Linux Foundation. All rights reserved.

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

#include <stdio.h>
#include <stdlib.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

#include "init_msm.h"

#define VIRTUAL_SIZE "/sys/class/graphics/fb0/virtual_size"
#define BUF_SIZE 64

void init_msm_properties(unsigned long msm_id, unsigned long msm_ver, char *board_type)
{
    char platform[PROP_VALUE_MAX];
    int rc;
    unsigned long virtual_size = 0;
    char str[BUF_SIZE];

    UNUSED(msm_id);
    UNUSED(msm_ver);
    UNUSED(board_type);

    rc = property_get("ro.board.platform", platform);
    if (!rc || !ISMATCH(platform, ANDROID_TARGET)){
        return;
    }

    rc = read_file2(VIRTUAL_SIZE, str, sizeof(str));
    if (rc) {
        virtual_size = strtoul(str, NULL, 0);
    }

    if(virtual_size >= 1080) {
        property_set(PROP_LCDDENSITY, "480");
    } else if (virtual_size >= 720) {
        // For 720x1280 resolution
        property_set(PROP_LCDDENSITY, "320");
    } else if (virtual_size >= 480) {
        // For 480x854 resolution QRD.
        property_set(PROP_LCDDENSITY, "240");
    } else
        property_set(PROP_LCDDENSITY, "320");

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("/proc/meminfo", "r");
    if (fp != NULL) {
        if ((read = getline(&line, &len, fp)) != -1) {
            int i = 0;
            while ((line[i] > '9') || (line[i] < '0')) {
                i++;
            }
            double memory_in_kb = 0, memory_in_mb = 0;
            while ((line[i] <= '9') && (line[i] >= '0')) {
                memory_in_kb = memory_in_kb*10 + (line[i]-'0');
                i++;
            }
            memory_in_mb = memory_in_kb/1024;
            ERROR("memory: %f, %f\n", memory_in_kb, memory_in_mb);
            if (memory_in_mb <= 256) {
                ERROR("Found 256MB memory device");
                property_set("ro.device.ram", "256");
            } else if ((memory_in_mb > 256) && (memory_in_mb <= 512)) {
                ERROR("Found 512MB memory device");
                property_set("ro.device.ram", "512");
            }
        }
        fclose(fp);
    } else {
        ERROR("failed to open /proc/meminfo");
    }
    if (msm_id == 206) {
        property_set("media.swhevccodectype", "1");
        property_set("vidc.enc.narrow.searchrange", "0");
    }
}
