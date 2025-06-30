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
	"encoding/json"
	"errors"
	"unsafe"
)

// AssetManager represents an asset manager instance
type AssetManager struct {
	handle unsafe.Pointer
}

// AssetInfo represents asset information
type AssetInfo struct {
	AssetID           string `json:"asset_id"`
	Name              string `json:"name"`
	Symbol            string `json:"symbol"`
	Description       string `json:"description"`
	Precision         uint8  `json:"precision"`
	Reissuable        bool   `json:"reissuable"`
	TotalSupply       uint64 `json:"total_supply"`
	CirculatingSupply uint64 `json:"circulating_supply"`
	IPFSHash          string `json:"ipfs_hash"`
	Owner             string `json:"owner"`
	CreationTxid      string `json:"creation_txid"`
	CreationBlock     uint64 `json:"creation_block"`
}

// AssetConfig represents asset creation configuration
type AssetConfig struct {
	Name        string `json:"name"`
	Symbol      string `json:"symbol"`
	Description string `json:"description"`
	Precision   uint8  `json:"precision"`
	Reissuable  bool   `json:"reissuable"`
	TotalSupply uint64 `json:"total_supply"`
	IPFSHash    string `json:"ipfs_hash,omitempty"`
}

// NewAssetManager creates a new asset manager
func NewAssetManager() (*AssetManager, error) {
	handle := C.satox_asset_manager_create()
	if handle == nil {
		return nil, errors.New("failed to create asset manager")
	}

	return &AssetManager{handle: handle}, nil
}

// Close destroys the asset manager
func (am *AssetManager) Close() {
	if am.handle != nil {
		C.satox_asset_manager_destroy(am.handle)
		am.handle = nil
	}
}

// CreateAsset creates a new asset
func (am *AssetManager) CreateAsset(config AssetConfig, ownerAddress string) (string, error) {
	if am.handle == nil {
		return "", errors.New("asset manager not initialized")
	}

	configJSON, err := json.Marshal(config)
	if err != nil {
		return "", err
	}

	cConfig := C.CString(string(configJSON))
	defer C.free(unsafe.Pointer(cConfig))

	cOwner := C.CString(ownerAddress)
	defer C.free(unsafe.Pointer(cOwner))

	result := C.satox_asset_manager_create_asset(am.handle, cConfig, cOwner)
	if result == nil {
		return "", errors.New("failed to create asset")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// GetAssetInfo returns information about a specific asset
func (am *AssetManager) GetAssetInfo(assetID string) (*AssetInfo, error) {
	if am.handle == nil {
		return nil, errors.New("asset manager not initialized")
	}

	cAssetID := C.CString(assetID)
	defer C.free(unsafe.Pointer(cAssetID))

	result := C.satox_asset_manager_get_asset_info(am.handle, cAssetID)
	if result == nil {
		return nil, errors.New("failed to get asset info")
	}
	defer C.free(unsafe.Pointer(result))

	var assetInfo AssetInfo
	err := json.Unmarshal([]byte(C.GoString(result)), &assetInfo)
	if err != nil {
		return nil, err
	}

	return &assetInfo, nil
}

// TransferAsset transfers assets between addresses
func (am *AssetManager) TransferAsset(assetID, fromAddress, toAddress string, amount uint64) (string, error) {
	if am.handle == nil {
		return "", errors.New("asset manager not initialized")
	}

	cAssetID := C.CString(assetID)
	defer C.free(unsafe.Pointer(cAssetID))

	cFrom := C.CString(fromAddress)
	defer C.free(unsafe.Pointer(cFrom))

	cTo := C.CString(toAddress)
	defer C.free(unsafe.Pointer(cTo))

	result := C.satox_asset_manager_transfer_asset(am.handle, cAssetID, cFrom, cTo, C.ulonglong(amount))
	if result == nil {
		return "", errors.New("failed to transfer asset")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// GetAssetBalance returns the balance of an asset for a specific address
func (am *AssetManager) GetAssetBalance(assetID, address string) (uint64, error) {
	if am.handle == nil {
		return 0, errors.New("asset manager not initialized")
	}

	cAssetID := C.CString(assetID)
	defer C.free(unsafe.Pointer(cAssetID))

	cAddress := C.CString(address)
	defer C.free(unsafe.Pointer(cAddress))

	var balance C.ulonglong
	result := C.satox_asset_manager_get_asset_balance(am.handle, cAssetID, cAddress, &balance)
	if result != 0 {
		return 0, errors.New("failed to get asset balance")
	}

	return uint64(balance), nil
}

// ReissueAsset reissues additional units of an asset
func (am *AssetManager) ReissueAsset(assetID, ownerAddress string, amount uint64) (string, error) {
	if am.handle == nil {
		return "", errors.New("asset manager not initialized")
	}

	cAssetID := C.CString(assetID)
	defer C.free(unsafe.Pointer(cAssetID))

	cOwner := C.CString(ownerAddress)
	defer C.free(unsafe.Pointer(cOwner))

	result := C.satox_asset_manager_reissue_asset(am.handle, cAssetID, cOwner, C.ulonglong(amount))
	if result == nil {
		return "", errors.New("failed to reissue asset")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// BurnAsset burns (destroys) asset units
func (am *AssetManager) BurnAsset(assetID, ownerAddress string, amount uint64) (string, error) {
	if am.handle == nil {
		return "", errors.New("asset manager not initialized")
	}

	cAssetID := C.CString(assetID)
	defer C.free(unsafe.Pointer(cAssetID))

	cOwner := C.CString(ownerAddress)
	defer C.free(unsafe.Pointer(cOwner))

	result := C.satox_asset_manager_burn_asset(am.handle, cAssetID, cOwner, C.ulonglong(amount))
	if result == nil {
		return "", errors.New("failed to burn asset")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}
