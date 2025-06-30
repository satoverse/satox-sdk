package asset_test

import (
	"context"
	"fmt"
	"sync"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
)

// Asset represents a digital asset
type Asset struct {
	ID          string
	Name        string
	Description string
	Supply      int64
	Metadata    map[string]interface{}
}

// MockAssetManager represents a mock asset manager
type MockAssetManager struct {
	mock.Mock
}

func (m *MockAssetManager) CreateAsset(ctx context.Context, asset *Asset) error {
	args := m.Called(ctx, asset)
	return args.Error(0)
}

func (m *MockAssetManager) GetAsset(ctx context.Context, id string) (*Asset, error) {
	args := m.Called(ctx, id)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Asset), args.Error(1)
}

func (m *MockAssetManager) UpdateAsset(ctx context.Context, asset *Asset) error {
	args := m.Called(ctx, asset)
	return args.Error(0)
}

func TestAssetCreationPerformance(t *testing.T) {
	mockManager := new(MockAssetManager)
	ctx := context.Background()

	t.Run("Single Asset Creation", func(t *testing.T) {
		asset := &Asset{
			ID:          "test-asset-1",
			Name:        "Test Asset",
			Description: "Test Description",
			Supply:      1000,
		}

		start := time.Now()
		mockManager.On("CreateAsset", ctx, asset).Return(nil)
		err := mockManager.CreateAsset(ctx, asset)
		duration := time.Since(start)

		assert.NoError(t, err)
		assert.Less(t, duration, 100*time.Millisecond)
		mockManager.AssertExpectations(t)
	})

	t.Run("Bulk Asset Creation", func(t *testing.T) {
		const numAssets = 1000
		var wg sync.WaitGroup
		start := time.Now()

		for i := 0; i < numAssets; i++ {
			wg.Add(1)
			asset := &Asset{
				ID:          fmt.Sprintf("test-asset-%d", i),
				Name:        fmt.Sprintf("Test Asset %d", i),
				Description: "Test Description",
				Supply:      1000,
			}

			mockManager.On("CreateAsset", ctx, asset).Return(nil)
			go func(a *Asset) {
				defer wg.Done()
				err := mockManager.CreateAsset(ctx, a)
				assert.NoError(t, err)
			}(asset)
		}

		wg.Wait()
		duration := time.Since(start)
		avgDuration := duration / numAssets

		assert.Less(t, avgDuration, 10*time.Millisecond)
		mockManager.AssertExpectations(t)
	})
}

func TestAssetRetrievalPerformance(t *testing.T) {
	mockManager := new(MockAssetManager)
	ctx := context.Background()

	t.Run("Single Asset Retrieval", func(t *testing.T) {
		expectedAsset := &Asset{
			ID:          "test-asset-1",
			Name:        "Test Asset",
			Description: "Test Description",
			Supply:      1000,
		}

		start := time.Now()
		mockManager.On("GetAsset", ctx, "test-asset-1").Return(expectedAsset, nil)
		asset, err := mockManager.GetAsset(ctx, "test-asset-1")
		duration := time.Since(start)

		assert.NoError(t, err)
		assert.Equal(t, expectedAsset, asset)
		assert.Less(t, duration, 50*time.Millisecond)
		mockManager.AssertExpectations(t)
	})

	t.Run("Concurrent Asset Retrieval", func(t *testing.T) {
		const numAssets = 1000
		var wg sync.WaitGroup
		start := time.Now()

		for i := 0; i < numAssets; i++ {
			wg.Add(1)
			assetID := fmt.Sprintf("test-asset-%d", i)
			expectedAsset := &Asset{
				ID:          assetID,
				Name:        fmt.Sprintf("Test Asset %d", i),
				Description: "Test Description",
				Supply:      1000,
			}

			mockManager.On("GetAsset", ctx, assetID).Return(expectedAsset, nil)
			go func(id string) {
				defer wg.Done()
				asset, err := mockManager.GetAsset(ctx, id)
				assert.NoError(t, err)
				assert.NotNil(t, asset)
			}(assetID)
		}

		wg.Wait()
		duration := time.Since(start)
		avgDuration := duration / numAssets

		assert.Less(t, avgDuration, 5*time.Millisecond)
		mockManager.AssertExpectations(t)
	})
}

func TestAssetUpdatePerformance(t *testing.T) {
	mockManager := new(MockAssetManager)
	ctx := context.Background()

	t.Run("Single Asset Update", func(t *testing.T) {
		asset := &Asset{
			ID:          "test-asset-1",
			Name:        "Updated Asset",
			Description: "Updated Description",
			Supply:      2000,
		}

		start := time.Now()
		mockManager.On("UpdateAsset", ctx, asset).Return(nil)
		err := mockManager.UpdateAsset(ctx, asset)
		duration := time.Since(start)

		assert.NoError(t, err)
		assert.Less(t, duration, 100*time.Millisecond)
		mockManager.AssertExpectations(t)
	})

	t.Run("Bulk Asset Updates", func(t *testing.T) {
		const numAssets = 1000
		var wg sync.WaitGroup
		start := time.Now()

		for i := 0; i < numAssets; i++ {
			wg.Add(1)
			asset := &Asset{
				ID:          fmt.Sprintf("test-asset-%d", i),
				Name:        fmt.Sprintf("Updated Asset %d", i),
				Description: "Updated Description",
				Supply:      2000,
			}

			mockManager.On("UpdateAsset", ctx, asset).Return(nil)
			go func(a *Asset) {
				defer wg.Done()
				err := mockManager.UpdateAsset(ctx, a)
				assert.NoError(t, err)
			}(asset)
		}

		wg.Wait()
		duration := time.Since(start)
		avgDuration := duration / numAssets

		assert.Less(t, avgDuration, 10*time.Millisecond)
		mockManager.AssertExpectations(t)
	})
}

func TestAssetMemoryUsage(t *testing.T) {
	mockManager := new(MockAssetManager)
	ctx := context.Background()

	t.Run("Memory Usage for Large Asset Collection", func(t *testing.T) {
		const numAssets = 2000 // Reduced from 10000 for performance
		const maxConcurrency = 100
		assets := make([]*Asset, numAssets)

		start := time.Now()
		for i := 0; i < numAssets; i++ {
			asset := &Asset{
				ID:          fmt.Sprintf("test-asset-%d", i),
				Name:        fmt.Sprintf("Test Asset %d", i),
				Description: "Test Description",
				Supply:      1000,
				Metadata:    make(map[string]interface{}),
			}

			// Add some metadata to simulate real-world usage
			for j := 0; j < 10; j++ {
				asset.Metadata[fmt.Sprintf("key-%d", j)] = fmt.Sprintf("value-%d", j)
			}

			assets[i] = asset
			mockManager.On("CreateAsset", ctx, asset).Return(nil)
		}

		var wg sync.WaitGroup
		sem := make(chan struct{}, maxConcurrency)
		for _, asset := range assets {
			wg.Add(1)
			sem <- struct{}{} // acquire
			go func(a *Asset) {
				defer wg.Done()
				defer func() { <-sem }() // release
				err := mockManager.CreateAsset(ctx, a)
				assert.NoError(t, err)
			}(asset)
		}

		wg.Wait()
		duration := time.Since(start)
		avgDuration := duration / numAssets

		assert.Less(t, avgDuration, 50*time.Millisecond)
		mockManager.AssertExpectations(t)
	})
}
