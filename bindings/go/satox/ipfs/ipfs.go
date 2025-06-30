package ipfs

import (
	"io"
	"sync"

	shell "github.com/ipfs/go-ipfs-api"
)

// IPFSManager handles IPFS operations
type IPFSManager struct {
	sh     *shell.Shell
	mu     sync.RWMutex
	config *Config
}

// Config holds IPFS configuration
type Config struct {
	APIURL     string
	GatewayURL string
	Timeout    int
}

// NewIPFSManager creates a new IPFSManager instance
func NewIPFSManager(config *Config) *IPFSManager {
	return &IPFSManager{
		sh:     shell.NewShell(config.APIURL),
		config: config,
	}
}

// Store stores data in IPFS
func (im *IPFSManager) Store(data []byte) (string, error) {
	im.mu.Lock()
	defer im.mu.Unlock()

	cid, err := im.sh.Add(data)
	if err != nil {
		return "", err
	}
	return cid, nil
}

// Retrieve retrieves data from IPFS
func (im *IPFSManager) Retrieve(cid string) ([]byte, error) {
	im.mu.RLock()
	defer im.mu.RUnlock()

	reader, err := im.sh.Cat(cid)
	if err != nil {
		return nil, err
	}
	defer reader.Close()

	return io.ReadAll(reader)
}

// Pin pins a CID in IPFS
func (im *IPFSManager) Pin(cid string) error {
	im.mu.Lock()
	defer im.mu.Unlock()

	return im.sh.Pin(cid)
}

// Unpin unpins a CID from IPFS
func (im *IPFSManager) Unpin(cid string) error {
	im.mu.Lock()
	defer im.mu.Unlock()

	return im.sh.Unpin(cid)
}

// GetGatewayURL returns the gateway URL for a CID
func (im *IPFSManager) GetGatewayURL(cid string) string {
	return im.config.GatewayURL + "/ipfs/" + cid
}

// StoreFile stores a file in IPFS
func (im *IPFSManager) StoreFile(reader io.Reader) (string, error) {
	im.mu.Lock()
	defer im.mu.Unlock()

	cid, err := im.sh.Add(reader)
	if err != nil {
		return "", err
	}
	return cid, nil
}

// RetrieveFile retrieves a file from IPFS
func (im *IPFSManager) RetrieveFile(cid string) (io.ReadCloser, error) {
	im.mu.RLock()
	defer im.mu.RUnlock()

	return im.sh.Cat(cid)
}

// ListPins lists all pinned CIDs
func (im *IPFSManager) ListPins() ([]string, error) {
	im.mu.RLock()
	defer im.mu.RUnlock()

	pins, err := im.sh.Pins()
	if err != nil {
		return nil, err
	}

	var cids []string
	for cid := range pins {
		cids = append(cids, cid)
	}
	return cids, nil
}

// IsPinned checks if a CID is pinned
func (im *IPFSManager) IsPinned(cid string) (bool, error) {
	im.mu.RLock()
	defer im.mu.RUnlock()

	pins, err := im.sh.Pins()
	if err != nil {
		return false, err
	}

	_, exists := pins[cid]
	return exists, nil
}
