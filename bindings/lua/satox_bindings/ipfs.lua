-- IPFS management module for Satox SDK

-- IPFSManager class
local IPFSManager = {}
IPFSManager.__index = IPFSManager

function IPFSManager.new(config)
    assert(config and config.api_url and config.gateway_url, "Missing IPFS config fields")
    local self = setmetatable({}, IPFSManager)
    self.config = config
    self.http = require("http")
    return self
end

function IPFSManager:store(data)
    local resp, err = self.http.post(self.config.api_url .. "/add", {
        body = data,
        headers = { ["Content-Type"] = "application/octet-stream" }
    })
    if err then return nil, "Failed to store data in IPFS: " .. err end
    local result = resp:json()
    return result.Hash
end

function IPFSManager:retrieve(cid)
    local resp, err = self.http.get(self.config.api_url .. "/cat?arg=" .. cid)
    if err then return nil, "Failed to retrieve data from IPFS: " .. err end
    return resp.body
end

function IPFSManager:pin(cid)
    local resp, err = self.http.post(self.config.api_url .. "/pin/add?arg=" .. cid)
    if err then return nil, "Failed to pin CID in IPFS: " .. err end
    return true
end

function IPFSManager:unpin(cid)
    local resp, err = self.http.post(self.config.api_url .. "/pin/rm?arg=" .. cid)
    if err then return nil, "Failed to unpin CID from IPFS: " .. err end
    return true
end

function IPFSManager:get_gateway_url(cid)
    return self.config.gateway_url .. "/ipfs/" .. cid
end

function IPFSManager:store_file(file_path)
    local file = io.open(file_path, "rb")
    if not file then return nil, "Failed to open file" end
    local content = file:read("*all")
    file:close()
    return self:store(content)
end

function IPFSManager:retrieve_file(cid, output_path)
    local content, err = self:retrieve(cid)
    if err then return nil, err end
    local file = io.open(output_path, "wb")
    if not file then return nil, "Failed to create output file" end
    file:write(content)
    file:close()
    return true
end

function IPFSManager:list_pins()
    local resp, err = self.http.post(self.config.api_url .. "/pin/ls")
    if err then return nil, "Failed to list pins: " .. err end
    local result = resp:json()
    local pins = {}
    if result.Keys then
        for cid, _ in pairs(result.Keys) do
            table.insert(pins, cid)
        end
    end
    return pins
end

function IPFSManager:is_pinned(cid)
    local pins, err = self:list_pins()
    if err then return nil, err end
    for _, pinned_cid in ipairs(pins or {}) do
        if pinned_cid == cid then return true end
    end
    return false
end

return { new = IPFSManager.new } 