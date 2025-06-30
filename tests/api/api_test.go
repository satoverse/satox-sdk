package api_test

import (
	"context"
	"errors"
	"sync"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

// MockAPI represents a mock API client
type MockAPI struct {
	mock.Mock
}

func (m *MockAPI) Get(ctx context.Context, endpoint string) ([]byte, error) {
	args := m.Called(ctx, endpoint)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).([]byte), args.Error(1)
}

func (m *MockAPI) Post(ctx context.Context, endpoint string, data interface{}) ([]byte, error) {
	args := m.Called(ctx, endpoint, data)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).([]byte), args.Error(1)
}

func TestAPIClient(t *testing.T) {
	mockAPI := new(MockAPI)
	ctx := context.Background()

	t.Run("Get Request", func(t *testing.T) {
		expectedResponse := []byte(`{"status":"success"}`)
		mockAPI.On("Get", ctx, "/test").Return(expectedResponse, nil)

		response, err := mockAPI.Get(ctx, "/test")
		assert.NoError(t, err)
		assert.Equal(t, expectedResponse, response)
		mockAPI.AssertExpectations(t)
	})

	t.Run("Post Request", func(t *testing.T) {
		expectedResponse := []byte(`{"status":"created"}`)
		testData := map[string]string{"key": "value"}
		mockAPI.On("Post", ctx, "/test", testData).Return(expectedResponse, nil)

		response, err := mockAPI.Post(ctx, "/test", testData)
		assert.NoError(t, err)
		assert.Equal(t, expectedResponse, response)
		mockAPI.AssertExpectations(t)
	})

	t.Run("Timeout Handling", func(t *testing.T) {
		ctx, cancel := context.WithTimeout(ctx, 100*time.Millisecond)
		defer cancel()

		mockAPI.On("Get", ctx, "/slow").Return(nil, context.DeadlineExceeded)

		_, err := mockAPI.Get(ctx, "/slow")
		assert.Error(t, err)
		assert.Equal(t, context.DeadlineExceeded, err)
		mockAPI.AssertExpectations(t)
	})

	t.Run("Error Handling", func(t *testing.T) {
		mockAPI.On("Get", ctx, "/error").Return(nil, errors.New("test error"))

		_, err := mockAPI.Get(ctx, "/error")
		assert.Error(t, err)
		assert.Equal(t, "test error", err.Error())
		mockAPI.AssertExpectations(t)
	})
}

func TestAPIRateLimiting(t *testing.T) {
	mockAPI := new(MockAPI)
	ctx := context.Background()

	t.Run("Rate Limit Exceeded", func(t *testing.T) {
		mockAPI.On("Get", ctx, "/rate-limited").Return(nil, errors.New("rate limit exceeded"))

		_, err := mockAPI.Get(ctx, "/rate-limited")
		assert.Error(t, err)
		assert.Contains(t, err.Error(), "rate limit exceeded")
		mockAPI.AssertExpectations(t)
	})

	t.Run("Concurrent Requests", func(t *testing.T) {
		expectedResponse := []byte(`{"status":"success"}`)
		mockAPI.On("Get", ctx, "/concurrent").Return(expectedResponse, nil).Times(5)

		var wg sync.WaitGroup
		for i := 0; i < 5; i++ {
			wg.Add(1)
			go func() {
				defer wg.Done()
				response, err := mockAPI.Get(ctx, "/concurrent")
				assert.NoError(t, err)
				assert.Equal(t, expectedResponse, response)
			}()
		}
		wg.Wait()
		mockAPI.AssertExpectations(t)
	})
}

func TestAPIErrorHandling(t *testing.T) {
	mockAPI := new(MockAPI)
	ctx := context.Background()

	t.Run("Invalid Response", func(t *testing.T) {
		invalidResponse := []byte(`invalid json`)
		mockAPI.On("Get", ctx, "/invalid").Return(invalidResponse, nil)

		_, err := mockAPI.Get(ctx, "/invalid")
		assert.NoError(t, err)
		mockAPI.AssertExpectations(t)
	})

	t.Run("Server Error", func(t *testing.T) {
		mockAPI.On("Get", ctx, "/server-error").Return(nil, errors.New("500 Internal Server Error"))

		_, err := mockAPI.Get(ctx, "/server-error")
		assert.Error(t, err)
		assert.Contains(t, err.Error(), "500")
		mockAPI.AssertExpectations(t)
	})

	t.Run("Network Error", func(t *testing.T) {
		mockAPI.On("Get", ctx, "/network-error").Return(nil, errors.New("connection refused"))

		_, err := mockAPI.Get(ctx, "/network-error")
		assert.Error(t, err)
		assert.Contains(t, err.Error(), "connection refused")
		mockAPI.AssertExpectations(t)
	})
}
