package satox_bindings

import (
	"errors"
	"fmt"
	"net/http"
	"sync"
)

// APIManager provides API server management and endpoint registration.
type APIManager struct {
	initialized   bool
	serverRunning bool
	host          string
	port          int
	server        *http.Server
	endpoints     map[string]http.HandlerFunc
	mu            sync.RWMutex
}

// NewAPIManager creates a new instance of APIManager.
func NewAPIManager() *APIManager {
	return &APIManager{
		endpoints: make(map[string]http.HandlerFunc),
	}
}

// Initialize initializes the API manager.
func (m *APIManager) Initialize() error {
	m.mu.Lock()
	defer m.mu.Unlock()
	if m.initialized {
		return errors.New("APIManager already initialized")
	}
	m.initialized = true
	return nil
}

// StartServer starts the API server on the specified host and port.
func (m *APIManager) StartServer(host string, port int) error {
	m.mu.Lock()
	defer m.mu.Unlock()
	if !m.initialized {
		return errors.New("APIManager not initialized")
	}
	if m.serverRunning {
		return errors.New("API server already running")
	}

	mux := http.NewServeMux()
	for path, handler := range m.endpoints {
		mux.HandleFunc(path, handler)
	}

	m.host = host
	m.port = port
	m.server = &http.Server{
		Addr:    fmt.Sprintf("%s:%d", host, port),
		Handler: mux,
	}

	m.serverRunning = true
	go func() {
		_ = m.server.ListenAndServe()
	}()
	return nil
}

// StopServer stops the API server.
func (m *APIManager) StopServer() error {
	m.mu.Lock()
	defer m.mu.Unlock()
	if !m.serverRunning {
		return errors.New("API server not running")
	}
	if m.server != nil {
		err := m.server.Close()
		m.serverRunning = false
		m.server = nil
		return err
	}
	m.serverRunning = false
	return nil
}

// IsServerRunning returns whether the API server is running.
func (m *APIManager) IsServerRunning() bool {
	m.mu.RLock()
	defer m.mu.RUnlock()
	return m.serverRunning
}

// RegisterEndpoint registers a new API endpoint.
func (m *APIManager) RegisterEndpoint(path string, handler http.HandlerFunc) error {
	m.mu.Lock()
	defer m.mu.Unlock()
	if !m.initialized {
		return errors.New("APIManager not initialized")
	}
	if _, exists := m.endpoints[path]; exists {
		return errors.New("Endpoint already registered")
	}
	m.endpoints[path] = handler
	return nil
}

// UnregisterEndpoint removes an API endpoint.
func (m *APIManager) UnregisterEndpoint(path string) error {
	m.mu.Lock()
	defer m.mu.Unlock()
	if !m.initialized {
		return errors.New("APIManager not initialized")
	}
	if _, exists := m.endpoints[path]; !exists {
		return errors.New("Endpoint not found")
	}
	delete(m.endpoints, path)
	return nil
}

// GetServerStatus returns the current server status.
func (m *APIManager) GetServerStatus() map[string]interface{} {
	m.mu.RLock()
	defer m.mu.RUnlock()
	return map[string]interface{}{
		"running": m.serverRunning,
		"host":    m.host,
		"port":    m.port,
	}
}

// Shutdown shuts down the API manager and server.
func (m *APIManager) Shutdown() error {
	m.mu.Lock()
	defer m.mu.Unlock()
	if m.serverRunning && m.server != nil {
		_ = m.server.Close()
		m.serverRunning = false
		m.server = nil
	}
	m.initialized = false
	m.endpoints = make(map[string]http.HandlerFunc)
	return nil
}
