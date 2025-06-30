# API Clients Documentation

## Overview
The API clients provide a simple interface for interacting with various APIs. This documentation covers the REST, WebSocket, and GraphQL clients, including usage examples, best practices, error handling, and performance optimization tips.

## Installation
```lua
local RestClient = require("satox.api.rest_client")
local WebSocketClient = require("satox.api.websocket_client")
local GraphQLClient = require("satox.api.graphql_client")
```

## Usage

### REST Client
```lua
local client = RestClient.new()
client:initialize("https://api.example.com", "your-api-key")
local success, result, err = client:get("/users")
if success then
    print("REST result:", result)
else
    print("Error:", err)
end
```

### WebSocket Client
```lua
local client = WebSocketClient.new()
client:initialize("wss://api.example.com/ws", "your-api-key")
local connected = client:connect()
if connected then
    client:send("Hello, WebSocket!")
    client:disconnect()
else
    print("Failed to connect to WebSocket")
end
```

### GraphQL Client
```lua
local client = GraphQLClient.new()
client:initialize("https://api.example.com/graphql", "your-api-key")
local query = [[
    query { user { id name } }
]]
local success, result, err = client:query(query)
if success then
    print("GraphQL result:", result)
else
    print("Error:", err)
end
```

## Best Practices
1. Always check the success flag before using the result.
2. Handle errors appropriately in your application.
3. Use appropriate timeout values for your use case.
4. Configure retry settings based on your network conditions.
5. Clean up resources (e.g., WebSocket connections) when they are no longer needed.

## Error Handling
The clients return errors in a consistent format:

- `success`: boolean indicating if the operation succeeded.
- `result`: the operation result (if successful).
- `err`: error message (if unsuccessful).

Common error messages:

- "REST client not initialized"
- "Invalid base URL"
- "Invalid API key"
- "WebSocket connection failed"
- "GraphQL client not initialized"
- "Invalid query"

## Performance Optimization
1. **Connection Pooling:** Use connection pooling to reuse connections and reduce overhead.
2. **Request Batching:** Batch multiple requests to reduce the number of network calls.
3. **Caching Strategies:** Implement caching to store frequently accessed data and reduce API calls.

## Examples

### Complete Example
```lua
local RestClient = require("satox.api.rest_client")
local WebSocketClient = require("satox.api.websocket_client")
local GraphQLClient = require("satox.api.graphql_client")

-- Initialize clients
local restClient = RestClient.new()
local wsClient = WebSocketClient.new()
local gqlClient = GraphQLClient.new()

restClient:initialize("https://api.example.com", "your-api-key")
wsClient:initialize("wss://api.example.com/ws", "your-api-key")
gqlClient:initialize("https://api.example.com/graphql", "your-api-key")

-- Execute REST request
local restResult, restErr = restClient:get("/users")
if restResult then
    print("REST result:", restResult)
else
    print("REST error:", restErr)
end

-- Connect to WebSocket
local connected = wsClient:connect()
if connected then
    wsClient:send("Hello, WebSocket!")
    wsClient:disconnect()
else
    print("Failed to connect to WebSocket")
end

-- Execute GraphQL query
local query = [[
    query { user { id name } }
]]
local gqlResult, gqlErr = gqlClient:query(query)
if gqlResult then
    print("GraphQL result:", gqlResult)
else
    print("GraphQL error:", gqlErr)
end
```

## API Reference

### RestClient.new()
Creates a new REST client instance.

### client:initialize(url, apiKey)
Initializes the client with the base URL and API key.

### client:get(endpoint, params)
Executes a GET request.

### client:post(endpoint, data)
Executes a POST request.

### client:put(endpoint, data)
Executes a PUT request.

### client:delete(endpoint)
Executes a DELETE request.

### WebSocketClient.new()
Creates a new WebSocket client instance.

### client:initialize(url, apiKey)
Initializes the client with the WebSocket URL and API key.

### client:connect()
Connects to the WebSocket server.

### client:send(message)
Sends a message to the WebSocket server.

### client:disconnect()
Disconnects from the WebSocket server.

### GraphQLClient.new()
Creates a new GraphQL client instance.

### client:initialize(url, apiKey)
Initializes the client with the GraphQL endpoint URL and API key.

### client:query(query)
Executes a GraphQL query.

### client:mutation(mutation)
Executes a GraphQL mutation.

### client:subscribe(subscription, callback)
Subscribes to a GraphQL subscription with a callback function.

### client:unsubscribe(subscriptionId)
Unsubscribes from a previously created subscription.
