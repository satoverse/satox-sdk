package satox

/*
#cgo CFLAGS: -I${SRCDIR}/../../../include
#cgo LDFLAGS: -L${SRCDIR}/../../../build/lib -lsatox-sdk-main -lsatox-sdk-c-shared -lstdc++
#include <stdlib.h>
#include <string.h>
#include "satox_sdk.h"
*/
import "C"
import (
	"errors"
	"unsafe"
)

// IpfsManager represents an IPFS manager instance
type IpfsManager struct {
	handle unsafe.Pointer
}

// NewIpfsManager creates a new IPFS manager
func NewIpfsManager(configJSON string) (*IpfsManager, error) {
	var config *C.char
	if configJSON != "" {
		config = C.CString(configJSON)
		defer C.free(unsafe.Pointer(config))
	}

	handle := C.satox_ipfs_manager_create(config)
	if handle == nil {
		return nil, errors.New("failed to create IPFS manager")
	}

	return &IpfsManager{handle: handle}, nil
}

// Close destroys the IPFS manager
func (im *IpfsManager) Close() {
	if im.handle != nil {
		C.satox_ipfs_manager_destroy(im.handle)
		im.handle = nil
	}
}

// UploadFile uploads a file to IPFS
func (im *IpfsManager) UploadFile(filePath string) (string, error) {
	if im.handle == nil {
		return "", errors.New("IPFS manager not initialized")
	}

	cFilePath := C.CString(filePath)
	defer C.free(unsafe.Pointer(cFilePath))

	result := C.satox_ipfs_manager_upload_file(im.handle, cFilePath)
	if result == nil {
		return "", errors.New("failed to upload file")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// UploadData uploads data to IPFS
func (im *IpfsManager) UploadData(data []byte, filename string) (string, error) {
	if im.handle == nil {
		return "", errors.New("IPFS manager not initialized")
	}

	cFilename := C.CString(filename)
	defer C.free(unsafe.Pointer(cFilename))

	result := C.satox_ipfs_manager_upload_data(im.handle, (*C.uchar)(&data[0]), C.ulonglong(len(data)), cFilename)
	if result == nil {
		return "", errors.New("failed to upload data")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// DownloadFile downloads a file from IPFS
func (im *IpfsManager) DownloadFile(hash, outputPath string) error {
	if im.handle == nil {
		return errors.New("IPFS manager not initialized")
	}

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	cOutputPath := C.CString(outputPath)
	defer C.free(unsafe.Pointer(cOutputPath))

	result := C.satox_ipfs_manager_download_file(im.handle, cHash, cOutputPath)
	if result != 0 {
		return errors.New("failed to download file")
	}

	return nil
}

// GetFileInfo returns information about a file in IPFS
func (im *IpfsManager) GetFileInfo(hash string) (string, error) {
	if im.handle == nil {
		return "", errors.New("IPFS manager not initialized")
	}

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	result := C.satox_ipfs_manager_get_file_info(im.handle, cHash)
	if result == nil {
		return "", errors.New("failed to get file info")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// PinFile pins a file in IPFS
func (im *IpfsManager) PinFile(hash string) error {
	if im.handle == nil {
		return errors.New("IPFS manager not initialized")
	}

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	result := C.satox_ipfs_manager_pin_file(im.handle, cHash)
	if result != 0 {
		return errors.New("failed to pin file")
	}

	return nil
}

// UnpinFile unpins a file from IPFS
func (im *IpfsManager) UnpinFile(hash string) error {
	if im.handle == nil {
		return errors.New("IPFS manager not initialized")
	}

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	result := C.satox_ipfs_manager_unpin_file(im.handle, cHash)
	if result != 0 {
		return errors.New("failed to unpin file")
	}

	return nil
}

// GetGatewayURL returns the gateway URL for a file
func (im *IpfsManager) GetGatewayURL(hash string) (string, error) {
	if im.handle == nil {
		return "", errors.New("IPFS manager not initialized")
	}

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	result := C.satox_ipfs_manager_get_gateway_url(im.handle, cHash)
	if result == nil {
		return "", errors.New("failed to get gateway URL")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}
