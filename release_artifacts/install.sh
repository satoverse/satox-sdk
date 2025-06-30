#!/bin/bash
# Satox SDK Installation Script

set -e

echo "Installing Satox SDK..."

# Install libraries
sudo cp *.so /usr/local/lib/ 2>/dev/null || true
sudo cp *.a /usr/local/lib/ 2>/dev/null || true

# Install headers
sudo cp -r include/* /usr/local/include/ 2>/dev/null || true

# Update library cache
sudo ldconfig

echo "Satox SDK installed successfully!"
