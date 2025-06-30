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

#include "satox/security/input_validator.hpp"
#include <nlohmann/json.hpp>
#include <regex>

namespace satox {
namespace security {

InputValidator::InputValidator() : initialized_(false) {}

InputValidator::~InputValidator() {
    shutdown();
}

bool InputValidator::initialize() {
    if (initialized_) {
        return true;
    }

    initializeBuiltInValidators();
    initialized_ = true;
    return true;
}

void InputValidator::shutdown() {
    if (!initialized_) {
        return;
    }

    validators_.clear();
    initialized_ = false;
}

bool InputValidator::validate(const std::string& input, const std::string& type) {
    if (!initialized_) {
        return false;
    }

    auto it = validators_.find(type);
    if (it == validators_.end()) {
        return false;
    }

    return it->second(input);
}

void InputValidator::registerValidator(const std::string& type, 
                                    std::function<bool(const std::string&)> validator) {
    if (!initialized_) {
        return;
    }

    validators_[type] = validator;
}

void InputValidator::removeValidator(const std::string& type) {
    if (!initialized_) {
        return;
    }

    validators_.erase(type);
}

bool InputValidator::validateEmail(const std::string& email) {
    std::regex emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, emailRegex);
}

bool InputValidator::validateUrl(const std::string& url) {
    std::regex urlRegex("^(https?://)?([\\da-z.-]+)\\.([a-z.]{2,6})([/\\w .-]*)*/?$");
    return std::regex_match(url, urlRegex);
}

bool InputValidator::validateIpAddress(const std::string& ip) {
    std::regex ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(ip, ipRegex);
}

bool InputValidator::validateJson(const std::string& json) {
    try {
        auto parsed = nlohmann::json::parse(json);
        (void)parsed; // Suppress unused variable warning
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool InputValidator::validateXml(const std::string& xml) {
    if (xml.empty()) {
        return false;
    }

    std::regex xmlRegex("^<[^>]+>.*</[^>]+>$");
    return std::regex_match(xml, xmlRegex);
}

bool InputValidator::validatePhoneNumber(const std::string& phone) {
    std::regex phoneRegex("^\\+?[1-9]\\d{1,14}$");
    return std::regex_match(phone, phoneRegex);
}

bool InputValidator::validateUsername(const std::string& username) {
    std::regex usernameRegex("^[a-zA-Z0-9_-]{3,16}$");
    return std::regex_match(username, usernameRegex);
}

bool InputValidator::validatePassword(const std::string& password) {
    // At least 8 characters, 1 uppercase, 1 lowercase, 1 number, 1 special character
    std::regex passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,}$");
    return std::regex_match(password, passwordRegex);
}

bool InputValidator::validateHexString(const std::string& hex) {
    std::regex hexRegex("^[0-9A-Fa-f]+$");
    return std::regex_match(hex, hexRegex);
}

bool InputValidator::validateBase64(const std::string& base64) {
    std::regex base64Regex("^[A-Za-z0-9+/]*={0,2}$");
    return std::regex_match(base64, base64Regex);
}

void InputValidator::initializeBuiltInValidators() {
    validators_["email"] = [this](const std::string& input) { return validateEmail(input); };
    validators_["url"] = [this](const std::string& input) { return validateUrl(input); };
    validators_["ip"] = [this](const std::string& input) { return validateIpAddress(input); };
    validators_["json"] = [this](const std::string& input) { return validateJson(input); };
    validators_["xml"] = [this](const std::string& input) { return validateXml(input); };
    validators_["phone"] = [this](const std::string& input) { return validatePhoneNumber(input); };
    validators_["username"] = [this](const std::string& input) { return validateUsername(input); };
    validators_["password"] = [this](const std::string& input) { return validatePassword(input); };
    validators_["hex"] = [this](const std::string& input) { return validateHexString(input); };
    validators_["base64"] = [this](const std::string& input) { return validateBase64(input); };
}

} // namespace security
} // namespace satox 