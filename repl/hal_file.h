#pragma once

// Enumeration of all file-related error codes
typedef enum hal_file_code {
    HAL_SUCCESS,        // Operation was successful
    HAL_ERROR_INIT,     // Error during initialization
    HAL_ERROR_ALLOC,    // Memory allocation error
    HAL_ERROR_OPEN,     // Error while opening a file
    HAL_ERROR_INPUT,    // Invalid input parameter(s)
    HAL_ERROR_READ,     // Error while reading from a file
    HAL_ERROR_WRITE,    // Error while writing to a file
    HAL_ERROR_CLOSE,    // Error while closing a file
    HAL_ERROR_TEARDOWN, // Error during teardown
    HAL_ERROR_MAX,      // Maximum error code value (used for range checking)
} hal_file_code;

// Forward declaration of the implementation defined file structure
typedef struct hal_file_type hal_file;

// Structure for different file operation functions
typedef struct hal_file_operations {
    // Function: setup
    // Initialize the file handling system.
    // Returns: HAL_SUCCESS on successful initialization, or appropriate error code on failure.
    hal_file_code (*setup)();

    // Function: open
    // Open a file with the specified filename and mode.
    // Returns: HAL_SUCCESS on successful read, or appropriate error code on failure.
    hal_file_code (*open)(hal_file** file, const char* filename, const char* mode);

    // Function: read
    // Read data from the file into the provided buffer.
    // Returns: HAL_SUCCESS on successful read, or appropriate error code on failure.
    hal_file_code (*read)(hal_file* file, char* buffer, const int size);

    // Function: write
    // Write the provided message to the file.
    // Returns: HAL_SUCCESS on successful write, or appropriate error code on failure.
    hal_file_code (*write)(hal_file* file, const char* message);

    // Function: close
    // Close the file and release associated resources.
    // Returns: HAL_SUCCESS on successful close, or appropriate error code on failure.
    hal_file_code (*close)(hal_file* file);

    // Function: teardown
    // Perform necessary cleanup and teardown operations for the file handling system.
    // Returns: HAL_SUCCESS on successful teardown, or appropriate error code on failure.
    hal_file_code (*teardown)();
} hal_file_operations;

// Global variable to access file operations
extern hal_file_operations hal_file_manager;

// Maximum size of a filename supported by implementation
extern const int HAL_MAX_FILENAME_SIZE;

// Maximum number of files that can be open simultaneously by implementation
extern const int HAL_MAX_FILES_OPEN;

// End-of-File (EOF) indicator value defined by implementation
extern const int HAL_EOF;