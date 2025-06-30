#include "satox/transactions/transaction_validator.hpp"
#include <regex>
#include <algorithm>
#include <chrono>

namespace satox::transactions {

TransactionValidator& TransactionValidator::getInstance() {
    static TransactionValidator instance;
    return instance;
}

bool TransactionValidator::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "TransactionValidator already initialized";
        return false;
    }

    try {
        config_ = config;
        initializeDefaultStrategies();
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to initialize TransactionValidator: ") + e.what();
        return false;
    }
}

void TransactionValidator::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    validationRules_.clear();
    initialized_ = false;
}

TransactionValidator::ValidationResult TransactionValidator::validateTransaction(const Transaction& transaction) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return {false, "TransactionValidator not initialized", {}};
    }

    ValidationResult result;
    result.isValid = true;

    // Run all validation rules
    for (const auto& [ruleName, rule] : validationRules_) {
        try {
            auto ruleResult = rule(transaction);
            if (!ruleResult.isValid) {
                result.isValid = false;
                result.error = ruleResult.error;
                break;
            }
            if (!ruleResult.warnings.empty()) {
                result.warnings.insert(result.warnings.end(), ruleResult.warnings.begin(), ruleResult.warnings.end());
            }
        } catch (const std::exception& e) {
            result.isValid = false;
            result.error = std::string("Validation rule '") + ruleName + "' failed: " + e.what();
            break;
        }
    }

    return result;
}

TransactionValidator::ValidationResult TransactionValidator::validateTransactionBatch(const std::vector<Transaction>& transactions) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        return {false, "TransactionValidator not initialized", {}};
    }

    ValidationResult result;
    result.isValid = true;

    for (const auto& transaction : transactions) {
        auto transactionResult = validateTransaction(transaction);
        if (!transactionResult.isValid) {
            result.isValid = false;
            result.error = "Batch validation failed: " + transactionResult.error;
            break;
        }
        if (!transactionResult.warnings.empty()) {
            result.warnings.insert(result.warnings.end(), transactionResult.warnings.begin(), transactionResult.warnings.end());
        }
    }

    return result;
}

bool TransactionValidator::addValidationRule(const std::string& ruleName, ValidationRule rule) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionValidator not initialized";
        return false;
    }

    try {
        validationRules_[ruleName] = rule;
        return true;
    } catch (const std::exception& e) {
        lastError_ = std::string("Failed to add validation rule: ") + e.what();
        return false;
    }
}

bool TransactionValidator::removeValidationRule(const std::string& ruleName) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_) {
        lastError_ = "TransactionValidator not initialized";
        return false;
    }

    auto it = validationRules_.find(ruleName);
    if (it != validationRules_.end()) {
        validationRules_.erase(it);
        return true;
    }

    lastError_ = "Validation rule not found";
    return false;
}

void TransactionValidator::clearValidationRules() {
    std::lock_guard<std::mutex> lock(mutex_);
    validationRules_.clear();
}

std::vector<std::string> TransactionValidator::getValidationRules() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> rules;
    rules.reserve(validationRules_.size());
    for (const auto& [ruleName, _] : validationRules_) {
        rules.push_back(ruleName);
    }
    return rules;
}

bool TransactionValidator::hasValidationRule(const std::string& ruleName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return validationRules_.find(ruleName) != validationRules_.end();
}

std::string TransactionValidator::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void TransactionValidator::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

void TransactionValidator::initializeDefaultStrategies() {
    // Address validation rule
    addValidationRule("address", [this](const Transaction& transaction) {
        ValidationResult result;
        if (!validateAddress(transaction.from)) {
            result.isValid = false;
            result.error = "Invalid sender address";
        } else if (!validateAddress(transaction.to)) {
            result.isValid = false;
            result.error = "Invalid receiver address";
        }
        return result;
    });

    // Amount validation rule
    addValidationRule("amount", [this](const Transaction& transaction) {
        ValidationResult result;
        if (!validateAmount(transaction.amount)) {
            result.isValid = false;
            result.error = "Invalid amount";
        }
        return result;
    });

    // Asset validation rule
    addValidationRule("asset", [this](const Transaction& transaction) {
        ValidationResult result;
        if (!validateAssetId(transaction.assetId)) {
            result.isValid = false;
            result.error = "Invalid asset ID";
        }
        return result;
    });

    // Timestamp validation rule
    addValidationRule("timestamp", [this](const Transaction& transaction) {
        ValidationResult result;
        if (!validateTimestamp(transaction.timestamp)) {
            result.isValid = false;
            result.error = "Invalid timestamp";
        }
        return result;
    });

    // Signature validation rule
    addValidationRule("signature", [this](const Transaction& transaction) {
        ValidationResult result;
        if (!validateSignature(transaction)) {
            result.isValid = false;
            result.error = "Invalid signature";
        }
        return result;
    });
}

bool TransactionValidator::validateAddress(const std::string& address) {
    if (address.empty()) {
        return false;
    }

    // Basic Ethereum-style address validation (0x followed by 40 hex characters)
    std::regex addressPattern("^0x[a-fA-F0-9]{40}$");
    return std::regex_match(address, addressPattern);
}

bool TransactionValidator::validateAmount(uint64_t amount) {
    if (amount == 0) {
        return false;
    }

    // Check against max amount from config
    if (config_.contains("maxAmount")) {
        uint64_t maxAmount = config_["maxAmount"];
        if (amount > maxAmount) {
            return false;
        }
    }

    return true;
}

bool TransactionValidator::validateAssetId(const std::string& assetId) {
    if (assetId.empty()) {
        return false;
    }

    // Basic asset ID validation (alphanumeric and hyphens only)
    std::regex assetPattern("^[a-zA-Z0-9-]+$");
    return std::regex_match(assetId, assetPattern);
}

bool TransactionValidator::validateTimestamp(const std::chrono::system_clock::time_point& timestamp) {
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::hours>(now - timestamp);
    
    // Allow transactions within 24 hours of current time
    return diff.count() >= -1 && diff.count() <= 24;
}

bool TransactionValidator::validateSignature(const Transaction& transaction) {
    // Basic signature validation - check if signature is not empty
    // In a real implementation, this would verify the cryptographic signature
    return !transaction.signature.empty();
}

} // namespace satox::transactions 