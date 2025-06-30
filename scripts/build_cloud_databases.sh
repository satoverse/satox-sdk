#!/bin/bash

# Satox SDK Cloud Database Integration Build Script
# This script builds the Satox SDK with Supabase and Firebase support

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command_exists apt-get; then
            echo "ubuntu"
        elif command_exists yum; then
            echo "centos"
        elif command_exists dnf; then
            echo "fedora"
        else
            echo "linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Function to install dependencies
install_dependencies() {
    local os=$(detect_os)
    print_status "Installing dependencies for $os..."
    
    case $os in
        "ubuntu"|"debian")
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                cmake \
                libcurl4-openssl-dev \
                libssl-dev \
                libspdlog-dev \
                libfmt-dev \
                nlohmann-json3-dev \
                git \
                pkg-config
            ;;
        "centos"|"rhel"|"fedora")
            if command_exists dnf; then
                sudo dnf install -y \
                    gcc-c++ \
                    cmake \
                    libcurl-devel \
                    openssl-devel \
                    spdlog-devel \
                    fmt-devel \
                    nlohmann-json-devel \
                    git \
                    pkg-config
            else
                sudo yum install -y \
                    gcc-c++ \
                    cmake \
                    libcurl-devel \
                    openssl-devel \
                    git \
                    pkg-config
                # Install spdlog, fmt, and nlohmann-json from source if not available
                install_spdlog_from_source
                install_fmt_from_source
                install_nlohmann_json_from_source
            fi
            ;;
        "macos")
            if command_exists brew; then
                brew install \
                    cmake \
                    curl \
                    openssl \
                    spdlog \
                    fmt \
                    nlohmann-json \
                    pkg-config
            else
                print_error "Homebrew not found. Please install Homebrew first."
                exit 1
            fi
            ;;
        "windows")
            print_warning "Windows support is experimental. Please install dependencies manually:"
            print_warning "- Visual Studio with C++ support"
            print_warning "- CMake"
            print_warning "- vcpkg for dependencies"
            ;;
        *)
            print_error "Unsupported operating system: $os"
            exit 1
            ;;
    esac
    
    print_success "Dependencies installed successfully"
}

# Function to install spdlog from source
install_spdlog_from_source() {
    print_status "Installing spdlog from source..."
    cd /tmp
    git clone https://github.com/gabime/spdlog.git
    cd spdlog
    mkdir build && cd build
    cmake .. -DSPDLOG_BUILD_SHARED=ON
    make -j$(nproc)
    sudo make install
    cd /tmp && rm -rf spdlog
}

# Function to install fmt from source
install_fmt_from_source() {
    print_status "Installing fmt from source..."
    cd /tmp
    git clone https://github.com/fmtlib/fmt.git
    cd fmt
    mkdir build && cd build
    cmake .. -DFMT_TEST=OFF -DFMT_DOC=OFF
    make -j$(nproc)
    sudo make install
    cd /tmp && rm -rf fmt
}

# Function to install nlohmann-json from source
install_nlohmann_json_from_source() {
    print_status "Installing nlohmann-json from source..."
    cd /tmp
    git clone https://github.com/nlohmann/json.git
    cd json
    mkdir build && cd build
    cmake .. -DJSON_BuildTests=OFF
    make -j$(nproc)
    sudo make install
    cd /tmp && rm -rf json
}

# Function to build the SDK
build_sdk() {
    print_status "Building Satox SDK with cloud database support..."
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with CMake
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=ON \
        -DBUILD_EXAMPLES=ON \
        -DCMAKE_INSTALL_PREFIX=/usr/local
    
    # Build
    make -j$(nproc)
    
    print_success "SDK built successfully"
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    
    cd build
    
    # Run unit tests
    if [ -f "tests/unit/test_cloud_integration" ]; then
        ./tests/unit/test_cloud_integration
        print_success "Unit tests passed"
    else
        print_warning "Unit tests not found"
    fi
    
    # Run integration tests
    if [ -f "tests/integration/test_cloud_integration" ]; then
        ./tests/integration/test_cloud_integration
        print_success "Integration tests passed"
    else
        print_warning "Integration tests not found"
    fi
}

# Function to build examples
build_examples() {
    print_status "Building examples..."
    
    cd build
    
    # Build C++ examples
    if [ -f "examples/cloud/supabase_example" ]; then
        print_success "Supabase example built successfully"
    fi
    
    if [ -f "examples/cloud/firebase_example" ]; then
        print_success "Firebase example built successfully"
    fi
    
    # Build binding examples
    print_status "Building binding examples..."
    
    # TypeScript
    if command_exists npm; then
        cd ../bindings/typescript
        npm install
        npm run build
        print_success "TypeScript bindings built successfully"
    fi
    
    # Python
    if command_exists python3; then
        cd ../python
        python3 setup.py build
        print_success "Python bindings built successfully"
    fi
    
    # Go
    if command_exists go; then
        cd ../go
        go build ./...
        print_success "Go bindings built successfully"
    fi
    
    # Java
    if command_exists mvn; then
        cd ../java
        mvn clean compile
        print_success "Java bindings built successfully"
    fi
    
    # C#
    if command_exists dotnet; then
        cd ../csharp
        dotnet build
        print_success "C# bindings built successfully"
    fi
}

# Function to install the SDK
install_sdk() {
    print_status "Installing SDK..."
    
    cd build
    sudo make install
    
    # Update library cache
    if command_exists ldconfig; then
        sudo ldconfig
    fi
    
    print_success "SDK installed successfully"
}

# Function to create configuration files
create_config_files() {
    print_status "Creating configuration files..."
    
    # Create config directory
    mkdir -p config/cloud
    
    # Create Supabase config template
    cat > config/cloud/supabase_config.json << 'EOF'
{
  "url": "https://your-project.supabase.co",
  "anon_key": "your-anon-key",
  "service_role_key": "your-service-role-key",
  "database_url": "postgresql://postgres:[YOUR-PASSWORD]@db.[YOUR-PROJECT-REF].supabase.co:5432/postgres",
  "enable_realtime": true,
  "connection_timeout": 30,
  "auth_scheme": "bearer",
  "enable_ssl": true,
  "schema": "public",
  "max_connections": 10,
  "enable_logging": true,
  "log_level": "info"
}
EOF
    
    # Create Firebase config template
    cat > config/cloud/firebase_config.json << 'EOF'
{
  "project_id": "your-firebase-project-id",
  "private_key_id": "your-private-key-id",
  "private_key": "-----BEGIN PRIVATE KEY-----\nYOUR_PRIVATE_KEY_HERE\n-----END PRIVATE KEY-----\n",
  "client_email": "firebase-adminsdk-xxxxx@your-project.iam.gserviceaccount.com",
  "client_id": "your-client-id",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-xxxxx%40your-project.iam.gserviceaccount.com",
  "database_url": "https://your-project.firebaseio.com",
  "enable_auth": true,
  "enable_firestore": true,
  "enable_realtime_db": false,
  "enable_storage": false,
  "enable_functions": false,
  "connection_timeout": 30,
  "max_connections": 10,
  "enable_ssl": true,
  "enable_logging": true,
  "log_level": "info"
}
EOF
    
    print_success "Configuration files created"
    print_warning "Please update the configuration files with your actual credentials"
}

# Function to run examples
run_examples() {
    print_status "Running examples..."
    
    cd build
    
    # Run C++ examples
    if [ -f "examples/cloud/supabase_example" ]; then
        print_status "Running Supabase example..."
        ./examples/cloud/supabase_example
    fi
    
    if [ -f "examples/cloud/firebase_example" ]; then
        print_status "Running Firebase example..."
        ./examples/cloud/firebase_example
    fi
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --deps-only      Install dependencies only"
    echo "  --build-only     Build SDK only"
    echo "  --test-only      Run tests only"
    echo "  --examples-only  Build examples only"
    echo "  --install-only   Install SDK only"
    echo "  --config-only    Create configuration files only"
    echo "  --run-examples   Run examples only"
    echo "  --all            Full build process (default)"
    echo "  --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --all                    # Full build process"
    echo "  $0 --deps-only              # Install dependencies only"
    echo "  $0 --build-only             # Build SDK only"
    echo "  $0 --test-only              # Run tests only"
}

# Main function
main() {
    print_status "Satox SDK Cloud Database Integration Build Script"
    print_status "Starting build process..."
    
    # Parse command line arguments
    case "${1:---all}" in
        --deps-only)
            install_dependencies
            ;;
        --build-only)
            build_sdk
            ;;
        --test-only)
            run_tests
            ;;
        --examples-only)
            build_examples
            ;;
        --install-only)
            install_sdk
            ;;
        --config-only)
            create_config_files
            ;;
        --run-examples)
            run_examples
            ;;
        --all)
            install_dependencies
            build_sdk
            run_tests
            build_examples
            install_sdk
            create_config_files
            ;;
        --help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
    
    print_success "Build process completed successfully!"
    print_status "Next steps:"
    print_status "1. Update configuration files in config/cloud/"
    print_status "2. Test your cloud database connections"
    print_status "3. Check the documentation for usage examples"
}

# Run main function
main "$@"
