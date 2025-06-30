package satox_bindings

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/rand"
	"crypto/sha256"
	"errors"
	"io"
	"sync"
)

// SecurityManager provides security and encryption functionality.
// Usage:
//
//	manager := NewSecurityManager()
//	err := manager.Initialize()
//	key, err := manager.GenerateKey()
//	err = manager.EnableEncryption(key)
type SecurityManager struct {
	initialized       bool
	encryptionEnabled bool
	keyStore          map[string][]byte
	mu                sync.RWMutex
}

// NewSecurityManager creates a new instance of SecurityManager.
func NewSecurityManager() *SecurityManager {
	return &SecurityManager{
		keyStore: make(map[string][]byte),
	}
}

// Initialize initializes the security manager.
func (m *SecurityManager) Initialize() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.initialized {
		return errors.New("SecurityManager already initialized")
	}
	m.initialized = true
	return nil
}

// EnableEncryption enables encryption with the provided key.
func (m *SecurityManager) EnableEncryption(key []byte) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("SecurityManager not initialized")
	}
	if m.encryptionEnabled {
		return errors.New("Encryption already enabled")
	}
	if len(key) != 32 {
		return errors.New("Invalid key length (must be 32 bytes)")
	}

	m.encryptionEnabled = true
	m.keyStore["default"] = key
	return nil
}

// DisableEncryption disables encryption.
func (m *SecurityManager) DisableEncryption() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("SecurityManager not initialized")
	}
	if !m.encryptionEnabled {
		return errors.New("Encryption not enabled")
	}

	m.encryptionEnabled = false
	m.keyStore = make(map[string][]byte)
	return nil
}

// IsEncryptionEnabled returns whether encryption is enabled.
func (m *SecurityManager) IsEncryptionEnabled() bool {
	m.mu.RLock()
	defer m.mu.RUnlock()
	return m.encryptionEnabled
}

// EncryptData encrypts data using the specified key.
func (m *SecurityManager) EncryptData(data []byte, keyID string) ([]byte, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return nil, errors.New("SecurityManager not initialized")
	}
	if !m.encryptionEnabled {
		return nil, errors.New("Encryption not enabled")
	}

	key, exists := m.keyStore[keyID]
	if !exists {
		return nil, errors.New("Key not found")
	}

	// Create AES cipher
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}

	// Create GCM mode
	gcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}

	// Create nonce
	nonce := make([]byte, gcm.NonceSize())
	if _, err := io.ReadFull(rand.Reader, nonce); err != nil {
		return nil, err
	}

	// Encrypt data
	ciphertext := gcm.Seal(nonce, nonce, data, nil)
	return ciphertext, nil
}

// DecryptData decrypts data using the specified key.
func (m *SecurityManager) DecryptData(encryptedData []byte, keyID string) ([]byte, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return nil, errors.New("SecurityManager not initialized")
	}
	if !m.encryptionEnabled {
		return nil, errors.New("Encryption not enabled")
	}

	key, exists := m.keyStore[keyID]
	if !exists {
		return nil, errors.New("Key not found")
	}

	// Create AES cipher
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}

	// Create GCM mode
	gcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}

	// Extract nonce
	nonceSize := gcm.NonceSize()
	if len(encryptedData) < nonceSize {
		return nil, errors.New("Invalid encrypted data")
	}

	nonce := encryptedData[:nonceSize]
	ciphertext := encryptedData[nonceSize:]

	// Decrypt data
	plaintext, err := gcm.Open(nil, nonce, ciphertext, nil)
	if err != nil {
		return nil, err
	}

	return plaintext, nil
}

// AddKey adds a new encryption key.
func (m *SecurityManager) AddKey(keyID string, key []byte) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("SecurityManager not initialized")
	}
	if _, exists := m.keyStore[keyID]; exists {
		return errors.New("Key already exists")
	}
	if len(key) != 32 {
		return errors.New("Invalid key length (must be 32 bytes)")
	}

	m.keyStore[keyID] = key
	return nil
}

// RemoveKey removes an encryption key.
func (m *SecurityManager) RemoveKey(keyID string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("SecurityManager not initialized")
	}
	if _, exists := m.keyStore[keyID]; !exists {
		return errors.New("Key not found")
	}

	delete(m.keyStore, keyID)
	return nil
}

// GenerateKey generates a new encryption key.
func (m *SecurityManager) GenerateKey() ([]byte, error) {
	if !m.initialized {
		return nil, errors.New("SecurityManager not initialized")
	}

	// Generate 32 random bytes
	key := make([]byte, 32)
	if _, err := io.ReadFull(rand.Reader, key); err != nil {
		return nil, err
	}

	return key, nil
}

// HashData creates a SHA-256 hash of the data.
func (m *SecurityManager) HashData(data []byte) ([]byte, error) {
	if !m.initialized {
		return nil, errors.New("SecurityManager not initialized")
	}

	hash := sha256.Sum256(data)
	return hash[:], nil
}

// Shutdown shuts down the security manager.
func (m *SecurityManager) Shutdown() error {
	if m.encryptionEnabled {
		return m.DisableEncryption()
	}
	m.mu.Lock()
	m.initialized = false
	m.mu.Unlock()
	return nil
}
