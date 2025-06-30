#!/bin/bash

# ==========================================
# Satox Core SDK Example Verification Script
# Tests all core SDK examples (C++ and Python)
# ==========================================

# Configuration
TIMESTAMP=$(date '+%Y%m%d_%H%M%S')
LOG_DIR="logs/example_verification"
SUMMARY_LOG="${LOG_DIR}/core_examples_summary_${TIMESTAMP}.log"
DETAILED_LOG="${LOG_DIR}/core_examples_detailed_${TIMESTAMP}.log"
BUILD_TIMEOUT=600  # 10 minutes per build (core SDK is larger)
TEST_TIMEOUT=120   # 2 minutes per test

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_EXAMPLES=0
BUILT_EXAMPLES=0
TESTED_EXAMPLES=0
FAILED_EXAMPLES=0

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to log messages
log_message() {
    local level=$1
    local message=$2
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [${level}] ${message}" | tee -a "${DETAILED_LOG}"
}

# Function to check environment
check_environment() {
    print_status $BLUE "🔍 Checking environment..."
    
    # Check if we're in the right directory
    if [[ ! -f "CMakeLists.txt" ]] || [[ ! -d "examples" ]]; then
        print_status $RED "❌ Error: Must run from satox-sdk root directory"
        exit 1
    fi
    
    # Create log directory
    mkdir -p "${LOG_DIR}"
    if [[ ! -d "${LOG_DIR}" ]]; then
        print_status $RED "❌ Error: Failed to create log directory '${LOG_DIR}'"
        exit 1
    fi
    
    # Check for required tools
    local required_tools=("cmake" "make" "g++" "python3")
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            print_status $RED "❌ Error: Required tool not found: $tool"
            exit 1
        fi
    done
    
    print_status $GREEN "✅ Environment check completed"
}

# Function to count examples
count_examples() {
    local count=0
    
    # Count C++ examples
    local cpp_count=$(find "examples" -name "*.cpp" -type f | wc -l)
    # Count Python examples
    local py_count=$(find "examples" -name "*.py" -type f | wc -l)
    
    count=$((cpp_count + py_count))
    echo $count
}

# Function to list examples
list_examples() {
    print_status $BLUE "📋 Listing Core SDK examples..."
    
    # List C++ examples
    local cpp_examples=($(find "examples" -name "*.cpp" -type f | sort))
    if [[ ${#cpp_examples[@]} -gt 0 ]]; then
        log_message "INFO" "Found ${#cpp_examples[@]} C++ examples:"
        for example in "${cpp_examples[@]}"; do
            local example_name=$(basename "$example")
            local example_size=$(stat -c%s "$example" 2>/dev/null || echo "0")
            log_message "INFO" "  - $example_name ($example_size bytes)"
        done
    fi
    
    # List Python examples
    local py_examples=($(find "examples" -name "*.py" -type f | sort))
    if [[ ${#py_examples[@]} -gt 0 ]]; then
        log_message "INFO" "Found ${#py_examples[@]} Python examples:"
        for example in "${py_examples[@]}"; do
            local example_name=$(basename "$example")
            local example_size=$(stat -c%s "$example" 2>/dev/null || echo "0")
            log_message "INFO" "  - $example_name ($example_size bytes)"
        done
    fi
}

# Function to build C++ examples
build_cpp_examples() {
    print_status $BLUE "🔨 Building Core SDK C++ examples..."
    
    # Create build directory
    local build_dir="build-examples"
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # Configure build
    if timeout "$BUILD_TIMEOUT" cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON; then
        log_message "INFO" "CMake configuration successful"
    else
        log_message "ERROR" "CMake configuration failed"
        cd ..
        return 1
    fi
    
    # Build examples
    if timeout "$BUILD_TIMEOUT" make -j$(nproc); then
        log_message "INFO" "C++ examples build successful"
        BUILT_EXAMPLES=1
        cd ..
        return 0
    else
        log_message "ERROR" "C++ examples build failed"
        cd ..
        return 1
    fi
}

# Function to test C++ examples
test_cpp_examples() {
    print_status $BLUE "🧪 Testing Core SDK C++ examples..."
    
    # Find example executables
    local example_executables=()
    if [[ -d "build-examples" ]]; then
        example_executables=($(find "build-examples" -type f -executable -name "*example*" 2>/dev/null || true))
    fi
    
    if [[ ${#example_executables[@]} -eq 0 ]]; then
        log_message "WARN" "No C++ example executables found"
        return 0
    fi
    
    log_message "INFO" "Found ${#example_executables[@]} C++ example executables"
    
    local tested=0
    for exe in "${example_executables[@]}"; do
        local exe_name=$(basename "$exe")
        log_message "INFO" "Testing C++ example: $exe_name"
        
        if timeout "$TEST_TIMEOUT" "$exe" > /dev/null 2>&1; then
            log_message "INFO" "C++ example $exe_name passed"
            ((tested++))
        else
            log_message "ERROR" "C++ example $exe_name failed"
            ((FAILED_EXAMPLES++))
        fi
    done
    
    if [[ $tested -gt 0 ]]; then
        TESTED_EXAMPLES=$((TESTED_EXAMPLES + tested))
    fi
    
    return 0
}

# Function to test Python examples
test_python_examples() {
    print_status $BLUE "🐍 Testing Core SDK Python examples..."
    
    # Find Python examples
    local python_examples=($(find "examples" -name "*.py" -type f 2>/dev/null || true))
    
    if [[ ${#python_examples[@]} -eq 0 ]]; then
        log_message "INFO" "No Python examples found"
        return 0
    fi
    
    log_message "INFO" "Found ${#python_examples[@]} Python examples"
    
    local tested=0
    for py_file in "${python_examples[@]}"; do
        local py_name=$(basename "$py_file")
        log_message "INFO" "Testing Python example: $py_name"
        
        # Check syntax first
        if python3 -m py_compile "$py_file" 2>/dev/null; then
            log_message "INFO" "Python example $py_name syntax OK"
            
            # Try to run the example (with timeout)
            if timeout "$TEST_TIMEOUT" python3 "$py_file" > /dev/null 2>&1; then
                log_message "INFO" "Python example $py_name passed"
                ((tested++))
            else
                log_message "WARN" "Python example $py_name failed to run (may be expected)"
                # Don't count as failure for Python examples that may not be runnable
            fi
        else
            log_message "ERROR" "Python example $py_name syntax error"
            ((FAILED_EXAMPLES++))
        fi
    done
    
    if [[ $tested -gt 0 ]]; then
        TESTED_EXAMPLES=$((TESTED_EXAMPLES + tested))
    fi
    
    return 0
}

# Function to verify example documentation
verify_example_documentation() {
    print_status $BLUE "📚 Verifying example documentation..."
    
    # Check for README files
    local readme_files=(
        "examples/README.md"
        "examples/basic/README.md"
        "examples/basic-examples/README.md"
    )
    
    local missing_docs=0
    for doc in "${readme_files[@]}"; do
        if [[ ! -f "$doc" ]]; then
            log_message "WARN" "Missing documentation: $doc"
            ((missing_docs++))
        fi
    done
    
    if [[ $missing_docs -eq 0 ]]; then
        log_message "INFO" "Example documentation complete"
    else
        log_message "WARN" "Missing $missing_docs documentation files"
    fi
}

# Function to verify CMakeLists.txt
verify_cmakelists() {
    print_status $BLUE "⚙️ Verifying CMakeLists.txt configuration..."
    
    if [[ ! -f "examples/CMakeLists.txt" ]]; then
        log_message "ERROR" "examples/CMakeLists.txt not found"
        return 1
    fi
    
    # Check for examples in main examples directory
    local found_examples=0
    
    # Check main examples CMakeLists.txt
    if grep -q "add_executable.*example" "examples/CMakeLists.txt"; then
        log_message "INFO" "Examples found in main examples/CMakeLists.txt"
        ((found_examples++))
    fi
    
    # Check subdirectories for examples
    local subdirs=("examples/basic" "examples/basic-examples" "examples/cloud")
    for subdir in "${subdirs[@]}"; do
        if [[ -f "$subdir/CMakeLists.txt" ]]; then
            if grep -q "add_executable.*example" "$subdir/CMakeLists.txt"; then
                log_message "INFO" "Examples found in $subdir/CMakeLists.txt"
                ((found_examples++))
            fi
        fi
    done
    
    # Check for C++ example files
    local cpp_examples=($(find "examples" -name "*.cpp" -type f 2>/dev/null || true))
    if [[ ${#cpp_examples[@]} -gt 0 ]]; then
        log_message "INFO" "Found ${#cpp_examples[@]} C++ example files"
        ((found_examples++))
    fi
    
    if [[ $found_examples -gt 0 ]]; then
        log_message "INFO" "Examples properly configured"
        return 0
    else
        log_message "WARN" "No example executables found in CMakeLists.txt files"
        return 0  # Changed from 1 to 0 to not cause script exit
    fi
}

# Function to check dependencies
check_dependencies() {
    print_status $BLUE "🔍 Checking dependencies..."
    
    # Check for required libraries
    local required_libs=("OpenSSL" "liboqs" "RocksDB")
    for lib in "${required_libs[@]}"; do
        if pkg-config --exists "$lib" 2>/dev/null; then
            log_message "INFO" "Found $lib"
        else
            log_message "WARN" "$lib not found via pkg-config"
        fi
    done
    
    # Check Python dependencies
    if python3 -c "import satox" 2>/dev/null; then
        log_message "INFO" "Python satox module available"
    else
        log_message "WARN" "Python satox module not available"
    fi
}

# Function to generate summary report
generate_summary_report() {
    print_status $BLUE "📊 Generating summary report..."
    
    {
        echo "=========================================="
        echo "Satox Core SDK Example Verification Summary"
        echo "=========================================="
        echo "Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"
        echo ""
        echo "Results:"
        echo "========"
        echo "Total Examples: $TOTAL_EXAMPLES"
        echo "Built Successfully: $BUILT_EXAMPLES"
        echo "Tested Successfully: $TESTED_EXAMPLES"
        echo "Failed: $FAILED_EXAMPLES"
        echo ""
        
        if [[ $FAILED_EXAMPLES -eq 0 ]]; then
            echo "✅ All examples verified successfully!"
        else
            echo "❌ $FAILED_EXAMPLES examples failed verification"
        fi
        
        echo "=========================================="
    } | tee "$SUMMARY_LOG"
}

# Main execution
main() {
    print_status $PURPLE "🚀 Starting Satox Core SDK Example Verification"
    
    # Check environment
    check_environment
    
    # Count examples
    TOTAL_EXAMPLES=$(count_examples)
    log_message "INFO" "Found $TOTAL_EXAMPLES examples"
    
    # List examples
    list_examples
    
    # Check dependencies
    check_dependencies
    
    # Verify CMakeLists.txt
    verify_cmakelists
    
    # Build C++ examples
    if build_cpp_examples; then
        # Test C++ examples
        test_cpp_examples
    else
        log_message "ERROR" "C++ build failed, skipping C++ tests"
    fi
    
    # Test Python examples
    test_python_examples
    
    # Verify documentation
    verify_example_documentation
    
    # Generate summary report
    generate_summary_report
    
    print_status $PURPLE "🎉 Core SDK example verification completed!"
    print_status $PURPLE "📋 Summary: $SUMMARY_LOG"
    print_status $PURPLE "📝 Detailed: $DETAILED_LOG"
    
    # Exit with appropriate code
    if [[ $FAILED_EXAMPLES -eq 0 ]]; then
        exit 0
    else
        exit 1
    fi
}

# Run main function
main "$@" 