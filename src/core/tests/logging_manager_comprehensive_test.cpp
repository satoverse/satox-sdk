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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "satox/core/logging_manager.hpp"
#include <thread>
#include <future>
#include <chrono>
#include <random>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using namespace satox::core;
using namespace std::chrono_literals;

class LoggingManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test log directory
        testLogDir_ = std::filesystem::temp_directory_path() / "satox_test_logs";
        std::filesystem::create_directories(testLogDir_);

        // Initialize Logging Manager with test configuration
        LogConfig config;
        config.logDir = testLogDir_.string();
        config.logFile = "test.log";
        config.maxFileSize = 1024 * 1024;  // 1MB
        config.maxFiles = 3;
        config.minLevel = Level::TRACE;
        config.consoleOutput = true;
        config.fileOutput = true;
        config.asyncLogging = true;
        config.queueSize = 1000;
        config.flushInterval = 100ms;
        config.includeTimestamp = true;
        config.includeThreadId = true;
        config.includeSource = true;
        config.logFormat = "[{timestamp}] [{level}] [{thread}] [{source}] {message} {metadata}";
        ASSERT_TRUE(LoggingManager::getInstance().initialize(config));
    }

    void TearDown() override {
        LoggingManager::getInstance().shutdown();
        std::filesystem::remove_all(testLogDir_);
    }

    // Helper function to read log file
    std::string readLogFile() {
        std::ifstream file(testLogDir_ / "test.log");
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Helper function to create test metadata
    nlohmann::json createTestMetadata() {
        return {
            {"key1", "value1"},
            {"key2", 42},
            {"key3", true},
            {"key4", {1, 2, 3}},
            {"key5", {
                {"nested", "value"}
            }}
        };
    }

    std::filesystem::path testLogDir_;
};

// Basic Functionality Tests
TEST_F(LoggingManagerComprehensiveTest, Initialization) {
    // Test initialization with valid config
    LogConfig validConfig;
    validConfig.logDir = testLogDir_.string();
    validConfig.logFile = "test2.log";
    validConfig.maxFileSize = 1024 * 1024;
    validConfig.maxFiles = 3;
    validConfig.minLevel = Level::TRACE;
    validConfig.consoleOutput = true;
    validConfig.fileOutput = true;
    EXPECT_TRUE(LoggingManager::getInstance().initialize(validConfig));

    // Test initialization with invalid config
    LogConfig invalidConfig;
    invalidConfig.logDir = "";
    invalidConfig.logFile = "";
    invalidConfig.maxFileSize = 0;
    invalidConfig.maxFiles = 0;
    EXPECT_FALSE(LoggingManager::getInstance().initialize(invalidConfig));
}

// Logging Methods Tests
TEST_F(LoggingManagerComprehensiveTest, LoggingMethods) {
    // Test all logging levels
    LoggingManager::getInstance().trace("Trace message", "TestSource", createTestMetadata());
    LoggingManager::getInstance().debug("Debug message", "TestSource", createTestMetadata());
    LoggingManager::getInstance().info("Info message", "TestSource", createTestMetadata());
    LoggingManager::getInstance().warning("Warning message", "TestSource", createTestMetadata());
    LoggingManager::getInstance().error("Error message", "TestSource", createTestMetadata());
    LoggingManager::getInstance().fatal("Fatal message", "TestSource", createTestMetadata());

    // Flush logs and verify content
    LoggingManager::getInstance().flush();
    std::string logContent = readLogFile();
    EXPECT_THAT(logContent, ::testing::HasSubstr("Trace message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Debug message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Info message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Warning message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Error message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Fatal message"));
}

// Log Level Tests
TEST_F(LoggingManagerComprehensiveTest, LogLevels) {
    // Test different log levels
    LoggingManager::getInstance().setLogLevel(Level::INFO);
    LoggingManager::getInstance().trace("Trace message");  // Should not be logged
    LoggingManager::getInstance().debug("Debug message");  // Should not be logged
    LoggingManager::getInstance().info("Info message");    // Should be logged
    LoggingManager::getInstance().warning("Warning message");  // Should be logged
    LoggingManager::getInstance().error("Error message");  // Should be logged
    LoggingManager::getInstance().fatal("Fatal message");  // Should be logged

    // Flush logs and verify content
    LoggingManager::getInstance().flush();
    std::string logContent = readLogFile();
    EXPECT_THAT(logContent, ::testing::Not(::testing::HasSubstr("Trace message")));
    EXPECT_THAT(logContent, ::testing::Not(::testing::HasSubstr("Debug message")));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Info message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Warning message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Error message"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Fatal message"));
}

// Log Format Tests
TEST_F(LoggingManagerComprehensiveTest, LogFormat) {
    // Test custom log format
    LoggingManager::getInstance().setLogFormat("[{level}] {message}");
    LoggingManager::getInstance().info("Test message", "TestSource", createTestMetadata());

    // Flush logs and verify format
    LoggingManager::getInstance().flush();
    std::string logContent = readLogFile();
    EXPECT_THAT(logContent, ::testing::MatchesRegex("\\[INFO\\] Test message.*"));
}

// File Rotation Tests
TEST_F(LoggingManagerComprehensiveTest, FileRotation) {
    // Create large log file
    std::string largeMessage(1024, 'x');  // 1KB message
    for (int i = 0; i < 1024; ++i) {  // Write 1MB of logs
        LoggingManager::getInstance().info(largeMessage);
    }

    // Force rotation
    LoggingManager::getInstance().rotate();

    // Verify rotation
    EXPECT_TRUE(std::filesystem::exists(testLogDir_ / "test.log.1"));
    EXPECT_TRUE(std::filesystem::exists(testLogDir_ / "test.log"));
}

// Callback Tests
TEST_F(LoggingManagerComprehensiveTest, Callbacks) {
    bool callbackCalled = false;
    LogCallback callback = [&callbackCalled](const LogEntry& entry) {
        callbackCalled = true;
        EXPECT_EQ(entry.level, Level::INFO);
        EXPECT_EQ(entry.message, "Test message");
        EXPECT_EQ(entry.source, "TestSource");
    };

    // Test callback registration
    LoggingManager::getInstance().registerCallback(callback);

    // Test callback invocation
    LoggingManager::getInstance().info("Test message", "TestSource");
    LoggingManager::getInstance().flush();
    EXPECT_TRUE(callbackCalled);

    // Test callback unregistration
    callbackCalled = false;
    LoggingManager::getInstance().unregisterCallback(callback);
    LoggingManager::getInstance().info("Test message", "TestSource");
    LoggingManager::getInstance().flush();
    EXPECT_FALSE(callbackCalled);
}

// Statistics Tests
TEST_F(LoggingManagerComprehensiveTest, Statistics) {
    // Log messages at different levels
    LoggingManager::getInstance().trace("Trace message");
    LoggingManager::getInstance().debug("Debug message");
    LoggingManager::getInstance().info("Info message");
    LoggingManager::getInstance().warning("Warning message");
    LoggingManager::getInstance().error("Error message");
    LoggingManager::getInstance().fatal("Fatal message");

    // Get statistics
    auto stats = LoggingManager::getInstance().getStats();
    EXPECT_EQ(stats.totalEntries, 6);
    EXPECT_EQ(stats.entriesByLevel[static_cast<size_t>(Level::TRACE)], 1);
    EXPECT_EQ(stats.entriesByLevel[static_cast<size_t>(Level::DEBUG)], 1);
    EXPECT_EQ(stats.entriesByLevel[static_cast<size_t>(Level::INFO)], 1);
    EXPECT_EQ(stats.entriesByLevel[static_cast<size_t>(Level::WARNING)], 1);
    EXPECT_EQ(stats.entriesByLevel[static_cast<size_t>(Level::ERROR)], 1);
    EXPECT_EQ(stats.entriesByLevel[static_cast<size_t>(Level::FATAL)], 1);
}

// Error Handling Tests
TEST_F(LoggingManagerComprehensiveTest, ErrorHandling) {
    // Test invalid configuration
    LogConfig invalidConfig;
    EXPECT_FALSE(LoggingManager::getInstance().initialize(invalidConfig));
    EXPECT_FALSE(LoggingManager::getInstance().getLastError().empty());

    // Test clearLastError
    LoggingManager::getInstance().clearLastError();
    EXPECT_TRUE(LoggingManager::getInstance().getLastError().empty());
}

// Concurrency Tests
TEST_F(LoggingManagerComprehensiveTest, Concurrency) {
    const int numThreads = 10;
    std::vector<std::future<void>> futures;

    // Test concurrent logging
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            LoggingManager::getInstance().info(
                "Thread " + std::to_string(i) + " message",
                "TestSource" + std::to_string(i));
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    // Flush logs and verify content
    LoggingManager::getInstance().flush();
    std::string logContent = readLogFile();
    for (int i = 0; i < numThreads; ++i) {
        EXPECT_THAT(logContent, ::testing::HasSubstr("Thread " + std::to_string(i) + " message"));
    }
}

// Edge Cases Tests
TEST_F(LoggingManagerComprehensiveTest, EdgeCases) {
    // Test empty message
    LoggingManager::getInstance().info("");

    // Test very long message
    std::string longMessage(1024 * 1024, 'x');  // 1MB message
    LoggingManager::getInstance().info(longMessage);

    // Test special characters
    LoggingManager::getInstance().info("Special chars: !@#$%^&*()_+{}|:\"<>?[]\\;',./");

    // Test Unicode characters
    LoggingManager::getInstance().info("Unicode: 你好世界");

    // Flush logs and verify content
    LoggingManager::getInstance().flush();
    std::string logContent = readLogFile();
    EXPECT_THAT(logContent, ::testing::HasSubstr("Special chars:"));
    EXPECT_THAT(logContent, ::testing::HasSubstr("Unicode:"));
}

// Cleanup Tests
TEST_F(LoggingManagerComprehensiveTest, Cleanup) {
    // Test proper cleanup on shutdown
    LoggingManager::getInstance().info("Test message");
    LoggingManager::getInstance().shutdown();

    // Verify log file exists and contains the message
    EXPECT_TRUE(std::filesystem::exists(testLogDir_ / "test.log"));
    std::string logContent = readLogFile();
    EXPECT_THAT(logContent, ::testing::HasSubstr("Test message"));
}

// Stress Tests
TEST_F(LoggingManagerComprehensiveTest, StressTest) {
    const int numMessages = 10000;
    std::vector<std::future<void>> futures;

    // Test high-volume logging
    for (int i = 0; i < numMessages; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            LoggingManager::getInstance().info(
                "Message " + std::to_string(i),
                "TestSource",
                {{"index", i}});
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    // Flush logs and verify content
    LoggingManager::getInstance().flush();
    std::string logContent = readLogFile();
    for (int i = 0; i < numMessages; i += 1000) {  // Check every 1000th message
        EXPECT_THAT(logContent, ::testing::HasSubstr("Message " + std::to_string(i)));
    }
} 