package api

import (
	"context"
	"encoding/json"
	"fmt"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

// WebSocketClient represents a WebSocket API client
type WebSocketClient struct {
	conn      *websocket.Conn
	apiKey    string
	handlers  map[string]MessageHandler
	mu        sync.RWMutex
	done      chan struct{}
	reconnect bool
}

// MessageHandler is a function that handles incoming WebSocket messages
type MessageHandler func([]byte) error

// Message represents a WebSocket message
type Message struct {
	Type    string          `json:"type"`
	Payload json.RawMessage `json:"payload"`
}

// NewWebSocketClient creates a new WebSocket client
func NewWebSocketClient(url string, apiKey string) (*WebSocketClient, error) {
	dialer := websocket.Dialer{
		HandshakeTimeout: 45 * time.Second,
	}

	conn, _, err := dialer.Dial(url, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to connect to WebSocket: %w", err)
	}

	client := &WebSocketClient{
		conn:      conn,
		apiKey:    apiKey,
		handlers:  make(map[string]MessageHandler),
		done:      make(chan struct{}),
		reconnect: true,
	}

	// Set default headers
	conn.SetWriteDeadline(time.Now().Add(10 * time.Second))
	err = conn.WriteMessage(websocket.TextMessage, []byte(fmt.Sprintf(`{"type":"auth","payload":{"api_key":"%s"}}`, apiKey)))
	if err != nil {
		conn.Close()
		return nil, fmt.Errorf("failed to authenticate: %w", err)
	}

	go client.readPump()
	return client, nil
}

// readPump reads messages from the WebSocket connection
func (c *WebSocketClient) readPump() {
	defer func() {
		c.conn.Close()
		if c.reconnect {
			// Implement reconnection logic here
		}
	}()

	for {
		_, message, err := c.conn.ReadMessage()
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				fmt.Printf("WebSocket error: %v\n", err)
			}
			break
		}

		var msg Message
		if err := json.Unmarshal(message, &msg); err != nil {
			fmt.Printf("Failed to unmarshal message: %v\n", err)
			continue
		}

		c.mu.RLock()
		handler, exists := c.handlers[msg.Type]
		c.mu.RUnlock()

		if exists {
			if err := handler(msg.Payload); err != nil {
				fmt.Printf("Handler error for message type %s: %v\n", msg.Type, err)
			}
		}
	}
}

// Subscribe adds a message handler for a specific message type
func (c *WebSocketClient) Subscribe(messageType string, handler MessageHandler) {
	c.mu.Lock()
	defer c.mu.Unlock()
	c.handlers[messageType] = handler
}

// Unsubscribe removes a message handler for a specific message type
func (c *WebSocketClient) Unsubscribe(messageType string) {
	c.mu.Lock()
	defer c.mu.Unlock()
	delete(c.handlers, messageType)
}

// Send sends a message through the WebSocket connection
func (c *WebSocketClient) Send(ctx context.Context, messageType string, payload interface{}) error {
	msg := Message{
		Type:    messageType,
		Payload: json.RawMessage{},
	}

	if payload != nil {
		payloadBytes, err := json.Marshal(payload)
		if err != nil {
			return fmt.Errorf("failed to marshal payload: %w", err)
		}
		msg.Payload = payloadBytes
	}

	messageBytes, err := json.Marshal(msg)
	if err != nil {
		return fmt.Errorf("failed to marshal message: %w", err)
	}

	c.conn.SetWriteDeadline(time.Now().Add(10 * time.Second))
	return c.conn.WriteMessage(websocket.TextMessage, messageBytes)
}

// Close closes the WebSocket connection
func (c *WebSocketClient) Close() error {
	c.reconnect = false
	close(c.done)
	return c.conn.Close()
}

// SetReconnect sets whether the client should attempt to reconnect
func (c *WebSocketClient) SetReconnect(reconnect bool) {
	c.reconnect = reconnect
}

// IsConnected returns whether the client is connected
func (c *WebSocketClient) IsConnected() bool {
	return c.conn != nil
}

// SetAPIKey sets the API key
func (c *WebSocketClient) SetAPIKey(apiKey string) {
	c.apiKey = apiKey
}
