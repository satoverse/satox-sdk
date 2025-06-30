# GraphQL Client Documentation

## Overview
The GraphQL client provides a simple interface for interacting with GraphQL APIs. It supports queries, mutations, and subscriptions with built-in error handling and retry mechanisms.

## Installation
```lua
local GraphQLClient = require("satox.api.graphql_client")
```

## Usage

### Initialization
```lua
local client = GraphQLClient.new()
client:initialize("https://api.example.com/graphql", "your-api-key")
```

### Configuration

#### Timeout
```lua
-- Set timeout in seconds
client:setTimeout(60)
```

#### Retry Configuration
```lua
-- Set retry attempts and delay in seconds
client:setRetryConfig(5, 2)
```

### Queries
```lua
local query = [[
    query { user { id name } }
]]
local success, result, err = client:query(query)
if success then
    print("Query result:", result)
else
    print("Error:", err)
end
```

### Mutations
```lua
local mutation = [[
    mutation { createUser(name: "Test User") { id name } }
]]
local success, result, err = client:mutation(mutation)
if success then
    print("Mutation result:", result)
else
    print("Error:", err)
end
```

### Subscriptions
```lua
local subscription = [[
    subscription { userCreated { id name } }
]]
local callback = function(data)
    print("Received subscription data:", data)
end
local success, subId, err = client:subscribe(subscription, callback)
if success then
    -- Later, to unsubscribe:
    client:unsubscribe(subId)
else
    print("Error:", err)
end
```

## Error Handling
The client returns errors in a consistent format:

- `success`: boolean indicating if the operation succeeded
- `result`: the operation result (if successful)
- `err`: error message (if unsuccessful)

Common error messages:

- "GraphQLClient not initialized"
- "Invalid URL"
- "Invalid API key"
- "Invalid query"
- "Invalid mutation"
- "Invalid subscription"
- "Invalid callback"
- "Invalid subscription ID"

## Best Practices

1. Always check the success flag before using the result
2. Handle errors appropriately in your application
3. Use appropriate timeout values for your use case
4. Configure retry settings based on your network conditions
5. Clean up subscriptions when they are no longer needed

## Examples

### Complete Example
```lua
local GraphQLClient = require("satox.api.graphql_client")

-- Initialize client
local client = GraphQLClient.new()
client:initialize("https://api.example.com/graphql", "your-api-key")

-- Configure client
client:setTimeout(60)
client:setRetryConfig(5, 2)

-- Execute query
local query = [[
    query { user { id name } }
]]
local success, result, err = client:query(query)
if success then
    print("Query result:", result)
else
    print("Error:", err)
end

-- Execute mutation
local mutation = [[
    mutation { createUser(name: "Test User") { id name } }
]]
success, result, err = client:mutation(mutation)
if success then
    print("Mutation result:", result)
else
    print("Error:", err)
end

-- Handle subscription
local subscription = [[
    subscription { userCreated { id name } }
]]
local callback = function(data)
    print("Received subscription data:", data)
end
success, subId, err = client:subscribe(subscription, callback)
if success then
    -- Later, to unsubscribe:
    client:unsubscribe(subId)
else
    print("Error:", err)
end
```

## API Reference

### GraphQLClient.new()
Creates a new GraphQL client instance.

### client:initialize(url, apiKey)
Initializes the client with the GraphQL endpoint URL and API key.

### client:setTimeout(timeout)
Sets the timeout for requests in seconds.

### client:setRetryConfig(attempts, delay)
Sets the number of retry attempts and delay between retries in seconds.

### client:query(query)
Executes a GraphQL query.

### client:mutation(mutation)
Executes a GraphQL mutation.

### client:subscribe(subscription, callback)
Subscribes to a GraphQL subscription with a callback function.

### client:unsubscribe(subscriptionId)
Unsubscribes from a previously created subscription.
