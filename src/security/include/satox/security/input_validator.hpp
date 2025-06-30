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

#pragma once

#include <string>
#include <regex>
#include <unordered_map>
#include <functional>
#include <memory>

namespace satox {
namespace security {

class InputValidator {
public:
    InputValidator();
    ~InputValidator();

    // Initialize the input validator
    bool initialize();

    // Shutdown the input validator
    void shutdown();

    // Validate input based on type
    bool validate(const std::string& input, const std::string& type);

    // Register a custom validator
    void registerValidator(const std::string& type, 
                         std::function<bool(const std::string&)> validator);

    // Remove a custom validator
    void removeValidator(const std::string& type);

    // Built-in validators
    bool validateEmail(const std::string& email);
    bool validateUrl(const std::string& url);
    bool validateIpAddress(const std::string& ip);
    bool validateJson(const std::string& json);
    bool validateXml(const std::string& xml);
    bool validatePhoneNumber(const std::string& phone);
    bool validateUsername(const std::string& username);
    bool validatePassword(const std::string& password);
    bool validateHexString(const std::string& hex);
    bool validateBase64(const std::string& base64);

private:
    bool initialized_;
    std::unordered_map<std::string, std::function<bool(const std::string&)>> validators_;

    // Initialize built-in validators
    void initializeBuiltInValidators();
};

} // namespace security
} // namespace satox 