#include "hal_file.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct hal_file_type {
    FILE* fp;
} hal_file;

// Initialization logic, if needed
hal_file_code setup() {
    return HAL_SUCCESS;
}

hal_file_code open(hal_file** file, const char* filename, const char* mode) {
    if (!file || !filename || !mode) {
        return HAL_ERROR_INPUT;
    }

    *file = (hal_file*)malloc(sizeof(hal_file));
    if (!(*file)) {
        return HAL_ERROR_ALLOC;
    }

    (*file)->fp = fopen(filename, mode);
    if (!(*file)->fp) {
        free(*file);
        return HAL_ERROR_OPEN;
    }

    return HAL_SUCCESS;
}

hal_file_code read(hal_file* file, char* buffer, const int size) {
    if (!file || !file->fp || !buffer || size <= 0) {
        return HAL_ERROR_INPUT;
    }

    if (fgets(buffer, size, file->fp) == NULL) {
        return HAL_ERROR_READ;
    }

    return HAL_SUCCESS;
}

hal_file_code write(hal_file* file, const char* message) {
    if (!file || !file->fp || !message) {
        return HAL_ERROR_INPUT;
    }

    if (fputs(message, file->fp) == EOF) {
        return HAL_ERROR_WRITE;
    }

    return HAL_SUCCESS;
}

hal_file_code close(hal_file* file) {
    if (!file || !file->fp) {
        return HAL_ERROR_INPUT;
    }

    if (fclose(file->fp) != 0) {
        return HAL_ERROR_CLOSE;
    }

    free(file);
    return HAL_SUCCESS;
}

// deletion logic, if needed
hal_file_code teardown() {
    return HAL_SUCCESS;
}

// Expose into global variable
hal_file_operations hal_file_manager = {
    .setup = setup,
    .open = open,
    .read = read,
    .write = write,
    .close = close,
    .teardown = teardown
};

const int HAL_MAX_FILENAME_SIZE = FILENAME_MAX;
const int HAL_MAX_FILES_OPEN = FOPEN_MAX;
const int HAL_EOF = EOF;