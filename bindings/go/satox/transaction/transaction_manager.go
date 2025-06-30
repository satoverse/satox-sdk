package transaction

import (
	"C"
	"errors"
	"unsafe"
)

// TransactionManager handles transaction operations
type TransactionManager struct {
	initialized bool
}

// NewTransactionManager creates a new TransactionManager instance
func NewTransactionManager() *TransactionManager {
	return &TransactionManager{
		initialized: false,
	}
}

// Initialize initializes the transaction manager
func (tm *TransactionManager) Initialize() error {
	if tm.initialized {
		return nil
	}

	result := C.satox_transaction_manager_initialize()
	if result != 0 {
		return errors.New("failed to initialize transaction manager")
	}

	tm.initialized = true
	return nil
}

// Shutdown shuts down the transaction manager
func (tm *TransactionManager) Shutdown() error {
	if !tm.initialized {
		return nil
	}

	result := C.satox_transaction_manager_shutdown()
	if result != 0 {
		return errors.New("failed to shutdown transaction manager")
	}

	tm.initialized = false
	return nil
}

// CreateTransaction creates a new transaction
func (tm *TransactionManager) CreateTransaction(config *TransactionConfig) (*Transaction, error) {
	tm.ensureInitialized()

	cConfig := C.struct_TransactionConfig{
		from_address: C.CString(config.FromAddress),
		to_address:   C.CString(config.ToAddress),
		amount:       C.int64_t(config.Amount),
		fee:          C.int64_t(config.Fee),
		data:         C.CString(config.Data),
	}
	defer func() {
		C.free(unsafe.Pointer(cConfig.from_address))
		C.free(unsafe.Pointer(cConfig.to_address))
		C.free(unsafe.Pointer(cConfig.data))
	}()

	var cTx C.struct_Transaction
	result := C.satox_transaction_manager_create_transaction(&cConfig, &cTx)
	if result != 0 {
		return nil, errors.New("failed to create transaction")
	}

	tx := &Transaction{
		Hash:        C.GoString(cTx.hash),
		FromAddress: C.GoString(cTx.from_address),
		ToAddress:   C.GoString(cTx.to_address),
		Amount:      int64(cTx.amount),
		Fee:         int64(cTx.fee),
		Data:        C.GoString(cTx.data),
		Timestamp:   int64(cTx.timestamp),
		Status:      C.GoString(cTx.status),
	}

	return tx, nil
}

// SignTransaction signs a transaction
func (tm *TransactionManager) SignTransaction(tx *Transaction, privateKey string) error {
	tm.ensureInitialized()

	cHash := C.CString(tx.Hash)
	cPrivateKey := C.CString(privateKey)
	defer func() {
		C.free(unsafe.Pointer(cHash))
		C.free(unsafe.Pointer(cPrivateKey))
	}()

	result := C.satox_transaction_manager_sign_transaction(cHash, cPrivateKey)
	if result != 0 {
		return errors.New("failed to sign transaction")
	}

	return nil
}

// BroadcastTransaction broadcasts a transaction to the network
func (tm *TransactionManager) BroadcastTransaction(tx *Transaction) error {
	tm.ensureInitialized()

	cHash := C.CString(tx.Hash)
	defer C.free(unsafe.Pointer(cHash))

	result := C.satox_transaction_manager_broadcast_transaction(cHash)
	if result != 0 {
		return errors.New("failed to broadcast transaction")
	}

	return nil
}

// GetTransaction gets a transaction by its hash
func (tm *TransactionManager) GetTransaction(hash string) (*Transaction, error) {
	tm.ensureInitialized()

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	var cTx C.struct_Transaction
	result := C.satox_transaction_manager_get_transaction(cHash, &cTx)
	if result != 0 {
		return nil, errors.New("failed to get transaction")
	}

	tx := &Transaction{
		Hash:        C.GoString(cTx.hash),
		FromAddress: C.GoString(cTx.from_address),
		ToAddress:   C.GoString(cTx.to_address),
		Amount:      int64(cTx.amount),
		Fee:         int64(cTx.fee),
		Data:        C.GoString(cTx.data),
		Timestamp:   int64(cTx.timestamp),
		Status:      C.GoString(cTx.status),
	}

	return tx, nil
}

// GetTransactionStatus gets the status of a transaction
func (tm *TransactionManager) GetTransactionStatus(hash string) (string, error) {
	tm.ensureInitialized()

	cHash := C.CString(hash)
	defer C.free(unsafe.Pointer(cHash))

	var cStatus *C.char
	result := C.satox_transaction_manager_get_transaction_status(cHash, &cStatus)
	if result != 0 {
		return "", errors.New("failed to get transaction status")
	}
	defer C.free(unsafe.Pointer(cStatus))

	return C.GoString(cStatus), nil
}

func (tm *TransactionManager) ensureInitialized() {
	if !tm.initialized {
		panic("transaction manager not initialized")
	}
}

// TransactionConfig represents transaction configuration
type TransactionConfig struct {
	FromAddress string
	ToAddress   string
	Amount      int64
	Fee         int64
	Data        string
}

// Transaction represents a blockchain transaction
type Transaction struct {
	Hash        string
	FromAddress string
	ToAddress   string
	Amount      int64
	Fee         int64
	Data        string
	Timestamp   int64
	Status      string
}
