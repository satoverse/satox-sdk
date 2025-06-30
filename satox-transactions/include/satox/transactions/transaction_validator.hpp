#pragma once

#include "satox/transactions/transaction_manager.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace satox::transactions {

using Transaction = TransactionManager::Transaction;

class TransactionValidator {
public:
    // Validation result structure
    struct ValidationResult {
        bool isValid;
        std::string error;
        std::vector<std::string> warnings;
    };

    // Validation rule type
    using ValidationRule = std::function<ValidationResult(const Transaction&)>;

    // Singleton instance
    static TransactionValidator& getInstance();

    // Prevent copying
    TransactionValidator(const TransactionValidator&) = delete;
    TransactionValidator& operator=(const TransactionValidator&) = delete;

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config);
    void shutdown();

    // Validation operations
    ValidationResult validateTransaction(const Transaction& transaction);
    ValidationResult validateTransactionBatch(const std::vector<Transaction>& transactions);
    bool addValidationRule(const std::string& ruleName, ValidationRule rule);
    bool removeValidationRule(const std::string& ruleName);
    void clearValidationRules();

    // Rule management
    std::vector<std::string> getValidationRules() const;
    bool hasValidationRule(const std::string& ruleName) const;

    // Error handling
    std::string getLastError() const;
    void clearLastError();

private:
    TransactionValidator() = default;
    ~TransactionValidator() = default;

    void initializeDefaultStrategies();

    // Helper methods
    bool validateAddress(const std::string& address);
    bool validateAmount(uint64_t amount);
    bool validateAssetId(const std::string& assetId);
    bool validateTimestamp(const std::chrono::system_clock::time_point& timestamp);
    bool validateSignature(const Transaction& transaction);

    // Member variables
    bool initialized_ = false;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, ValidationRule> validationRules_;
    std::string lastError_;
    nlohmann::json config_;
};

} // namespace satox::transactions 