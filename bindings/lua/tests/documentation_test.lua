local luaunit = require('luaunit')
local DocumentationGenerator = require('../documentation')

TestDocumentationGenerator = {}

function TestDocumentationGenerator:setUp()
    self.generator = DocumentationGenerator.new()
    self.generator:initialize({ outputFile = "test_api_doc.md" })
end

function TestDocumentationGenerator:tearDown()
    os.remove("test_api_doc.md")
    os.remove("test_module.lua")
end

function TestDocumentationGenerator:testInitialization()
    local gen = DocumentationGenerator.new()
    luaunit.assertFalse(gen.initialized)
    gen:initialize()
    luaunit.assertTrue(gen.initialized)
    luaunit.assertError(function()
        gen:initialize()
    end, "DocumentationGenerator already initialized")
end

function TestDocumentationGenerator:testRegisterModule()
    local success = self.generator:registerModule("TestModule", "test_module.lua")
    luaunit.assertTrue(success)
    luaunit.assertEquals(self.generator.modules["TestModule"], "test_module.lua")
    luaunit.assertError(function()
        self.generator:registerModule(nil, "test_module.lua")
    end, "Module name is required")
    luaunit.assertError(function()
        self.generator:registerModule("TestModule2", nil)
    end, "Module path is required")
end

function TestDocumentationGenerator:testGenerateWithDocstrings()
    -- Write a test module with docstrings
    local file = io.open("test_module.lua", "w")
    file:write([[--- TestModule
--- This is a test module.
local TestModule = {}

--- testFunction
--- This function does something.
function TestModule.testFunction()
end

return TestModule
]])
    file:close()
    self.generator:registerModule("TestModule", "test_module.lua")
    local success = self.generator:generate()
    luaunit.assertTrue(success)
    local docFile = io.open("test_api_doc.md", "r")
    luaunit.assertNotNil(docFile)
    local content = docFile:read("*all")
    docFile:close()
    luaunit.assertStrContains(content, "TestModule")
    luaunit.assertStrContains(content, "This is a test module.")
    luaunit.assertStrContains(content, "testFunction")
    luaunit.assertStrContains(content, "does something")
end

function TestDocumentationGenerator:testGenerateWithMissingFile()
    self.generator:registerModule("MissingModule", "missing_file.lua")
    local success = self.generator:generate()
    luaunit.assertTrue(success)
    local docFile = io.open("test_api_doc.md", "r")
    luaunit.assertNotNil(docFile)
    local content = docFile:read("*all")
    docFile:close()
    luaunit.assertStrContains(content, "Documentation not found")
end

-- Run tests
os.exit(luaunit.LuaUnit.run()) 