package api

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"time"
)

// GraphQLClient represents a GraphQL API client
type GraphQLClient struct {
	baseURL    string
	httpClient *http.Client
	apiKey     string
}

// GraphQLRequest represents a GraphQL request
type GraphQLRequest struct {
	Query         string                 `json:"query"`
	Variables     map[string]interface{} `json:"variables,omitempty"`
	OperationName string                 `json:"operationName,omitempty"`
}

// GraphQLResponse represents a GraphQL response
type GraphQLResponse struct {
	Data       json.RawMessage        `json:"data"`
	Errors     []GraphQLError         `json:"errors,omitempty"`
	Extensions map[string]interface{} `json:"extensions,omitempty"`
}

// GraphQLError represents a GraphQL error
type GraphQLError struct {
	Message    string                 `json:"message"`
	Locations  []ErrorLocation        `json:"locations,omitempty"`
	Path       []interface{}          `json:"path,omitempty"`
	Extensions map[string]interface{} `json:"extensions,omitempty"`
}

// ErrorLocation represents the location of a GraphQL error
type ErrorLocation struct {
	Line   int `json:"line"`
	Column int `json:"column"`
}

// NewGraphQLClient creates a new GraphQL client
func NewGraphQLClient(baseURL string, apiKey string) *GraphQLClient {
	return &GraphQLClient{
		baseURL: baseURL,
		httpClient: &http.Client{
			Timeout: time.Second * 30,
		},
		apiKey: apiKey,
	}
}

// Execute sends a GraphQL request and returns the response
func (c *GraphQLClient) Execute(ctx context.Context, req GraphQLRequest) (*GraphQLResponse, error) {
	jsonBody, err := json.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal request: %w", err)
	}

	httpReq, err := http.NewRequestWithContext(ctx, http.MethodPost, c.baseURL, bytes.NewReader(jsonBody))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	// Set headers
	httpReq.Header.Set("Content-Type", "application/json")
	httpReq.Header.Set("X-API-Key", c.apiKey)

	resp, err := c.httpClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to send request: %w", err)
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	var graphqlResp GraphQLResponse
	if err := json.Unmarshal(body, &graphqlResp); err != nil {
		return nil, fmt.Errorf("failed to unmarshal response: %w", err)
	}

	if len(graphqlResp.Errors) > 0 {
		return &graphqlResp, fmt.Errorf("graphql errors: %v", graphqlResp.Errors)
	}

	return &graphqlResp, nil
}

// Query sends a GraphQL query
func (c *GraphQLClient) Query(ctx context.Context, query string, variables map[string]interface{}, operationName string) (*GraphQLResponse, error) {
	return c.Execute(ctx, GraphQLRequest{
		Query:         query,
		Variables:     variables,
		OperationName: operationName,
	})
}

// Mutation sends a GraphQL mutation
func (c *GraphQLClient) Mutation(ctx context.Context, mutation string, variables map[string]interface{}, operationName string) (*GraphQLResponse, error) {
	return c.Execute(ctx, GraphQLRequest{
		Query:         mutation,
		Variables:     variables,
		OperationName: operationName,
	})
}

// SetTimeout sets the client timeout
func (c *GraphQLClient) SetTimeout(timeout time.Duration) {
	c.httpClient.Timeout = timeout
}

// SetAPIKey sets the API key
func (c *GraphQLClient) SetAPIKey(apiKey string) {
	c.apiKey = apiKey
}

// SetBaseURL sets the base URL
func (c *GraphQLClient) SetBaseURL(baseURL string) {
	c.baseURL = baseURL
}

// Example usage:
/*
client := NewGraphQLClient("https://api.example.com/graphql", "your-api-key")

query := `
query GetUser($id: ID!) {
    user(id: $id) {
        id
        name
        email
    }
}
`

variables := map[string]interface{}{
    "id": "123",
}

resp, err := client.Query(context.Background(), query, variables, "GetUser")
if err != nil {
    log.Fatal(err)
}

var result struct {
    User struct {
        ID    string `json:"id"`
        Name  string `json:"name"`
        Email string `json:"email"`
    } `json:"user"`
}

if err := json.Unmarshal(resp.Data, &result); err != nil {
    log.Fatal(err)
}

fmt.Printf("User: %+v\n", result.User)
*/
