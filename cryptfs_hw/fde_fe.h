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

#ifndef FDE_GVM_FE_H
#define FDE_GVM_FE_H

#include <stdint.h>

typedef enum {
    FDE_ICE_BYPASS = 0,
    FDE_ICE_WRITE = 1,
    FDE_ICE_READ = 2,
    FDE_ICE_FULL = 3
} fde_ice_state_t;

//FDE errors
typedef enum {
  FDE_SUCCESS_NEEDS_FORMAT   = 1,
  FDE_SUCCESS                = 0,
  FDE_GENERAL_ERROR          = -1,
  FDE_NOT_SUPPORTED          = -2,
  FDE_INVALID_INPUT          = -3,
  FDE_BLOCK_DEVICE_ERROR     = -4,
  FDE_PARTITION_NOT_FOUND    = -5,
  FDE_KEY_ERROR              = -6,
  FDE_SOCKET_ERROR           = -7
} fde_err_t;

//create FDE key (QSEOS_GENERATE_KEY)
fde_err_t fde_fe_set_key(const char* partition_name, uint8_t* hash, size_t hash_size);
//update key info (QSEOS_UPDATE_KEY_USERINFO)
fde_err_t fde_fe_update_user_info(const char* partition_name,
                                const uint8_t* curr_hash, size_t curr_hash_size,
                                const uint8_t* new_hash, size_t new_hash_size);
//wipe key
fde_err_t fde_fe_wipe_key(const char* partition_name);
//clear key
fde_err_t fde_fe_clear_key(const char* partition_name);
//set ICE state
fde_err_t fde_fe_set_ice_state(const char* partition_name, fde_ice_state_t state);


#endif //FDE_GVM_FE_H
