local DatabaseManager = {}
DatabaseManager.__index = DatabaseManager

-- Load required libraries
local sqlite3 = require("lsqlite3")

function DatabaseManager.new()
    local self = setmetatable({}, DatabaseManager)
    self.initialized = false
    self.connected = false
    self.db = nil
    return self
end

function DatabaseManager:initialize()
    if self.initialized then
        error("DatabaseManager already initialized")
    end
    self.initialized = true
end

function DatabaseManager:isInitialized()
    return self.initialized
end

function DatabaseManager:connect(connectionString)
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if self.connected then
        error("Already connected to database")
    end
    self.db = sqlite3.open(connectionString)
    if not self.db then
        error("Failed to connect to database")
    end
    self.connected = true
end

function DatabaseManager:disconnect()
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    self.db:close()
    self.db = nil
    self.connected = false
end

function DatabaseManager:isConnected()
    return self.connected
end

function DatabaseManager:executeQuery(query, params)
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    local stmt = self.db:prepare(query)
    if not stmt then
        error("Failed to prepare query")
    end
    
    if params then
        for i, param in ipairs(params) do
            stmt:bind(i, param)
        end
    end
    
    local results = {}
    for row in stmt:nrows() do
        table.insert(results, row)
    end
    
    stmt:finalize()
    return results
end

function DatabaseManager:executeNonQuery(query, params)
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    local stmt = self.db:prepare(query)
    if not stmt then
        error("Failed to prepare query")
    end
    
    if params then
        for i, param in ipairs(params) do
            stmt:bind(i, param)
        end
    end
    
    local success = stmt:step()
    stmt:finalize()
    return success
end

function DatabaseManager:beginTransaction()
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    return self.db:exec("BEGIN TRANSACTION")
end

function DatabaseManager:commitTransaction()
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    return self.db:exec("COMMIT")
end

function DatabaseManager:rollbackTransaction()
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    return self.db:exec("ROLLBACK")
end

function DatabaseManager:backupDatabase(targetPath)
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    local backup = sqlite3.open(targetPath)
    if not backup then
        error("Failed to create backup database")
    end
    self.db:backup(backup)
    backup:close()
    return true
end

function DatabaseManager:restoreDatabase(sourcePath)
    if not self.initialized then
        error("DatabaseManager not initialized")
    end
    if not self.connected then
        error("Not connected to database")
    end
    local source = sqlite3.open(sourcePath)
    if not source then
        error("Failed to open source database")
    end
    source:backup(self.db)
    source:close()
    return true
end

return DatabaseManager 