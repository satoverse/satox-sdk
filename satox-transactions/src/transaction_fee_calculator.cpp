#include "satox/transactions/transaction_fee_calculator.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace satox::transactions {

TransactionFeeCalculator& TransactionFeeCalculator::getInstance() {
    static TransactionFeeCalculator instance;
    return instance;
}

bool TransactionFeeCalculator::initialize(const nlohmann::json& config) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        lastError_ = "TransactionFeeCalculator already initialized";
        return false;
    }

    if (!validateConfig(config)) {
        return false;
    }

    config_ = config;
    initializeDefaultStrategies();
    initialized_ = true;
    return true;
}

void TransactionFeeCalculator::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    initialized_ = false;
    feeStrategies_.clear();
    config_ = nlohmann::json::object();
    lastError_.clear();
}

TransactionFeeCalculator::FeeCalculation TransactionFeeCalculator::calculateFee(const Transaction& transaction) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionFeeCalculator not initialized";
        return FeeCalculation{0, 0, 0, "", "Not initialized"};
    }

    // Get the appropriate fee strategy based on transaction type
    std::string strategyName = "default";
    if (transaction.type == "priority") {
        strategyName = "priority";
    } else if (transaction.type == "batch") {
        strategyName = "batch";
    }

    auto it = feeStrategies_.find(strategyName);
    if (it == feeStrategies_.end()) {
        lastError_ = "Fee strategy not found: " + strategyName;
        return FeeCalculation{0, 0, 0, "", "Strategy not found"};
    }

    return it->second(transaction, config_);
}

std::vector<TransactionFeeCalculator::FeeCalculation> TransactionFeeCalculator::calculateBatchFees(
    const std::vector<Transaction>& transactions) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionFeeCalculator not initialized";
        return std::vector<FeeCalculation>();
    }

    std::vector<FeeCalculation> results;
    results.reserve(transactions.size());

    for (const auto& transaction : transactions) {
        results.push_back(calculateFee(transaction));
    }

    return results;
}

bool TransactionFeeCalculator::addFeeStrategy(const std::string& name, FeeStrategy strategy) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionFeeCalculator not initialized";
        return false;
    }

    if (name.empty()) {
        lastError_ = "Strategy name cannot be empty";
        return false;
    }

    feeStrategies_[name] = std::move(strategy);
    return true;
}

bool TransactionFeeCalculator::removeFeeStrategy(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_) {
        lastError_ = "TransactionFeeCalculator not initialized";
        return false;
    }

    if (name == "default") {
        lastError_ = "Cannot remove default strategy";
        return false;
    }

    return feeStrategies_.erase(name) > 0;
}

std::vector<std::string> TransactionFeeCalculator::getFeeStrategies() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> strategies;
    strategies.reserve(feeStrategies_.size());
    for (const auto& pair : feeStrategies_) {
        strategies.push_back(pair.first);
    }
    return strategies;
}

bool TransactionFeeCalculator::hasFeeStrategy(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return feeStrategies_.find(name) != feeStrategies_.end();
}

std::string TransactionFeeCalculator::getLastError() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastError_;
}

void TransactionFeeCalculator::clearLastError() {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_.clear();
}

void TransactionFeeCalculator::initializeDefaultStrategies() {
    // Default fee strategy
    feeStrategies_["default"] = [](const Transaction& tx, const nlohmann::json& config) {
        FeeCalculation result;
        result.baseFee = config["baseFee"].get<uint64_t>();
        result.priorityFee = 0;
        result.totalFee = result.baseFee;
        result.currency = config["currency"].get<std::string>();
        result.explanation = "Base fee only";
        return result;
    };

    // Priority fee strategy
    feeStrategies_["priority"] = [](const Transaction& tx, const nlohmann::json& config) {
        FeeCalculation result;
        result.baseFee = config["baseFee"].get<uint64_t>();
        result.priorityFee = config["priorityFee"].get<uint64_t>();
        result.totalFee = result.baseFee + result.priorityFee;
        result.currency = config["currency"].get<std::string>();
        result.explanation = "Base fee + priority fee";
        return result;
    };

    // Batch fee strategy
    feeStrategies_["batch"] = [](const Transaction& tx, const nlohmann::json& config) {
        FeeCalculation result;
        result.baseFee = config["baseFee"].get<uint64_t>();
        result.priorityFee = 0;
        result.totalFee = result.baseFee * config["batchDiscount"].get<double>();
        result.currency = config["currency"].get<std::string>();
        result.explanation = "Base fee with batch discount";
        return result;
    };
}

bool TransactionFeeCalculator::validateConfig(const nlohmann::json& config) {
    if (!config.is_object()) {
        lastError_ = "Invalid configuration format";
        return false;
    }

    // Check required fields
    if (!config.contains("baseFee") || !config["baseFee"].is_number()) {
        lastError_ = "Missing or invalid baseFee in configuration";
        return false;
    }

    if (!config.contains("currency") || !config["currency"].is_string()) {
        lastError_ = "Missing or invalid currency in configuration";
        return false;
    }

    if (!config.contains("priorityFee") || !config["priorityFee"].is_number()) {
        lastError_ = "Missing or invalid priorityFee in configuration";
        return false;
    }

    if (!config.contains("batchDiscount") || !config["batchDiscount"].is_number()) {
        lastError_ = "Missing or invalid batchDiscount in configuration";
        return false;
    }

    return true;
}

} // namespace satox::transactions 