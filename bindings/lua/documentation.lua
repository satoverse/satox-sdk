local DocumentationGenerator = {}
DocumentationGenerator.__index = DocumentationGenerator

local DEFAULT_OUTPUT_FILE = "API_DOCUMENTATION.md"

function DocumentationGenerator.new()
    local self = setmetatable({}, DocumentationGenerator)
    self.initialized = false
    self.modules = {}
    self.outputFile = DEFAULT_OUTPUT_FILE
    return self
end

function DocumentationGenerator:initialize(options)
    if self.initialized then
        error("DocumentationGenerator already initialized")
    end
    options = options or {}
    self.outputFile = options.outputFile or DEFAULT_OUTPUT_FILE
    self.initialized = true
end

function DocumentationGenerator:registerModule(name, path)
    if not self.initialized then
        error("DocumentationGenerator not initialized")
    end
    if not name or type(name) ~= "string" then
        error("Module name is required")
    end
    if not path or type(path) ~= "string" then
        error("Module path is required")
    end
    self.modules[name] = path
    return true
end

function DocumentationGenerator:generate()
    if not self.initialized then
        error("DocumentationGenerator not initialized")
    end
    local docs = {}
    for name, path in pairs(self.modules) do
        local doc = self:_extractDocumentation(name, path)
        table.insert(docs, doc)
    end
    local output = table.concat(docs, "\n\n")
    local file = io.open(self.outputFile, "w")
    if not file then
        error("Failed to open documentation output file")
    end
    file:write(output)
    file:close()
    return true
end

function DocumentationGenerator:_extractDocumentation(name, path)
    local file = io.open(path, "r")
    if not file then
        return string.format("## %s\n*Documentation not found (file missing: %s)*", name, path)
    end
    local lines = {}
    for line in file:lines() do
        table.insert(lines, line)
    end
    file:close()
    local docstrings = {}
    local inDoc = false
    local docBlock = {}
    for _, line in ipairs(lines) do
        if line:match("^%-%-%-") then
            inDoc = true
            table.insert(docBlock, line:gsub("^%-%-%-", ""))
        elseif inDoc and (line:match("^%s*$") or not line:match("^%-%-")) then
            if #docBlock > 0 then
                table.insert(docstrings, table.concat(docBlock, "\n"))
                docBlock = {}
            end
            inDoc = false
        elseif inDoc then
            table.insert(docBlock, line:gsub("^%-%-", ""))
        end
    end
    if #docBlock > 0 then
        table.insert(docstrings, table.concat(docBlock, "\n"))
    end
    local docSection = string.format("## %s\n%s", name, table.concat(docstrings, "\n\n"))
    return docSection
end

return DocumentationGenerator 