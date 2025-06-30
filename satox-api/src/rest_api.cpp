#include "satox/api/rest_api.hpp"
#include <crow.h>
#include <iostream>
#include <thread>
#include <mutex>

namespace satox {
namespace api {

RESTAPI& RESTAPI::getInstance() {
    static RESTAPI instance;
    return instance;
}

bool RESTAPI::initialize(const std::string& host, int port) {
    if (initialized_) {
        return true;
    }

    try {
        host_ = host;
        port_ = port;
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing REST API: " << e.what() << std::endl;
        return false;
    }
}

void RESTAPI::shutdown() {
    if (!initialized_) {
        return;
    }

    stop();
    endpoints_.clear();
    initialized_ = false;
}

bool RESTAPI::start() {
    if (!initialized_) {
        std::cerr << "REST API not initialized" << std::endl;
        return false;
    }

    try {
        crow::SimpleApp app;

        // Register all endpoints
        for (const auto& endpoint : endpoints_) {
            const auto& path = endpoint.first;
            const auto& handler = endpoint.second;

            // Handle GET requests
            CROW_ROUTE(app, path)
            .methods("GET"_method)
            ([handler](const crow::request& req) {
                try {
                    nlohmann::json requestData;
                    if (!req.body.empty()) {
                        requestData = nlohmann::json::parse(req.body);
                    }

                    auto response = handler(requestData);
                    return crow::response(response.dump());
                } catch (const std::exception& e) {
                    return crow::response(500, e.what());
                }
            });

            // Handle POST requests
            CROW_ROUTE(app, path)
            .methods("POST"_method)
            ([handler](const crow::request& req) {
                try {
                    nlohmann::json requestData;
                    if (!req.body.empty()) {
                        requestData = nlohmann::json::parse(req.body);
                    }

                    auto response = handler(requestData);
                    return crow::response(response.dump());
                } catch (const std::exception& e) {
                    return crow::response(500, e.what());
                }
            });

            // Handle PUT requests
            CROW_ROUTE(app, path)
            .methods("PUT"_method)
            ([handler](const crow::request& req) {
                try {
                    nlohmann::json requestData;
                    if (!req.body.empty()) {
                        requestData = nlohmann::json::parse(req.body);
                    }

                    auto response = handler(requestData);
                    return crow::response(response.dump());
                } catch (const std::exception& e) {
                    return crow::response(500, e.what());
                }
            });

            // Handle DELETE requests
            CROW_ROUTE(app, path)
            .methods("DELETE"_method)
            ([handler](const crow::request& req) {
                try {
                    nlohmann::json requestData;
                    if (!req.body.empty()) {
                        requestData = nlohmann::json::parse(req.body);
                    }

                    auto response = handler(requestData);
                    return crow::response(response.dump());
                } catch (const std::exception& e) {
                    return crow::response(500, e.what());
                }
            });
        }

        // Start the server in a separate thread
        std::thread serverThread([&app, this]() {
            app.port(port_).multithreaded().run();
        });
        serverThread.detach();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error starting REST API: " << e.what() << std::endl;
        return false;
    }
}

void RESTAPI::stop() {
    // Crow doesn't provide a direct way to stop the server
    // We'll need to implement a custom shutdown mechanism
    // For now, we'll just clear the endpoints
    endpoints_.clear();
}

bool RESTAPI::registerEndpoint(const std::string& path,
                             const std::string& method,
                             std::function<nlohmann::json(const nlohmann::json&)> handler) {
    if (!initialized_) {
        std::cerr << "REST API not initialized" << std::endl;
        return false;
    }

    try {
        endpoints_[path] = handler;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error registering endpoint: " << e.what() << std::endl;
        return false;
    }
}

nlohmann::json RESTAPI::getStatus() const {
    nlohmann::json status;
    status["initialized"] = initialized_;
    status["host"] = host_;
    status["port"] = port_;
    status["endpoints"] = nlohmann::json::array();
    
    for (const auto& endpoint : endpoints_) {
        status["endpoints"].push_back(endpoint.first);
    }

    return status;
}

} // namespace api
} // namespace satox 