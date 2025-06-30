#include <gtest/gtest.h>
#include "satox/transactions/transaction_fee_calculator.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

using namespace satox::transactions;
using Transaction = TransactionManager::Transaction;

class TransactionFeeCalculatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test configuration
        config_ = nlohmann::json::object();
        config_["baseFee"] = 1000;
        config_["priorityFee"] = 500;
        config_["batchDiscount"] = 0.8;
        config_["currency"] = "SATX";
    }

    void TearDown() override {
        // Clean up after each test
        TransactionFeeCalculator::getInstance().shutdown();
    }

    nlohmann::json config_;
};

TEST_F(TransactionFeeCalculatorTest, SingletonInstance) {
    auto& instance1 = TransactionFeeCalculator::getInstance();
    auto& instance2 = TransactionFeeCalculator::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TransactionFeeCalculatorTest, Initialization) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));
}

TEST_F(TransactionFeeCalculatorTest, DoubleInitialization) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));
    EXPECT_FALSE(calculator.initialize(config_));
    EXPECT_EQ(calculator.getLastError(), "TransactionFeeCalculator already initialized");
}

TEST_F(TransactionFeeCalculatorTest, DefaultFeeStrategy) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    Transaction transaction;
    transaction.type = "default";
    transaction.amount = 1000;

    auto result = calculator.calculateFee(transaction);
    EXPECT_EQ(result.baseFee, 1000);
    EXPECT_EQ(result.priorityFee, 0);
    EXPECT_EQ(result.totalFee, 1000);
    EXPECT_EQ(result.currency, "SATX");
    EXPECT_EQ(result.explanation, "Base fee only");
}

TEST_F(TransactionFeeCalculatorTest, PriorityFeeStrategy) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    Transaction transaction;
    transaction.type = "priority";
    transaction.amount = 1000;

    auto result = calculator.calculateFee(transaction);
    EXPECT_EQ(result.baseFee, 1000);
    EXPECT_EQ(result.priorityFee, 500);
    EXPECT_EQ(result.totalFee, 1500);
    EXPECT_EQ(result.currency, "SATX");
    EXPECT_EQ(result.explanation, "Base fee + priority fee");
}

TEST_F(TransactionFeeCalculatorTest, BatchFeeStrategy) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    Transaction transaction;
    transaction.type = "batch";
    transaction.amount = 1000;

    auto result = calculator.calculateFee(transaction);
    EXPECT_EQ(result.baseFee, 1000);
    EXPECT_EQ(result.priorityFee, 0);
    EXPECT_EQ(result.totalFee, 800); // 1000 * 0.8
    EXPECT_EQ(result.currency, "SATX");
    EXPECT_EQ(result.explanation, "Base fee with batch discount");
}

TEST_F(TransactionFeeCalculatorTest, BatchFeeCalculation) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    std::vector<Transaction> transactions;
    for (int i = 0; i < 3; ++i) {
        Transaction transaction;
        transaction.type = "default";
        transaction.amount = 1000;
        transactions.push_back(transaction);
    }

    auto results = calculator.calculateBatchFees(transactions);
    EXPECT_EQ(results.size(), 3);
    for (const auto& result : results) {
        EXPECT_EQ(result.baseFee, 1000);
        EXPECT_EQ(result.priorityFee, 0);
        EXPECT_EQ(result.totalFee, 1000);
        EXPECT_EQ(result.currency, "SATX");
    }
}

TEST_F(TransactionFeeCalculatorTest, CustomFeeStrategy) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    // Add custom fee strategy
    bool strategyAdded = calculator.addFeeStrategy("custom", [](const Transaction& tx, const nlohmann::json& config) {
        TransactionFeeCalculator::FeeCalculation result;
        result.baseFee = config["baseFee"].get<uint64_t>();
        result.priorityFee = tx.amount * 0.1; // 10% of transaction amount
        result.totalFee = result.baseFee + result.priorityFee;
        result.currency = config["currency"].get<std::string>();
        result.explanation = "Custom fee calculation";
        return result;
    });
    EXPECT_TRUE(strategyAdded);

    Transaction transaction;
    transaction.type = "custom";
    transaction.amount = 1000;

    auto result = calculator.calculateFee(transaction);
    EXPECT_EQ(result.baseFee, 1000);
    EXPECT_EQ(result.priorityFee, 100); // 10% of 1000
    EXPECT_EQ(result.totalFee, 1100);
    EXPECT_EQ(result.currency, "SATX");
    EXPECT_EQ(result.explanation, "Custom fee calculation");
}

TEST_F(TransactionFeeCalculatorTest, RemoveFeeStrategy) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    // Add custom strategy
    calculator.addFeeStrategy("custom", [](const Transaction&, const nlohmann::json&) {
        return TransactionFeeCalculator::FeeCalculation{};
    });

    // Try to remove default strategy (should fail)
    EXPECT_FALSE(calculator.removeFeeStrategy("default"));
    EXPECT_EQ(calculator.getLastError(), "Cannot remove default strategy");

    // Remove custom strategy
    EXPECT_TRUE(calculator.removeFeeStrategy("custom"));
    EXPECT_FALSE(calculator.hasFeeStrategy("custom"));
}

TEST_F(TransactionFeeCalculatorTest, GetFeeStrategies) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    auto strategies = calculator.getFeeStrategies();
    EXPECT_FALSE(strategies.empty());
    EXPECT_TRUE(std::find(strategies.begin(), strategies.end(), "default") != strategies.end());
    EXPECT_TRUE(std::find(strategies.begin(), strategies.end(), "priority") != strategies.end());
    EXPECT_TRUE(std::find(strategies.begin(), strategies.end(), "batch") != strategies.end());
}

TEST_F(TransactionFeeCalculatorTest, ErrorHandling) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    
    // Try operations before initialization
    Transaction transaction;
    auto result = calculator.calculateFee(transaction);
    EXPECT_EQ(result.totalFee, 0);
    EXPECT_EQ(result.explanation, "Not initialized");

    EXPECT_FALSE(calculator.addFeeStrategy("test", [](const Transaction&, const nlohmann::json&) {
        return TransactionFeeCalculator::FeeCalculation{};
    }));
    EXPECT_EQ(calculator.getLastError(), "TransactionFeeCalculator not initialized");

    calculator.clearLastError();
    EXPECT_TRUE(calculator.getLastError().empty());
}

TEST_F(TransactionFeeCalculatorTest, ConcurrentAccess) {
    auto& calculator = TransactionFeeCalculator::getInstance();
    EXPECT_TRUE(calculator.initialize(config_));

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            Transaction transaction;
            transaction.type = "default";
            transaction.amount = 1000;

            calculator.calculateFee(transaction);
            calculator.getFeeStrategies();
            calculator.hasFeeStrategy("default");
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 