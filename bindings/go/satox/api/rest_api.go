package api

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"sync"
	"time"
)

type RESTAPI struct {
	mu sync.Mutex
}

func NewRESTAPI() *RESTAPI {
	return &RESTAPI{}
}

func (r *RESTAPI) Initialize() error {
	return nil
}

func (r *RESTAPI) Shutdown() error {
	return nil
}

func (r *RESTAPI) StartServer() error {
	r.mu.Lock()
	defer r.mu.Unlock()
	// Implement server start logic here
	return nil
}

func (r *RESTAPI) StopServer() error {
	r.mu.Lock()
	defer r.mu.Unlock()
	// Implement server stop logic here
	return nil
}

func (r *RESTAPI) HandleRequest(endpoint string, data []byte) ([]byte, error) {
	r.mu.Lock()
	defer r.mu.Unlock()
	// Implement request handling logic here
	return nil, errors.New("Request handling failed")
}

// RestAPIClient represents a REST API client
type RestAPIClient struct {
	baseURL    string
	httpClient *http.Client
	apiKey     string
}

// NewRestAPIClient creates a new REST API client
func NewRestAPIClient(baseURL string, apiKey string) *RestAPIClient {
	return &RestAPIClient{
		baseURL: baseURL,
		httpClient: &http.Client{
			Timeout: time.Second * 30,
		},
		apiKey: apiKey,
	}
}

// Request represents an API request
type Request struct {
	Method  string
	Path    string
	Body    interface{}
	Headers map[string]string
}

// Response represents an API response
type Response struct {
	StatusCode int
	Body       []byte
	Headers    http.Header
}

// Do sends an API request and returns the response
func (c *RestAPIClient) Do(ctx context.Context, req Request) (*Response, error) {
	var bodyReader io.Reader
	if req.Body != nil {
		jsonBody, err := json.Marshal(req.Body)
		if err != nil {
			return nil, fmt.Errorf("failed to marshal request body: %w", err)
		}
		bodyReader = bytes.NewReader(jsonBody)
	}

	httpReq, err := http.NewRequestWithContext(ctx, req.Method, c.baseURL+req.Path, bodyReader)
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	// Set default headers
	httpReq.Header.Set("Content-Type", "application/json")
	httpReq.Header.Set("X-API-Key", c.apiKey)

	// Set custom headers
	for key, value := range req.Headers {
		httpReq.Header.Set(key, value)
	}

	resp, err := c.httpClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("failed to send request: %w", err)
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	return &Response{
		StatusCode: resp.StatusCode,
		Body:       body,
		Headers:    resp.Header,
	}, nil
}

// Get sends a GET request
func (c *RestAPIClient) Get(ctx context.Context, path string, headers map[string]string) (*Response, error) {
	return c.Do(ctx, Request{
		Method:  http.MethodGet,
		Path:    path,
		Headers: headers,
	})
}

// Post sends a POST request
func (c *RestAPIClient) Post(ctx context.Context, path string, body interface{}, headers map[string]string) (*Response, error) {
	return c.Do(ctx, Request{
		Method:  http.MethodPost,
		Path:    path,
		Body:    body,
		Headers: headers,
	})
}

// Put sends a PUT request
func (c *RestAPIClient) Put(ctx context.Context, path string, body interface{}, headers map[string]string) (*Response, error) {
	return c.Do(ctx, Request{
		Method:  http.MethodPut,
		Path:    path,
		Body:    body,
		Headers: headers,
	})
}

// Delete sends a DELETE request
func (c *RestAPIClient) Delete(ctx context.Context, path string, headers map[string]string) (*Response, error) {
	return c.Do(ctx, Request{
		Method:  http.MethodDelete,
		Path:    path,
		Headers: headers,
	})
}

// Patch sends a PATCH request
func (c *RestAPIClient) Patch(ctx context.Context, path string, body interface{}, headers map[string]string) (*Response, error) {
	return c.Do(ctx, Request{
		Method:  http.MethodPatch,
		Path:    path,
		Body:    body,
		Headers: headers,
	})
}

// SetTimeout sets the client timeout
func (c *RestAPIClient) SetTimeout(timeout time.Duration) {
	c.httpClient.Timeout = timeout
}

// SetAPIKey sets the API key
func (c *RestAPIClient) SetAPIKey(apiKey string) {
	c.apiKey = apiKey
}

// SetBaseURL sets the base URL
func (c *RestAPIClient) SetBaseURL(baseURL string) {
	c.baseURL = baseURL
}
