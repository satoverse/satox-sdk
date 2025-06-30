package quantum

import (
	"testing"
)

func TestNewQuantumManager(t *testing.T) {
	manager := NewQuantumManager()
	if manager == nil {
		t.Fatal("NewQuantumManager returned nil")
	}
}

func TestGenerateKeyPair(t *testing.T) {
	manager := NewQuantumManager()
	keyPair, err := manager.GenerateKeyPair()
	if err != nil {
		t.Fatalf("GenerateKeyPair failed: %v", err)
	}
	if keyPair == nil {
		t.Fatal("GenerateKeyPair returned nil key pair")
	}
	if len(keyPair.PublicKey) != 32 {
		t.Errorf("Expected public key length 32, got %d", len(keyPair.PublicKey))
	}
	if len(keyPair.PrivateKey) != 32 {
		t.Errorf("Expected private key length 32, got %d", len(keyPair.PrivateKey))
	}
}

func TestEncryptDecrypt(t *testing.T) {
	manager := NewQuantumManager()
	keyPair, err := manager.GenerateKeyPair()
	if err != nil {
		t.Fatalf("GenerateKeyPair failed: %v", err)
	}

	data := []byte("Hello, Quantum World!")
	encrypted, err := manager.Encrypt(keyPair.PublicKey, data)
	if err != nil {
		t.Fatalf("Encrypt failed: %v", err)
	}
	if len(encrypted) != len(data) {
		t.Errorf("Expected encrypted length %d, got %d", len(data), len(encrypted))
	}

	decrypted, err := manager.Decrypt(keyPair.PrivateKey, encrypted)
	if err != nil {
		t.Fatalf("Decrypt failed: %v", err)
	}
	if string(decrypted) != string(data) {
		t.Errorf("Expected decrypted data %s, got %s", data, decrypted)
	}
}

func TestSignVerify(t *testing.T) {
	manager := NewQuantumManager()
	keyPair, err := manager.GenerateKeyPair()
	if err != nil {
		t.Fatalf("GenerateKeyPair failed: %v", err)
	}

	data := []byte("Hello, Quantum World!")
	signature, err := manager.Sign(keyPair.PrivateKey, data)
	if err != nil {
		t.Fatalf("Sign failed: %v", err)
	}
	if len(signature) != 64 {
		t.Errorf("Expected signature length 64, got %d", len(signature))
	}

	valid, err := manager.Verify(keyPair.PublicKey, data, signature)
	if err != nil {
		t.Fatalf("Verify failed: %v", err)
	}
	if !valid {
		t.Error("Signature verification failed")
	}
}

func TestGenerateRandomBytes(t *testing.T) {
	manager := NewQuantumManager()
	length := 32
	bytes, err := manager.GenerateRandomBytes(length)
	if err != nil {
		t.Fatalf("GenerateRandomBytes failed: %v", err)
	}
	if len(bytes) != length {
		t.Errorf("Expected length %d, got %d", length, len(bytes))
	}
}

func TestGenerateEntropy(t *testing.T) {
	manager := NewQuantumManager()
	length := 32
	entropy, err := manager.GenerateEntropy(length)
	if err != nil {
		t.Fatalf("GenerateEntropy failed: %v", err)
	}
	if len(entropy) != length {
		t.Errorf("Expected length %d, got %d", length, len(entropy))
	}
}

func TestBase64Encoding(t *testing.T) {
	manager := NewQuantumManager()
	data := []byte("Hello, Quantum World!")
	encoded := manager.EncodeBase64(data)
	decoded, err := manager.DecodeBase64(encoded)
	if err != nil {
		t.Fatalf("DecodeBase64 failed: %v", err)
	}
	if string(decoded) != string(data) {
		t.Errorf("Expected decoded data %s, got %s", data, decoded)
	}
}

func TestInvalidInputs(t *testing.T) {
	manager := NewQuantumManager()

	// Test empty public key
	_, err := manager.Encrypt([]byte{}, []byte("test"))
	if err == nil {
		t.Error("Expected error for empty public key")
	}

	// Test empty private key
	_, err = manager.Decrypt([]byte{}, []byte("test"))
	if err == nil {
		t.Error("Expected error for empty private key")
	}

	// Test empty data
	_, err = manager.Sign([]byte("test"), []byte{})
	if err == nil {
		t.Error("Expected error for empty data")
	}

	// Test empty signature
	_, err = manager.Verify([]byte("test"), []byte("test"), []byte{})
	if err == nil {
		t.Error("Expected error for empty signature")
	}

	// Test invalid base64
	_, err = manager.DecodeBase64("invalid base64")
	if err == nil {
		t.Error("Expected error for invalid base64")
	}
}
