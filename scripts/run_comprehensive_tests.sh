#!/bin/bash

# ==========================================
# Satox SDK Comprehensive Test Suite
# Enhanced version with better error handling and reporting
# ==========================================

set -e  # Exit on any error

# Configuration
BUILD_DIR="build"
LOG_DIR="${BUILD_DIR}/test_logs"
TIMESTAMP=$(date '+%Y%m%d_%H%M%S')
SUMMARY_LOG="${BUILD_DIR}/test_summary_${TIMESTAMP}.log"
DETAILED_LOG="${BUILD_DIR}/test_detailed_${TIMESTAMP}.log"
TEST_TIMEOUT=300  # 5 minutes per test suite
RETRY_COUNT=2
MAX_PARALLEL_TESTS=4

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
TIMEOUT_TESTS=0
SKIPPED_TESTS=0

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

# Function to check if a test executable exists and is runnable
check_test_executable() {
    local test_path=$1
    if [[ ! -f "${test_path}" ]]; then
        log_message "ERROR" "Test executable not found: ${test_path}"
        return 1
    fi
    if [[ ! -x "${test_path}" ]]; then
        log_message "ERROR" "Test executable not executable: ${test_path}"
        return 1
    fi
    return 0
}

# Function to run a single test with timeout and retry logic
run_test_with_timeout() {
    local test_name=$1
    local test_path=$2
    local test_args=$3
    local timeout_seconds=$4
    
    log_message "INFO" "Starting test: ${test_name}"
    
    # Check if test executable exists
    if ! check_test_executable "${test_path}"; then
        log_message "ERROR" "Skipping ${test_name} - executable not found"
        ((SKIPPED_TESTS++))
        return 1
    fi
    
    local retry_count=0
    local success=false
    
    while [[ $retry_count -le $RETRY_COUNT && $success == false ]]; do
        if [[ $retry_count -gt 0 ]]; then
            log_message "WARN" "Retrying ${test_name} (attempt $((retry_count + 1)))"
            sleep 2  # Brief pause between retries
        fi
        
        # Run test with timeout
        if timeout "${timeout_seconds}" "${test_path}" ${test_args} > "${LOG_DIR}/${test_name}.log" 2>&1; then
            success=true
            log_message "INFO" "Test ${test_name} completed successfully"
            ((PASSED_TESTS++))
        else
            local exit_code=$?
            if [[ $exit_code -eq 124 ]]; then
                log_message "ERROR" "Test ${test_name} timed out after ${timeout_seconds} seconds"
                ((TIMEOUT_TESTS++))
                break  # Don't retry timeouts
            else
                log_message "ERROR" "Test ${test_name} failed with exit code ${exit_code}"
                ((retry_count++))
            fi
        fi
    done
    
    if [[ $success == false ]]; then
        log_message "ERROR" "Test ${test_name} failed after ${RETRY_COUNT} retries"
        ((FAILED_TESTS++))
        return 1
    fi
    
    return 0
}

# Function to run tests in parallel with limited concurrency
run_parallel_tests() {
    local tests=("$@")
    local running_jobs=()
    local max_jobs=$MAX_PARALLEL_TESTS
    
    for test_info in "${tests[@]}"; do
        IFS='|' read -r test_name test_path test_args <<< "${test_info}"
        
        # Wait if we've reached max parallel jobs
        while [[ ${#running_jobs[@]} -ge $max_jobs ]]; do
            for i in "${!running_jobs[@]}"; do
                if ! kill -0 "${running_jobs[$i]}" 2>/dev/null; then
                    unset "running_jobs[$i]"
                fi
            done
            running_jobs=("${running_jobs[@]}")  # Reindex array
            sleep 1
        done
        
        # Start the test in background
        run_test_with_timeout "${test_name}" "${test_path}" "${test_args}" $TEST_TIMEOUT &
        local job_pid=$!
        running_jobs+=($job_pid)
        ((TOTAL_TESTS++))
        
        log_message "INFO" "Started test ${test_name} with PID ${job_pid}"
    done
    
    # Wait for all remaining jobs
    for job in "${running_jobs[@]}"; do
        if kill -0 "$job" 2>/dev/null; then
            log_message "INFO" "Waiting for job ${job} to complete"
            wait "$job"
        fi
    done
}

# Function to check environment
check_environment() {
    print_status $BLUE "Checking environment..."
    
    # Check if we're in the right directory
    if [[ ! -f "CMakeLists.txt" ]]; then
        print_status $RED "Error: CMakeLists.txt not found. Please run from the project root."
        exit 1
    fi
    
    # Check if build directory exists
    if [[ ! -d "${BUILD_DIR}" ]]; then
        print_status $RED "Error: Build directory '${BUILD_DIR}' not found. Please build the project first."
        exit 1
    fi
    
    # Create log directory
    mkdir -p "${LOG_DIR}"
    if [[ ! -d "${LOG_DIR}" ]]; then
        print_status $RED "Error: Failed to create log directory '${LOG_DIR}'"
        exit 1
    fi
    
    # Check if test executables exist
    local test_executables=(
        "${BUILD_DIR}/src/core/tests/satox-core-tests"
        "${BUILD_DIR}/src/wallet/tests/satox-wallet-tests"
        "${BUILD_DIR}/src/database/tests/database_tests"
        "${BUILD_DIR}/src/asset/satox-asset-tests"
        "${BUILD_DIR}/src/network/satox-network-tests"
        "${BUILD_DIR}/src/security/tests/security_tests"
        "${BUILD_DIR}/src/ipfs/tests/ipfs_tests"
        "${BUILD_DIR}/src/nft/tests/nft_tests"
        "${BUILD_DIR}/src/blockchain/tests/satox-blockchain-tests"
    )
    
    local missing_tests=0
    for test_exe in "${test_executables[@]}"; do
        if [[ ! -f "${test_exe}" ]]; then
            print_status $YELLOW "Warning: Test executable not found: ${test_exe}"
            ((missing_tests++))
        fi
    done
    
    if [[ $missing_tests -gt 0 ]]; then
        print_status $YELLOW "Warning: ${missing_tests} test executables are missing. Some tests will be skipped."
    fi
    
    # Set library path for all tests
    export LD_LIBRARY_PATH="${BUILD_DIR}/src/core:${BUILD_DIR}/src/asset:${BUILD_DIR}/src/network:${BUILD_DIR}/src/blockchain:${BUILD_DIR}/src/security:${BUILD_DIR}/src/database:${BUILD_DIR}/src/wallet:${BUILD_DIR}/src/ipfs:${BUILD_DIR}/src/nft:${BUILD_DIR}/satox-assets:${BUILD_DIR}/satox-transactions:${LD_LIBRARY_PATH}"
    
    print_status $GREEN "Environment check completed"
}

# Function to generate test summary
generate_summary() {
    local end_time=$(date '+%Y-%m-%d %H:%M:%S')
    
    cat > "${SUMMARY_LOG}" << EOF
==========================================
Satox SDK Test Summary Report
Generated: ${end_time}
==========================================

Test Results:
- Total Tests: ${TOTAL_TESTS}
- Passed: ${PASSED_TESTS}
- Failed: ${FAILED_TESTS}
- Timeouts: ${TIMEOUT_TESTS}
- Skipped: ${SKIPPED_TESTS}

Success Rate: $(( (PASSED_TESTS * 100) / TOTAL_TESTS ))% (${PASSED_TESTS}/${TOTAL_TESTS})

Detailed logs available in: ${LOG_DIR}
Full test log: ${DETAILED_LOG}

EOF

    if [[ $FAILED_TESTS -gt 0 || $TIMEOUT_TESTS -gt 0 ]]; then
        print_status $RED "Test suite completed with ${FAILED_TESTS} failures and ${TIMEOUT_TESTS} timeouts"
        return 1
    else
        print_status $GREEN "All tests passed successfully!"
        return 0
    fi
}

# Function to check for known issues and provide recommendations
check_known_issues() {
    print_status $BLUE "Checking for known issues..."
    
    # Check for segmentation faults in logs
    if grep -q "Segmentation fault\|core dumped" "${LOG_DIR}"/*.log 2>/dev/null; then
        print_status $YELLOW "Warning: Segmentation faults detected in test logs"
        print_status $YELLOW "Recommendation: Check for memory issues and buffer overflows"
    fi
    
    # Check for OpenSSL deprecation warnings
    if grep -q "deprecated.*OpenSSL" "${LOG_DIR}"/*.log 2>/dev/null; then
        print_status $YELLOW "Warning: OpenSSL deprecation warnings detected"
        print_status $YELLOW "Recommendation: Update to OpenSSL 3.0 compatible APIs"
    fi
    
    # Check for missing dependencies
    if grep -q "library not found\|symbol not found" "${LOG_DIR}"/*.log 2>/dev/null; then
        print_status $YELLOW "Warning: Missing library dependencies detected"
        print_status $YELLOW "Recommendation: Install missing development libraries"
    fi
}

# Main execution
main() {
    print_status $BLUE "=========================================="
    print_status $BLUE "Satox SDK Comprehensive Test Suite"
    print_status $BLUE "Date: $(date)"
    print_status $BLUE "Build Directory: ${BUILD_DIR}"
    print_status $BLUE "=========================================="
    
    # Initialize logging
    echo "Satox SDK Test Run - $(date)" > "${DETAILED_LOG}"
    
    # Check environment
    check_environment
    
    print_status $BLUE "Starting comprehensive test suite..."
    
    # Define test suites with their paths and arguments
    local test_suites=(
        "Core Tests|${BUILD_DIR}/src/core/tests/satox-core-tests|--gtest_output=xml:${LOG_DIR}/core_tests.xml"
        "Wallet Tests|${BUILD_DIR}/src/wallet/tests/satox-wallet-tests|--gtest_output=xml:${LOG_DIR}/wallet_tests.xml"
        "Database Tests|${BUILD_DIR}/src/database/tests/database_tests|--gtest_output=xml:${LOG_DIR}/database_tests.xml"
        "Asset Tests|${BUILD_DIR}/src/asset/satox-asset-tests|--gtest_output=xml:${LOG_DIR}/asset_tests.xml"
        "Network Tests|${BUILD_DIR}/src/network/satox-network-tests|--gtest_output=xml:${LOG_DIR}/network_tests.xml"
        "Security Tests|${BUILD_DIR}/src/security/tests/security_tests|--gtest_output=xml:${LOG_DIR}/security_tests.xml"
        "IPFS Tests|${BUILD_DIR}/src/ipfs/tests/ipfs_tests|--gtest_output=xml:${LOG_DIR}/ipfs_tests.xml"
        "NFT Tests|${BUILD_DIR}/src/nft/tests/nft_tests|--gtest_output=xml:${LOG_DIR}/nft_tests.xml"
        "Blockchain Tests|${BUILD_DIR}/src/blockchain/tests/satox-blockchain-tests|--gtest_output=xml:${LOG_DIR}/blockchain_tests.xml"
    )
    
    # Run tests in parallel
    run_parallel_tests "${test_suites[@]}"
    
    # Check for known issues
    check_known_issues
    
    # Generate summary
    generate_summary
    
    # Display summary
    cat "${SUMMARY_LOG}"
    
    # Exit with appropriate code
    if [[ $FAILED_TESTS -gt 0 || $TIMEOUT_TESTS -gt 0 ]]; then
        exit 1
    else
        exit 0
    fi
}

# Run main function
main "$@" 