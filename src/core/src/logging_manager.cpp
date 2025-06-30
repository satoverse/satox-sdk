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

#include "satox/core/logging_manager.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace satox::core {

LoggingManager::LoggingManager() : initialized_(false), running_(false) {
    // Initialize default configuration
    config_.logDir = "logs/components";
    config_.logFile = "satox.log";
    config_.maxFileSize = 10 * 1024 * 1024; // 10MB
    config_.maxFiles = 5;
    config_.minLevel = Level::INFO;
    config_.consoleOutput = true;
    config_.fileOutput = true;
    config_.asyncLogging = false;
    config_.queueSize = 1000;
    config_.flushInterval = std::chrono::milliseconds(100);
    config_.includeTimestamp = true;
    config_.includeThreadId = true;
    config_.includeSource = true;
    config_.logFormat = "[{timestamp}] [{level}] [{source}] {message}";
    
    stats_ = LogStats{};
}

LoggingManager::~LoggingManager() {
    shutdown();
}

bool LoggingManager::initialize(const LogConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        lastError_ = "Logging manager already initialized";
        return false;
    }

    if (!validateConfig(config)) {
        return false;
    }

    config_ = config;
    stats_ = LogStats{};
    stats_.lastFlush = std::chrono::system_clock::now();

    // Create log directory if it doesn't exist
    if (config_.fileOutput) {
        std::filesystem::create_directories(config_.logDir);
        logFile_.open(config_.logDir + "/" + config_.logFile, std::ios::app);
        if (!logFile_.is_open()) {
            lastError_ = "Failed to open log file";
            return false;
        }
    }

    // Start async logging worker if enabled
    if (config_.asyncLogging) {
        running_ = true;
        workerThread_ = std::thread(&LoggingManager::asyncLoggingWorker, this);
    }

    initialized_ = true;
    return true;
}

void LoggingManager::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        if (config_.asyncLogging) {
            running_ = false;
            if (workerThread_.joinable()) {
                workerThread_.join();
            }
        }

        if (logFile_.is_open()) {
            logFile_.close();
        }

        initialized_ = false;
    }
}

void LoggingManager::log(Level level, const std::string& message,
                        const std::string& source,
                        const nlohmann::json& metadata) {
    if (!initialized_ || level < config_.minLevel) {
        return;
    }

    LogEntry entry;
    entry.level = level;
    entry.message = message;
    entry.source = source;
    entry.threadId = getThreadId();
    entry.timestamp = std::chrono::system_clock::now();
    entry.metadata = metadata;

    if (config_.asyncLogging) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (logQueue_.size() < config_.queueSize) {
            logQueue_.push(entry);
            stats_.queueSize = logQueue_.size();
        } else {
            stats_.droppedEntries++;
        }
    } else {
        processLogEntry(entry);
    }
}

void LoggingManager::trace(const std::string& message, const std::string& source,
                          const nlohmann::json& metadata) {
    log(Level::TRACE, message, source, metadata);
}

void LoggingManager::debug(const std::string& message, const std::string& source,
                          const nlohmann::json& metadata) {
    log(Level::DEBUG, message, source, metadata);
}

void LoggingManager::info(const std::string& message, const std::string& source,
                         const nlohmann::json& metadata) {
    log(Level::INFO, message, source, metadata);
}

void LoggingManager::warning(const std::string& message, const std::string& source,
                            const nlohmann::json& metadata) {
    log(Level::WARNING, message, source, metadata);
}

void LoggingManager::error(const std::string& message, const std::string& source,
                          const nlohmann::json& metadata) {
    log(Level::ERROR, message, source, metadata);
}

void LoggingManager::fatal(const std::string& message, const std::string& source,
                          const nlohmann::json& metadata) {
    log(Level::FATAL, message, source, metadata);
}

void LoggingManager::setLogLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.minLevel = level;
}

LoggingManager::Level LoggingManager::getLogLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.minLevel;
}

void LoggingManager::setLogFormat(const std::string& format) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.logFormat = format;
}

std::string LoggingManager::getLogFormat() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_.logFormat;
}

void LoggingManager::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (config_.asyncLogging) {
        while (!logQueue_.empty()) {
            processLogEntry(logQueue_.front());
            logQueue_.pop();
        }
        stats_.queueSize = 0;
    }

    if (logFile_.is_open()) {
        logFile_.flush();
    }

    stats_.lastFlush = std::chrono::system_clock::now();
}

void LoggingManager::rotate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!config_.fileOutput) {
        return;
    }

    // Close current log file
    if (logFile_.is_open()) {
        logFile_.close();
    }

    // Rotate existing log files
    for (size_t i = config_.maxFiles - 1; i > 0; --i) {
        std::string oldFile = config_.logDir + "/" + config_.logFile + "." + std::to_string(i);
        std::string newFile = config_.logDir + "/" + config_.logFile + "." + std::to_string(i + 1);
        std::filesystem::rename(oldFile, newFile);
    }

    // Rename current log file
    std::string currentFile = config_.logDir + "/" + config_.logFile;
    std::string newFile = config_.logDir + "/" + config_.logFile + ".1";
    std::filesystem::rename(currentFile, newFile);

    // Open new log file
    logFile_.open(currentFile, std::ios::app);
}

void LoggingManager::registerCallback(LogCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(callback);
}

void LoggingManager::unregisterCallback() {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
}

LoggingManager::LogStats LoggingManager::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

std::string LoggingManager::getLastError() const {
    return lastError_;
}

void LoggingManager::clearLastError() {
    lastError_.clear();
}

void LoggingManager::processLogEntry(const LogEntry& entry) {
    if (config_.consoleOutput) {
        writeToConsole(entry);
    }

    if (config_.fileOutput) {
        writeToFile(entry);
    }

    notifyCallbacks(entry);
    updateStats(entry);
}

void LoggingManager::writeToFile(const LogEntry& entry) {
    if (!logFile_.is_open()) {
        return;
    }

    logFile_ << formatLogEntry(entry) << std::endl;
    checkRotation();
}

void LoggingManager::writeToConsole(const LogEntry& entry) {
    std::cout << formatLogEntry(entry) << std::endl;
}

std::string LoggingManager::formatLogEntry(const LogEntry& entry) {
    std::stringstream ss;
    
    if (config_.includeTimestamp) {
        auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << " ";
    }

    ss << "[" << levelToString(entry.level) << "] ";

    if (config_.includeThreadId) {
        ss << "[" << entry.threadId << "] ";
    }

    if (config_.includeSource && !entry.source.empty()) {
        ss << "[" << entry.source << "] ";
    }

    ss << entry.message;

    if (!entry.metadata.empty()) {
        ss << " " << entry.metadata.dump();
    }

    return ss.str();
}

void LoggingManager::checkRotation() {
    if (config_.maxFileSize > 0) {
        std::filesystem::path logPath(config_.logDir + "/" + config_.logFile);
        if (std::filesystem::exists(logPath) &&
            std::filesystem::file_size(logPath) >= config_.maxFileSize) {
            rotate();
        }
    }
}

void LoggingManager::asyncLoggingWorker() {
    while (running_) {
        LogEntry entry;
        bool hasEntry = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!logQueue_.empty()) {
                entry = logQueue_.front();
                logQueue_.pop();
                stats_.queueSize = logQueue_.size();
                hasEntry = true;
            }
        }

        if (hasEntry) {
            processLogEntry(entry);
        }

        // Check if it's time to flush
        auto now = std::chrono::system_clock::now();
        if (now - stats_.lastFlush >= config_.flushInterval) {
            flush();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void LoggingManager::notifyCallbacks(const LogEntry& entry) {
    for (const auto& callback : callbacks_) {
        callback(entry);
    }
}

std::string LoggingManager::getThreadId() {
    auto threadId = std::this_thread::get_id();
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = threadIds_.find(threadId);
    if (it == threadIds_.end()) {
        std::stringstream ss;
        ss << threadId;
        threadIds_[threadId] = ss.str();
        return ss.str();
    }
    
    return it->second;
}

std::string LoggingManager::levelToString(Level level) const {
    switch (level) {
        case Level::TRACE:   return "TRACE";
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        case Level::FATAL:   return "FATAL";
        default:            return "UNKNOWN";
    }
}

LoggingManager::Level LoggingManager::stringToLevel(const std::string& level) const {
    std::string upperLevel = level;
    std::transform(upperLevel.begin(), upperLevel.end(), upperLevel.begin(), ::toupper);
    
    if (upperLevel == "TRACE")   return Level::TRACE;
    if (upperLevel == "DEBUG")   return Level::DEBUG;
    if (upperLevel == "INFO")    return Level::INFO;
    if (upperLevel == "WARNING") return Level::WARNING;
    if (upperLevel == "ERROR")   return Level::ERROR;
    if (upperLevel == "FATAL")   return Level::FATAL;
    
    return Level::INFO;
}

bool LoggingManager::validateConfig(const LogConfig& config) {
    if (config.logDir.empty()) {
        lastError_ = "Log directory not specified";
        return false;
    }

    if (config.logFile.empty()) {
        lastError_ = "Log file not specified";
        return false;
    }

    if (config.maxFileSize == 0) {
        lastError_ = "Invalid max file size";
        return false;
    }

    if (config.maxFiles == 0) {
        lastError_ = "Invalid max files";
        return false;
    }

    if (config.queueSize == 0) {
        lastError_ = "Invalid queue size";
        return false;
    }

    return true;
}

void LoggingManager::updateStats(const LogEntry& entry) {
    stats_.totalEntries++;
    stats_.entriesByLevel[static_cast<size_t>(entry.level)]++;
}

} // namespace satox::core 