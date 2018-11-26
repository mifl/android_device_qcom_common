/* Copyright (c) 2014, 2017-2018 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <linux/qseecom.h>
#include "cutils/log.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "keymaster_common.h"
#include "hardware.h"
#include "cryptfs_hw.h"
#include "fde_fe.h"


/*
 * When device comes up or when user tries to change the password, user can
 * try wrong password upto a certain number of times. If user enters wrong
 * password further, HW would wipe all disk encryption related crypto data
 * and would return an error ERR_MAX_PASSWORD_ATTEMPTS to VOLD. VOLD would
 * wipe userdata partition once this error is received.
 */
#define ERR_MAX_PASSWORD_ATTEMPTS			-10
#define MAX_PASSWORD_LEN				32
#define SET_HW_DISK_ENC_KEY				1
#define UPDATE_HW_DISK_ENC_KEY				2

#define CRYPTFS_HW_KMS_CLEAR_KEY			0
#define CRYPTFS_HW_KMS_WIPE_KEY				1
#define CRYPTFS_HW_UP_CHECK_COUNT			10
#define CRYPTFS_HW_CLEAR_KEY_FAILED			-11
#define CRYPTFS_HW_KMS_MAX_FAILURE			-10
#define CRYPTFS_HW_UPDATE_KEY_FAILED			-9
#define CRYPTFS_HW_WIPE_KEY_FAILED			-8
#define CRYPTFS_HW_CREATE_KEY_FAILED			-7

#define CRYPTFS_HW_ALGO_MODE_AES_XTS			0x3

//only userdata partition is supported with HW FDE
#define HW_FDE_PARTITION "userdata"

static inline void* secure_memset(void* v, int c , size_t n)
{
	volatile unsigned char* p = (volatile unsigned char* )v;
	while (n--) *p++ = c;
	return v;
}

static size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
	size_t min_size = (dst_size < src_size) ? dst_size : src_size;
	memcpy(dst, src, min_size);
	return min_size;
}

static int cryptfs_hw_create_key(unsigned char *hash32)
{
    if (fde_fe_set_key(HW_FDE_PARTITION, hash32, MAX_PASSWORD_LEN) < 0)
        return CRYPTFS_HW_CREATE_KEY_FAILED;

    return 0;
}

static int __cryptfs_hw_wipe_clear_key(int wipe_key_flag)
{
    if (wipe_key_flag) {
        if (fde_fe_wipe_key(HW_FDE_PARTITION) < 0)
            return CRYPTFS_HW_WIPE_KEY_FAILED;
    } else {
        if (fde_fe_clear_key(HW_FDE_PARTITION) < 0)
            return CRYPTFS_HW_CLEAR_KEY_FAILED;
    }

    return 0;
}

static int cryptfs_hw_wipe_key(void)
{
	int32_t ret;
	ret = __cryptfs_hw_wipe_clear_key(CRYPTFS_HW_KMS_WIPE_KEY);
	if (ret) {
		SLOGE("Error::ioctl call to wipe the encryption key failed with ret = %d, errno = %d\n",
			ret, errno);
		ret = CRYPTFS_HW_WIPE_KEY_FAILED;
	} else {
		SLOGE("SUCCESS::ioctl call to wipe the encryption key for success with ret = %d\n",
			ret);
	}
	return ret;
}

//map ice flag from cryptfs to FDE ICE state
static fde_ice_state_t map_ice_flag(int flag)
{
    switch (flag) {
        case 0:
            return FDE_ICE_BYPASS;
        case START_ENC:
            return FDE_ICE_WRITE;
        case START_ENCDEC:
            return FDE_ICE_FULL;
    }
    SLOGE("Invalid ICE state requested from cryptf_hw. Putting ICE to bypass.");
    return FDE_ICE_BYPASS;
}

int set_ice_param(int flag)
{
    return fde_fe_set_ice_state(HW_FDE_PARTITION, map_ice_flag(flag));
}


static int cryptfs_hw_clear_key(void)
{
	int32_t ret;

	ret = __cryptfs_hw_wipe_clear_key(CRYPTFS_HW_KMS_CLEAR_KEY);
	if (ret) {
		SLOGE("Error::ioctl call to wipe the encryption key failed with ret = %d, errno = %d\n",
			ret, errno);
		ret = CRYPTFS_HW_CLEAR_KEY_FAILED;
	} else {
		SLOGE("SUCCESS::ioctl call to wipe the encryption key success with ret = %d\n",
			ret);
	}
	return ret;
}

static int cryptfs_hw_update_key(unsigned char *current_hash32, unsigned char *new_hash32)
{
    if (fde_fe_update_user_info(HW_FDE_PARTITION, current_hash32, MAX_PASSWORD_LEN, new_hash32, MAX_PASSWORD_LEN) != 0)
        return CRYPTFS_HW_UPDATE_KEY_FAILED;

	return 0;
}

static unsigned char* get_tmp_passwd(const char* passwd)
{
    int passwd_len = 0;
    unsigned char * tmp_passwd = NULL;
    if(passwd) {
        tmp_passwd = (unsigned char*)malloc(MAX_PASSWORD_LEN);
        if(tmp_passwd) {
            secure_memset(tmp_passwd, 0, MAX_PASSWORD_LEN);
            passwd_len = strnlen(passwd, MAX_PASSWORD_LEN);
            memscpy(tmp_passwd, MAX_PASSWORD_LEN, passwd, passwd_len);
        } else {
            SLOGE("%s: Failed to allocate memory for tmp passwd \n", __func__);
        }
    } else {
        SLOGE("%s: Passed argument is NULL \n", __func__);
    }
    return tmp_passwd;
}

static int is_qseecom_up()
{
    int i = 0;
    char value[PROPERTY_VALUE_MAX] = {0};

    for (; i<CRYPTFS_HW_UP_CHECK_COUNT; i++) {
        property_get("sys.listeners.registered", value, "");
        if (!strncmp(value, "true", PROPERTY_VALUE_MAX))
            return 1;
        usleep(100000);
    }
    SLOGE("%s Qseecom daemon timed out", __func__);
    return 0;
}

/*
 * For NON-ICE targets, it would return 0 on success. On ICE based targets,
 * it would return key index in the ICE Key LUT
 * For GVM the ICE LUT is handled internally by FDE backend, so only the error value is used
 */
static int set_key(const char* currentpasswd, const char* passwd, const char* enc_mode, int operation)
{
    int err = -1;
    if (is_hw_disk_encryption(enc_mode) && is_qseecom_up()) {
        unsigned char* tmp_passwd = get_tmp_passwd(passwd);
        unsigned char* tmp_currentpasswd = get_tmp_passwd(currentpasswd);
        if (tmp_passwd) {
            if (operation == UPDATE_HW_DISK_ENC_KEY) {
                if (tmp_currentpasswd) {
                   err = cryptfs_hw_update_key(tmp_currentpasswd, tmp_passwd);
                   secure_memset(tmp_currentpasswd, 0, MAX_PASSWORD_LEN);
                }
            } else if (operation == SET_HW_DISK_ENC_KEY) {
                err = cryptfs_hw_create_key(tmp_passwd);
            }
            if(err < 0) {
                if(ERR_MAX_PASSWORD_ATTEMPTS == err)
                    SLOGI("Maximum wrong password attempts reached, will erase userdata\n");
            }
            secure_memset(tmp_passwd, 0, MAX_PASSWORD_LEN);
            free(tmp_passwd);
            free(tmp_currentpasswd);
        }
    }
    return err;
}

int set_hw_device_encryption_key(const char* passwd, const char* enc_mode)
{
    return set_key(NULL, passwd, enc_mode, SET_HW_DISK_ENC_KEY);
}

int update_hw_device_encryption_key(const char* oldpw, const char* newpw, const char* enc_mode)
{
    return set_key(oldpw, newpw, enc_mode, UPDATE_HW_DISK_ENC_KEY);
}

unsigned int is_hw_disk_encryption(const char* encryption_mode)
{
    int ret = 0;
    if(encryption_mode) {
        if (!strcmp(encryption_mode, "aes-xts")) {
            SLOGD("HW based disk encryption is enabled \n");
            ret = 1;
        }
    }
    return ret;
}

int is_ice_enabled(void)
{
    return 1;
}

int clear_hw_device_encryption_key()
{
    SLOGD("clear_hw_device_encryption_key()");
	if(is_qseecom_up())
		return cryptfs_hw_wipe_key();
	return -1;
}

int should_use_keymaster()
{
    /*
     * HW FDE key should be tied to keymaster
     */
    return 1;
}
