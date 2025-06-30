package satox

import (
	"testing"
)

func TestSimpleCGO(t *testing.T) {
	// Test that CGO is working by calling a simple C function
	version := GetVersion()
	if version == "" {
		t.Error("Expected non-empty version string")
	}
	t.Logf("SDK Version: %s", version)
}
