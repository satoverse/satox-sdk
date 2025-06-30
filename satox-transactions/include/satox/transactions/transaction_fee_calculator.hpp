#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include "satox/transactions/transaction_manager.hpp"

namespace satox::transactions {

using Transaction = TransactionManager::Transaction;

class TransactionFeeCalculator {
public:
    // Fee calculation result structure
    struct FeeCalculation {
        uint64_t baseFee;
        uint64_t priorityFee;
        uint64_t totalFee;
        std::string currency;
        std::string explanation;
    };

    // Fee calculation strategy type
    using FeeStrategy = std::function<FeeCalculation(const Transaction&, const nlohmann::json&)>;

    // Get singleton instance
    static TransactionFeeCalculator& getInstance();

    // Initialize the fee calculator
    bool initialize(const nlohmann::json& config);

    // Shutdown the fee calculator
    void shutdown();

    // Calculate fee for a transaction
    FeeCalculation calculateFee(const Transaction& transaction);

    // Calculate fee for a batch of transactions
    std::vector<FeeCalculation> calculateBatchFees(const std::vector<Transaction>& transactions);

    // Add custom fee calculation strategy
    bool addFeeStrategy(const std::string& name, FeeStrategy strategy);

    // Remove fee calculation strategy
    bool removeFeeStrategy(const std::string& name);

    // Get available fee strategies
    std::vector<std::string> getFeeStrategies() const;

    // Check if a fee strategy exists
    bool hasFeeStrategy(const std::string& name) const;

    // Get last error message
    std::string getLastError() const;

    // Clear last error
    void clearLastError();

private:
    // Private constructor for singleton
    TransactionFeeCalculator() = default;
    ~TransactionFeeCalculator() = default;

    // Prevent copying
    TransactionFeeCalculator(const TransactionFeeCalculator&) = delete;
    TransactionFeeCalculator& operator=(const TransactionFeeCalculator&) = delete;

    // Initialize default fee strategies
    void initializeDefaultStrategies();

    // Validate configuration
    bool validateConfig(const nlohmann::json& config);

    // Member variables
    bool initialized_ = false;
    std::string lastError_;
    nlohmann::json config_;
    std::unordered_map<std::string, FeeStrategy> feeStrategies_;
    mutable std::mutex mutex_;
};

} // namespace satox::transactions 