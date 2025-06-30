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

// BlockchainManager represents a blockchain manager instance
type BlockchainManager struct {
	handle unsafe.Pointer
}

// BlockInfo represents blockchain block information
type BlockInfo struct {
	Hash         string   `json:"hash"`
	Height       uint64   `json:"height"`
	Timestamp    uint64   `json:"timestamp"`
	Transactions []string `json:"transactions"`
	Difficulty   float64  `json:"difficulty"`
}

// TransactionInfo represents transaction information
type TransactionInfo struct {
	Txid          string  `json:"txid"`
	BlockHash     string  `json:"block_hash"`
	Confirmations uint64  `json:"confirmations"`
	Amount        float64 `json:"amount"`
	Fee           float64 `json:"fee"`
}

// NewBlockchainManager creates a new blockchain manager
func NewBlockchainManager(configJSON string) (*BlockchainManager, error) {
	var config *C.char
	if configJSON != "" {
		config = C.CString(configJSON)
		defer C.free(unsafe.Pointer(config))
	}

	handle := C.satox_blockchain_manager_create(config)
	if handle == nil {
		return nil, errors.New("failed to create blockchain manager")
	}

	return &BlockchainManager{handle: handle}, nil
}

// Close destroys the blockchain manager
func (bm *BlockchainManager) Close() {
	if bm.handle != nil {
		C.satox_blockchain_manager_destroy(bm.handle)
		bm.handle = nil
	}
}

// GetBlockHeight returns the current block height
func (bm *BlockchainManager) GetBlockHeight() (uint64, error) {
	if bm.handle == nil {
		return 0, errors.New("blockchain manager not initialized")
	}

	var height C.ulonglong
	result := C.satox_blockchain_manager_get_block_height(bm.handle, &height)
	if result != 0 {
		return 0, errors.New("failed to get block height")
	}

	return uint64(height), nil
}

// GetBlockInfo returns information about a specific block
func (bm *BlockchainManager) GetBlockInfo(hash string) (*BlockInfo, error) {
	if bm.handle == nil {
		return nil, errors.New("blockchain manager not initialized")
	}

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	result := C.satox_blockchain_manager_get_block_info(bm.handle, cHash)
	if result == nil {
		return nil, errors.New("failed to get block info")
	}
	defer C.free(unsafe.Pointer(result))

	var blockInfo BlockInfo
	err := json.Unmarshal([]byte(C.GoString(result)), &blockInfo)
	if err != nil {
		return nil, err
	}

	return &blockInfo, nil
}

// GetTransactionInfo returns information about a specific transaction
func (bm *BlockchainManager) GetTransactionInfo(txid string) (*TransactionInfo, error) {
	if bm.handle == nil {
		return nil, errors.New("blockchain manager not initialized")
	}

	cTxid := C.CString(txid)
	defer C.free(unsafe.Pointer(cTxid))

	result := C.satox_blockchain_manager_get_transaction_info(bm.handle, cTxid)
	if result == nil {
		return nil, errors.New("failed to get transaction info")
	}
	defer C.free(unsafe.Pointer(result))

	var txInfo TransactionInfo
	err := json.Unmarshal([]byte(C.GoString(result)), &txInfo)
	if err != nil {
		return nil, err
	}

	return &txInfo, nil
}

// GetBestBlockHash returns the hash of the best block
func (bm *BlockchainManager) GetBestBlockHash() (string, error) {
	if bm.handle == nil {
		return "", errors.New("blockchain manager not initialized")
	}

	result := C.satox_blockchain_manager_get_best_block_hash(bm.handle)
	if result == nil {
		return "", errors.New("failed to get best block hash")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// GetBlockHash returns the hash of a block at a specific height
func (bm *BlockchainManager) GetBlockHash(height uint64) (string, error) {
	if bm.handle == nil {
		return "", errors.New("blockchain manager not initialized")
	}

	result := C.satox_blockchain_manager_get_block_hash(bm.handle, C.ulonglong(height))
	if result == nil {
		return "", errors.New("failed to get block hash")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}
