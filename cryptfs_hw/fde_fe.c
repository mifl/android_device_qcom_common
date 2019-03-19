/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//logging support
#include "cutils/log.h"
#undef LOG_TAG
#define LOG_TAG "fde_fe"
#define FDE_LOGE SLOGE
#define FDE_LOGI SLOGI
#define FDE_LOGD SLOGD

#include "habmm.h"
#include "fde_fe.h"

#define MAX_PASS_LEN 32
#define MAX_PARTITION_LEN   (36*sizeof(uint16_t))

typedef enum {
    FDE_CMD_SET_KEY,
    FDE_CMD_UPDATE_PASSWORD ,
    FDE_CMD_CLEAR_KEY,
    FDE_CMD_WIPE_KEY,
    FDE_SET_ICE,
} fde_cmd_t;

typedef struct{
    char partition[MAX_PARTITION_LEN];
    //new fields start here for compatibility
    fde_cmd_t cmd;
    uint8_t password[MAX_PASS_LEN];
    uint8_t new_password[MAX_PASS_LEN];
    int32_t ice_flag;
} fde_request_t;

//HAB channel for GVM FDE
#define HAB_CHANNEL (901)
#define HAB_OK (0)

//// HAB Communication
fde_err_t hab_send_to_fde_be(const fde_request_t* fde_req)
{
    int32_t handle, res;
    fde_err_t retval = FDE_SUCCESS;

    if (fde_req == NULL) {
        FDE_LOGE("Error: FDE request parameter is NULL");
        return FDE_INVALID_INPUT;
    }

    if ( HAB_OK != (res = habmm_socket_open ( &handle , HAB_CHANNEL , 0 , 0 ))){
        FDE_LOGE("habmm_socket_open(%u) failed, rc =%d", HAB_CHANNEL, res);
        return FDE_SOCKET_ERROR;
    }
    FDE_LOGI("habmm_socket_open(%u) returned successfully",HAB_CHANNEL);

    do {
        if (HAB_OK != (res = habmm_socket_send(handle,
                (void*)fde_req, sizeof(*fde_req), 0)))
        {
            FDE_LOGE("habmm_socket_send failed, rc=%d", res);
            retval = FDE_SOCKET_ERROR;
            break;
        }
        FDE_LOGD("habmm_socket_send(%u) returned successfully", HAB_CHANNEL);
        fde_err_t status = FDE_GENERAL_ERROR;
        uint32_t status_size = 0;
        do {
            status_size = sizeof(status);
            res = habmm_socket_recv(handle, &status, &status_size, -1, 0 );
        } while (-EINTR == res);
        if (HAB_OK != res) {
            FDE_LOGE("habmm_socket_recv failed, rc=%d", res);
            retval = FDE_SOCKET_ERROR;
            break;
        }
        FDE_LOGD("habmm_socket_recv(%u) returned successfully", HAB_CHANNEL);
        if ( status_size != sizeof(status)) {
            FDE_LOGE("Expected %zd got %d bytes\n", sizeof( status ) ,status_size ) ;
            retval = FDE_GENERAL_ERROR;
            break;
        }
        retval = status;
    } while (0);

    habmm_socket_close (handle);
    return retval;
}

static size_t memscpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
    size_t min_size = (dst_size < src_size) ? dst_size : src_size;
    memcpy(dst, src, min_size);
    return min_size;
}

//create FDE key (QSEOS_GENERATE_KEY)
fde_err_t fde_fe_set_key(const char* partition_name, uint8_t* hash, size_t hash_size)
{

    if (partition_name == NULL || hash == NULL) {
        FDE_LOGE("Error: function parameter is NULL");
        return FDE_INVALID_INPUT;
    }

    if (hash_size != MAX_PASS_LEN) {
        FDE_LOGE("Error: only %d byte HASH size is supported", MAX_PASS_LEN);
        return FDE_INVALID_INPUT;
    }

    fde_request_t req = {0};
    req.cmd = FDE_CMD_SET_KEY;
    strlcpy(req.partition, partition_name, sizeof(req.partition));
    memscpy(req.password, sizeof(req.password), hash, hash_size);

    fde_err_t status = hab_send_to_fde_be(&req);
    return status;
}


//update key info (QSEOS_UPDATE_KEY_USERINFO)
fde_err_t fde_fe_update_user_info(const char* partition_name,
                                const uint8_t* curr_hash, size_t curr_hash_size,
                                const uint8_t* new_hash, size_t new_hash_size)
{
    if (partition_name == NULL || curr_hash == NULL || new_hash == NULL) {
        FDE_LOGE("Error: function parameter is NULL");
        return FDE_INVALID_INPUT;
    }

    if (curr_hash_size != MAX_PASS_LEN || new_hash_size != MAX_PASS_LEN) {
        FDE_LOGE("Error: only %d byte HASH size is supported", MAX_PASS_LEN);
        return FDE_INVALID_INPUT;
    }

    fde_request_t req = {0};
    req.cmd = FDE_CMD_UPDATE_PASSWORD;
    strlcpy(req.partition, partition_name, sizeof(req.partition));
    memscpy(req.password, sizeof(req.password), curr_hash, curr_hash_size);
    memscpy(req.new_password, sizeof(req.new_password), new_hash, new_hash_size);

    fde_err_t status = hab_send_to_fde_be(&req);
    return status;
}


//wipe key
fde_err_t fde_fe_wipe_key(const char* partition_name)
{
    if (partition_name == NULL) {
        FDE_LOGE("Error: function parameter is NULL");
        return FDE_INVALID_INPUT;
    }

    fde_request_t req = {0};
    req.cmd = FDE_CMD_WIPE_KEY;
    strlcpy(req.partition, partition_name, sizeof(req.partition));

    fde_err_t status = hab_send_to_fde_be(&req);
    return status;
}

//wipe key
fde_err_t fde_fe_clear_key(const char* partition_name)
{
    if (partition_name == NULL) {
        FDE_LOGE("Error: function parameter is NULL");
        return FDE_INVALID_INPUT;
    }

    fde_request_t req = {0};
    req.cmd = FDE_CMD_CLEAR_KEY;
    strlcpy(req.partition, partition_name, sizeof(req.partition));

    fde_err_t status = hab_send_to_fde_be(&req);
    return status;
}

//set ICE state
fde_err_t fde_fe_set_ice_state(const char* partition_name, fde_ice_state_t state)
{
    if (partition_name == NULL) {
        FDE_LOGE("Error: function parameter is NULL");
        return FDE_INVALID_INPUT;
    }

    fde_request_t req = {0};
    req.cmd = FDE_SET_ICE;
    strlcpy(req.partition, partition_name, sizeof(req.partition));
    req.ice_flag = state;

    fde_err_t status = hab_send_to_fde_be(&req);
    return status;
}
