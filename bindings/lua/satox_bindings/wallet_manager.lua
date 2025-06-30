local WalletManager = {}
WalletManager.__index = WalletManager

function WalletManager.new()
    local self = setmetatable({}, WalletManager)
    self.initialized = false
    self.wallets = {}
    self.currentWallet = nil
    return self
end

function WalletManager:initialize()
    if self.initialized then
        error("WalletManager already initialized")
    end
    self.initialized = true
end

function WalletManager:isInitialized()
    return self.initialized
end

function WalletManager:createWallet(name, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not name then
        error("Invalid wallet name")
    end
    if not password then
        error("Invalid password")
    end
    if self.wallets[name] then
        error("Wallet already exists: " .. name)
    end
    
    local wallet = {
        name = name,
        password = password,
        address = "S" .. string.format("%040x", math.random(0, 2^160-1)),
        publicKey = "0x" .. string.format("%064x", math.random(0, 2^256-1)),
        privateKey = "0x" .. string.format("%064x", math.random(0, 2^256-1)),
        balance = 0,
        created = os.time(),
        lastUsed = os.time(),
        version = "1.0.0"
    }
    
    self.wallets[name] = wallet
    return wallet
end

function WalletManager:importWallet(name, privateKey, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not name then
        error("Invalid wallet name")
    end
    if not privateKey or not privateKey:match("^0x[0-9a-fA-F]{64}$") then
        error("Invalid private key format")
    end
    if not password then
        error("Invalid password")
    end
    if self.wallets[name] then
        error("Wallet already exists: " .. name)
    end
    
    local wallet = {
        name = name,
        password = password,
        address = "S" .. string.format("%040x", math.random(0, 2^160-1)),
        publicKey = "0x" .. string.format("%064x", math.random(0, 2^256-1)),
        privateKey = privateKey,
        balance = 0,
        created = os.time(),
        lastUsed = os.time(),
        version = "1.0.0"
    }
    
    self.wallets[name] = wallet
    return wallet
end

function WalletManager:getWallet(name)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.wallets[name] then
        error("Wallet not found: " .. name)
    end
    
    return self.wallets[name]
end

function WalletManager:listWallets()
    if not self.initialized then
        error("WalletManager not initialized")
    end
    
    return self.wallets
end

function WalletManager:deleteWallet(name, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.wallets[name] then
        error("Wallet not found: " .. name)
    end
    if self.wallets[name].password ~= password then
        error("Invalid password")
    end
    
    self.wallets[name] = nil
    if self.currentWallet == name then
        self.currentWallet = nil
    end
    return true
end

function WalletManager:loadWallet(name, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.wallets[name] then
        error("Wallet not found")
    end
    if self.wallets[name].password ~= password then
        error("Invalid password")
    end
    
    self.currentWallet = name
    return self.wallets[name]
end

function WalletManager:unloadWallet()
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.currentWallet then
        error("No wallet loaded")
    end
    
    self.currentWallet = nil
    return true
end

function WalletManager:getCurrentWallet()
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.currentWallet then
        error("No wallet loaded")
    end
    
    return self.wallets[self.currentWallet]
end

function WalletManager:generateAddress()
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.currentWallet then
        error("No wallet loaded")
    end
    
    local address = "S" .. string.format("%040x", math.random(0, 2^160-1))
    table.insert(self.wallets[self.currentWallet].addresses, address)
    return address
end

function WalletManager:getAddresses()
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.currentWallet then
        error("No wallet loaded")
    end
    
    return self.wallets[self.currentWallet].addresses
end

function WalletManager:getBalance()
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.currentWallet then
        error("No wallet loaded")
    end
    
    return self.wallets[self.currentWallet].balance
end

function WalletManager:signTransaction(walletName, transaction, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.wallets[walletName] then
        error("Wallet not found: " .. walletName)
    end
    if self.wallets[walletName].password ~= password then
        error("Invalid password")
    end
    
    local signature = "signed_" .. string.format("%064x", math.random(0, 2^256-1))
    return signature
end

function WalletManager:verifySignature(transaction, signature, publicKey)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    
    return signature:match("^signed_") ~= nil
end

function WalletManager:changePassword(walletName, oldPassword, newPassword)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.wallets[walletName] then
        error("Wallet not found: " .. walletName)
    end
    if self.wallets[walletName].password ~= oldPassword then
        error("Invalid password")
    end
    
    self.wallets[walletName].password = newPassword
    return true
end

function WalletManager:backupWallet(walletName, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.wallets[walletName] then
        error("Wallet not found: " .. walletName)
    end
    if self.wallets[walletName].password ~= password then
        error("Invalid password")
    end
    
    return {
        name = self.wallets[walletName].name,
        address = self.wallets[walletName].address,
        publicKey = self.wallets[walletName].publicKey,
        privateKey = self.wallets[walletName].privateKey,
        created = self.wallets[walletName].created,
        version = self.wallets[walletName].version
    }
end

function WalletManager:restoreWallet(backup, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if self.wallets[backup.name] then
        error("Wallet already exists: " .. backup.name)
    end
    
    local wallet = {
        name = backup.name,
        password = password,
        address = backup.address,
        publicKey = backup.publicKey,
        privateKey = backup.privateKey,
        balance = 0,
        created = backup.created,
        lastUsed = os.time(),
        version = backup.version
    }
    
    self.wallets[backup.name] = wallet
    return wallet
end

function WalletManager:exportWallet(walletName, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.wallets[walletName] then
        error("Wallet not found: " .. walletName)
    end
    if self.wallets[walletName].password ~= password then
        error("Invalid password")
    end
    
    return {
        name = self.wallets[walletName].name,
        address = self.wallets[walletName].address,
        publicKey = self.wallets[walletName].publicKey,
        privateKey = self.wallets[walletName].privateKey,
        created = self.wallets[walletName].created,
        version = self.wallets[walletName].version
    }
end

function WalletManager:backupWalletToFile(walletName, path, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.currentWallet then
        error("No wallet loaded")
    end

    local file, err = io.open(path, "w")
    if not file then
        error("Failed to create backup file: " .. tostring(err))
    end

    local success, write_err

    success, write_err = file:write(string.format("Wallet: %s\\n", self.currentWallet))
    if not success then
        local _, close_err = file:close()
        error("Failed to write to backup file: " .. tostring(write_err) .. (close_err and (" and failed to close file: " .. tostring(close_err)) or ""))
    end

    success, write_err = file:write(string.format("Created: %s\\n", os.date("%Y-%m-%d %H:%M:%S", self.wallets[self.currentWallet].created)))
    if not success then
        local _, close_err = file:close()
        error("Failed to write to backup file: " .. tostring(write_err) .. (close_err and (" and failed to close file: " .. tostring(close_err)) or ""))
    end

    success, write_err = file:write("Addresses:\\n")
    if not success then
        local _, close_err = file:close()
        error("Failed to write to backup file: " .. tostring(write_err) .. (close_err and (" and failed to close file: " .. tostring(close_err)) or ""))
    end

    for _, address in ipairs(self.wallets[self.currentWallet].addresses or {}) do
        success, write_err = file:write(string.format("  %s\\n", address))
        if not success then
            local _, close_err = file:close()
            error("Failed to write to backup file: " .. tostring(write_err) .. (close_err and (" and failed to close file: " .. tostring(close_err)) or ""))
        end
    end

    local _, close_err = file:close()
    if close_err then
        error("Failed to close backup file: " .. tostring(close_err))
    end
    return true
end

function WalletManager:restoreWalletFromFile(path, password)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    
    local file, err = io.open(path, "r")
    if not file then
        error("Failed to open backup file: " .. tostring(err))
    end
    
    local wallet_name_line = file:read("*l")
    if not wallet_name_line then
        local _, close_err = file:close()
        error("Backup file is empty or corrupted." .. (close_err and (" and failed to close file: " .. tostring(close_err)) or ""))
    end
    
    local wallet = {
        name = wallet_name_line:match("Wallet: (.+)"),
        password = password,
        addresses = {},
        balance = 0,
        created = os.time()
    }
    
    local _ = file:read("*l") 
    local _ = file:read("*l") 
    
    for line in file:lines() do
        local address = line:match("  (.+)")
        if address then
            table.insert(wallet.addresses, address)
        end
    end
    
    local _, close_err = file:close()
    if close_err then
        error("Failed to close backup file after reading: " .. tostring(close_err))
    end
    
    self.wallets[wallet.name] = wallet
    return wallet
end

function WalletManager:verifyTransaction(transaction, signature)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    if not self.currentWallet then
        error("No wallet loaded")
    end
    
    return signature:match("^signed_") ~= nil
end

function WalletManager:createMnemonic()
    if not self.initialized then
        error("WalletManager not initialized")
    end
    
    local mnemonic = "test test test test test test test test test test test junk"
    return mnemonic, nil
end

function WalletManager:validateMnemonic(mnemonic)
    if not self.initialized then
        error("WalletManager not initialized")
    end
    
    return mnemonic == "test test test test test test test test test test test junk"
end

return WalletManager
