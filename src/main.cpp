#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// Include SDK headers
#include "satox/core/satox_manager.hpp"
#include "satox/core/config_manager.hpp"

volatile bool running = true;

void signal_handler(int signal) {
    spdlog::info("Received signal {}, shutting down...", signal);
    running = false;
}

int main(int argc, char* argv[]) {
    // Set up logging
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("satox-sdk", console_sink);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);
    
    spdlog::info("=== Satox SDK Service Starting ===");
    spdlog::info("Version: 1.0.0");
    spdlog::info("Build: {}", __DATE__);
    
    // Set up signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // Initialize SDK configuration using singleton reference
        satox::core::ConfigManager& config = satox::core::ConfigManager::getInstance();
        spdlog::info("Configuration manager initialized");
        
        // Initialize Satox Manager using singleton reference
        satox::core::SatoxManager& satox_manager = satox::core::SatoxManager::getInstance();
        spdlog::info("Satox Manager initialized");
        
        // Start SDK services
        if (satox_manager.initialize()) {
            spdlog::info("Satox SDK initialized successfully");
        } else {
            spdlog::error("Failed to initialize Satox SDK");
            return 1;
        }
        
        spdlog::info("Satox SDK service is running. Press Ctrl+C to stop.");
        
        // Main service loop
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            // Health check - log status every 30 seconds
            static int counter = 0;
            if (++counter % 6 == 0) {
                spdlog::info("Satox SDK service is healthy - running for {} seconds", counter * 5);
            }
        }
        
        // Cleanup
        spdlog::info("Shutting down Satox SDK...");
        satox_manager.shutdown();
        spdlog::info("Satox SDK shutdown complete");
        
    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }
    
    spdlog::info("=== Satox SDK Service Stopped ===");
    return 0;
} 