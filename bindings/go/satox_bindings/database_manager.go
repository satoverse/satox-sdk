package satox_bindings

import (
	"database/sql"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sync"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

// DatabaseManager provides database management functionality.
// Usage:
//
//	manager := NewDatabaseManager()
//	err := manager.Initialize()
//	err = manager.Connect("database_name")
type DatabaseManager struct {
	initialized     bool
	connected       bool
	currentDatabase string
	databases       map[string]*sql.DB
	mu              sync.RWMutex
}

// DatabaseInfo represents information about a database.
type DatabaseInfo struct {
	Name         string
	Size         int64
	Tables       []string
	LastModified time.Time
}

// NewDatabaseManager creates a new instance of DatabaseManager.
func NewDatabaseManager() *DatabaseManager {
	return &DatabaseManager{
		databases: make(map[string]*sql.DB),
	}
}

// Initialize initializes the database manager.
func (m *DatabaseManager) Initialize() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.initialized {
		return errors.New("DatabaseManager already initialized")
	}
	m.initialized = true
	return nil
}

// Connect connects to the specified database.
func (m *DatabaseManager) Connect(databaseName string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("DatabaseManager not initialized")
	}
	if m.connected {
		return errors.New("Already connected to a database")
	}

	db, exists := m.databases[databaseName]
	if !exists {
		return errors.New("Database does not exist")
	}

	// Test connection
	if err := db.Ping(); err != nil {
		return fmt.Errorf("failed to connect to database: %v", err)
	}

	m.currentDatabase = databaseName
	m.connected = true
	return nil
}

// Disconnect disconnects from the current database.
func (m *DatabaseManager) Disconnect() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("DatabaseManager not initialized")
	}
	if !m.connected {
		return errors.New("Not connected to any database")
	}

	m.connected = false
	m.currentDatabase = ""
	return nil
}

// IsConnected returns whether the manager is connected to a database.
func (m *DatabaseManager) IsConnected() bool {
	m.mu.RLock()
	defer m.mu.RUnlock()
	return m.connected
}

// GetCurrentDatabase returns the current database name.
func (m *DatabaseManager) GetCurrentDatabase() (string, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.connected {
		return "", errors.New("Not connected to any database")
	}
	return m.currentDatabase, nil
}

// CreateDatabase creates a new database.
func (m *DatabaseManager) CreateDatabase(databaseName string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("DatabaseManager not initialized")
	}
	if _, exists := m.databases[databaseName]; exists {
		return errors.New("Database already exists")
	}

	// Create database directory if it doesn't exist
	dbDir := filepath.Join("data", "databases")
	if err := os.MkdirAll(dbDir, 0755); err != nil {
		return fmt.Errorf("failed to create database directory: %v", err)
	}

	// Create database file
	dbPath := filepath.Join(dbDir, databaseName+".db")
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		return fmt.Errorf("failed to create database: %v", err)
	}

	// Test connection
	if err := db.Ping(); err != nil {
		db.Close()
		return fmt.Errorf("failed to connect to new database: %v", err)
	}

	m.databases[databaseName] = db
	return nil
}

// DeleteDatabase deletes a database.
func (m *DatabaseManager) DeleteDatabase(databaseName string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("DatabaseManager not initialized")
	}

	db, exists := m.databases[databaseName]
	if !exists {
		return errors.New("Database does not exist")
	}

	// Close database connection
	if err := db.Close(); err != nil {
		return fmt.Errorf("failed to close database: %v", err)
	}

	// Delete database file
	dbPath := filepath.Join("data", "databases", databaseName+".db")
	if err := os.Remove(dbPath); err != nil {
		return fmt.Errorf("failed to delete database file: %v", err)
	}

	delete(m.databases, databaseName)

	// If this was the current database, disconnect
	if m.connected && m.currentDatabase == databaseName {
		m.connected = false
		m.currentDatabase = ""
	}

	return nil
}

// GetDatabaseInfo returns information about a database.
func (m *DatabaseManager) GetDatabaseInfo(databaseName string) (*DatabaseInfo, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return nil, errors.New("DatabaseManager not initialized")
	}

	db, exists := m.databases[databaseName]
	if !exists {
		return nil, errors.New("Database does not exist")
	}

	// Get database file info
	dbPath := filepath.Join("data", "databases", databaseName+".db")
	fileInfo, err := os.Stat(dbPath)
	if err != nil {
		return nil, fmt.Errorf("failed to get database file info: %v", err)
	}

	// Get tables
	rows, err := db.Query("SELECT name FROM sqlite_master WHERE type='table'")
	if err != nil {
		return nil, fmt.Errorf("failed to get tables: %v", err)
	}
	defer rows.Close()

	var tables []string
	for rows.Next() {
		var tableName string
		if err := rows.Scan(&tableName); err != nil {
			return nil, fmt.Errorf("failed to scan table name: %v", err)
		}
		tables = append(tables, tableName)
	}

	return &DatabaseInfo{
		Name:         databaseName,
		Size:         fileInfo.Size(),
		Tables:       tables,
		LastModified: fileInfo.ModTime(),
	}, nil
}

// ListDatabases returns a list of all databases.
func (m *DatabaseManager) ListDatabases() ([]string, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return nil, errors.New("DatabaseManager not initialized")
	}

	databases := make([]string, 0, len(m.databases))
	for name := range m.databases {
		databases = append(databases, name)
	}
	return databases, nil
}

// ExecuteQuery executes a query on the current database.
func (m *DatabaseManager) ExecuteQuery(query string) (*sql.Rows, error) {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return nil, errors.New("DatabaseManager not initialized")
	}
	if !m.connected {
		return nil, errors.New("Not connected to any database")
	}

	db := m.databases[m.currentDatabase]
	return db.Query(query)
}

// BackupDatabase creates a backup of a database.
func (m *DatabaseManager) BackupDatabase(databaseName, backupPath string) error {
	m.mu.RLock()
	defer m.mu.RUnlock()

	if !m.initialized {
		return errors.New("DatabaseManager not initialized")
	}

	db, exists := m.databases[databaseName]
	if !exists {
		return errors.New("Database does not exist")
	}

	// Create backup directory if it doesn't exist
	backupDir := filepath.Dir(backupPath)
	if err := os.MkdirAll(backupDir, 0755); err != nil {
		return fmt.Errorf("failed to create backup directory: %v", err)
	}

	// Create backup file
	backupDB, err := sql.Open("sqlite3", backupPath)
	if err != nil {
		return fmt.Errorf("failed to create backup database: %v", err)
	}
	defer backupDB.Close()

	// Begin transaction
	tx, err := backupDB.Begin()
	if err != nil {
		return fmt.Errorf("failed to begin backup transaction: %v", err)
	}

	// Copy schema and data
	if _, err := tx.Exec("ATTACH DATABASE ? AS source", db); err != nil {
		tx.Rollback()
		return fmt.Errorf("failed to attach source database: %v", err)
	}

	if _, err := tx.Exec("SELECT sql FROM source.sqlite_master WHERE type='table'"); err != nil {
		tx.Rollback()
		return fmt.Errorf("failed to copy schema: %v", err)
	}

	// Commit transaction
	if err := tx.Commit(); err != nil {
		return fmt.Errorf("failed to commit backup: %v", err)
	}

	return nil
}

// RestoreDatabase restores a database from a backup.
func (m *DatabaseManager) RestoreDatabase(databaseName, backupPath string) error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if !m.initialized {
		return errors.New("DatabaseManager not initialized")
	}
	if _, exists := m.databases[databaseName]; exists {
		return errors.New("Database already exists")
	}

	// Create database directory if it doesn't exist
	dbDir := filepath.Join("data", "databases")
	if err := os.MkdirAll(dbDir, 0755); err != nil {
		return fmt.Errorf("failed to create database directory: %v", err)
	}

	// Create new database
	dbPath := filepath.Join(dbDir, databaseName+".db")
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		return fmt.Errorf("failed to create database: %v", err)
	}

	// Begin transaction
	tx, err := db.Begin()
	if err != nil {
		db.Close()
		return fmt.Errorf("failed to begin restore transaction: %v", err)
	}

	// Copy schema and data from backup
	if _, err := tx.Exec("ATTACH DATABASE ? AS backup", backupPath); err != nil {
		tx.Rollback()
		db.Close()
		return fmt.Errorf("failed to attach backup database: %v", err)
	}

	if _, err := tx.Exec("SELECT sql FROM backup.sqlite_master WHERE type='table'"); err != nil {
		tx.Rollback()
		db.Close()
		return fmt.Errorf("failed to copy schema: %v", err)
	}

	// Commit transaction
	if err := tx.Commit(); err != nil {
		db.Close()
		return fmt.Errorf("failed to commit restore: %v", err)
	}

	m.databases[databaseName] = db
	return nil
}

// Shutdown shuts down the database manager.
func (m *DatabaseManager) Shutdown() error {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.connected {
		if err := m.Disconnect(); err != nil {
			return err
		}
	}

	// Close all database connections
	for name, db := range m.databases {
		if err := db.Close(); err != nil {
			return fmt.Errorf("failed to close database %s: %v", name, err)
		}
	}

	m.initialized = false
	m.databases = make(map[string]*sql.DB)
	return nil
}
