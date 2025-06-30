# Local Storage Guide

## Overview
This guide covers local storage options for Satox SDK development and testing, including SQLite and file-based storage. Local storage is ideal for rapid prototyping, mobile, and game SDKs where external database dependencies are not required.

## SQLite Setup

### Installation
- **Linux:** `sudo apt install sqlite3 libsqlite3-dev`
- **macOS:** `brew install sqlite3`
- **Windows:** Download from [sqlite.org](https://sqlite.org/download.html)

### Initialization
```bash
# Create data directory
mkdir -p data/local
chmod 755 data/local

# Initialize SQLite database
sqlite3 data/local/satox_sdk.db < scripts/init_sqlite.sql
```

### Example Schema
```sql
-- scripts/init_sqlite.sql
CREATE TABLE users (
    id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    public_key TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE wallets (
    id TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    name TEXT NOT NULL,
    address TEXT UNIQUE NOT NULL,
    private_key_encrypted TEXT NOT NULL,
    balance REAL DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE assets (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    symbol TEXT NOT NULL,
    asset_id TEXT UNIQUE NOT NULL,
    issuer_address TEXT NOT NULL,
    total_supply REAL NOT NULL,
    decimals INTEGER DEFAULT 8,
    metadata TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE transactions (
    id TEXT PRIMARY KEY,
    txid TEXT UNIQUE NOT NULL,
    from_address TEXT NOT NULL,
    to_address TEXT NOT NULL,
    amount REAL NOT NULL,
    asset_id TEXT,
    fee REAL NOT NULL,
    status TEXT DEFAULT 'pending',
    block_height INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

## File-Based Storage
- Store JSON, YAML, or CSV files in `data/local/` for lightweight data persistence.
- Use for configuration, logs, or small datasets.

## Usage in SDKs
- Mobile and game SDKs use SQLite for on-device storage.
- Core SDK can use SQLite for local development/testing.
- File-based storage is used for settings, logs, and cache.

## Backup & Restore
```bash
# Backup SQLite database
cp data/local/satox_sdk.db backups/satox_sdk_$(date +%Y%m%d_%H%M%S).db

# Restore SQLite database
cp backups/satox_sdk_YYYYMMDD_HHMMSS.db data/local/satox_sdk.db
```

## Best Practices
- Use SQLite for development, switch to PostgreSQL/MongoDB for production.
- Regularly backup local databases.
- Use file-based storage for non-relational data.
- Secure local files with proper permissions.

## Troubleshooting
- **Database locked:** Ensure no other process is using the database.
- **Corruption:** Restore from backup or re-initialize.

## Next Steps
- [Database Isolation](isolation.md)
- [PostgreSQL Configuration](postgresql.md)
- [MongoDB Setup](mongodb.md)
- [Redis Caching](redis.md)

---

*Last updated: $(date)* 