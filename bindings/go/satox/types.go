package satox

import (
	"time"
)

// DatabaseType represents supported database types
type DatabaseType string

const (
	DatabaseTypeSQLite     DatabaseType = "sqlite"
	DatabaseTypePostgreSQL DatabaseType = "postgresql"
	DatabaseTypeMySQL      DatabaseType = "mysql"
	DatabaseTypeRedis      DatabaseType = "redis"
	DatabaseTypeMongoDB    DatabaseType = "mongodb"
	DatabaseTypeSupabase   DatabaseType = "supabase"
	DatabaseTypeFirebase   DatabaseType = "firebase"
	DatabaseTypeAWS        DatabaseType = "aws"
	DatabaseTypeAzure      DatabaseType = "azure"
	DatabaseTypeGoogleCloud DatabaseType = "google_cloud"
)

// AssetType represents asset types
type AssetType string

const (
	AssetTypeToken    AssetType = "token"
	AssetTypeNFT      AssetType = "nft"
	AssetTypeCurrency AssetType = "currency"
)

// NetworkType represents network types
type NetworkType string

const (
	NetworkTypeMainnet NetworkType = "mainnet"
	NetworkTypeTestnet NetworkType = "testnet"
	NetworkTypeRegtest NetworkType = "regtest"
)

// QueryResult represents a database query result
type QueryResult struct {
	Success      bool                     `json:"success"`
	Rows         []map[string]interface{} `json:"rows"`
	Error        string                   `json:"error"`
	AffectedRows int                      `json:"affected_rows"`
	LastInsertID *int                     `json:"last_insert_id"`
}

// DatabaseConfig represents database configuration
type DatabaseConfig struct {
	Type             DatabaseType `json:"type"`
	Host             string       `json:"host,omitempty"`
	Port             int          `json:"port,omitempty"`
	Database         string       `json:"database,omitempty"`
	Username         string       `json:"username,omitempty"`
	Password         string       `json:"password,omitempty"`
	ConnectionString string       `json:"connection_string,omitempty"`
	MaxConnections   int          `json:"max_connections,omitempty"`
	Timeout          int          `json:"timeout,omitempty"`
}

// CoreConfig represents core configuration
type CoreConfig struct {
	Name          string `json:"name,omitempty"`
	EnableLogging bool   `json:"enable_logging,omitempty"`
	LogLevel      string `json:"log_level,omitempty"`
	LogPath       string `json:"log_path,omitempty"`
	MaxThreads    int    `json:"max_threads,omitempty"`
	Timeout       int    `json:"timeout,omitempty"`
}

// CoreStatus represents core status
type CoreStatus struct {
	Initialized bool                   `json:"initialized"`
	Version     string                 `json:"version"`
	Uptime      float64                `json:"uptime"`
	MemoryUsage map[string]interface{} `json:"memory_usage"`
	ThreadCount int                    `json:"thread_count"`
}

// Asset represents an asset
type Asset struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Symbol      string                 `json:"symbol"`
	Type        AssetType              `json:"type"`
	Decimals    int                    `json:"decimals"`
	TotalSupply int64                  `json:"total_supply"`
	Metadata    map[string]interface{} `json:"metadata"`
}

// NFT represents an NFT
type NFT struct {
	ID          string                 `json:"id"`
	Name        string                 `json:"name"`
	Description string                 `json:"description"`
	ImageURL    string                 `json:"image_url"`
	Metadata    map[string]interface{} `json:"metadata"`
	Owner       string                 `json:"owner"`
	CreatedAt   time.Time              `json:"created_at"`
}

// Wallet represents a wallet
type Wallet struct {
	Address     string             `json:"address"`
	Balance     map[string]float64 `json:"balance"`
	Assets      []Asset            `json:"assets"`
	NFTs        []NFT              `json:"nfts"`
	CreatedAt   time.Time          `json:"created_at"`
	LastUpdated time.Time          `json:"last_updated"`
}

// SatoxError represents a Satox SDK error
type SatoxError struct {
	Message string `json:"message"`
	Code    string `json:"code"`
}

func (e *SatoxError) Error() string {
	return e.Message
}
