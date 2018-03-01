/*
 * Copyright (c) 2013,2016-2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/stat.h>
#include "edify/expr.h"
#include "dec.h"
#include "gpt-utils.h"

#define COPY_BUFFER_SIZE (32*1024*1024)

Value* DecryptFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    int rc = -1;

    if (argv.size() != 2)
        return ErrorAbort(state,kArgsParsingFailure, "%s() expects 2 arg, got %zu", name, argv.size());

    std::vector<std::string> args;
    if (!ReadArgs(state,  argv, &args))
        return NULL;

    const std::string& src_file = args[0];
    const std::string& dst_file = args[1];

    rc = decrypt_image(src_file.c_str(), dst_file.c_str());

    return StringValue(strdup(rc >= 0 ? "t" : ""));
}

Value* BootUpdateFn(const char* name, State* state,  const std::vector<std::unique_ptr<Expr>>& argv)
{
    int rc = 0;
    enum boot_update_stage stage;

    if (argv.size() != 1)
        return ErrorAbort(state,kArgsParsingFailure, "%s() expects 1 arg, got %zu", name, argv.size());

    std::vector<std::string> args;
    if (!ReadArgs(state,  argv, &args))
        return NULL;

    const std::string& stageStr = args[0];

    if (!strcmp(stageStr.c_str(), "main"))
        stage = UPDATE_MAIN;
    else if (!strcmp(stageStr.c_str(), "backup"))
        stage = UPDATE_BACKUP;
    else if (!strcmp(stageStr.c_str(), "finalize"))
        stage = UPDATE_FINALIZE;
    else {
        fprintf(stderr, "Unrecognized boot update stage, exitting\n");
        rc = -1;
    }

    if (!rc)
        rc = prepare_boot_update(stage);

    return StringValue(strdup(rc ? "" : "t"));
}

Value* SwapGPTFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    int rc = -1;
    const char *src_file, *dst_file;

    char** dev_list;

    if (argv.size() == 0)
        return ErrorAbort(state, "%s expects at least 1 args, got %zu", name, argv.size());

    std::vector<std::string> args;
    if (!ReadArgs(state,  argv, &args))
        return NULL;

    rc = swap_primary_and_secondary_gpt(args);

    return StringValue(strdup(rc >= 0 ? "t" : ""));
}

Value* CopyFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    int rc = 0;
    FILE* ffrom;
    FILE* fto;
    char* buf;
    size_t len;
    off_t progress = 0;
    struct stat fstat;

    if (argv.size() != 2)
        return ErrorAbort(state,kArgsParsingFailure, "%s() expects 2 arg, got %zu", name, argv.size());

    std::vector<std::string> args;
    if (!ReadArgs(state,  argv, &args))
        return NULL;

    const std::string& from = args[0];
    const std::string& to = args[1];

    printf("msm.copy from %s to %s: ", from.c_str(), to.c_str());

    /* check "from" is exists */
    if (stat(from.c_str(), &fstat)) {
        fprintf(stderr, "%s not present. Skipping\n",
                from.c_str());
        rc = -1;
        goto EXIT;
    }

    /* check "to" is exists */
    if (stat(to.c_str(), &fstat)) {
        fprintf(stderr, "%s not present. Skipping\n",
                to.c_str());
        rc = -1;
        goto EXIT;
    }

    ffrom = fopen(from.c_str(), "rb");
    if (NULL == ffrom) {
        fprintf(stderr, "fopen %s failed: %s!\n", from.c_str(), strerror(errno));
        rc = -1;
        goto EXIT;
    }

    fto = fopen(to.c_str(), "rb+");
    if (NULL == fto) {
        fprintf(stderr, "fopen %s failed: %s!\n", to.c_str(), strerror(errno));
        fclose(ffrom);
        rc = -1;
        goto EXIT;
    }

    buf = (char*)malloc(COPY_BUFFER_SIZE);
    if (NULL == buf) {
        fprintf(stderr, "msm.copy malloc failed!\n");
        rc = -1;
        fclose(ffrom);
        fclose(fto);
        goto EXIT;
    }

    do {
        len = fread(buf, 1, COPY_BUFFER_SIZE, ffrom);
        if (len) {
            if (len != fwrite(buf, 1, len, fto)) {
                fprintf(stderr, "do_copy write failed: %s!\n", strerror(errno));
                rc = -1;
                break;
            } else {
                progress += len;
                printf("%d%%->", (int)(progress*100/fstat.st_size));
            }
        }
    } while (len);

    printf("done!\n");

    free(buf);
    fclose(ffrom);
    fclose(fto);

EXIT:
    return StringValue(strdup(rc >= 0 ? "t" : ""));
}

void Register_librecovery_updater_msm() {
    RegisterFunction("msm.decrypt", DecryptFn);
    RegisterFunction("msm.boot_update", BootUpdateFn);
    RegisterFunction("msm.swap_gpt", SwapGPTFn);
    RegisterFunction("msm.copy", CopyFn);
}
