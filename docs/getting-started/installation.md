# Installation Guide

This guide provides step-by-step instructions for installing and setting up the Satox SDK on your development environment.

## Prerequisites

Before installing the Satox SDK, ensure your system meets the following requirements:

### System Requirements
- **Operating System**: Linux (Ubuntu 20.04+), macOS (10.15+), or Windows 10+
- **Memory**: Minimum 8GB RAM (16GB recommended)
- **Storage**: At least 10GB free disk space
- **Network**: Stable internet connection for dependency downloads

### Required Dependencies
- **CMake**: Version 3.14 or higher
- **GCC/Clang**: C++17 compatible compiler
- **Python**: Version 3.8 or higher
- **Git**: Version 2.20 or higher
- **Docker**: Version 20.10 or higher (optional, for containerized deployment)

## Installation Methods

### Method 1: Source Installation (Recommended)

#### Step 1: Clone the Repository
```bash
git clone https://github.com/satox/satox-sdk.git
cd satox-sdk
```

#### Step 2: Install System Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install -y build-essential cmake git python3 python3-pip
sudo apt install -y libssl-dev libboost-all-dev libsqlite3-dev
```

**macOS:**
```bash
brew install cmake boost openssl sqlite3
```

**Windows:**
```bash
# Install Visual Studio Build Tools
# Install CMake from https://cmake.org/download/
# Install Git from https://git-scm.com/download/win
```

#### Step 3: Install Python Dependencies
```bash
pip3 install -r requirements.txt
```

#### Step 4: Build the SDK
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Step 5: Install the SDK
```bash
sudo make install
```

### Method 2: Docker Installation

#### Step 1: Pull the Docker Image
```bash
docker pull satox/satox-sdk:latest
```

#### Step 2: Run the Container
```bash
docker run -it --name satox-sdk satox/satox-sdk:latest
```

### Method 3: Package Manager Installation

#### Ubuntu/Debian (APT)
```bash
# Add Satox repository
wget -qO- https://packages.satox.com/gpg | sudo apt-key add -
echo "deb https://packages.satox.com/ubuntu focal main" | sudo tee /etc/apt/sources.list.d/satox.list

# Install SDK
sudo apt update
sudo apt install satox-sdk
```

#### macOS (Homebrew)
```bash
brew tap satox/sdk
brew install satox-sdk
```

## Configuration

### Environment Setup

#### Step 1: Set Environment Variables
```bash
export SATOX_SDK_HOME=/usr/local/satox-sdk
export PATH=$PATH:$SATOX_SDK_HOME/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SATOX_SDK_HOME/lib
```

#### Step 2: Create Configuration File
```bash
mkdir -p ~/.satox
cp $SATOX_SDK_HOME/config/satox.conf.example ~/.satox/satox.conf
```

#### Step 3: Edit Configuration
```bash
nano ~/.satox/satox.conf
```

Example configuration:
```ini
[network]
node_url = https://mainnet.satox.com
api_key = your_api_key_here

[database]
path = ~/.satox/database
type = sqlite

[security]
encryption_level = high
key_derivation = pbkdf2
```

### Database Setup

#### Step 1: Initialize Database
```bash
satox-sdk init-db
```

#### Step 2: Verify Database
```bash
satox-sdk verify-db
```

## Verification

### Test Installation

#### Step 1: Run Basic Tests
```bash
cd build
ctest --output-on-failure
```

#### Step 2: Run SDK Examples
```bash
cd examples
./basic_wallet_example
./blockchain_example
```

#### Step 3: Verify API Access
```bash
satox-sdk --version
satox-sdk --help
```

## Troubleshooting

### Common Issues

#### Build Failures
- **CMake not found**: Install CMake 3.14+
- **Compiler errors**: Ensure C++17 support
- **Missing dependencies**: Install required system packages

#### Runtime Errors
- **Library not found**: Set LD_LIBRARY_PATH
- **Permission denied**: Check file permissions
- **Database errors**: Verify database initialization

#### Network Issues
- **Connection timeout**: Check firewall settings
- **API errors**: Verify API key and endpoint

### Getting Help

- Check the [troubleshooting guide](../troubleshooting/common-issues.md)
- Review [error codes](../troubleshooting/error-codes.md)
- Create an issue on GitHub
- Join the developer community

## Next Steps

After successful installation:

1. Read the [Quick Start Guide](quick-start.md)
2. Explore [Basic Examples](../examples/basic-examples.md)
3. Review [API Reference](../api/core-api-reference.md)
4. Set up your [Development Environment](../development/development-setup.md)

## Uninstallation

### Remove Source Installation
```bash
cd build
sudo make uninstall
rm -rf ~/.satox
```

### Remove Docker Installation
```bash
docker stop satox-sdk
docker rm satox-sdk
docker rmi satox/satox-sdk:latest
```

### Remove Package Installation
```bash
# Ubuntu/Debian
sudo apt remove satox-sdk

# macOS
brew uninstall satox-sdk
```

---

**Last Updated**: $(date '+%Y-%m-%d %H:%M:%S')
**SDK Version**: 1.0.0 