// Copyright (c) 2015, The Linux Foundation. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of The Linux Foundation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "private/android_filesystem_config.h"

int main() {
  char path[1024];
  struct stat s;

  while (fgets(path, sizeof(path), stdin) != NULL) {
    // Remove trailing newline and terminate path
    *(path + strcspn(path, "\n")) = 0;

    if (lstat(path, &s) != 0) {
      fprintf(stderr, "Failed to lstat %s: %s\n", path, strerror(errno));
      return errno;
    }

    uint64_t capabilities;
    fs_config(path, S_ISDIR(s.st_mode), &s.st_uid, &s.st_gid, &s.st_mode, &capabilities);

    // Remove top directory from output. Leading slash is kept for mkfsubifs
    // e.g. data/local/tmp -> /local/tmp
    char *rel_path = strstr(path, "/");
    if (rel_path == NULL) {
      rel_path = path;
    }

    if (S_ISREG(s.st_mode)) {
      printf("%s\t\tf\t%o\t%d\t%d\n", rel_path, s.st_mode, s.st_uid, s.st_gid);
    }
    else if (S_ISDIR(s.st_mode)) {
      printf("%s\t\td\t%o\t%d\t%d\n", rel_path, s.st_mode, s.st_uid, s.st_gid);
    }
    else if (S_ISLNK(s.st_mode)) {
      printf("%s\t\ts\t%o\t%d\t%d\n", rel_path, s.st_mode, s.st_uid, s.st_gid);
    }
  }

  return 0;
}
