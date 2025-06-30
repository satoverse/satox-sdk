#include <gtest/gtest.h>
#include "satox/transactions/transaction_validator.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

using namespace satox::transactions;
using Transaction = TransactionManager::Transaction;

class TransactionValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test configuration
        config_ = nlohmann::json::object();
        config_["maxAmount"] = 1000000;
        config_["network"] = "testnet";
    }

    void TearDown() override {
        // Clean up after each test
        TransactionValidator::getInstance().shutdown();
    }

    nlohmann::json config_;
};

TEST_F(TransactionValidatorTest, SingletonInstance) {
    auto& instance1 = TransactionValidator::getInstance();
    auto& instance2 = TransactionValidator::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TransactionValidatorTest, Initialization) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));
}

TEST_F(TransactionValidatorTest, DoubleInitialization) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));
    EXPECT_FALSE(validator.initialize(config_));
    EXPECT_EQ(validator.getLastError(), "TransactionValidator already initialized");
}

TEST_F(TransactionValidatorTest, DefaultValidationRules) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    auto rules = validator.getValidationRules();
    EXPECT_FALSE(rules.empty());
    EXPECT_TRUE(validator.hasValidationRule("address"));
    EXPECT_TRUE(validator.hasValidationRule("amount"));
    EXPECT_TRUE(validator.hasValidationRule("asset"));
    EXPECT_TRUE(validator.hasValidationRule("timestamp"));
    EXPECT_TRUE(validator.hasValidationRule("signature"));
}

TEST_F(TransactionValidatorTest, AddressValidation) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();

    auto result = validator.validateTransaction(transaction);
    EXPECT_TRUE(result.isValid);

    // Test invalid addresses
    transaction.from = "invalid";
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid sender address");

    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "invalid";
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid receiver address");
}

TEST_F(TransactionValidatorTest, AmountValidation) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();

    auto result = validator.validateTransaction(transaction);
    EXPECT_TRUE(result.isValid);

    // Test invalid amounts
    transaction.amount = 0;
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid amount");

    transaction.amount = 2000000; // Exceeds maxAmount
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid amount");
}

TEST_F(TransactionValidatorTest, AssetValidation) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();

    auto result = validator.validateTransaction(transaction);
    EXPECT_TRUE(result.isValid);

    // Test invalid asset IDs
    transaction.assetId = "";
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid asset ID");

    transaction.assetId = "invalid!@#";
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid asset ID");
}

TEST_F(TransactionValidatorTest, TimestampValidation) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();

    auto result = validator.validateTransaction(transaction);
    EXPECT_TRUE(result.isValid);

    // Test future timestamp
    transaction.timestamp = std::chrono::system_clock::now() + std::chrono::hours(1);
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid timestamp");

    // Test old timestamp
    transaction.timestamp = std::chrono::system_clock::now() - std::chrono::hours(25);
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid timestamp");
}

TEST_F(TransactionValidatorTest, SignatureValidation) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();
    transaction.signature = "0123456789abcdef";

    auto result = validator.validateTransaction(transaction);
    EXPECT_TRUE(result.isValid);

    // Test missing signature
    transaction.signature = "";
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Invalid signature");
}

TEST_F(TransactionValidatorTest, CustomValidationRule) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    // Add custom validation rule
    bool ruleAdded = validator.addValidationRule("custom", [](const Transaction& tx) {
        TransactionValidator::ValidationResult result;
        result.isValid = tx.amount % 2 == 0;
        if (!result.isValid) {
            result.error = "Amount must be even";
        }
        return result;
    });
    EXPECT_TRUE(ruleAdded);

    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();

    auto result = validator.validateTransaction(transaction);
    EXPECT_TRUE(result.isValid);

    // Test with odd amount
    transaction.amount = 1001;
    result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Amount must be even");
}

TEST_F(TransactionValidatorTest, BatchValidation) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    std::vector<Transaction> transactions;
    for (int i = 0; i < 5; ++i) {
        Transaction transaction;
        transaction.from = "0x1234567890123456789012345678901234567890";
        transaction.to = "0x0987654321098765432109876543210987654321";
        transaction.amount = 1000 + i;
        transaction.assetId = "TEST123";
        transaction.timestamp = std::chrono::system_clock::now();
        transactions.push_back(transaction);
    }

    auto result = validator.validateTransactionBatch(transactions);
    EXPECT_TRUE(result.isValid);

    // Add an invalid transaction
    transactions[2].amount = 0;
    result = validator.validateTransactionBatch(transactions);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "Batch validation failed: Invalid amount");
}

TEST_F(TransactionValidatorTest, ErrorHandling) {
    auto& validator = TransactionValidator::getInstance();
    
    // Try operations before initialization
    Transaction transaction;
    auto result = validator.validateTransaction(transaction);
    EXPECT_FALSE(result.isValid);
    EXPECT_EQ(result.error, "TransactionValidator not initialized");

    EXPECT_FALSE(validator.addValidationRule("test", [](const Transaction&) {
        return TransactionValidator::ValidationResult{true, "", {}};
    }));
    EXPECT_EQ(validator.getLastError(), "TransactionValidator not initialized");

    validator.clearLastError();
    EXPECT_TRUE(validator.getLastError().empty());
}

TEST_F(TransactionValidatorTest, ConcurrentAccess) {
    auto& validator = TransactionValidator::getInstance();
    EXPECT_TRUE(validator.initialize(config_));

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            Transaction transaction;
            transaction.from = "0x1234567890123456789012345678901234567890";
            transaction.to = "0x0987654321098765432109876543210987654321";
            transaction.amount = 1000;
            transaction.assetId = "TEST123";
            transaction.timestamp = std::chrono::system_clock::now();

            validator.validateTransaction(transaction);
            validator.getValidationRules();
            validator.hasValidationRule("address");
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
} 