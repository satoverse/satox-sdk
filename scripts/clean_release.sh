#!/bin/bash

# Satox SDK Clean Release Script
# This script cleans up build artifacts and temporary files for release

set -e

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

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --dry-run     Show what would be cleaned without actually cleaning"
    echo "  --help        Show this help message"
    echo "  --verbose     Show detailed output"
    echo ""
    echo "This script cleans up build artifacts and temporary files for release."
}

# Parse command line arguments
DRY_RUN=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
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
done

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

print_status "Starting Satox SDK cleanup process..."
print_status "Project root: $PROJECT_ROOT"
print_status "Dry run mode: $DRY_RUN"
print_status "Verbose mode: $VERBOSE"

# Function to safely remove files/directories
safe_remove() {
    local target="$1"
    local description="$2"
    
    if [[ -e "$target" ]]; then
        if [[ "$DRY_RUN" == "true" ]]; then
            print_warning "Would remove: $target ($description)"
        else
            if [[ "$VERBOSE" == "true" ]]; then
                print_status "Removing: $target ($description)"
            fi
            rm -rf "$target"
            print_success "Removed: $target"
        fi
    else
        if [[ "$VERBOSE" == "true" ]]; then
            print_status "Skipping (not found): $target"
        fi
    fi
}

# Function to clean build directories
clean_build_dirs() {
    print_status "Cleaning build directories..."
    
    local build_dirs=(
        "build"
        "build_release"
        "build_debug"
        "build_test"
        "cmake-build-*"
        ".cmake"
    )
    
    for dir in "${build_dirs[@]}"; do
        safe_remove "$PROJECT_ROOT/$dir" "build directory"
    done
}

# Function to clean temporary files
clean_temp_files() {
    print_status "Cleaning temporary files..."
    
    local temp_patterns=(
        "*.o"
        "*.obj"
        "*.so"
        "*.dylib"
        "*.dll"
        "*.a"
        "*.lib"
        "*.exe"
        "*.out"
        "*.log"
        "*.tmp"
        "*.temp"
        "*.swp"
        "*.swo"
        "*~"
        ".DS_Store"
        "Thumbs.db"
    )
    
    for pattern in "${temp_patterns[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "temporary file"
        done
    done
}

# Function to clean Python artifacts
clean_python_artifacts() {
    print_status "Cleaning Python artifacts..."
    
    local python_dirs=(
        "__pycache__"
        "*.pyc"
        "*.pyo"
        "*.pyd"
        "*.egg-info"
        "dist"
        "build"
        ".pytest_cache"
        ".coverage"
        "htmlcov"
    )
    
    for pattern in "${python_dirs[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type d 2>/dev/null | while read -r dir; do
            safe_remove "$dir" "Python cache directory"
        done
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "Python artifact"
        done
    done
}

# Function to clean Node.js artifacts
clean_nodejs_artifacts() {
    print_status "Cleaning Node.js artifacts..."
    
    local nodejs_dirs=(
        "node_modules"
        "npm-debug.log*"
        "yarn-debug.log*"
        "yarn-error.log*"
        ".nyc_output"
        "coverage"
    )
    
    for pattern in "${nodejs_dirs[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type d 2>/dev/null | while read -r dir; do
            safe_remove "$dir" "Node.js directory"
        done
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "Node.js log file"
        done
    done
}

# Function to clean Rust artifacts
clean_rust_artifacts() {
    print_status "Cleaning Rust artifacts..."
    
    local rust_dirs=(
        "target"
        "Cargo.lock"
    )
    
    for pattern in "${rust_dirs[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type d 2>/dev/null | while read -r dir; do
            safe_remove "$dir" "Rust build directory"
        done
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "Rust lock file"
        done
    done
}

# Function to clean Go artifacts
clean_go_artifacts() {
    print_status "Cleaning Go artifacts..."
    
    local go_dirs=(
        "go.sum"
        "vendor"
    )
    
    for pattern in "${go_dirs[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "Go artifact"
        done
    done
}

# Function to clean Java artifacts
clean_java_artifacts() {
    print_status "Cleaning Java artifacts..."
    
    local java_dirs=(
        "target"
        "*.class"
        "*.jar"
        "*.war"
    )
    
    for pattern in "${java_dirs[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type d 2>/dev/null | while read -r dir; do
            safe_remove "$dir" "Java build directory"
        done
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "Java artifact"
        done
    done
}

# Function to clean .NET artifacts
clean_dotnet_artifacts() {
    print_status "Cleaning .NET artifacts..."
    
    local dotnet_dirs=(
        "bin"
        "obj"
        "*.dll"
        "*.exe"
        "*.pdb"
    )
    
    for pattern in "${dotnet_dirs[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type d 2>/dev/null | while read -r dir; do
            safe_remove "$dir" ".NET build directory"
        done
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" ".NET artifact"
        done
    done
}

# Function to clean IDE artifacts
clean_ide_artifacts() {
    print_status "Cleaning IDE artifacts..."
    
    local ide_dirs=(
        ".vscode"
        ".idea"
        "*.sublime-*"
        ".vs"
        "*.swp"
        "*.swo"
        "*~"
    )
    
    for pattern in "${ide_dirs[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type d 2>/dev/null | while read -r dir; do
            safe_remove "$dir" "IDE directory"
        done
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "IDE file"
        done
    done
}

# Function to clean documentation artifacts
clean_doc_artifacts() {
    print_status "Cleaning documentation artifacts..."
    
    local doc_dirs=(
        "docs/html"
        "docs/latex"
        "docs/man"
        "docs/rtf"
        "docs/xml"
    )
    
    for dir in "${doc_dirs[@]}"; do
        safe_remove "$PROJECT_ROOT/$dir" "documentation build directory"
    done
}

# Function to clean test artifacts
clean_test_artifacts() {
    print_status "Cleaning test artifacts..."
    
    local test_files=(
        "test_*.db"
        "test_*.sqlite"
        "test_*.log"
        "coverage.xml"
        "junit.xml"
        "test-results.xml"
    )
    
    for pattern in "${test_files[@]}"; do
        find "$PROJECT_ROOT" -name "$pattern" -type f 2>/dev/null | while read -r file; do
            safe_remove "$file" "test artifact"
        done
    done
}

# Function to clean release artifacts (but keep the directory)
clean_release_artifacts() {
    print_status "Cleaning release artifacts..."
    
    if [[ -d "$PROJECT_ROOT/release_artifacts" ]]; then
        if [[ "$DRY_RUN" == "true" ]]; then
            print_warning "Would clean contents of: $PROJECT_ROOT/release_artifacts"
        else
            if [[ "$VERBOSE" == "true" ]]; then
                print_status "Cleaning contents of: $PROJECT_ROOT/release_artifacts"
            fi
            rm -rf "$PROJECT_ROOT/release_artifacts"/*
            print_success "Cleaned release artifacts directory"
        fi
    fi
}

# Main cleanup process
main() {
    print_status "Starting comprehensive cleanup..."
    
    # Change to project root
    cd "$PROJECT_ROOT"
    
    # Run all cleanup functions
    clean_build_dirs
    clean_temp_files
    clean_python_artifacts
    clean_nodejs_artifacts
    clean_rust_artifacts
    clean_go_artifacts
    clean_java_artifacts
    clean_dotnet_artifacts
    clean_ide_artifacts
    clean_doc_artifacts
    clean_test_artifacts
    clean_release_artifacts
    
    if [[ "$DRY_RUN" == "true" ]]; then
        print_success "Dry run completed - no files were actually removed"
    else
        print_success "Cleanup completed successfully!"
    fi
    
    print_status "Repository is now clean and ready for release"
}

# Run main function
main "$@"
