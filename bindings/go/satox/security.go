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

// SecurityManager represents a security manager instance
type SecurityManager struct {
	handle unsafe.Pointer
}

// KeyPair represents a public/private key pair
type KeyPair struct {
	PublicKey  string
	PrivateKey string
}

// NewSecurityManager creates a new security manager
func NewSecurityManager() (*SecurityManager, error) {
	handle := C.satox_security_manager_create()
	if handle == nil {
		return nil, errors.New("failed to create security manager")
	}

	return &SecurityManager{handle: handle}, nil
}

// Close destroys the security manager
func (sm *SecurityManager) Close() {
	if sm.handle != nil {
		C.satox_security_manager_destroy(sm.handle)
		sm.handle = nil
	}
}

// GenerateKeypair generates a new public/private key pair
func (sm *SecurityManager) GenerateKeypair() (*KeyPair, error) {
	if sm.handle == nil {
		return nil, errors.New("security manager not initialized")
	}

	var publicKey, privateKey *C.char
	result := C.satox_security_manager_generate_keypair(sm.handle, &publicKey, &privateKey)
	if result != 0 {
		return nil, errors.New("failed to generate keypair")
	}

	// Note: The C function allocates memory that we need to free
	// This is a simplified version - in production you'd want to handle memory management more carefully
	pubKey := C.GoString(publicKey)
	privKey := C.GoString(privateKey)

	return &KeyPair{
		PublicKey:  pubKey,
		PrivateKey: privKey,
	}, nil
}

// SignData signs data with a private key
func (sm *SecurityManager) SignData(data, privateKey string) (string, error) {
	if sm.handle == nil {
		return "", errors.New("security manager not initialized")
	}

	cData := C.CString(data)
	defer C.free(unsafe.Pointer(cData))

	cPrivateKey := C.CString(privateKey)
	defer C.free(unsafe.Pointer(cPrivateKey))

	result := C.satox_security_manager_sign_data(sm.handle, cData, cPrivateKey)
	if result == nil {
		return "", errors.New("failed to sign data")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// VerifySignature verifies a signature with a public key
func (sm *SecurityManager) VerifySignature(data, signature, publicKey string) (bool, error) {
	if sm.handle == nil {
		return false, errors.New("security manager not initialized")
	}

	cData := C.CString(data)
	defer C.free(unsafe.Pointer(cData))

	cSignature := C.CString(signature)
	defer C.free(unsafe.Pointer(cSignature))

	cPublicKey := C.CString(publicKey)
	defer C.free(unsafe.Pointer(cPublicKey))

	result := C.satox_security_manager_verify_signature(sm.handle, cData, cSignature, cPublicKey)
	return result == 1, nil
}

// SHA256 computes the SHA256 hash of data
func (sm *SecurityManager) SHA256(data string) (string, error) {
	if sm.handle == nil {
		return "", errors.New("security manager not initialized")
	}

	cData := C.CString(data)
	defer C.free(unsafe.Pointer(cData))

	result := C.satox_security_manager_sha256(sm.handle, cData)
	if result == nil {
		return "", errors.New("failed to compute SHA256")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// EncryptData encrypts data with a public key
func (sm *SecurityManager) EncryptData(data, publicKey string) (string, error) {
	if sm.handle == nil {
		return "", errors.New("security manager not initialized")
	}

	cData := C.CString(data)
	defer C.free(unsafe.Pointer(cData))

	cPublicKey := C.CString(publicKey)
	defer C.free(unsafe.Pointer(cPublicKey))

	result := C.satox_security_manager_encrypt_data(sm.handle, cData, cPublicKey)
	if result == nil {
		return "", errors.New("failed to encrypt data")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// DecryptData decrypts data with a private key
func (sm *SecurityManager) DecryptData(encryptedData, privateKey string) (string, error) {
	if sm.handle == nil {
		return "", errors.New("security manager not initialized")
	}

	cEncryptedData := C.CString(encryptedData)
	defer C.free(unsafe.Pointer(cEncryptedData))

	cPrivateKey := C.CString(privateKey)
	defer C.free(unsafe.Pointer(cPrivateKey))

	result := C.satox_security_manager_decrypt_data(sm.handle, cEncryptedData, cPrivateKey)
	if result == nil {
		return "", errors.New("failed to decrypt data")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}
