const sqlite3 = require("sqlite3").verbose();
const { open } = require("sqlite");
const EventEmitter = require("events");
const fs = require("fs").promises;
const path = require("path");

class DatabaseManager extends EventEmitter {
  constructor() {
    super();
    this.initialized = false;
    this.db = null;
    this.inTransaction = false;
    this.config = {
      timeout: 5000,
      journalMode: "WAL",
      synchronous: "NORMAL",
      tempStore: "MEMORY",
      cacheSize: -2000, // 2MB cache
    };
  }

  async initialize(config = {}) {
    if (this.initialized) {
      throw new Error("DatabaseManager already initialized");
    }

    // Merge provided config with defaults
    this.config = { ...this.config, ...config };
    this.initialized = true;
  }

  isInitialized() {
    return this.initialized;
  }

  async connect(connectionString) {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (this.db) {
      throw new Error("Already connected to database");
    }

    try {
      this.db = await open({
        filename: connectionString,
        driver: sqlite3.Database,
      });

      // Configure database
      await this.db.exec(`
                PRAGMA journal_mode = ${this.config.journalMode};
                PRAGMA synchronous = ${this.config.synchronous};
                PRAGMA temp_store = ${this.config.tempStore};
                PRAGMA cache_size = ${this.config.cacheSize};
            `);

      this.emit("connected", connectionString);
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to connect to database: ${error.message}`);
    }
  }

  async disconnect() {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      await this.db.close();
      this.db = null;
      this.emit("disconnected");
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to disconnect from database: ${error.message}`);
    }
  }

  async executeQuery(query, params = []) {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      const result = await this.db.all(query, params);
      this.emit("queryExecuted", { query, params, rowCount: result.length });
      return result;
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to execute query: ${error.message}`);
    }
  }

  async executeNonQuery(query, params = []) {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      const result = await this.db.run(query, params);
      this.emit("queryExecuted", { query, params, changes: result.changes });
      return {
        changes: result.changes,
        lastID: result.lastID,
      };
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to execute non-query: ${error.message}`);
    }
  }

  async beginTransaction() {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }
    if (this.inTransaction) {
      throw new Error("Transaction already in progress");
    }

    try {
      await this.db.exec("BEGIN TRANSACTION");
      this.inTransaction = true;
      this.emit("transactionStarted");
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to begin transaction: ${error.message}`);
    }
  }

  async commitTransaction() {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }
    if (!this.inTransaction) {
      throw new Error("No transaction in progress");
    }

    try {
      await this.db.exec("COMMIT");
      this.inTransaction = false;
      this.emit("transactionCommitted");
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to commit transaction: ${error.message}`);
    }
  }

  async rollbackTransaction() {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }
    if (!this.inTransaction) {
      throw new Error("No transaction in progress");
    }

    try {
      await this.db.exec("ROLLBACK");
      this.inTransaction = false;
      this.emit("transactionRolledBack");
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to rollback transaction: ${error.message}`);
    }
  }

  async backupDatabase(targetPath) {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      const backup = await this.db.backup(targetPath);
      await backup.step(-1);
      await backup.finish();
      this.emit("databaseBackedUp", targetPath);
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to backup database: ${error.message}`);
    }
  }

  async restoreDatabase(sourcePath) {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      // Close current connection
      await this.disconnect();

      // Copy backup file to current database
      const currentDbPath = this.db.filename;
      await fs.copyFile(sourcePath, currentDbPath);

      // Reconnect to database
      await this.connect(currentDbPath);
      this.emit("databaseRestored", sourcePath);
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to restore database: ${error.message}`);
    }
  }

  async getTableInfo(tableName) {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      const columns = await this.db.all(`PRAGMA table_info(${tableName})`);
      const indexes = await this.db.all(`PRAGMA index_list(${tableName})`);

      return {
        name: tableName,
        columns,
        indexes,
      };
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to get table info: ${error.message}`);
    }
  }

  async getDatabaseInfo() {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      const tables = await this.db.all(`
                SELECT name FROM sqlite_master 
                WHERE type='table' AND name NOT LIKE 'sqlite_%'
            `);

      const info = {
        tables: [],
        pageSize: await this.db.get("PRAGMA page_size"),
        pageCount: await this.db.get("PRAGMA page_count"),
        size:
          (await this.db.get("PRAGMA page_size")) *
          (await this.db.get("PRAGMA page_count")),
      };

      for (const table of tables) {
        info.tables.push(await this.getTableInfo(table.name));
      }

      return info;
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to get database info: ${error.message}`);
    }
  }

  async vacuum() {
    if (!this.initialized) {
      throw new Error("DatabaseManager not initialized");
    }
    if (!this.db) {
      throw new Error("Not connected to database");
    }

    try {
      await this.db.exec("VACUUM");
      this.emit("databaseVacuumed");
    } catch (error) {
      this.emit("error", error);
      throw new Error(`Failed to vacuum database: ${error.message}`);
    }
  }
}

module.exports = DatabaseManager;
