package quantum

// #cgo LDFLAGS: -L${SRCDIR}/../../target/release -lsatox_quantum
// #include <stdlib.h>
// #include <stdint.h>
// #include "post_quantum_algorithms.h"
import "C"
import (
	"errors"
	"unsafe"
)

// AlgorithmInfo represents information about a post-quantum algorithm.
type AlgorithmInfo struct {
	Name          string
	SecurityLevel int
	KeySize       int
	SignatureSize int
	IsRecommended bool
	Description   string
}

// PostQuantumAlgorithms provides access to post-quantum cryptographic algorithms.
type PostQuantumAlgorithms struct {
	initialized bool
}

// NewPostQuantumAlgorithms creates a new instance of PostQuantumAlgorithms.
func NewPostQuantumAlgorithms() *PostQuantumAlgorithms {
	return &PostQuantumAlgorithms{
		initialized: false,
	}
}

// Initialize initializes the post-quantum algorithms system.
func (pqa *PostQuantumAlgorithms) Initialize() error {
	if pqa.initialized {
		return nil
	}

	result := C.satox_post_quantum_algorithms_initialize()
	if result != 0 {
		return errors.New("failed to initialize post-quantum algorithms")
	}

	pqa.initialized = true
	return nil
}

// Shutdown shuts down the post-quantum algorithms system.
func (pqa *PostQuantumAlgorithms) Shutdown() error {
	if !pqa.initialized {
		return nil
	}

	result := C.satox_post_quantum_algorithms_shutdown()
	if result != 0 {
		return errors.New("failed to shutdown post-quantum algorithms")
	}

	pqa.initialized = false
	return nil
}

// GenerateKeyPair generates a new key pair for the specified algorithm.
func (pqa *PostQuantumAlgorithms) GenerateKeyPair(algorithmName string) ([]byte, []byte, error) {
	if !pqa.initialized {
		return nil, nil, errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return nil, nil, errors.New("invalid input: algorithm name must not be empty")
	}

	var publicKeyPtr *C.uint8_t
	var privateKeyPtr *C.uint8_t
	var publicKeyLen C.size_t
	var privateKeyLen C.size_t

	result := C.satox_post_quantum_algorithms_generate_key_pair(
		C.CString(algorithmName),
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

	C.satox_post_quantum_algorithms_free_buffer(unsafe.Pointer(publicKeyPtr))
	C.satox_post_quantum_algorithms_free_buffer(unsafe.Pointer(privateKeyPtr))

	return publicKey, privateKey, nil
}

// Sign signs data using the provided private key.
func (pqa *PostQuantumAlgorithms) Sign(privateKey, data []byte) ([]byte, error) {
	if !pqa.initialized {
		return nil, errors.New("post-quantum algorithms is not initialized")
	}

	if len(privateKey) == 0 || len(data) == 0 {
		return nil, errors.New("invalid input: private key and data must not be empty")
	}

	var signaturePtr *C.uint8_t
	var signatureLen C.size_t

	result := C.satox_post_quantum_algorithms_sign(
		(*C.uint8_t)(unsafe.Pointer(&privateKey[0])),
		C.size_t(len(privateKey)),
		(*C.uint8_t)(unsafe.Pointer(&data[0])),
		C.size_t(len(data)),
		&signaturePtr,
		&signatureLen,
	)

	if result != 0 {
		return nil, errors.New("failed to sign data")
	}

	signature := C.GoBytes(unsafe.Pointer(signaturePtr), C.int(signatureLen))
	C.satox_post_quantum_algorithms_free_buffer(unsafe.Pointer(signaturePtr))

	return signature, nil
}

// Verify verifies a signature using the provided public key.
func (pqa *PostQuantumAlgorithms) Verify(publicKey, data, signature []byte) (bool, error) {
	if !pqa.initialized {
		return false, errors.New("post-quantum algorithms is not initialized")
	}

	if len(publicKey) == 0 || len(data) == 0 || len(signature) == 0 {
		return false, errors.New("invalid input: public key, data, and signature must not be empty")
	}

	var isValid C.int

	result := C.satox_post_quantum_algorithms_verify(
		(*C.uint8_t)(unsafe.Pointer(&publicKey[0])),
		C.size_t(len(publicKey)),
		(*C.uint8_t)(unsafe.Pointer(&data[0])),
		C.size_t(len(data)),
		(*C.uint8_t)(unsafe.Pointer(&signature[0])),
		C.size_t(len(signature)),
		&isValid,
	)

	if result != 0 {
		return false, errors.New("failed to verify signature")
	}

	return isValid != 0, nil
}

// Encrypt encrypts data using the provided public key.
func (pqa *PostQuantumAlgorithms) Encrypt(publicKey, data []byte) ([]byte, error) {
	if !pqa.initialized {
		return nil, errors.New("post-quantum algorithms is not initialized")
	}

	if len(publicKey) == 0 || len(data) == 0 {
		return nil, errors.New("invalid input: public key and data must not be empty")
	}

	var encryptedDataPtr *C.uint8_t
	var encryptedDataLen C.size_t

	result := C.satox_post_quantum_algorithms_encrypt(
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
	C.satox_post_quantum_algorithms_free_buffer(unsafe.Pointer(encryptedDataPtr))

	return encryptedData, nil
}

// Decrypt decrypts data using the provided private key.
func (pqa *PostQuantumAlgorithms) Decrypt(privateKey, encryptedData []byte) ([]byte, error) {
	if !pqa.initialized {
		return nil, errors.New("post-quantum algorithms is not initialized")
	}

	if len(privateKey) == 0 || len(encryptedData) == 0 {
		return nil, errors.New("invalid input: private key and encrypted data must not be empty")
	}

	var decryptedDataPtr *C.uint8_t
	var decryptedDataLen C.size_t

	result := C.satox_post_quantum_algorithms_decrypt(
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
	C.satox_post_quantum_algorithms_free_buffer(unsafe.Pointer(decryptedDataPtr))

	return decryptedData, nil
}

// GetAlgorithmInfo retrieves information about a specific algorithm.
func (pqa *PostQuantumAlgorithms) GetAlgorithmInfo(algorithmName string) (AlgorithmInfo, error) {
	if !pqa.initialized {
		return AlgorithmInfo{}, errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return AlgorithmInfo{}, errors.New("invalid input: algorithm name must not be empty")
	}

	var cInfo C.struct_satox_algorithm_info

	result := C.satox_post_quantum_algorithms_get_algorithm_info(
		C.CString(algorithmName),
		&cInfo,
	)

	if result != 0 {
		return AlgorithmInfo{}, errors.New("failed to get algorithm info")
	}

	info := AlgorithmInfo{
		Name:          C.GoString(cInfo.name),
		SecurityLevel: int(cInfo.security_level),
		KeySize:       int(cInfo.key_size),
		SignatureSize: int(cInfo.signature_size),
		IsRecommended: cInfo.is_recommended != 0,
		Description:   C.GoString(cInfo.description),
	}

	return info, nil
}

// GetAvailableAlgorithms returns a list of available algorithms.
func (pqa *PostQuantumAlgorithms) GetAvailableAlgorithms() ([]string, error) {
	if !pqa.initialized {
		return nil, errors.New("post-quantum algorithms is not initialized")
	}

	var algorithmsPtr **C.char
	var algorithmsLen C.size_t

	result := C.satox_post_quantum_algorithms_get_available_algorithms(
		&algorithmsPtr,
		&algorithmsLen,
	)

	if result != 0 {
		return nil, errors.New("failed to get available algorithms")
	}

	algorithms := make([]string, algorithmsLen)
	for i := 0; i < int(algorithmsLen); i++ {
		algorithms[i] = C.GoString(*(**C.char)(unsafe.Pointer(uintptr(unsafe.Pointer(algorithmsPtr)) + uintptr(i)*unsafe.Sizeof(*algorithmsPtr))))
	}

	return algorithms, nil
}

// GetRecommendedAlgorithms returns a list of recommended algorithms.
func (pqa *PostQuantumAlgorithms) GetRecommendedAlgorithms() ([]string, error) {
	if !pqa.initialized {
		return nil, errors.New("post-quantum algorithms is not initialized")
	}

	var algorithmsPtr **C.char
	var algorithmsLen C.size_t

	result := C.satox_post_quantum_algorithms_get_recommended_algorithms(
		&algorithmsPtr,
		&algorithmsLen,
	)

	if result != 0 {
		return nil, errors.New("failed to get recommended algorithms")
	}

	algorithms := make([]string, algorithmsLen)
	for i := 0; i < int(algorithmsLen); i++ {
		algorithms[i] = C.GoString(*(**C.char)(unsafe.Pointer(uintptr(unsafe.Pointer(algorithmsPtr)) + uintptr(i)*unsafe.Sizeof(*algorithmsPtr))))
	}

	return algorithms, nil
}

// IsAlgorithmAvailable checks if an algorithm is available.
func (pqa *PostQuantumAlgorithms) IsAlgorithmAvailable(algorithmName string) (bool, error) {
	if !pqa.initialized {
		return false, errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return false, errors.New("invalid input: algorithm name must not be empty")
	}

	var isAvailable C.int

	result := C.satox_post_quantum_algorithms_is_algorithm_available(
		C.CString(algorithmName),
		&isAvailable,
	)

	if result != 0 {
		return false, errors.New("failed to check algorithm availability")
	}

	return isAvailable != 0, nil
}

// IsAlgorithmRecommended checks if an algorithm is recommended.
func (pqa *PostQuantumAlgorithms) IsAlgorithmRecommended(algorithmName string) (bool, error) {
	if !pqa.initialized {
		return false, errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return false, errors.New("invalid input: algorithm name must not be empty")
	}

	var isRecommended C.int

	result := C.satox_post_quantum_algorithms_is_algorithm_recommended(
		C.CString(algorithmName),
		&isRecommended,
	)

	if result != 0 {
		return false, errors.New("failed to check algorithm recommendation")
	}

	return isRecommended != 0, nil
}

// GetAlgorithmSecurityLevel returns the security level of an algorithm.
func (pqa *PostQuantumAlgorithms) GetAlgorithmSecurityLevel(algorithmName string) (int, error) {
	if !pqa.initialized {
		return 0, errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return 0, errors.New("invalid input: algorithm name must not be empty")
	}

	var securityLevel C.int

	result := C.satox_post_quantum_algorithms_get_algorithm_security_level(
		C.CString(algorithmName),
		&securityLevel,
	)

	if result != 0 {
		return 0, errors.New("failed to get algorithm security level")
	}

	return int(securityLevel), nil
}

// GetAlgorithmKeySize returns the key size of an algorithm.
func (pqa *PostQuantumAlgorithms) GetAlgorithmKeySize(algorithmName string) (int, error) {
	if !pqa.initialized {
		return 0, errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return 0, errors.New("invalid input: algorithm name must not be empty")
	}

	var keySize C.int

	result := C.satox_post_quantum_algorithms_get_algorithm_key_size(
		C.CString(algorithmName),
		&keySize,
	)

	if result != 0 {
		return 0, errors.New("failed to get algorithm key size")
	}

	return int(keySize), nil
}

// GetAlgorithmSignatureSize returns the signature size of an algorithm.
func (pqa *PostQuantumAlgorithms) GetAlgorithmSignatureSize(algorithmName string) (int, error) {
	if !pqa.initialized {
		return 0, errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return 0, errors.New("invalid input: algorithm name must not be empty")
	}

	var signatureSize C.int

	result := C.satox_post_quantum_algorithms_get_algorithm_signature_size(
		C.CString(algorithmName),
		&signatureSize,
	)

	if result != 0 {
		return 0, errors.New("failed to get algorithm signature size")
	}

	return int(signatureSize), nil
}

// GetDefaultAlgorithm returns the default algorithm.
func (pqa *PostQuantumAlgorithms) GetDefaultAlgorithm() (string, error) {
	if !pqa.initialized {
		return "", errors.New("post-quantum algorithms is not initialized")
	}

	var algorithmPtr *C.char

	result := C.satox_post_quantum_algorithms_get_default_algorithm(&algorithmPtr)
	if result != 0 {
		return "", errors.New("failed to get default algorithm")
	}

	algorithm := C.GoString(algorithmPtr)
	C.satox_post_quantum_algorithms_free_buffer(unsafe.Pointer(algorithmPtr))

	return algorithm, nil
}

// SetDefaultAlgorithm sets the default algorithm.
func (pqa *PostQuantumAlgorithms) SetDefaultAlgorithm(algorithmName string) error {
	if !pqa.initialized {
		return errors.New("post-quantum algorithms is not initialized")
	}

	if algorithmName == "" {
		return errors.New("invalid input: algorithm name must not be empty")
	}

	result := C.satox_post_quantum_algorithms_set_default_algorithm(C.CString(algorithmName))
	if result != 0 {
		return errors.New("failed to set default algorithm")
	}

	return nil
}

// IsInitialized returns whether the post-quantum algorithms system is initialized.
func (pqa *PostQuantumAlgorithms) IsInitialized() bool {
	return pqa.initialized
}
