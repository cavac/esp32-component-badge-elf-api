// SPDX-License-Identifier: MIT

#include "asp/file.h"
#include <string.h>
#include "sdkconfig.h"

#ifdef CONFIG_FATFS_USE_FASTOPEN

#include <stdlib.h>
#include "esp_heap_caps.h"

// Independent buffer-tracking table from the launcher's main/fastopen.c so
// plugin-side opens don't share state with launcher-side opens. Sized by the
// same Kconfig value the launcher uses.
typedef struct {
    FILE* file;
    void* buffer;
} fast_file_entry_t;

static fast_file_entry_t fast_file_table[CONFIG_FATFS_MAX_FILES_OPEN] = {0};

static bool path_needs_fast_io(const char* path) {
    return (strncmp(path, "/sd", 3) == 0) || (strncmp(path, "/int", 4) == 0);
}

FILE* asp_fastopen(const char* path, const char* mode) {
    FILE* f = fopen(path, mode);
    if (f == NULL) return NULL;

    // Only allocate DMA buffer for /sd and /int paths
    if (path_needs_fast_io(path)) {
        void* buf = heap_caps_malloc(CONFIG_FATFS_STDIO_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
        if (buf != NULL) {
            setvbuf(f, buf, _IOFBF, CONFIG_FATFS_STDIO_BUF_SIZE);
            // Track the buffer so we can free it on close
            for (int i = 0; i < CONFIG_FATFS_MAX_FILES_OPEN; i++) {
                if (fast_file_table[i].file == NULL) {
                    fast_file_table[i].file   = f;
                    fast_file_table[i].buffer = buf;
                    break;
                }
            }
        }
    }
    return f;
}

void asp_fastclose(FILE* f) {
    if (f == NULL) return;

    // Find and free the tracked buffer
    for (int i = 0; i < CONFIG_FATFS_MAX_FILES_OPEN; i++) {
        if (fast_file_table[i].file == f) {
            fclose(f);
            free(fast_file_table[i].buffer);
            fast_file_table[i].file   = NULL;
            fast_file_table[i].buffer = NULL;
            return;
        }
    }
    // Not tracked (either not a fast path or buffer allocation failed),
    // just close.
    fclose(f);
}

#else  // !CONFIG_FATFS_USE_FASTOPEN

// Pass-through implementation when fast I/O is disabled.
FILE* asp_fastopen(const char* path, const char* mode) {
    return fopen(path, mode);
}

void asp_fastclose(FILE* f) {
    if (f != NULL) {
        fclose(f);
    }
}

#endif  // CONFIG_FATFS_USE_FASTOPEN
