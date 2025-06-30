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

// NetworkManager represents a network manager instance
type NetworkManager struct {
	handle unsafe.Pointer
}

// NetworkConfig represents network configuration
type NetworkConfig struct {
	Host string `json:"host"`
	Port uint16 `json:"port"`
}

// NewNetworkManager creates a new network manager
func NewNetworkManager(configJSON string) (*NetworkManager, error) {
	var config *C.char
	if configJSON != "" {
		config = C.CString(configJSON)
		defer C.free(unsafe.Pointer(config))
	}

	handle := C.satox_network_manager_create(config)
	if handle == nil {
		return nil, errors.New("failed to create network manager")
	}

	return &NetworkManager{handle: handle}, nil
}

// Close destroys the network manager
func (nm *NetworkManager) Close() {
	if nm.handle != nil {
		C.satox_network_manager_destroy(nm.handle)
		nm.handle = nil
	}
}

// Start starts the network manager
func (nm *NetworkManager) Start() error {
	if nm.handle == nil {
		return errors.New("network manager not initialized")
	}

	result := C.satox_network_manager_start(nm.handle)
	if result != 0 {
		return errors.New("failed to start network manager")
	}

	return nil
}

// Stop stops the network manager
func (nm *NetworkManager) Stop() error {
	if nm.handle == nil {
		return errors.New("network manager not initialized")
	}

	result := C.satox_network_manager_stop(nm.handle)
	if result != 0 {
		return errors.New("failed to stop network manager")
	}

	return nil
}

// GetPeers returns the list of connected peers
func (nm *NetworkManager) GetPeers() (string, error) {
	if nm.handle == nil {
		return "", errors.New("network manager not initialized")
	}

	result := C.satox_network_manager_get_peers(nm.handle)
	if result == nil {
		return "", errors.New("failed to get peers")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// GetStats returns network statistics
func (nm *NetworkManager) GetStats() (string, error) {
	if nm.handle == nil {
		return "", errors.New("network manager not initialized")
	}

	result := C.satox_network_manager_get_stats(nm.handle)
	if result == nil {
		return "", errors.New("failed to get stats")
	}
	defer C.free(unsafe.Pointer(result))

	return C.GoString(result), nil
}

// AddPeer adds a peer to the network
func (nm *NetworkManager) AddPeer(address string, port uint16) error {
	if nm.handle == nil {
		return errors.New("network manager not initialized")
	}

	cAddress := C.CString(address)
	defer C.free(unsafe.Pointer(cAddress))

	result := C.satox_network_manager_add_peer(nm.handle, cAddress, C.ushort(port))
	if result != 0 {
		return errors.New("failed to add peer")
	}

	return nil
}

// RemovePeer removes a peer from the network
func (nm *NetworkManager) RemovePeer(address string, port uint16) error {
	if nm.handle == nil {
		return errors.New("network manager not initialized")
	}

	cAddress := C.CString(address)
	defer C.free(unsafe.Pointer(cAddress))

	result := C.satox_network_manager_remove_peer(nm.handle, cAddress, C.ushort(port))
	if result != 0 {
		return errors.New("failed to remove peer")
	}

	return nil
}

// SendMessage sends a message to a specific peer
func (nm *NetworkManager) SendMessage(address string, port uint16, message string) error {
	if nm.handle == nil {
		return errors.New("network manager not initialized")
	}

	cAddress := C.CString(address)
	defer C.free(unsafe.Pointer(cAddress))

	cMessage := C.CString(message)
	defer C.free(unsafe.Pointer(cMessage))

	result := C.satox_network_manager_send_message(nm.handle, cAddress, C.ushort(port), cMessage)
	if result != 0 {
		return errors.New("failed to send message")
	}

	return nil
}
