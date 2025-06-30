package security

import (
	"testing"
)

func TestNewSecurityManager(t *testing.T) {
	manager := NewSecurityManager()
	if manager == nil {
		t.Fatal("NewSecurityManager returned nil")
	}
}

func TestCreatePolicy(t *testing.T) {
	manager := NewSecurityManager()

	policy, err := manager.CreatePolicy(
		"test-policy",
		"Test security policy",
		[]string{"rule1", "rule2"},
	)
	if err != nil {
		t.Fatalf("CreatePolicy failed: %v", err)
	}
	if policy == nil {
		t.Fatal("CreatePolicy returned nil policy")
	}
	if policy.ID == "" {
		t.Error("Policy ID is empty")
	}
	if policy.Name != "test-policy" {
		t.Errorf("Expected name 'test-policy', got '%s'", policy.Name)
	}
	if policy.Description != "Test security policy" {
		t.Errorf("Expected description 'Test security policy', got '%s'", policy.Description)
	}
	if len(policy.Rules) != 2 {
		t.Errorf("Expected 2 rules, got %d", len(policy.Rules))
	}
	if policy.CreatedAt.IsZero() {
		t.Error("CreatedAt is zero")
	}
	if policy.UpdatedAt.IsZero() {
		t.Error("UpdatedAt is zero")
	}
}

func TestGetPolicy(t *testing.T) {
	manager := NewSecurityManager()

	policy, err := manager.CreatePolicy(
		"test-policy",
		"Test security policy",
		[]string{"rule1", "rule2"},
	)
	if err != nil {
		t.Fatalf("CreatePolicy failed: %v", err)
	}

	retrieved, err := manager.GetPolicy(policy.ID)
	if err != nil {
		t.Fatalf("GetPolicy failed: %v", err)
	}
	if retrieved == nil {
		t.Fatal("GetPolicy returned nil policy")
	}
	if retrieved.ID != policy.ID {
		t.Errorf("Expected ID %s, got %s", policy.ID, retrieved.ID)
	}
}

func TestGetPolicyNotFound(t *testing.T) {
	manager := NewSecurityManager()

	_, err := manager.GetPolicy("nonexistent")
	if err == nil {
		t.Error("Expected error for nonexistent policy")
	}
}

func TestListPolicies(t *testing.T) {
	manager := NewSecurityManager()

	policy1, err := manager.CreatePolicy(
		"test-policy-1",
		"Test security policy 1",
		[]string{"rule1"},
	)
	if err != nil {
		t.Fatalf("CreatePolicy failed: %v", err)
	}

	policy2, err := manager.CreatePolicy(
		"test-policy-2",
		"Test security policy 2",
		[]string{"rule2"},
	)
	if err != nil {
		t.Fatalf("CreatePolicy failed: %v", err)
	}

	policies := manager.ListPolicies()
	if len(policies) != 2 {
		t.Errorf("Expected 2 policies, got %d", len(policies))
	}

	found1 := false
	found2 := false
	for _, p := range policies {
		if p.ID == policy1.ID {
			found1 = true
		}
		if p.ID == policy2.ID {
			found2 = true
		}
	}

	if !found1 {
		t.Error("Policy 1 not found in list")
	}
	if !found2 {
		t.Error("Policy 2 not found in list")
	}
}

func TestUpdatePolicy(t *testing.T) {
	manager := NewSecurityManager()

	policy, err := manager.CreatePolicy(
		"test-policy",
		"Test security policy",
		[]string{"rule1"},
	)
	if err != nil {
		t.Fatalf("CreatePolicy failed: %v", err)
	}

	err = manager.UpdatePolicy(
		policy.ID,
		"updated-policy",
		"Updated security policy",
		[]string{"rule1", "rule2"},
	)
	if err != nil {
		t.Fatalf("UpdatePolicy failed: %v", err)
	}

	retrieved, err := manager.GetPolicy(policy.ID)
	if err != nil {
		t.Fatalf("GetPolicy failed: %v", err)
	}
	if retrieved.Name != "updated-policy" {
		t.Errorf("Expected name 'updated-policy', got '%s'", retrieved.Name)
	}
	if retrieved.Description != "Updated security policy" {
		t.Errorf("Expected description 'Updated security policy', got '%s'", retrieved.Description)
	}
	if len(retrieved.Rules) != 2 {
		t.Errorf("Expected 2 rules, got %d", len(retrieved.Rules))
	}
}

func TestDeletePolicy(t *testing.T) {
	manager := NewSecurityManager()

	policy, err := manager.CreatePolicy(
		"test-policy",
		"Test security policy",
		[]string{"rule1"},
	)
	if err != nil {
		t.Fatalf("CreatePolicy failed: %v", err)
	}

	err = manager.DeletePolicy(policy.ID)
	if err != nil {
		t.Fatalf("DeletePolicy failed: %v", err)
	}

	_, err = manager.GetPolicy(policy.ID)
	if err == nil {
		t.Error("Expected error for deleted policy")
	}
}

func TestLogAuditEvent(t *testing.T) {
	manager := NewSecurityManager()

	err := manager.LogAuditEvent(
		"test-event",
		"Test audit event",
		[]byte("test-data"),
	)
	if err != nil {
		t.Fatalf("LogAuditEvent failed: %v", err)
	}

	log := manager.GetAuditLog()
	if len(log) != 1 {
		t.Errorf("Expected 1 audit event, got %d", len(log))
	}

	event := log[0]
	if event.ID == "" {
		t.Error("Event ID is empty")
	}
	if event.Type != "test-event" {
		t.Errorf("Expected type 'test-event', got '%s'", event.Type)
	}
	if event.Message != "Test audit event" {
		t.Errorf("Expected message 'Test audit event', got '%s'", event.Message)
	}
	if string(event.Data) != "test-data" {
		t.Errorf("Expected data 'test-data', got '%s'", event.Data)
	}
	if event.Timestamp.IsZero() {
		t.Error("Timestamp is zero")
	}
}

func TestGenerateRandomBytes(t *testing.T) {
	manager := NewSecurityManager()

	bytes, err := manager.GenerateRandomBytes(32)
	if err != nil {
		t.Fatalf("GenerateRandomBytes failed: %v", err)
	}
	if len(bytes) != 32 {
		t.Errorf("Expected 32 bytes, got %d", len(bytes))
	}
}

func TestEncodeDecodeBase64(t *testing.T) {
	manager := NewSecurityManager()

	data := []byte("Hello, World!")
	encoded := manager.EncodeBase64(data)
	decoded, err := manager.DecodeBase64(encoded)
	if err != nil {
		t.Fatalf("DecodeBase64 failed: %v", err)
	}
	if string(decoded) != string(data) {
		t.Errorf("Expected '%s', got '%s'", data, decoded)
	}
}

func TestGenerateKeyPair(t *testing.T) {
	manager := NewSecurityManager()

	privateKey, publicKey, err := manager.GenerateKeyPair()
	if err != nil {
		t.Fatalf("GenerateKeyPair failed: %v", err)
	}
	if len(privateKey) != 32 {
		t.Errorf("Expected private key length 32, got %d", len(privateKey))
	}
	if len(publicKey) != 32 {
		t.Errorf("Expected public key length 32, got %d", len(publicKey))
	}
}

func TestEncryptDecrypt(t *testing.T) {
	manager := NewSecurityManager()

	privateKey, publicKey, err := manager.GenerateKeyPair()
	if err != nil {
		t.Fatalf("GenerateKeyPair failed: %v", err)
	}

	data := []byte("Hello, World!")
	encrypted, err := manager.Encrypt(publicKey, data)
	if err != nil {
		t.Fatalf("Encrypt failed: %v", err)
	}

	decrypted, err := manager.Decrypt(privateKey, encrypted)
	if err != nil {
		t.Fatalf("Decrypt failed: %v", err)
	}
	if string(decrypted) != string(data) {
		t.Errorf("Expected '%s', got '%s'", data, decrypted)
	}
}

func TestSignVerify(t *testing.T) {
	manager := NewSecurityManager()

	privateKey, publicKey, err := manager.GenerateKeyPair()
	if err != nil {
		t.Fatalf("GenerateKeyPair failed: %v", err)
	}

	data := []byte("Hello, World!")
	signature, err := manager.Sign(privateKey, data)
	if err != nil {
		t.Fatalf("Sign failed: %v", err)
	}
	if len(signature) != 64 {
		t.Errorf("Expected signature length 64, got %d", len(signature))
	}

	valid, err := manager.Verify(publicKey, data, signature)
	if err != nil {
		t.Fatalf("Verify failed: %v", err)
	}
	if !valid {
		t.Error("Expected valid signature")
	}
}

func TestHashPassword(t *testing.T) {
	manager := NewSecurityManager()

	hash, err := manager.HashPassword("password123")
	if err != nil {
		t.Fatalf("HashPassword failed: %v", err)
	}
	if hash == "" {
		t.Error("Hash is empty")
	}
}

func TestVerifyPassword(t *testing.T) {
	manager := NewSecurityManager()

	hash, err := manager.HashPassword("password123")
	if err != nil {
		t.Fatalf("HashPassword failed: %v", err)
	}

	valid, err := manager.VerifyPassword("password123", hash)
	if err != nil {
		t.Fatalf("VerifyPassword failed: %v", err)
	}
	if !valid {
		t.Error("Expected valid password")
	}
}

func TestGenerateVerifyToken(t *testing.T) {
	manager := NewSecurityManager()

	claims := map[string]interface{}{
		"user_id": 123,
		"role":    "admin",
	}

	token, err := manager.GenerateToken(claims)
	if err != nil {
		t.Fatalf("GenerateToken failed: %v", err)
	}
	if token == "" {
		t.Error("Token is empty")
	}

	verifiedClaims, err := manager.VerifyToken(token)
	if err != nil {
		t.Fatalf("VerifyToken failed: %v", err)
	}
	if verifiedClaims == nil {
		t.Error("Verified claims is nil")
	}
}
