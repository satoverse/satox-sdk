/**
 * @file cloud_monitoring.cpp
 * @brief Cloud database monitoring and logging system implementation
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#include "satox/core/cloud/cloud_monitoring.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace satox {
namespace core {

CloudMonitoring& CloudMonitoring::getInstance() {
    static CloudMonitoring instance;
    return instance;
}

CloudMonitoring::CloudMonitoring() {
    // Initialize metrics for all providers
    metrics_[CloudProvider::SUPABASE] = CloudMetrics{};
    metrics_[CloudProvider::FIREBASE] = CloudMetrics{};
    metrics_[CloudProvider::AWS] = CloudMetrics{};
    metrics_[CloudProvider::AZURE] = CloudMetrics{};
    metrics_[CloudProvider::GOOGLE_CLOUD] = CloudMetrics{};
}

CloudMonitoring::~CloudMonitoring() {
    shutdown();
}

bool CloudMonitoring::initialize(const CloudMonitoringConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_.load()) {
        spdlog::warn("CloudMonitoring already initialized");
        return true;
    }
    
    try {
        config_ = config;
        
        // Create log directory
        if (config.enable_logging && !config.log_path.empty()) {
            std::filesystem::create_directories(config.log_path);
        }
        
        // Initialize logging
        if (config.enable_logging) {
            initializeLogging();
        }
        
        // Initialize metrics
        if (config.enable_metrics) {
            initializeMetrics();
        }
        
        // Initialize health checks
        if (config.enable_health_checks) {
            initializeHealthChecks();
        }
        
        initialized_.store(true);
        
        if (logger_) {
            logger_->info("CloudMonitoring initialized successfully");
            logger_->info("Log path: {}", config.log_path);
            logger_->info("Log level: {}", config.log_level);
            logger_->info("Health check interval: {}s", config.health_check_interval_seconds);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize CloudMonitoring: {}", e.what());
        return false;
    }
}

void CloudMonitoring::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!initialized_.load()) {
        return;
    }
    
    try {
        shutdown_requested_.store(true);
        
        // Notify background workers
        health_check_cv_.notify_all();
        metrics_cv_.notify_all();
        
        // Wait for background threads to finish
        if (health_check_thread_.joinable()) {
            health_check_thread_.join();
        }
        if (metrics_thread_.joinable()) {
            metrics_thread_.join();
        }
        
        // Flush all loggers
        if (logger_) logger_->flush();
        if (error_logger_) error_logger_->flush();
        if (performance_logger_) performance_logger_->flush();
        if (security_logger_) security_logger_->flush();
        if (audit_logger_) audit_logger_->flush();
        
        initialized_.store(false);
        
        spdlog::info("CloudMonitoring shutdown completed");
        
    } catch (const std::exception& e) {
        spdlog::error("Error during CloudMonitoring shutdown: {}", e.what());
    }
}

void CloudMonitoring::logOperation(const CloudLogEntry& entry) {
    if (!initialized_.load()) {
        return;
    }
    
    try {
        total_log_entries_++;
        processLogEntry(entry);
        updateMetrics(entry);
        checkAlerts(entry);
        notifyCallbacks(entry);
        
    } catch (const std::exception& e) {
        spdlog::error("Error logging cloud operation: {}", e.what());
    }
}

std::string CloudMonitoring::startOperation(CloudProvider provider, 
                                           CloudOperation operation,
                                           const std::string& connection_id,
                                           const nlohmann::json& metadata) {
    std::string operation_id = generateOperationId();
    
    CloudLogEntry entry;
    entry.operation_id = operation_id;
    entry.provider = provider;
    entry.operation = operation;
    entry.status = CloudOperationStatus::SUCCESS; // Will be updated on completion
    entry.connection_id = connection_id;
    entry.user_id = user_id_;
    entry.session_id = session_id_;
    entry.correlation_id = correlation_id_;
    entry.timestamp = std::chrono::system_clock::now();
    entry.duration = std::chrono::milliseconds(0);
    entry.metadata = metadata;
    entry.context = {
        {"ip_address", ip_address_},
        {"user_agent", user_agent_}
    };
    entry.thread_id = std::this_thread::get_id();
    
    // Store operation start time
    {
        std::lock_guard<std::mutex> lock(operations_mutex_);
        active_operations_[operation_id] = entry.timestamp;
        operation_cache_[operation_id] = entry;
    }
    
    if (logger_) {
        logger_->debug("Started operation {} for provider {}: {}", 
                      operation_id, providerToString(provider), operationToString(operation));
    }
    
    return operation_id;
}

void CloudMonitoring::completeOperation(const std::string& operation_id,
                                       CloudOperationStatus status,
                                       const std::string& error_message,
                                       const nlohmann::json& result) {
    std::chrono::system_clock::time_point start_time;
    CloudLogEntry entry;
    
    {
        std::lock_guard<std::mutex> lock(operations_mutex_);
        
        auto start_it = active_operations_.find(operation_id);
        if (start_it == active_operations_.end()) {
            spdlog::warn("Operation {} not found in active operations", operation_id);
            return;
        }
        
        start_time = start_it->second;
        active_operations_.erase(start_it);
        
        auto cache_it = operation_cache_.find(operation_id);
        if (cache_it != operation_cache_.end()) {
            entry = cache_it->second;
            operation_cache_.erase(cache_it);
        }
    }
    
    // Calculate duration
    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Update entry
    entry.status = status;
    entry.error_message = error_message;
    entry.duration = duration;
    if (!result.empty()) {
        entry.metadata["result"] = result;
    }
    
    // Log the completed operation
    logOperation(entry);
    
    if (logger_) {
        if (status == CloudOperationStatus::SUCCESS) {
            logger_->info("Completed operation {} in {}ms", operation_id, duration.count());
        } else {
            logger_->error("Failed operation {} in {}ms: {}", operation_id, duration.count(), error_message);
        }
    }
}

void CloudMonitoring::logError(CloudProvider provider,
                              CloudOperation operation,
                              const std::string& error_message,
                              const std::string& error_code,
                              const std::string& connection_id,
                              const nlohmann::json& metadata) {
    CloudLogEntry entry;
    entry.operation_id = generateOperationId();
    entry.provider = provider;
    entry.operation = operation;
    entry.status = CloudOperationStatus::FAILED;
    entry.connection_id = connection_id;
    entry.user_id = user_id_;
    entry.session_id = session_id_;
    entry.correlation_id = correlation_id_;
    entry.error_message = error_message;
    entry.error_code = error_code;
    entry.timestamp = std::chrono::system_clock::now();
    entry.duration = std::chrono::milliseconds(0);
    entry.metadata = metadata;
    entry.context = {
        {"ip_address", ip_address_},
        {"user_agent", user_agent_}
    };
    entry.thread_id = std::this_thread::get_id();
    
    logOperation(entry);
    
    if (error_logger_) {
        error_logger_->error("Cloud error [{}] [{}]: {} - {}", 
                            providerToString(provider), operationToString(operation), 
                            error_code, error_message);
    }
}

void CloudMonitoring::logPerformance(CloudProvider provider,
                                    CloudOperation operation,
                                    std::chrono::milliseconds duration,
                                    bool success,
                                    const nlohmann::json& metadata) {
    CloudLogEntry entry;
    entry.operation_id = generateOperationId();
    entry.provider = provider;
    entry.operation = operation;
    entry.status = success ? CloudOperationStatus::SUCCESS : CloudOperationStatus::FAILED;
    entry.timestamp = std::chrono::system_clock::now();
    entry.duration = duration;
    entry.metadata = metadata;
    entry.context = {
        {"ip_address", ip_address_},
        {"user_agent", user_agent_}
    };
    entry.thread_id = std::this_thread::get_id();
    
    logOperation(entry);
    
    if (performance_logger_) {
        performance_logger_->info("Performance [{}] [{}]: {}ms", 
                                 providerToString(provider), operationToString(operation), 
                                 duration.count());
    }
}

void CloudMonitoring::logSecurityEvent(CloudProvider provider,
                                      const std::string& event_type,
                                      const std::string& severity,
                                      const std::string& details,
                                      const nlohmann::json& metadata) {
    CloudLogEntry entry;
    entry.operation_id = generateOperationId();
    entry.provider = provider;
    entry.operation = CloudOperation::AUTHENTICATE; // Default for security events
    entry.status = CloudOperationStatus::SUCCESS;
    entry.timestamp = std::chrono::system_clock::now();
    entry.duration = std::chrono::milliseconds(0);
    entry.metadata = metadata;
    entry.metadata["event_type"] = event_type;
    entry.metadata["severity"] = severity;
    entry.metadata["details"] = details;
    entry.context = {
        {"ip_address", ip_address_},
        {"user_agent", user_agent_}
    };
    entry.thread_id = std::this_thread::get_id();
    
    logOperation(entry);
    
    if (security_logger_) {
        security_logger_->warn("Security event [{}] [{}]: {} - {}", 
                              providerToString(provider), severity, event_type, details);
    }
}

void CloudMonitoring::logAuditEvent(CloudProvider provider,
                                   const std::string& user_id,
                                   const std::string& action,
                                   const std::string& resource,
                                   const std::string& result,
                                   const nlohmann::json& metadata) {
    CloudLogEntry entry;
    entry.operation_id = generateOperationId();
    entry.provider = provider;
    entry.operation = CloudOperation::AUTHENTICATE; // Default for audit events
    entry.status = CloudOperationStatus::SUCCESS;
    entry.user_id = user_id;
    entry.timestamp = std::chrono::system_clock::now();
    entry.duration = std::chrono::milliseconds(0);
    entry.metadata = metadata;
    entry.metadata["action"] = action;
    entry.metadata["resource"] = resource;
    entry.metadata["result"] = result;
    entry.context = {
        {"ip_address", ip_address_},
        {"user_agent", user_agent_}
    };
    entry.thread_id = std::this_thread::get_id();
    
    logOperation(entry);
    
    if (audit_logger_) {
        audit_logger_->info("Audit [{}] [{}]: {} {} {} - {}", 
                           providerToString(provider), user_id, action, resource, result);
    }
}

CloudMetrics CloudMonitoring::getMetrics(CloudProvider provider) const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    auto it = metrics_.find(provider);
    if (it != metrics_.end()) {
        return it->second;
    }
    return CloudMetrics{};
}

CloudMetrics CloudMonitoring::getAggregatedMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    CloudMetrics aggregated;
    
    for (const auto& [provider, metrics] : metrics_) {
        aggregated.total_operations += metrics.total_operations.load();
        aggregated.successful_operations += metrics.successful_operations.load();
        aggregated.failed_operations += metrics.failed_operations.load();
        aggregated.timeout_operations += metrics.timeout_operations.load();
        aggregated.retry_operations += metrics.retry_operations.load();
        
        aggregated.total_queries += metrics.total_queries.load();
        aggregated.successful_queries += metrics.successful_queries.load();
        aggregated.failed_queries += metrics.failed_queries.load();
        
        aggregated.total_connections += metrics.total_connections.load();
        aggregated.active_connections += metrics.active_connections.load();
        aggregated.connection_errors += metrics.connection_errors.load();
        
        aggregated.total_subscriptions += metrics.total_subscriptions.load();
        aggregated.active_subscriptions += metrics.active_subscriptions.load();
        aggregated.subscription_errors += metrics.subscription_errors.load();
        
        aggregated.total_requests += metrics.total_requests.load();
        aggregated.successful_requests += metrics.successful_requests.load();
        aggregated.failed_requests += metrics.failed_requests.load();
        
        aggregated.total_errors += metrics.total_errors.load();
        aggregated.authentication_errors += metrics.authentication_errors.load();
        aggregated.network_errors += metrics.network_errors.load();
        aggregated.timeout_errors += metrics.timeout_errors.load();
        aggregated.rate_limit_errors += metrics.rate_limit_errors.load();
        aggregated.quota_errors += metrics.quota_errors.load();
    }
    
    return aggregated;
}

void CloudMonitoring::resetMetrics(CloudProvider provider) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    auto it = metrics_.find(provider);
    if (it != metrics_.end()) {
        it->second = CloudMetrics{};
    }
}

void CloudMonitoring::resetAllMetrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    for (auto& [provider, metrics] : metrics_) {
        metrics = CloudMetrics{};
    }
}

nlohmann::json CloudMonitoring::getHealthStatus(CloudProvider provider) const {
    auto metrics = getMetrics(provider);
    auto now = std::chrono::system_clock::now();
    
    bool healthy = true;
    std::string status = "healthy";
    
    // Check for recent errors
    if (metrics.failed_operations.load() > config_.alert_threshold_errors) {
        healthy = false;
        status = "degraded";
    }
    
    // Check for timeouts
    if (metrics.timeout_errors.load() > 0) {
        healthy = false;
        status = "timeout_issues";
    }
    
    // Check last operation time
    if (metrics.last_operation.time_since_epoch().count() > 0) {
        auto time_since_last = std::chrono::duration_cast<std::chrono::minutes>(
            now - metrics.last_operation).count();
        if (time_since_last > 60) { // No operations in last hour
            status = "inactive";
        }
    }
    
    return {
        {"provider", providerToString(provider)},
        {"status", status},
        {"healthy", healthy},
        {"last_check", std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count()},
        {"metrics", metricsToJson(metrics)}
    };
}

nlohmann::json CloudMonitoring::getAllHealthStatus() const {
    nlohmann::json result = {
        {"overall_status", "healthy"},
        {"providers", nlohmann::json::object()}
    };
    
    bool all_healthy = true;
    
    for (const auto& provider : {CloudProvider::SUPABASE, CloudProvider::FIREBASE, 
                                CloudProvider::AWS, CloudProvider::AZURE, CloudProvider::GOOGLE_CLOUD}) {
        auto status = getHealthStatus(provider);
        result["providers"][providerToString(provider)] = status;
        
        if (!status["healthy"].get<bool>()) {
            all_healthy = false;
        }
    }
    
    result["overall_status"] = all_healthy ? "healthy" : "degraded";
    return result;
}

bool CloudMonitoring::performHealthCheck(CloudProvider provider) {
    // This is a basic health check - in a real implementation,
    // you would perform actual connectivity tests
    auto status = getHealthStatus(provider);
    bool healthy = status["healthy"].get<bool>();
    
    total_health_checks_++;
    last_health_check_ = std::chrono::system_clock::now();
    
    // Notify health callbacks
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        for (const auto& callback : health_callbacks_) {
            try {
                callback(provider, healthy);
            } catch (const std::exception& e) {
                spdlog::error("Health callback error: {}", e.what());
            }
        }
    }
    
    if (logger_) {
        logger_->debug("Health check for {}: {}", providerToString(provider), 
                      healthy ? "healthy" : "unhealthy");
    }
    
    return healthy;
}

nlohmann::json CloudMonitoring::getMonitoringSummary(int hours) const {
    auto now = std::chrono::system_clock::now();
    auto cutoff = now - std::chrono::hours(hours);
    
    nlohmann::json summary = {
        {"period_hours", hours},
        {"generated_at", std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count()},
        {"providers", nlohmann::json::object()},
        {"aggregated", nlohmann::json::object()}
    };
    
    // Get aggregated metrics
    auto aggregated = getAggregatedMetrics();
    summary["aggregated"] = metricsToJson(aggregated);
    
    // Get per-provider metrics
    for (const auto& provider : {CloudProvider::SUPABASE, CloudProvider::FIREBASE, 
                                CloudProvider::AWS, CloudProvider::AZURE, CloudProvider::GOOGLE_CLOUD}) {
        auto metrics = getMetrics(provider);
        summary["providers"][providerToString(provider)] = metricsToJson(metrics);
    }
    
    // Add statistics
    summary["statistics"] = {
        {"total_log_entries", total_log_entries_.load()},
        {"total_alerts", total_alerts_.load()},
        {"total_health_checks", total_health_checks_.load()},
        {"last_health_check", std::chrono::duration_cast<std::chrono::seconds>(
            last_health_check_.time_since_epoch()).count()},
        {"last_metrics_update", std::chrono::duration_cast<std::chrono::seconds>(
            last_metrics_update_.time_since_epoch()).count()}
    };
    
    return summary;
}

void CloudMonitoring::setContext(const std::string& correlation_id,
                                const std::string& user_id,
                                const std::string& session_id,
                                const std::string& ip_address,
                                const std::string& user_agent) {
    correlation_id_ = correlation_id;
    user_id_ = user_id;
    session_id_ = session_id;
    ip_address_ = ip_address;
    user_agent_ = user_agent;
}

void CloudMonitoring::clearContext() {
    correlation_id_.clear();
    user_id_.clear();
    session_id_.clear();
    ip_address_.clear();
    user_agent_.clear();
}

void CloudMonitoring::registerLogCallback(CloudLogCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    log_callbacks_.push_back(callback);
}

void CloudMonitoring::registerAlertCallback(CloudAlertCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    alert_callbacks_.push_back(callback);
}

void CloudMonitoring::registerHealthCallback(CloudHealthCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    health_callbacks_.push_back(callback);
}

void CloudMonitoring::registerMetricsCallback(CloudMetricsCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    metrics_callbacks_.push_back(callback);
}

CloudMonitoringConfig CloudMonitoring::getConfig() const {
    return config_;
}

bool CloudMonitoring::updateConfig(const CloudMonitoringConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    return true;
}

bool CloudMonitoring::isInitialized() const {
    return initialized_.load();
}

nlohmann::json CloudMonitoring::getStatus() const {
    return {
        {"initialized", initialized_.load()},
        {"config", {
            {"enable_logging", config_.enable_logging},
            {"enable_metrics", config_.enable_metrics},
            {"enable_health_checks", config_.enable_health_checks},
            {"log_level", config_.log_level},
            {"log_path", config_.log_path}
        }},
        {"statistics", {
            {"total_log_entries", total_log_entries_.load()},
            {"total_alerts", total_alerts_.load()},
            {"total_health_checks", total_health_checks_.load()}
        }},
        {"health_status", getAllHealthStatus()}
    };
}

// Private helper methods

void CloudMonitoring::initializeLogging() {
    try {
        std::vector<std::shared_ptr<spdlog::sink>> sinks;
        
        // Console sink
        if (config_.enable_console_logging) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            sinks.push_back(console_sink);
        }
        
        // File sinks
        if (config_.enable_file_logging && !config_.log_path.empty()) {
            // Main log file
            auto main_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config_.log_path + "/cloud_monitoring.log",
                config_.max_log_size,
                config_.max_log_files
            );
            sinks.push_back(main_sink);
            
            // Error log file
            auto error_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config_.log_path + "/cloud_errors.log",
                config_.max_log_size,
                config_.max_log_files
            );
            error_logger_ = std::make_shared<spdlog::logger>("cloud_errors", error_sink);
            error_logger_->set_level(spdlog::level::err);
            
            // Performance log file
            auto perf_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config_.log_path + "/cloud_performance.log",
                config_.max_log_size,
                config_.max_log_files
            );
            performance_logger_ = std::make_shared<spdlog::logger>("cloud_performance", perf_sink);
            performance_logger_->set_level(spdlog::level::info);
            
            // Security log file
            auto sec_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config_.log_path + "/cloud_security.log",
                config_.max_log_size,
                config_.max_log_files
            );
            security_logger_ = std::make_shared<spdlog::logger>("cloud_security", sec_sink);
            security_logger_->set_level(spdlog::level::warn);
            
            // Audit log file
            auto audit_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                config_.log_path + "/cloud_audit.log",
                config_.max_log_size,
                config_.max_log_files
            );
            audit_logger_ = std::make_shared<spdlog::logger>("cloud_audit", audit_sink);
            audit_logger_->set_level(spdlog::level::info);
        }
        
        // Create main logger
        if (!sinks.empty()) {
            logger_ = std::make_shared<spdlog::logger>("cloud_monitoring", sinks.begin(), sinks.end());
            
            // Set log level
            if (config_.log_level == "trace") logger_->set_level(spdlog::level::trace);
            else if (config_.log_level == "debug") logger_->set_level(spdlog::level::debug);
            else if (config_.log_level == "info") logger_->set_level(spdlog::level::info);
            else if (config_.log_level == "warn") logger_->set_level(spdlog::level::warn);
            else if (config_.log_level == "error") logger_->set_level(spdlog::level::err);
            else if (config_.log_level == "critical") logger_->set_level(spdlog::level::critical);
            else logger_->set_level(spdlog::level::info);
            
            // Set pattern
            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
            
            spdlog::register_logger(logger_);
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to initialize cloud monitoring logging: {}", e.what());
    }
}

void CloudMonitoring::initializeMetrics() {
    // Metrics are already initialized in constructor
    last_metrics_update_ = std::chrono::system_clock::now();
}

void CloudMonitoring::initializeHealthChecks() {
    if (config_.enable_health_checks) {
        health_check_thread_ = std::thread(&CloudMonitoring::healthCheckWorker, this);
        metrics_thread_ = std::thread(&CloudMonitoring::metricsWorker, this);
    }
}

void CloudMonitoring::processLogEntry(const CloudLogEntry& entry) {
    if (logger_) {
        std::string level_str = statusToString(entry.status);
        std::string provider_str = providerToString(entry.provider);
        std::string operation_str = operationToString(entry.operation);
        
        logger_->log(spdlog::level::info, 
                    "Cloud operation [{}] [{}] [{}]: {} ({}ms)", 
                    provider_str, operation_str, level_str, 
                    entry.operation_id, entry.duration.count());
    }
}

void CloudMonitoring::updateMetrics(const CloudLogEntry& entry) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    auto& metrics = metrics_[entry.provider];
    
    // Update operation counts
    metrics.total_operations++;
    metrics.last_operation = entry.timestamp;
    
    switch (entry.status) {
        case CloudOperationStatus::SUCCESS:
            metrics.successful_operations++;
            metrics.last_success = entry.timestamp;
            break;
        case CloudOperationStatus::FAILED:
            metrics.failed_operations++;
            metrics.total_errors++;
            metrics.last_error = entry.timestamp;
            break;
        case CloudOperationStatus::TIMEOUT:
            metrics.timeout_operations++;
            metrics.timeout_errors++;
            metrics.last_error = entry.timestamp;
            break;
        case CloudOperationStatus::RETRY:
            metrics.retry_operations++;
            break;
        case CloudOperationStatus::CANCELLED:
            metrics.failed_operations++;
            break;
    }
    
    // Update operation-specific metrics
    switch (entry.operation) {
        case CloudOperation::QUERY:
            metrics.total_queries++;
            if (entry.status == CloudOperationStatus::SUCCESS) {
                metrics.successful_queries++;
            } else {
                metrics.failed_queries++;
            }
            
            // Update query time metrics
            double query_time = static_cast<double>(entry.duration.count());
            if (query_time > 0) {
                if (query_time > metrics.max_query_time.load()) {
                    metrics.max_query_time.store(query_time);
                }
                if (query_time < metrics.min_query_time.load()) {
                    metrics.min_query_time.store(query_time);
                }
                
                // Update average query time
                int64_t total_queries = metrics.total_queries.load();
                double current_avg = metrics.average_query_time.load();
                double new_avg = (current_avg * (total_queries - 1) + query_time) / total_queries;
                metrics.average_query_time.store(new_avg);
            }
            break;
            
        case CloudOperation::CONNECT:
            metrics.total_connections++;
            if (entry.status == CloudOperationStatus::SUCCESS) {
                metrics.active_connections++;
            } else {
                metrics.connection_errors++;
            }
            break;
            
        case CloudOperation::DISCONNECT:
            if (entry.status == CloudOperationStatus::SUCCESS) {
                if (metrics.active_connections.load() > 0) {
                    metrics.active_connections--;
                }
            }
            break;
            
        case CloudOperation::SUBSCRIBE:
            metrics.total_subscriptions++;
            if (entry.status == CloudOperationStatus::SUCCESS) {
                metrics.active_subscriptions++;
            } else {
                metrics.subscription_errors++;
            }
            break;
            
        case CloudOperation::UNSUBSCRIBE:
            if (entry.status == CloudOperationStatus::SUCCESS) {
                if (metrics.active_subscriptions.load() > 0) {
                    metrics.active_subscriptions--;
                }
            }
            break;
            
        default:
            break;
    }
    
    // Update request metrics
    metrics.total_requests++;
    if (entry.status == CloudOperationStatus::SUCCESS) {
        metrics.successful_requests++;
    } else {
        metrics.failed_requests++;
    }
    
    // Update response time metrics
    double response_time = static_cast<double>(entry.duration.count());
    if (response_time > 0) {
        if (response_time > metrics.max_response_time.load()) {
            metrics.max_response_time.store(response_time);
        }
        if (response_time < metrics.min_response_time.load()) {
            metrics.min_response_time.store(response_time);
        }
        
        // Update average response time
        int64_t total_requests = metrics.total_requests.load();
        double current_avg = metrics.average_response_time.load();
        double new_avg = (current_avg * (total_requests - 1) + response_time) / total_requests;
        metrics.average_response_time.store(new_avg);
    }
    
    // Update error type metrics
    if (entry.status != CloudOperationStatus::SUCCESS) {
        if (entry.error_code.find("auth") != std::string::npos) {
            metrics.authentication_errors++;
        } else if (entry.error_code.find("network") != std::string::npos) {
            metrics.network_errors++;
        } else if (entry.error_code.find("rate_limit") != std::string::npos) {
            metrics.rate_limit_errors++;
        } else if (entry.error_code.find("quota") != std::string::npos) {
            metrics.quota_errors++;
        }
    }
}

void CloudMonitoring::checkAlerts(const CloudLogEntry& entry) {
    // Check for error threshold alerts
    auto metrics = getMetrics(entry.provider);
    if (metrics.failed_operations.load() >= config_.alert_threshold_errors) {
        total_alerts_++;
        
        // Notify alert callbacks
        {
            std::lock_guard<std::mutex> lock(callbacks_mutex_);
            for (const auto& callback : alert_callbacks_) {
                try {
                    callback(entry.operation_id, entry.status);
                } catch (const std::exception& e) {
                    spdlog::error("Alert callback error: {}", e.what());
                }
            }
        }
    }
    
    // Check for timeout alerts
    if (entry.duration.count() > config_.alert_threshold_timeout) {
        total_alerts_++;
        
        if (logger_) {
            logger_->warn("Operation timeout alert: {} took {}ms (threshold: {}ms)", 
                         entry.operation_id, entry.duration.count(), config_.alert_threshold_timeout);
        }
    }
}

void CloudMonitoring::notifyCallbacks(const CloudLogEntry& entry) {
    // Notify log callbacks
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        for (const auto& callback : log_callbacks_) {
            try {
                callback(entry);
            } catch (const std::exception& e) {
                spdlog::error("Log callback error: {}", e.what());
            }
        }
    }
    
    // Notify metrics callbacks periodically
    auto now = std::chrono::system_clock::now();
    auto time_since_update = std::chrono::duration_cast<std::chrono::seconds>(
        now - last_metrics_update_).count();
    
    if (time_since_update >= 60) { // Update every minute
        auto metrics = getMetrics(entry.provider);
        {
            std::lock_guard<std::mutex> lock(callbacks_mutex_);
            for (const auto& callback : metrics_callbacks_) {
                try {
                    callback(entry.provider, metrics);
                } catch (const std::exception& e) {
                    spdlog::error("Metrics callback error: {}", e.what());
                }
            }
        }
        last_metrics_update_ = now;
    }
}

void CloudMonitoring::healthCheckWorker() {
    while (!shutdown_requested_.load()) {
        try {
            // Perform health checks for all providers
            for (const auto& provider : {CloudProvider::SUPABASE, CloudProvider::FIREBASE, 
                                        CloudProvider::AWS, CloudProvider::AZURE, CloudProvider::GOOGLE_CLOUD}) {
                performHealthCheck(provider);
            }
            
            // Wait for next health check interval
            std::unique_lock<std::mutex> lock(mutex_);
            health_check_cv_.wait_for(lock, 
                std::chrono::seconds(config_.health_check_interval_seconds),
                [this] { return shutdown_requested_.load(); });
                
        } catch (const std::exception& e) {
            spdlog::error("Health check worker error: {}", e.what());
        }
    }
}

void CloudMonitoring::metricsWorker() {
    while (!shutdown_requested_.load()) {
        try {
            // Update metrics retention
            auto now = std::chrono::system_clock::now();
            auto cutoff = now - std::chrono::hours(config_.metrics_retention_hours);
            
            // Clean up old metrics data if needed
            // This is a placeholder for metrics retention logic
            
            // Wait for next metrics update
            std::unique_lock<std::mutex> lock(mutex_);
            metrics_cv_.wait_for(lock, std::chrono::minutes(5),
                [this] { return shutdown_requested_.load(); });
                
        } catch (const std::exception& e) {
            spdlog::error("Metrics worker error: {}", e.what());
        }
    }
}

std::string CloudMonitoring::generateOperationId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex_chars = "0123456789abcdef";
    
    std::string id = "op_";
    for (int i = 0; i < 16; ++i) {
        id += hex_chars[dis(gen)];
    }
    return id;
}

std::string CloudMonitoring::providerToString(CloudProvider provider) const {
    switch (provider) {
        case CloudProvider::SUPABASE: return "supabase";
        case CloudProvider::FIREBASE: return "firebase";
        case CloudProvider::AWS: return "aws";
        case CloudProvider::AZURE: return "azure";
        case CloudProvider::GOOGLE_CLOUD: return "google_cloud";
        default: return "unknown";
    }
}

std::string CloudMonitoring::operationToString(CloudOperation operation) const {
    switch (operation) {
        case CloudOperation::CONNECT: return "connect";
        case CloudOperation::DISCONNECT: return "disconnect";
        case CloudOperation::QUERY: return "query";
        case CloudOperation::TRANSACTION: return "transaction";
        case CloudOperation::SUBSCRIBE: return "subscribe";
        case CloudOperation::UNSUBSCRIBE: return "unsubscribe";
        case CloudOperation::AUTHENTICATE: return "authenticate";
        case CloudOperation::UPLOAD: return "upload";
        case CloudOperation::DOWNLOAD: return "download";
        case CloudOperation::DELETE: return "delete";
        case CloudOperation::UPDATE: return "update";
        case CloudOperation::CREATE: return "create";
        default: return "unknown";
    }
}

std::string CloudMonitoring::statusToString(CloudOperationStatus status) const {
    switch (status) {
        case CloudOperationStatus::SUCCESS: return "success";
        case CloudOperationStatus::FAILED: return "failed";
        case CloudOperationStatus::TIMEOUT: return "timeout";
        case CloudOperationStatus::RETRY: return "retry";
        case CloudOperationStatus::CANCELLED: return "cancelled";
        default: return "unknown";
    }
}

nlohmann::json CloudMonitoring::metricsToJson(const CloudMetrics& metrics) const {
    return {
        {"operations", {
            {"total", metrics.total_operations.load()},
            {"successful", metrics.successful_operations.load()},
            {"failed", metrics.failed_operations.load()},
            {"timeout", metrics.timeout_operations.load()},
            {"retry", metrics.retry_operations.load()}
        }},
        {"queries", {
            {"total", metrics.total_queries.load()},
            {"successful", metrics.successful_queries.load()},
            {"failed", metrics.failed_queries.load()},
            {"average_time_ms", metrics.average_query_time.load()},
            {"max_time_ms", metrics.max_query_time.load()},
            {"min_time_ms", metrics.min_query_time.load()}
        }},
        {"connections", {
            {"total", metrics.total_connections.load()},
            {"active", metrics.active_connections.load()},
            {"errors", metrics.connection_errors.load()}
        }},
        {"subscriptions", {
            {"total", metrics.total_subscriptions.load()},
            {"active", metrics.active_subscriptions.load()},
            {"errors", metrics.subscription_errors.load()}
        }},
        {"requests", {
            {"total", metrics.total_requests.load()},
            {"successful", metrics.successful_requests.load()},
            {"failed", metrics.failed_requests.load()},
            {"average_response_time_ms", metrics.average_response_time.load()},
            {"max_response_time_ms", metrics.max_response_time.load()},
            {"min_response_time_ms", metrics.min_response_time.load()}
        }},
        {"errors", {
            {"total", metrics.total_errors.load()},
            {"authentication", metrics.authentication_errors.load()},
            {"network", metrics.network_errors.load()},
            {"timeout", metrics.timeout_errors.load()},
            {"rate_limit", metrics.rate_limit_errors.load()},
            {"quota", metrics.quota_errors.load()}
        }},
        {"timestamps", {
            {"last_operation", std::chrono::duration_cast<std::chrono::seconds>(
                metrics.last_operation.time_since_epoch()).count()},
            {"last_error", std::chrono::duration_cast<std::chrono::seconds>(
                metrics.last_error.time_since_epoch()).count()},
            {"last_success", std::chrono::duration_cast<std::chrono::seconds>(
                metrics.last_success.time_since_epoch()).count()}
        }}
    };
}

} // namespace core
} // namespace satox
 