package satox_bindings

import (
	"errors"
	"net"
	"strconv"
	"sync"
)

// NetworkManager provides network connectivity and peer management.
// Usage:
//
//	manager := NewNetworkManager()
//	err := manager.Initialize()
//	err = manager.Connect("mainnet", 60777)
type NetworkManager struct {
	initialized    bool
	connected      bool
	currentNetwork string
	port           int
	peers          map[string]*Peer
	mu             sync.RWMutex
}

// Peer represents a network peer.
type Peer struct {
	ID      string
	Address string
	Port    int
	Info    map[string]interface{}
}

// NewNetworkManager creates a new instance of NetworkManager.
func NewNetworkManager() *NetworkManager {
	return &NetworkManager{
		peers: make(map[string]*Peer),
	}
}

// Initialize initializes the network manager.
func (m *NetworkManager) Initialize() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.initialized {
		return errors.New("NetworkManager already initialized")
	}
	m.initialized = true
	return nil
}

// Connect connects to the specified network.
func (m *NetworkManager) Connect(networkID string, port int) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("NetworkManager not initialized")
	}
	if m.connected {
		return errors.New("Already connected to network")
	}

	// Validate port
	if port <= 0 || port > 65535 {
		return errors.New("Invalid port number")
	}

	// For Satoxcoin, validate network and port
	if networkID != "mainnet" {
		return errors.New("Only mainnet is supported")
	}
	if port != 60777 {
		return errors.New("Invalid port for Satoxcoin mainnet (must be 60777)")
	}

	m.currentNetwork = networkID
	m.port = port
	m.connected = true
	return nil
}

// Disconnect disconnects from the current network.
func (m *NetworkManager) Disconnect() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("NetworkManager not initialized")
	}
	if !m.connected {
		return errors.New("Not connected to any network")
	}

	m.connected = false
	m.currentNetwork = ""
	m.port = 0
	m.peers = make(map[string]*Peer)
	return nil
}

// IsConnected returns whether the manager is connected to a network.
func (m *NetworkManager) IsConnected() bool {
	m.mu.RLock()
	defer m.mu.RUnlock()
	return m.connected
}

// GetCurrentNetwork returns the current network ID.
func (m *NetworkManager) GetCurrentNetwork() (string, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.connected {
		return "", errors.New("Not connected to any network")
	}
	return m.currentNetwork, nil
}

// GetPort returns the current port.
func (m *NetworkManager) GetPort() (int, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.connected {
		return 0, errors.New("Not connected to any network")
	}
	return m.port, nil
}

// SendMessage sends a message to a peer.
func (m *NetworkManager) SendMessage(peerID string, message []byte) error {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return errors.New("NetworkManager not initialized")
	}
	if !m.connected {
		return errors.New("Not connected to any network")
	}

	peer, exists := m.peers[peerID]
	if !exists {
		return errors.New("Peer not found")
	}

	// Send message to peer
	addr := net.JoinHostPort(peer.Address, strconv.Itoa(peer.Port))
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return err
	}
	defer conn.Close()

	_, err = conn.Write(message)
	return err
}

// ReceiveMessage receives a message from a peer.
func (m *NetworkManager) ReceiveMessage(peerID string) ([]byte, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return nil, errors.New("NetworkManager not initialized")
	}
	if !m.connected {
		return nil, errors.New("Not connected to any network")
	}

	peer, exists := m.peers[peerID]
	if !exists {
		return nil, errors.New("Peer not found")
	}

	// Receive message from peer
	addr := net.JoinHostPort(peer.Address, strconv.Itoa(peer.Port))
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return nil, err
	}
	defer conn.Close()

	buffer := make([]byte, 1024)
	n, err := conn.Read(buffer)
	if err != nil {
		return nil, err
	}

	return buffer[:n], nil
}

// GetPeerInfo returns information about a peer.
func (m *NetworkManager) GetPeerInfo(peerID string) (*Peer, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return nil, errors.New("NetworkManager not initialized")
	}
	if !m.connected {
		return nil, errors.New("Not connected to any network")
	}

	peer, exists := m.peers[peerID]
	if !exists {
		return nil, errors.New("Peer not found")
	}

	return peer, nil
}

// Shutdown shuts down the network manager.
func (m *NetworkManager) Shutdown() error {
	if m.connected {
		return m.Disconnect()
	}
	m.mu.Lock()
	m.initialized = false
	m.mu.Unlock()
	return nil
}
