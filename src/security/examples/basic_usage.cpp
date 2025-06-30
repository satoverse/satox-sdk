/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "satox/security/security_manager.hpp"
#include <iostream>

int main() {
    try {
        // Get the security manager instance
        auto& security = satox::security::SecurityManager::getInstance();
        
        // Initialize the security manager
        satox::security::SecurityConfig config;
        config.name = "example_security";
        config.enablePQC = true;
        config.enableInputValidation = true;
        config.enableRateLimiting = true;
        config.enableLogging = true;
        
        if (!security.initialize(config)) {
            std::cerr << "Failed to initialize security manager: " 
                      << security.getLastError() << std::endl;
            return 1;
        }
        
        std::cout << "Security manager initialized successfully!" << std::endl;
        
        // Test PQC operations
        std::cout << "Testing PQC operations..." << std::endl;
        if (security.generatePQCKeyPair("ML-DSA")) {
            std::cout << "PQC key pair generated successfully!" << std::endl;
        }
        
        // Test input validation
        std::cout << "Testing input validation..." << std::endl;
        if (security.validateEmail("user@example.com")) {
            std::cout << "Email validation successful!" << std::endl;
        }
        
        if (security.validateURL("https://example.com")) {
            std::cout << "URL validation successful!" << std::endl;
        }
        
        // Test rate limiting
        std::cout << "Testing rate limiting..." << std::endl;
        if (security.checkRateLimit("user123", "login")) {
            std::cout << "Rate limit check successful!" << std::endl;
        }
        
        // Get security statistics
        auto stats = security.getStats();
        std::cout << "Security operations performed: " << stats.totalOperations << std::endl;
        std::cout << "Successful operations: " << stats.successfulOperations << std::endl;
        std::cout << "Failed operations: " << stats.failedOperations << std::endl;
        
        // Shutdown
        security.shutdown();
        std::cout << "Security manager shut down successfully!" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
