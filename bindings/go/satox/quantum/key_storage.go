package quantum

// #cgo LDFLAGS: -L${SRCDIR}/../../target/release -lsatox_quantum
// #include <stdlib.h>
// #include <stdint.h>
// #include "key_storage.h"
import "C"
import (
	"errors"
	"time"
	"unsafe"
)

// KeyMetadata represents metadata for a stored key.
type KeyMetadata struct {
	Algorithm    string
	CreationTime time.Time
	Expiration   time.Time
	AccessLevels []string
	Tags         []string
}

// KeyStorage provides key storage capabilities.
type KeyStorage struct {
	initialized bool
}

// NewKeyStorage creates a new instance of KeyStorage.
func NewKeyStorage() *KeyStorage {
	return &KeyStorage{
		initialized: false,
	}
}

// Initialize initializes the key storage system.
func (ks *KeyStorage) Initialize() error {
	if ks.initialized {
		return nil
	}

	result := C.satox_key_storage_initialize()
	if result != 0 {
		return errors.New("failed to initialize key storage")
	}

	ks.initialized = true
	return nil
}

// Shutdown shuts down the key storage system.
func (ks *KeyStorage) Shutdown() error {
	if !ks.initialized {
		return nil
	}

	result := C.satox_key_storage_shutdown()
	if result != 0 {
		return errors.New("failed to shutdown key storage")
	}

	ks.initialized = false
	return nil
}

// StoreKey stores a key with the specified ID and metadata.
func (ks *KeyStorage) StoreKey(keyID string, key []byte, metadata KeyMetadata) error {
	if !ks.initialized {
		return errors.New("key storage is not initialized")
	}

	if keyID == "" || len(key) == 0 {
		return errors.New("invalid input: key ID and key must not be empty")
	}

	// Convert metadata to C struct
	cMetadata := C.struct_satox_key_metadata{
		algorithm:     C.CString(metadata.Algorithm),
		creation_time: C.int64_t(metadata.CreationTime.Unix()),
		expiration:    C.int64_t(metadata.Expiration.Unix()),
	}

	// Convert access levels to C array
	cAccessLevels := make([]*C.char, len(metadata.AccessLevels))
	for i, level := range metadata.AccessLevels {
		cAccessLevels[i] = C.CString(level)
	}
	defer func() {
		for _, level := range cAccessLevels {
			C.free(unsafe.Pointer(level))
		}
	}()

	// Convert tags to C array
	cTags := make([]*C.char, len(metadata.Tags))
	for i, tag := range metadata.Tags {
		cTags[i] = C.CString(tag)
	}
	defer func() {
		for _, tag := range cTags {
			C.free(unsafe.Pointer(tag))
		}
	}()

	result := C.satox_key_storage_store_key(
		C.CString(keyID),
		(*C.uint8_t)(unsafe.Pointer(&key[0])),
		C.size_t(len(key)),
		&cMetadata,
		(**C.char)(unsafe.Pointer(&cAccessLevels[0])),
		C.size_t(len(cAccessLevels)),
		(**C.char)(unsafe.Pointer(&cTags[0])),
		C.size_t(len(cTags)),
	)

	if result != 0 {
		return errors.New("failed to store key")
	}

	return nil
}

// RetrieveKey retrieves a key by its ID.
func (ks *KeyStorage) RetrieveKey(keyID string) ([]byte, error) {
	if !ks.initialized {
		return nil, errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return nil, errors.New("invalid input: key ID must not be empty")
	}

	var keyPtr *C.uint8_t
	var keyLen C.size_t

	result := C.satox_key_storage_retrieve_key(
		C.CString(keyID),
		&keyPtr,
		&keyLen,
	)

	if result != 0 {
		return nil, errors.New("failed to retrieve key")
	}

	key := C.GoBytes(unsafe.Pointer(keyPtr), C.int(keyLen))
	C.satox_key_storage_free_buffer(unsafe.Pointer(keyPtr))

	return key, nil
}

// DeleteKey deletes a key by its ID.
func (ks *KeyStorage) DeleteKey(keyID string) error {
	if !ks.initialized {
		return errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return errors.New("invalid input: key ID must not be empty")
	}

	result := C.satox_key_storage_delete_key(C.CString(keyID))
	if result != 0 {
		return errors.New("failed to delete key")
	}

	return nil
}

// UpdateKey updates a key's metadata.
func (ks *KeyStorage) UpdateKey(keyID string, metadata KeyMetadata) error {
	if !ks.initialized {
		return errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return errors.New("invalid input: key ID must not be empty")
	}

	// Convert metadata to C struct
	cMetadata := C.struct_satox_key_metadata{
		algorithm:     C.CString(metadata.Algorithm),
		creation_time: C.int64_t(metadata.CreationTime.Unix()),
		expiration:    C.int64_t(metadata.Expiration.Unix()),
	}

	// Convert access levels to C array
	cAccessLevels := make([]*C.char, len(metadata.AccessLevels))
	for i, level := range metadata.AccessLevels {
		cAccessLevels[i] = C.CString(level)
	}
	defer func() {
		for _, level := range cAccessLevels {
			C.free(unsafe.Pointer(level))
		}
	}()

	// Convert tags to C array
	cTags := make([]*C.char, len(metadata.Tags))
	for i, tag := range metadata.Tags {
		cTags[i] = C.CString(tag)
	}
	defer func() {
		for _, tag := range cTags {
			C.free(unsafe.Pointer(tag))
		}
	}()

	result := C.satox_key_storage_update_key(
		C.CString(keyID),
		&cMetadata,
		(**C.char)(unsafe.Pointer(&cAccessLevels[0])),
		C.size_t(len(cAccessLevels)),
		(**C.char)(unsafe.Pointer(&cTags[0])),
		C.size_t(len(cTags)),
	)

	if result != 0 {
		return errors.New("failed to update key")
	}

	return nil
}

// GetKeyMetadata retrieves a key's metadata.
func (ks *KeyStorage) GetKeyMetadata(keyID string) (KeyMetadata, error) {
	if !ks.initialized {
		return KeyMetadata{}, errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return KeyMetadata{}, errors.New("invalid input: key ID must not be empty")
	}

	var cMetadata C.struct_satox_key_metadata
	var cAccessLevelsPtr **C.char
	var cAccessLevelsLen C.size_t
	var cTagsPtr **C.char
	var cTagsLen C.size_t

	result := C.satox_key_storage_get_key_metadata(
		C.CString(keyID),
		&cMetadata,
		&cAccessLevelsPtr,
		&cAccessLevelsLen,
		&cTagsPtr,
		&cTagsLen,
	)

	if result != 0 {
		return KeyMetadata{}, errors.New("failed to get key metadata")
	}

	// Convert C metadata to Go struct
	metadata := KeyMetadata{
		Algorithm:    C.GoString(cMetadata.algorithm),
		CreationTime: time.Unix(int64(cMetadata.creation_time), 0),
		Expiration:   time.Unix(int64(cMetadata.expiration), 0),
	}

	// Convert access levels
	metadata.AccessLevels = make([]string, cAccessLevelsLen)
	for i := 0; i < int(cAccessLevelsLen); i++ {
		metadata.AccessLevels[i] = C.GoString(*(**C.char)(unsafe.Pointer(uintptr(unsafe.Pointer(cAccessLevelsPtr)) + uintptr(i)*unsafe.Sizeof(*cAccessLevelsPtr))))
	}

	// Convert tags
	metadata.Tags = make([]string, cTagsLen)
	for i := 0; i < int(cTagsLen); i++ {
		metadata.Tags[i] = C.GoString(*(**C.char)(unsafe.Pointer(uintptr(unsafe.Pointer(cTagsPtr)) + uintptr(i)*unsafe.Sizeof(*cTagsPtr))))
	}

	return metadata, nil
}

// RotateKey rotates a key.
func (ks *KeyStorage) RotateKey(keyID string) error {
	if !ks.initialized {
		return errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return errors.New("invalid input: key ID must not be empty")
	}

	result := C.satox_key_storage_rotate_key(C.CString(keyID))
	if result != 0 {
		return errors.New("failed to rotate key")
	}

	return nil
}

// ReencryptKey reencrypts a key.
func (ks *KeyStorage) ReencryptKey(keyID string) error {
	if !ks.initialized {
		return errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return errors.New("invalid input: key ID must not be empty")
	}

	result := C.satox_key_storage_reencrypt_key(C.CString(keyID))
	if result != 0 {
		return errors.New("failed to reencrypt key")
	}

	return nil
}

// ValidateKey validates a key.
func (ks *KeyStorage) ValidateKey(keyID string) (bool, error) {
	if !ks.initialized {
		return false, errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return false, errors.New("invalid input: key ID must not be empty")
	}

	var isValid C.int

	result := C.satox_key_storage_validate_key(
		C.CString(keyID),
		&isValid,
	)

	if result != 0 {
		return false, errors.New("failed to validate key")
	}

	return isValid != 0, nil
}

// IsKeyExpired checks if a key is expired.
func (ks *KeyStorage) IsKeyExpired(keyID string) (bool, error) {
	if !ks.initialized {
		return false, errors.New("key storage is not initialized")
	}

	if keyID == "" {
		return false, errors.New("invalid input: key ID must not be empty")
	}

	var isExpired C.int

	result := C.satox_key_storage_is_key_expired(
		C.CString(keyID),
		&isExpired,
	)

	if result != 0 {
		return false, errors.New("failed to check key expiration")
	}

	return isExpired != 0, nil
}

// AddKeyAccessLevel adds an access level to a key.
func (ks *KeyStorage) AddKeyAccessLevel(keyID, accessLevel string) error {
	if !ks.initialized {
		return errors.New("key storage is not initialized")
	}

	if keyID == "" || accessLevel == "" {
		return errors.New("invalid input: key ID and access level must not be empty")
	}

	result := C.satox_key_storage_add_key_access_level(
		C.CString(keyID),
		C.CString(accessLevel),
	)

	if result != 0 {
		return errors.New("failed to add key access level")
	}

	return nil
}

// RemoveKeyAccessLevel removes an access level from a key.
func (ks *KeyStorage) RemoveKeyAccessLevel(keyID, accessLevel string) error {
	if !ks.initialized {
		return errors.New("key storage is not initialized")
	}

	if keyID == "" || accessLevel == "" {
		return errors.New("invalid input: key ID and access level must not be empty")
	}

	result := C.satox_key_storage_remove_key_access_level(
		C.CString(keyID),
		C.CString(accessLevel),
	)

	if result != 0 {
		return errors.New("failed to remove key access level")
	}

	return nil
}

// CountKeys returns the number of stored keys.
func (ks *KeyStorage) CountKeys() (int, error) {
	if !ks.initialized {
		return 0, errors.New("key storage is not initialized")
	}

	var count C.size_t

	result := C.satox_key_storage_count_keys(&count)
	if result != 0 {
		return 0, errors.New("failed to count keys")
	}

	return int(count), nil
}

// ListKeyIDs returns a list of all key IDs.
func (ks *KeyStorage) ListKeyIDs() ([]string, error) {
	if !ks.initialized {
		return nil, errors.New("key storage is not initialized")
	}

	var keyIDsPtr **C.char
	var keyIDsLen C.size_t

	result := C.satox_key_storage_list_key_ids(
		&keyIDsPtr,
		&keyIDsLen,
	)

	if result != 0 {
		return nil, errors.New("failed to list key IDs")
	}

	keyIDs := make([]string, keyIDsLen)
	for i := 0; i < int(keyIDsLen); i++ {
		keyIDs[i] = C.GoString(*(**C.char)(unsafe.Pointer(uintptr(unsafe.Pointer(keyIDsPtr)) + uintptr(i)*unsafe.Sizeof(*keyIDsPtr))))
	}

	return keyIDs, nil
}

// IsInitialized returns whether the key storage system is initialized.
func (ks *KeyStorage) IsInitialized() bool {
	return ks.initialized
}
