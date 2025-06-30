package quantum

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestNewPostQuantumAlgorithms(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	assert.NotNil(t, pqa)
	assert.False(t, pqa.IsInitialized())
}

func TestInitialize(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)
	assert.True(t, pqa.IsInitialized())

	// Test double initialization
	err = pqa.Initialize()
	assert.NoError(t, err)
	assert.True(t, pqa.IsInitialized())
}

func TestShutdown(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	err = pqa.Shutdown()
	assert.NoError(t, err)
	assert.False(t, pqa.IsInitialized())

	// Test shutdown when not initialized
	err = pqa.Shutdown()
	assert.NoError(t, err)
}

func TestGetAvailableAlgorithms(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	algorithms, err := pqa.GetAvailableAlgorithms()
	assert.NoError(t, err)
	assert.NotEmpty(t, algorithms)

	// Verify known algorithms are present
	knownAlgorithms := []string{"CRYSTALS-Kyber", "NTRU", "SABER", "CRYSTALS-Dilithium", "FALCON", "SPHINCS+"}
	for _, algo := range knownAlgorithms {
		assert.Contains(t, algorithms, algo)
	}
}

func TestGetAlgorithmInfo(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with valid algorithm
	info, err := pqa.GetAlgorithmInfo("CRYSTALS-Kyber")
	assert.NoError(t, err)
	assert.Equal(t, "CRYSTALS-Kyber", info.Name)
	assert.Greater(t, info.SecurityLevel, 0)
	assert.Greater(t, info.KeySize, 0)
	assert.Greater(t, info.SignatureSize, 0)
	assert.NotEmpty(t, info.Description)

	// Test with invalid algorithm
	_, err = pqa.GetAlgorithmInfo("InvalidAlgorithm")
	assert.Error(t, err)
}

func TestIsAlgorithmAvailable(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with valid algorithm
	available, err := pqa.IsAlgorithmAvailable("CRYSTALS-Kyber")
	assert.NoError(t, err)
	assert.True(t, available)

	// Test with invalid algorithm
	available, err = pqa.IsAlgorithmAvailable("InvalidAlgorithm")
	assert.NoError(t, err)
	assert.False(t, available)
}

func TestIsAlgorithmRecommended(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with recommended algorithm
	recommended, err := pqa.IsAlgorithmRecommended("CRYSTALS-Kyber")
	assert.NoError(t, err)
	assert.True(t, recommended)

	// Test with non-recommended algorithm
	recommended, err = pqa.IsAlgorithmRecommended("InvalidAlgorithm")
	assert.NoError(t, err)
	assert.False(t, recommended)
}

func TestGenerateKeyPairForAlgorithm(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with valid algorithm
	publicKey, privateKey, err := pqa.GenerateKeyPair("CRYSTALS-Kyber")
	assert.NoError(t, err)
	assert.NotNil(t, publicKey)
	assert.NotNil(t, privateKey)

	// Test with invalid algorithm
	_, _, err = pqa.GenerateKeyPair("InvalidAlgorithm")
	assert.Error(t, err)
}

func TestSignAndVerify(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Generate key pair
	publicKey, privateKey, err := pqa.GenerateKeyPair("CRYSTALS-Dilithium")
	assert.NoError(t, err)

	// Test data
	data := []byte("Test message for signing")

	// Sign data
	signature, err := pqa.Sign(privateKey, data)
	assert.NoError(t, err)
	assert.NotNil(t, signature)

	// Verify signature
	valid, err := pqa.Verify(publicKey, data, signature)
	assert.NoError(t, err)
	assert.True(t, valid)

	// Test with modified data
	modifiedData := []byte("Modified test message")
	valid, err = pqa.Verify(publicKey, modifiedData, signature)
	assert.NoError(t, err)
	assert.False(t, valid)
}

func TestEncryptAndDecrypt(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Generate key pair
	publicKey, privateKey, err := pqa.GenerateKeyPair("CRYSTALS-Kyber")
	assert.NoError(t, err)

	// Test data
	data := []byte("Test message for encryption")

	// Encrypt data
	encrypted, err := pqa.Encrypt(publicKey, data)
	assert.NoError(t, err)
	assert.NotNil(t, encrypted)

	// Decrypt data
	decrypted, err := pqa.Decrypt(privateKey, encrypted)
	assert.NoError(t, err)
	assert.Equal(t, data, decrypted)
}

func TestGetAlgorithmSecurityLevel(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with valid algorithm
	level, err := pqa.GetAlgorithmSecurityLevel("CRYSTALS-Kyber")
	assert.NoError(t, err)
	assert.Greater(t, level, 0)

	// Test with invalid algorithm
	_, err = pqa.GetAlgorithmSecurityLevel("InvalidAlgorithm")
	assert.Error(t, err)
}

func TestGetAlgorithmKeySize(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with valid algorithm
	size, err := pqa.GetAlgorithmKeySize("CRYSTALS-Kyber")
	assert.NoError(t, err)
	assert.Greater(t, size, 0)

	// Test with invalid algorithm
	_, err = pqa.GetAlgorithmKeySize("InvalidAlgorithm")
	assert.Error(t, err)
}

func TestGetAlgorithmSignatureSize(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with valid algorithm
	size, err := pqa.GetAlgorithmSignatureSize("CRYSTALS-Dilithium")
	assert.NoError(t, err)
	assert.Greater(t, size, 0)

	// Test with invalid algorithm
	_, err = pqa.GetAlgorithmSignatureSize("InvalidAlgorithm")
	assert.Error(t, err)
}

func TestGetDefaultAlgorithm(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Get default algorithm
	algorithm, err := pqa.GetDefaultAlgorithm()
	assert.NoError(t, err)
	assert.NotEmpty(t, algorithm)

	// Verify it's a valid algorithm
	available, err := pqa.IsAlgorithmAvailable(algorithm)
	assert.NoError(t, err)
	assert.True(t, available)
}

func TestSetDefaultAlgorithm(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	// Test with valid algorithm
	err = pqa.SetDefaultAlgorithm("CRYSTALS-Kyber")
	assert.NoError(t, err)

	// Verify the change
	algorithm, err := pqa.GetDefaultAlgorithm()
	assert.NoError(t, err)
	assert.Equal(t, "CRYSTALS-Kyber", algorithm)

	// Test with invalid algorithm
	err = pqa.SetDefaultAlgorithm("InvalidAlgorithm")
	assert.Error(t, err)
}

func TestOperationsAfterShutdown(t *testing.T) {
	pqa := NewPostQuantumAlgorithms()
	err := pqa.Initialize()
	assert.NoError(t, err)

	err = pqa.Shutdown()
	assert.NoError(t, err)

	// Test all operations after shutdown
	_, err = pqa.GetAvailableAlgorithms()
	assert.Error(t, err)

	_, err = pqa.GetAlgorithmInfo("CRYSTALS-Kyber")
	assert.Error(t, err)

	_, err = pqa.IsAlgorithmAvailable("CRYSTALS-Kyber")
	assert.Error(t, err)

	_, err = pqa.IsAlgorithmRecommended("CRYSTALS-Kyber")
	assert.Error(t, err)

	_, _, err = pqa.GenerateKeyPair("CRYSTALS-Kyber")
	assert.Error(t, err)

	_, err = pqa.GetDefaultAlgorithm()
	assert.Error(t, err)

	err = pqa.SetDefaultAlgorithm("CRYSTALS-Kyber")
	assert.Error(t, err)
}
