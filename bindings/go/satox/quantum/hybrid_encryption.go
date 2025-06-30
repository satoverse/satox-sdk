package quantum

// #cgo LDFLAGS: -L${SRCDIR}/../../target/release -lsatox_quantum
// #include <stdlib.h>
// #include <stdint.h>
// #include "hybrid_encryption.h"
import "C"
import (
	"errors"
	"unsafe"
)

// HybridEncryption provides hybrid encryption capabilities.
type HybridEncryption struct {
	initialized bool
}

// NewHybridEncryption creates a new instance of HybridEncryption.
func NewHybridEncryption() *HybridEncryption {
	return &HybridEncryption{
		initialized: false,
	}
}

// Initialize initializes the hybrid encryption system.
func (he *HybridEncryption) Initialize() error {
	if he.initialized {
		return nil
	}

	result := C.satox_hybrid_encryption_initialize()
	if result != 0 {
		return errors.New("failed to initialize hybrid encryption")
	}

	he.initialized = true
	return nil
}

// Shutdown shuts down the hybrid encryption system.
func (he *HybridEncryption) Shutdown() error {
	if !he.initialized {
		return nil
	}

	result := C.satox_hybrid_encryption_shutdown()
	if result != 0 {
		return errors.New("failed to shutdown hybrid encryption")
	}

	he.initialized = false
	return nil
}

// GenerateKeyPair generates a new key pair.
func (he *HybridEncryption) GenerateKeyPair() ([]byte, []byte, error) {
	if !he.initialized {
		return nil, nil, errors.New("hybrid encryption is not initialized")
	}

	var publicKeyPtr *C.uint8_t
	var privateKeyPtr *C.uint8_t
	var publicKeyLen C.size_t
	var privateKeyLen C.size_t

	result := C.satox_hybrid_encryption_generate_key_pair(
		&publicKeyPtr,
		&publicKeyLen,
		&privateKeyPtr,
		&privateKeyLen,
	)

	if result != 0 {
		return nil, nil, errors.New("failed to generate key pair")
	}

	publicKey := C.GoBytes(unsafe.Pointer(publicKeyPtr), C.int(publicKeyLen))
	privateKey := C.GoBytes(unsafe.Pointer(privateKeyPtr), C.int(privateKeyLen))

	C.satox_hybrid_encryption_free_buffer(unsafe.Pointer(publicKeyPtr))
	C.satox_hybrid_encryption_free_buffer(unsafe.Pointer(privateKeyPtr))

	return publicKey, privateKey, nil
}

// Encrypt encrypts data using the provided public key.
func (he *HybridEncryption) Encrypt(publicKey, data []byte) ([]byte, error) {
	if !he.initialized {
		return nil, errors.New("hybrid encryption is not initialized")
	}

	if len(publicKey) == 0 || len(data) == 0 {
		return nil, errors.New("invalid input: public key and data must not be empty")
	}

	var encryptedDataPtr *C.uint8_t
	var encryptedDataLen C.size_t

	result := C.satox_hybrid_encryption_encrypt(
		(*C.uint8_t)(unsafe.Pointer(&publicKey[0])),
		C.size_t(len(publicKey)),
		(*C.uint8_t)(unsafe.Pointer(&data[0])),
		C.size_t(len(data)),
		&encryptedDataPtr,
		&encryptedDataLen,
	)

	if result != 0 {
		return nil, errors.New("failed to encrypt data")
	}

	encryptedData := C.GoBytes(unsafe.Pointer(encryptedDataPtr), C.int(encryptedDataLen))
	C.satox_hybrid_encryption_free_buffer(unsafe.Pointer(encryptedDataPtr))

	return encryptedData, nil
}

// Decrypt decrypts data using the provided private key.
func (he *HybridEncryption) Decrypt(privateKey, encryptedData []byte) ([]byte, error) {
	if !he.initialized {
		return nil, errors.New("hybrid encryption is not initialized")
	}

	if len(privateKey) == 0 || len(encryptedData) == 0 {
		return nil, errors.New("invalid input: private key and encrypted data must not be empty")
	}

	var decryptedDataPtr *C.uint8_t
	var decryptedDataLen C.size_t

	result := C.satox_hybrid_encryption_decrypt(
		(*C.uint8_t)(unsafe.Pointer(&privateKey[0])),
		C.size_t(len(privateKey)),
		(*C.uint8_t)(unsafe.Pointer(&encryptedData[0])),
		C.size_t(len(encryptedData)),
		&decryptedDataPtr,
		&decryptedDataLen,
	)

	if result != 0 {
		return nil, errors.New("failed to decrypt data")
	}

	decryptedData := C.GoBytes(unsafe.Pointer(decryptedDataPtr), C.int(decryptedDataLen))
	C.satox_hybrid_encryption_free_buffer(unsafe.Pointer(decryptedDataPtr))

	return decryptedData, nil
}

// GetSessionKey retrieves the current session key.
func (he *HybridEncryption) GetSessionKey() ([]byte, error) {
	if !he.initialized {
		return nil, errors.New("hybrid encryption is not initialized")
	}

	var sessionKeyPtr *C.uint8_t
	var sessionKeyLen C.size_t

	result := C.satox_hybrid_encryption_get_session_key(
		&sessionKeyPtr,
		&sessionKeyLen,
	)

	if result != 0 {
		return nil, errors.New("failed to get session key")
	}

	sessionKey := C.GoBytes(unsafe.Pointer(sessionKeyPtr), C.int(sessionKeyLen))
	C.satox_hybrid_encryption_free_buffer(unsafe.Pointer(sessionKeyPtr))

	return sessionKey, nil
}

// RotateSessionKey rotates the session key.
func (he *HybridEncryption) RotateSessionKey() error {
	if !he.initialized {
		return errors.New("hybrid encryption is not initialized")
	}

	result := C.satox_hybrid_encryption_rotate_session_key()
	if result != 0 {
		return errors.New("failed to rotate session key")
	}

	return nil
}

// GetVersion returns the version of the hybrid encryption system.
func (he *HybridEncryption) GetVersion() (string, error) {
	if !he.initialized {
		return "", errors.New("hybrid encryption is not initialized")
	}

	var versionPtr *C.char

	result := C.satox_hybrid_encryption_get_version(&versionPtr)
	if result != 0 {
		return "", errors.New("failed to get version")
	}

	version := C.GoString(versionPtr)
	C.satox_hybrid_encryption_free_buffer(unsafe.Pointer(versionPtr))

	return version, nil
}

// IsInitialized returns whether the hybrid encryption system is initialized.
func (he *HybridEncryption) IsInitialized() bool {
	return he.initialized
}
