package satox

/*
#cgo CFLAGS: -I${SRCDIR}/../../../include
#cgo LDFLAGS: -L${SRCDIR}/../../../build/lib -lsatox-sdk-main -lsatox-sdk-c-shared -lstdc++
#include <stdlib.h>
#include <string.h>
#include "satox_sdk.h"

// C wrapper functions for better error handling
char* satox_sdk_initialize_c() {
    int result = satox_sdk_initialize();
    if (result == 0) {
        return strdup("success");
    } else {
        return strdup("error");
    }
}

void satox_sdk_shutdown_c() {
    satox_sdk_shutdown();
}

char* satox_sdk_get_version_c() {
    const char* version = satox_sdk_get_version();
    if (version != NULL) {
        return strdup(version);
    } else {
        return strdup("unknown");
    }
}
*/
import "C"
import (
	"errors"
	"unsafe"
)

// Initialize initializes the Satox SDK
func Initialize() error {
	result := C.satox_sdk_initialize_c()
	defer C.free(unsafe.Pointer(result))

	if C.GoString(result) == "success" {
		return nil
	}
	return errors.New("failed to initialize Satox SDK")
}

// Shutdown shuts down the Satox SDK
func Shutdown() {
	C.satox_sdk_shutdown_c()
}

// GetVersion returns the SDK version
func GetVersion() string {
	version := C.satox_sdk_get_version_c()
	defer C.free(unsafe.Pointer(version))
	return C.GoString(version)
}
