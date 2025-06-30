/**
 * @file cloud_monitoring.hpp
 * @brief Cloud database monitoring and logging system for Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace satox {
namespace core {

/**
 * @brief Cloud provider types
 */
enum class CloudProvider {
    SUPABASE,
    FIREBASE,
    AWS,
    AZURE,
    GOOGLE_CLOUD
};

/**
 * @brief Cloud operation types
 */
enum class CloudOperation {
    CONNECT,
    DISCONNECT,
    QUERY,
    TRANSACTION,
    SUBSCRIBE,
    UNSUBSCRIBE,
    AUTHENTICATE,
    UPLOAD,
    DOWNLOAD,
    DELETE,
    UPDATE,
    CREATE
};

/**
 * @brief Cloud operation status
 */
enum class CloudOperationStatus {
    SUCCESS,
    FAILED,
    TIMEOUT,
    RETRY,
    CANCELLED
};

/**
 * @brief Cloud monitoring metrics
 */
struct CloudMetrics {
    std::atomic<int64_t> total_operations{0};
    std::atomic<int64_t> successful_operations{0};
    std::atomic<int64_t> failed_operations{0};
    std::atomic<int64_t> timeout_operations{0};
    std::atomic<int64_t> retry_operations{0};
    
    std::atomic<int64_t> total_queries{0};
    std::atomic<int64_t> successful_queries{0};
    std::atomic<int64_t> failed_queries{0};
    std::atomic<double> average_query_time{0.0};
    std::atomic<double> max_query_time{0.0};
    std::atomic<double> min_query_time{std::numeric_limits<double>::max()};
    
    std::atomic<int64_t> total_connections{0};
    std::atomic<int64_t> active_connections{0};
    std::atomic<int64_t> connection_errors{0};
    
    std::atomic<int64_t> total_subscriptions{0};
    std::atomic<int64_t> active_subscriptions{0};
    std::atomic<int64_t> subscription_errors{0};
    
    std::atomic<int64_t> total_requests{0};
    std::atomic<int64_t> successful_requests{0};
    std::atomic<int64_t> failed_requests{0};
    std::atomic<double> average_response_time{0.0};
    std::atomic<double> max_response_time{0.0};
    std::atomic<double> min_response_time{std::numeric_limits<double>::max()};
    
    std::atomic<int64_t> total_errors{0};
    std::atomic<int64_t> authentication_errors{0};
    std::atomic<int64_t> network_errors{0};
    std::atomic<int64_t> timeout_errors{0};
    std::atomic<int64_t> rate_limit_errors{0};
    std::atomic<int64_t> quota_errors{0};
    
    std::chrono::system_clock::time_point last_operation;
    std::chrono::system_clock::time_point last_error;
    std::chrono::system_clock::time_point last_success;
};

/**
 * @brief Cloud operation log entry
 */
struct CloudLogEntry {
    std::string operation_id;
    CloudProvider provider;
    CloudOperation operation;
    CloudOperationStatus status;
    std::string connection_id;
    std::string user_id;
    std::string session_id;
    std::string correlation_id;
    std::string error_message;
    std::string error_code;
    std::chrono::system_clock::time_point timestamp;
    std::chrono::milliseconds duration;
    nlohmann::json metadata;
    nlohmann::json context;
    std::string ip_address;
    std::string user_agent;
    std::thread::id thread_id;
};

/**
 * @brief Cloud monitoring configuration
 */
struct CloudMonitoringConfig {
    bool enable_logging = true;
    bool enable_metrics = true;
    bool enable_tracing = true;
    bool enable_alerts = true;
    bool enable_health_checks = true;
    
    std::string log_level = "info";
    std::string log_path = "logs/cloud";
    int max_log_size = 100 * 1024 * 1024; // 100MB
    int max_log_files = 10;
    bool enable_console_logging = true;
    bool enable_file_logging = true;
    bool enable_json_logging = true;
    
    int metrics_retention_hours = 24;
    int health_check_interval_seconds = 30;
    int alert_threshold_errors = 10;
    int alert_threshold_timeout = 5000; // ms
    
    bool enable_performance_monitoring = true;
    bool enable_security_monitoring = true;
    bool enable_audit_logging = true;
    
    nlohmann::json additional_config;
};

/**
 * @brief Cloud monitoring callback types
 */
using CloudLogCallback = std::function<void(const CloudLogEntry&)>;
using CloudAlertCallback = std::function<void(const std::string&, CloudOperationStatus)>;
using CloudHealthCallback = std::function<void(CloudProvider, bool)>;
using CloudMetricsCallback = std::function<void(CloudProvider, const CloudMetrics&)>;

/**
 * @brief Cloud monitoring and logging manager
 * 
 * Provides comprehensive monitoring, logging, metrics collection, and alerting
 * for all cloud database providers in the Satox SDK.
 */
class CloudMonitoring {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static CloudMonitoring& getInstance();
    
    /**
     * @brief Constructor
     */
    CloudMonitoring();
    
    /**
     * @brief Destructor
     */
    ~CloudMonitoring();
    
    /**
     * @brief Initialize monitoring system
     * @param config Monitoring configuration
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const CloudMonitoringConfig& config);
    
    /**
     * @brief Shutdown monitoring system
     */
    void shutdown();
    
    /**
     * @brief Log cloud operation
     * @param entry Log entry to record
     */
    void logOperation(const CloudLogEntry& entry);
    
    /**
     * @brief Log operation with automatic timing
     * @param provider Cloud provider
     * @param operation Operation type
     * @param connection_id Connection ID
     * @param metadata Additional metadata
     * @return Operation ID for tracking
     */
    std::string startOperation(CloudProvider provider, 
                              CloudOperation operation,
                              const std::string& connection_id = "",
                              const nlohmann::json& metadata = {});
    
    /**
     * @brief Complete operation with status
     * @param operation_id Operation ID
     * @param status Operation status
     * @param error_message Error message if failed
     * @param result Result data
     */
    void completeOperation(const std::string& operation_id,
                          CloudOperationStatus status,
                          const std::string& error_message = "",
                          const nlohmann::json& result = {});
    
    /**
     * @brief Log error
     * @param provider Cloud provider
     * @param operation Operation type
     * @param error_message Error message
     * @param error_code Error code
     * @param connection_id Connection ID
     * @param metadata Additional metadata
     */
    void logError(CloudProvider provider,
                  CloudOperation operation,
                  const std::string& error_message,
                  const std::string& error_code = "",
                  const std::string& connection_id = "",
                  const nlohmann::json& metadata = {});
    
    /**
     * @brief Log performance metrics
     * @param provider Cloud provider
     * @param operation Operation type
     * @param duration Operation duration
     * @param success Whether operation was successful
     * @param metadata Additional metadata
     */
    void logPerformance(CloudProvider provider,
                        CloudOperation operation,
                        std::chrono::milliseconds duration,
                        bool success,
                        const nlohmann::json& metadata = {});
    
    /**
     * @brief Log security event
     * @param provider Cloud provider
     * @param event_type Security event type
     * @param severity Security severity level
     * @param details Event details
     * @param metadata Additional metadata
     */
    void logSecurityEvent(CloudProvider provider,
                         const std::string& event_type,
                         const std::string& severity,
                         const std::string& details,
                         const nlohmann::json& metadata = {});
    
    /**
     * @brief Log audit event
     * @param provider Cloud provider
     * @param user_id User ID
     * @param action Action performed
     * @param resource Resource accessed
     * @param result Result of action
     * @param metadata Additional metadata
     */
    void logAuditEvent(CloudProvider provider,
                      const std::string& user_id,
                      const std::string& action,
                      const std::string& resource,
                      const std::string& result,
                      const nlohmann::json& metadata = {});
    
    /**
     * @brief Get metrics for provider
     * @param provider Cloud provider
     * @return Metrics for the provider
     */
    CloudMetrics getMetrics(CloudProvider provider) const;
    
    /**
     * @brief Get aggregated metrics for all providers
     * @return Aggregated metrics
     */
    CloudMetrics getAggregatedMetrics() const;
    
    /**
     * @brief Reset metrics for provider
     * @param provider Cloud provider
     */
    void resetMetrics(CloudProvider provider);
    
    /**
     * @brief Reset all metrics
     */
    void resetAllMetrics();
    
    /**
     * @brief Get health status for provider
     * @param provider Cloud provider
     * @return Health status JSON
     */
    nlohmann::json getHealthStatus(CloudProvider provider) const;
    
    /**
     * @brief Get health status for all providers
     * @return Health status JSON for all providers
     */
    nlohmann::json getAllHealthStatus() const;
    
    /**
     * @brief Perform health check for provider
     * @param provider Cloud provider
     * @return true if healthy, false otherwise
     */
    bool performHealthCheck(CloudProvider provider);
    
    /**
     * @brief Get monitoring summary
     * @param hours Number of hours to include in summary
     * @return Monitoring summary JSON
     */
    nlohmann::json getMonitoringSummary(int hours = 24) const;
    
    /**
     * @brief Set context information
     * @param correlation_id Correlation ID
     * @param user_id User ID
     * @param session_id Session ID
     * @param ip_address IP address
     * @param user_agent User agent
     */
    void setContext(const std::string& correlation_id = "",
                   const std::string& user_id = "",
                   const std::string& session_id = "",
                   const std::string& ip_address = "",
                   const std::string& user_agent = "");
    
    /**
     * @brief Clear context information
     */
    void clearContext();
    
    /**
     * @brief Register log callback
     * @param callback Log callback function
     */
    void registerLogCallback(CloudLogCallback callback);
    
    /**
     * @brief Register alert callback
     * @param callback Alert callback function
     */
    void registerAlertCallback(CloudAlertCallback callback);
    
    /**
     * @brief Register health callback
     * @param callback Health callback function
     */
    void registerHealthCallback(CloudHealthCallback callback);
    
    /**
     * @brief Register metrics callback
     * @param callback Metrics callback function
     */
    void registerMetricsCallback(CloudMetricsCallback callback);
    
    /**
     * @brief Get configuration
     * @return Current configuration
     */
    CloudMonitoringConfig getConfig() const;
    
    /**
     * @brief Update configuration
     * @param config New configuration
     * @return true if update successful, false otherwise
     */
    bool updateConfig(const CloudMonitoringConfig& config);
    
    /**
     * @brief Check if monitoring is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const;
    
    /**
     * @brief Get monitoring status
     * @return Status JSON
     */
    nlohmann::json getStatus() const;

private:
    // Disable copy and assignment
    CloudMonitoring(const CloudMonitoring&) = delete;
    CloudMonitoring& operator=(const CloudMonitoring&) = delete;
    
    // Private helper methods
    void initializeLogging();
    void initializeMetrics();
    void initializeHealthChecks();
    void processLogEntry(const CloudLogEntry& entry);
    void updateMetrics(const CloudLogEntry& entry);
    void checkAlerts(const CloudLogEntry& entry);
    void notifyCallbacks(const CloudLogEntry& entry);
    void healthCheckWorker();
    void metricsWorker();
    std::string generateOperationId();
    std::string providerToString(CloudProvider provider) const;
    std::string operationToString(CloudOperation operation) const;
    std::string statusToString(CloudOperationStatus status) const;
    nlohmann::json metricsToJson(const CloudMetrics& metrics) const;
    
    // Member variables
    mutable std::mutex mutex_;
    std::atomic<bool> initialized_{false};
    CloudMonitoringConfig config_;
    
    // Logging
    std::shared_ptr<spdlog::logger> logger_;
    std::shared_ptr<spdlog::logger> error_logger_;
    std::shared_ptr<spdlog::logger> performance_logger_;
    std::shared_ptr<spdlog::logger> security_logger_;
    std::shared_ptr<spdlog::logger> audit_logger_;
    
    // Metrics
    std::unordered_map<CloudProvider, CloudMetrics> metrics_;
    mutable std::mutex metrics_mutex_;
    
    // Operations tracking
    std::unordered_map<std::string, std::chrono::system_clock::time_point> active_operations_;
    std::unordered_map<std::string, CloudLogEntry> operation_cache_;
    mutable std::mutex operations_mutex_;
    
    // Callbacks
    std::vector<CloudLogCallback> log_callbacks_;
    std::vector<CloudAlertCallback> alert_callbacks_;
    std::vector<CloudHealthCallback> health_callbacks_;
    std::vector<CloudMetricsCallback> metrics_callbacks_;
    mutable std::mutex callbacks_mutex_;
    
    // Context
    std::string correlation_id_;
    std::string user_id_;
    std::string session_id_;
    std::string ip_address_;
    std::string user_agent_;
    
    // Background workers
    std::thread health_check_thread_;
    std::thread metrics_thread_;
    std::atomic<bool> shutdown_requested_{false};
    std::condition_variable health_check_cv_;
    std::condition_variable metrics_cv_;
    
    // Statistics
    std::atomic<int64_t> total_log_entries_{0};
    std::atomic<int64_t> total_alerts_{0};
    std::atomic<int64_t> total_health_checks_{0};
    std::chrono::system_clock::time_point last_health_check_;
    std::chrono::system_clock::time_point last_metrics_update_;
};

} // namespace core
} // namespace satox 