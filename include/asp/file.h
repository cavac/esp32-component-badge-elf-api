// SPDX-License-Identifier: MIT

#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Open a file with a DMA-capable internal-RAM stdio buffer. For paths under
// /sd or /int, the underlying SD/flash driver can DMA directly into the
// buffer, avoiding the PSRAM bounce-copy that plain fopen() incurs. The
// buffer is also large (CONFIG_FATFS_STDIO_BUF_SIZE) so reads happen in
// fewer, bigger chunks. Reduces worst-case read latency dramatically — use
// this for time-sensitive I/O like audio streaming.
//
// When CONFIG_FATFS_USE_FASTOPEN is disabled, this is a thin wrapper around
// fopen(). Files opened with asp_fastopen() must be closed with
// asp_fastclose() so the tracked buffer (if any) is freed.
FILE* asp_fastopen(const char* path, const char* mode);

// Close a file previously opened with asp_fastopen() and free its
// associated DMA buffer (if one was allocated).
void asp_fastclose(FILE* f);

#ifdef __cplusplus
}
#endif
