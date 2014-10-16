/*
 *
 *  Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *  Not a Contribution, Apache license notifications and license are retained
 *  for attribution purposes only.
 *
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H
#define BTM_DEF_LOCAL_NAME   "QCOM-BTD"
// Disables read remote device feature
#define BTA_SKIP_BLE_READ_REMOTE_FEAT FALSE
#define MAX_L2CAP_CHANNELS    14
// skips conn update at conn completion
#define BTA_BLE_SKIP_CONN_UPD  FALSE

#define BTA_DM_COD {0x00, BTM_COD_MAJOR_WEARABLE, BTM_COD_MINOR_WRIST_WATCH}

#define BTA_AR_INCLUDED               TRUE
#define A2D_INCLUDED                  TRUE
#define AVDT_INCLUDED                 TRUE
#define AVCT_INCLUDED                 TRUE
#define AVRC_INCLUDED                 TRUE
#define PAN_NAP_DISABLED              TRUE

#define BTA_FTS_OPS_IDLE_TO_SNIFF_DELAY_MS 1000

#define BTA_DM_PM_SNIFF_MAX      2048
#define BTA_DM_PM_SNIFF_MIN      1024
#define BTA_DM_PM_SNIFF_ATTEMPT  4
#define BTA_DM_PM_SNIFF_TIMEOUT  1

#endif
