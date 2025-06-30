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

// NftManager represents an NFT manager instance
type NftManager struct {
	handle unsafe.Pointer
}

// NewNftManager creates a new NFT manager
func NewNftManager() (*NftManager, error) {
	handle := C.satox_nft_manager_create()
	if handle == nil {
		return nil, errors.New("failed to create NFT manager")
	}

	return &NftManager{handle: handle}, nil
}

// Close destroys the NFT manager
func (nm *NftManager) Close() {
	if nm.handle != nil {
		C.satox_nft_manager_destroy(nm.handle)
		nm.handle = nil
	}
}

// CreateNFT creates a new NFT
func (nm *NftManager) CreateNFT(assetID, metadataJSON, ownerAddress string) (string, error) {
	if nm.handle == nil {
		return "", errors.New("NFT manager not initialized")
	}

	cAssetID := C.CString(assetID)
	defer C.free(unsafe.Pointer(cAssetID))

	cMetadata := C.CString(metadataJSON)
	defer C.free(unsafe.Pointer(cMetadata))

	cOwner := C.CString(ownerAddress)
	defer C.free(unsafe.Pointer(cOwner))

	result := C.satox_nft_manager_create_nft(nm.handle, cAssetID, cMetadata, cOwner)
	if result == nil {
		return "", errors.New("failed to create NFT")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// GetNFTInfo returns information about a specific NFT
func (nm *NftManager) GetNFTInfo(nftID string) (string, error) {
	if nm.handle == nil {
		return "", errors.New("NFT manager not initialized")
	}

	cNftID := C.CString(nftID)
	defer C.free(unsafe.Pointer(cNftID))

	result := C.satox_nft_manager_get_nft_info(nm.handle, cNftID)
	if result == nil {
		return "", errors.New("failed to get NFT info")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// TransferNFT transfers an NFT between addresses
func (nm *NftManager) TransferNFT(nftID, fromAddress, toAddress string) (string, error) {
	if nm.handle == nil {
		return "", errors.New("NFT manager not initialized")
	}

	cNftID := C.CString(nftID)
	defer C.free(unsafe.Pointer(cNftID))

	cFrom := C.CString(fromAddress)
	defer C.free(unsafe.Pointer(cFrom))

	cTo := C.CString(toAddress)
	defer C.free(unsafe.Pointer(cTo))

	result := C.satox_nft_manager_transfer_nft(nm.handle, cNftID, cFrom, cTo)
	if result == nil {
		return "", errors.New("failed to transfer NFT")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// GetNFTsByOwner returns all NFTs owned by a specific address
func (nm *NftManager) GetNFTsByOwner(address string) (string, error) {
	if nm.handle == nil {
		return "", errors.New("NFT manager not initialized")
	}

	cAddress := C.CString(address)
	defer C.free(unsafe.Pointer(cAddress))

	result := C.satox_nft_manager_get_nfts_by_owner(nm.handle, cAddress)
	if result == nil {
		return "", errors.New("failed to get NFTs by owner")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// UpdateMetadata updates the metadata of an NFT
func (nm *NftManager) UpdateMetadata(nftID, metadataJSON, ownerAddress string) (string, error) {
	if nm.handle == nil {
		return "", errors.New("NFT manager not initialized")
	}

	cNftID := C.CString(nftID)
	defer C.free(unsafe.Pointer(cNftID))

	cMetadata := C.CString(metadataJSON)
	defer C.free(unsafe.Pointer(cMetadata))

	cOwner := C.CString(ownerAddress)
	defer C.free(unsafe.Pointer(cOwner))

	result := C.satox_nft_manager_update_metadata(nm.handle, cNftID, cMetadata, cOwner)
	if result == nil {
		return "", errors.New("failed to update NFT metadata")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}
