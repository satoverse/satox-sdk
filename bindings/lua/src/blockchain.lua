local BlockchainManager = {}
BlockchainManager.__index = BlockchainManager

-- Constants
local BLOCK_SIZE = 1024 * 1024  -- 1MB block size
local MAX_TRANSACTIONS_PER_BLOCK = 1000
local DIFFICULTY_ADJUSTMENT_INTERVAL = 2016  -- ~2 weeks at 10 min per block

function BlockchainManager.new()
    local self = setmetatable({}, BlockchainManager)
    self.initialized = false
    self.blocks = {}
    self.transactions = {}
    self.pendingTransactions = {}
    self.difficulty = 1
    self.lastDifficultyAdjustment = 0
    return self
end

function BlockchainManager:initialize()
    if self.initialized then
        error("BlockchainManager already initialized")
    end
    self.initialized = true
end

function BlockchainManager:getBlock(hash)
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    if not hash then
        error("Invalid block hash")
    end

    return self.blocks[hash]
end

function BlockchainManager:getLatestBlock()
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    local latestBlock = nil
    local latestHeight = -1

    for _, block in pairs(self.blocks) do
        if block.height > latestHeight then
            latestBlock = block
            latestHeight = block.height
        end
    end

    return latestBlock
end

function BlockchainManager:getBlockByHeight(height)
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    if not height or height < 0 then
        error("Invalid block height")
    end

    for _, block in pairs(self.blocks) do
        if block.height == height then
            return block
        end
    end

    return nil
end

function BlockchainManager:getTransaction(hash)
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    if not hash then
        error("Invalid transaction hash")
    end

    -- First check pending transactions
    if self.pendingTransactions[hash] then
        return self.pendingTransactions[hash]
    end

    -- Then check confirmed transactions
    return self.transactions[hash]
end

function BlockchainManager:addTransaction(transaction)
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    if not transaction or not transaction.hash then
        error("Invalid transaction")
    end

    -- Validate transaction
    if not self:_validateTransaction(transaction) then
        error("Invalid transaction")
    end

    -- Add to pending transactions
    self.pendingTransactions[transaction.hash] = transaction
    return true
end

function BlockchainManager:_validateTransaction(transaction)
    -- Basic validation
    if not transaction.inputs or not transaction.outputs then
        return false
    end

    -- Check input/output amounts
    local inputSum = 0
    local outputSum = 0

    for _, input in ipairs(transaction.inputs) do
        if not input.amount or input.amount <= 0 then
            return false
        end
        inputSum = inputSum + input.amount
    end

    for _, output in ipairs(transaction.outputs) do
        if not output.amount or output.amount <= 0 then
            return false
        end
        outputSum = outputSum + output.amount
    end

    -- Check if outputs don't exceed inputs
    if outputSum > inputSum then
        return false
    end

    return true
end

function BlockchainManager:createBlock(transactions, minerAddress)
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    if not minerAddress then
        error("Invalid miner address")
    end

    local latestBlock = self:getLatestBlock()
    local newBlock = {
        hash = self:_generateBlockHash(),
        previousHash = latestBlock and latestBlock.hash or nil,
        height = latestBlock and latestBlock.height + 1 or 0,
        timestamp = os.time(),
        transactions = transactions or {},
        minerAddress = minerAddress,
        difficulty = self.difficulty,
        nonce = 0
    }

    -- Mine the block
    while not self:_isValidBlock(newBlock) do
        newBlock.nonce = newBlock.nonce + 1
        newBlock.hash = self:_generateBlockHash()
    end

    -- Add block to chain
    self.blocks[newBlock.hash] = newBlock

    -- Move transactions from pending to confirmed
    for _, tx in ipairs(newBlock.transactions) do
        self.transactions[tx.hash] = tx
        self.pendingTransactions[tx.hash] = nil
    end

    -- Adjust difficulty if needed
    self:_adjustDifficulty(newBlock)

    return newBlock
end

function BlockchainManager:_generateBlockHash()
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would use a proper hashing algorithm
    return "block_" .. os.time() .. "_" .. math.random(1000000)
end

function BlockchainManager:_isValidBlock(block)
    -- This is a placeholder for the actual implementation
    -- In a real implementation, this would verify the block's hash meets the difficulty target
    return true
end

function BlockchainManager:_adjustDifficulty(block)
    if block.height % DIFFICULTY_ADJUSTMENT_INTERVAL ~= 0 then
        return
    end

    local previousAdjustmentBlock = self:getBlockByHeight(self.lastDifficultyAdjustment)
    local timeExpected = DIFFICULTY_ADJUSTMENT_INTERVAL * 600  -- 10 minutes per block
    local timeActual = block.timestamp - previousAdjustmentBlock.timestamp

    if timeActual < timeExpected / 2 then
        self.difficulty = self.difficulty + 1
    elseif timeActual > timeExpected * 2 then
        self.difficulty = math.max(1, self.difficulty - 1)
    end

    self.lastDifficultyAdjustment = block.height
end

function BlockchainManager:getBalance(address)
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    if not address then
        error("Invalid address")
    end

    local balance = 0

    -- Check confirmed transactions
    for _, tx in pairs(self.transactions) do
        for _, input in ipairs(tx.inputs) do
            if input.address == address then
                balance = balance - input.amount
            end
        end
        for _, output in ipairs(tx.outputs) do
            if output.address == address then
                balance = balance + output.amount
            end
        end
    end

    -- Check pending transactions
    for _, tx in pairs(self.pendingTransactions) do
        for _, input in ipairs(tx.inputs) do
            if input.address == address then
                balance = balance - input.amount
            end
        end
        for _, output in ipairs(tx.outputs) do
            if output.address == address then
                balance = balance + output.amount
            end
        end
    end

    return balance
end

function BlockchainManager:getPendingTransactions()
    if not self.initialized then
        error("BlockchainManager not initialized")
    end

    local pending = {}
    for _, tx in pairs(self.pendingTransactions) do
        table.insert(pending, tx)
    end
    return pending
end

return BlockchainManager 