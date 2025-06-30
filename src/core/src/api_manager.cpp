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

#include "satox/core/api_manager.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstring>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <regex>

using json = nlohmann::json;

namespace satox {
namespace core {

class APIManager::Impl {
public:
    Impl() : running_(false), ssl_ctx_(nullptr) {
        // Initialize OpenSSL
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        SSL_library_init();
    }

    ~Impl() {
        stop();
        if (ssl_ctx_) {
            SSL_CTX_free(ssl_ctx_);
        }
        EVP_cleanup();
    }

    bool initialize(const Config& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (running_) {
            setError("API manager is already running");
            return false;
        }

        config_ = config;

        // Initialize SSL if enabled
        if (config.enable_ssl) {
            ssl_ctx_ = SSL_CTX_new(TLS_server_method());
            if (!ssl_ctx_) {
                setError("Failed to create SSL context: " + getSSLError());
                return false;
            }

            // Load certificate and private key
            if (SSL_CTX_use_certificate_file(ssl_ctx_, config.ssl_cert.c_str(), SSL_FILETYPE_PEM) <= 0) {
                setError("Failed to load SSL certificate: " + getSSLError());
                return false;
            }

            if (SSL_CTX_use_PrivateKey_file(ssl_ctx_, config.ssl_key.c_str(), SSL_FILETYPE_PEM) <= 0) {
                setError("Failed to load SSL private key: " + getSSLError());
                return false;
            }

            // Load CA certificate if provided
            if (!config.ssl_ca.empty()) {
                if (SSL_CTX_load_verify_locations(ssl_ctx_, config.ssl_ca.c_str(), nullptr) <= 0) {
                    setError("Failed to load CA certificate: " + getSSLError());
                    return false;
                }
            }
        }

        return true;
    }

    bool start() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (running_) {
            setError("API manager is already running");
            return false;
        }

        // Create server socket
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            setError("Failed to create server socket: " + std::string(strerror(errno)));
            return false;
        }

        // Set socket options
        int opt = 1;
        if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            setError("Failed to set socket options: " + std::string(strerror(errno)));
            close(server_fd_);
            return false;
        }

        // Bind socket
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(config_.host.c_str());
        addr.sin_port = htons(config_.port);

        if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            setError("Failed to bind socket: " + std::string(strerror(errno)));
            close(server_fd_);
            return false;
        }

        // Listen for connections
        if (listen(server_fd_, config_.max_connections) < 0) {
            setError("Failed to listen on socket: " + std::string(strerror(errno)));
            close(server_fd_);
            return false;
        }

        // Set non-blocking mode
        int flags = fcntl(server_fd_, F_GETFL, 0);
        fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);

        running_ = true;
        server_thread_ = std::thread(&Impl::serverLoop, this);
        spdlog::info("API manager started on {}:{}", config_.host, config_.port);

        return true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!running_) {
            return;
        }

        running_ = false;
        
        // Close server socket
        if (server_fd_ >= 0) {
            close(server_fd_);
            server_fd_ = -1;
        }

        // Close all client connections
        for (auto& client : clients_) {
            if (client.ssl) {
                SSL_free(client.ssl);
            }
            close(client.fd);
        }
        clients_.clear();

        // Join server thread
        if (server_thread_.joinable()) {
            server_thread_.join();
        }

        spdlog::info("API manager stopped");
    }

    bool isRunning() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return running_;
    }

    bool registerEndpoint(const std::string& method, const std::string& path, RequestHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        endpoints_[method + ":" + path] = handler;
        return true;
    }

    bool unregisterEndpoint(const std::string& method, const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        return endpoints_.erase(method + ":" + path) > 0;
    }

    bool registerWebSocketHandler(const std::string& path, WebSocketHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        ws_handlers_[path] = handler;
        return true;
    }

    bool unregisterWebSocketHandler(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        return ws_handlers_.erase(path) > 0;
    }

    bool sendWebSocketMessage(const std::string& client_id, const WebSocketMessage& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = std::find_if(clients_.begin(), clients_.end(),
            [&](const Client& client) { return client.id == client_id; });
        
        if (it == clients_.end()) {
            setError("Client not found: " + client_id);
            return false;
        }

        return sendWebSocketFrame(it->fd, it->ssl, message);
    }

    bool broadcastWebSocketMessage(const WebSocketMessage& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        bool success = true;

        for (auto& client : clients_) {
            if (!client.is_websocket) {
                continue;
            }

            if (!sendWebSocketFrame(client.fd, client.ssl, message)) {
                success = false;
            }
        }

        return success;
    }

    bool setRateLimit(int requests_per_minute) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.rate_limit_requests = requests_per_minute;
        return true;
    }

    bool isRateLimited(const std::string& client_ip) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::steady_clock::now();
        auto& client_requests = rate_limits_[client_ip];
        
        // Remove old requests
        client_requests.erase(
            std::remove_if(client_requests.begin(), client_requests.end(),
                [&](const auto& time) {
                    return std::chrono::duration_cast<std::chrono::seconds>(
                        now - time).count() > config_.rate_limit_window;
                }),
            client_requests.end()
        );

        return client_requests.size() >= config_.rate_limit_requests;
    }

    std::string getLastError() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_error_;
    }

    void clearError() {
        std::lock_guard<std::mutex> lock(mutex_);
        last_error_.clear();
    }

private:
    struct Client {
        int fd;
        SSL* ssl;
        std::string id;
        bool is_websocket;
        std::string buffer;
        std::chrono::steady_clock::time_point last_activity;
    };

    void serverLoop() {
        std::vector<pollfd> poll_fds;
        poll_fds.push_back({server_fd_, POLLIN, 0});

        while (running_) {
            // Update poll descriptors
            {
                std::lock_guard<std::mutex> lock(mutex_);
                poll_fds.resize(1);
                for (const auto& client : clients_) {
                    poll_fds.push_back({client.fd, POLLIN, 0});
                }
            }

            // Poll for events
            int poll_result = poll(poll_fds.data(), poll_fds.size(), 100);
            if (poll_result < 0) {
                if (errno == EINTR) {
                    continue;
                }
                spdlog::error("Poll error: {}", strerror(errno));
                continue;
            }

            // Check server socket
            if (poll_fds[0].revents & POLLIN) {
                handleNewConnection();
            }

            // Check client sockets
            {
                std::lock_guard<std::mutex> lock(mutex_);
                for (size_t i = 0; i < clients_.size(); ++i) {
                    if (poll_fds[i + 1].revents & POLLIN) {
                        handleClientData(clients_[i]);
                    }
                }
            }

            // Clean up inactive clients
            cleanupInactiveClients();
        }
    }

    void handleNewConnection() {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                spdlog::error("Accept error: {}", strerror(errno));
            }
            return;
        }

        // Set non-blocking mode
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

        // Create SSL connection if enabled
        SSL* ssl = nullptr;
        if (config_.enable_ssl) {
            ssl = SSL_new(ssl_ctx_);
            if (!ssl) {
                spdlog::error("Failed to create SSL connection: {}", getSSLError());
                close(client_fd);
                return;
            }

            SSL_set_fd(ssl, client_fd);
            if (SSL_accept(ssl) <= 0) {
                spdlog::error("SSL handshake failed: {}", getSSLError());
                SSL_free(ssl);
                close(client_fd);
                return;
            }
        }

        // Add client
        std::lock_guard<std::mutex> lock(mutex_);
        clients_.push_back({
            client_fd,
            ssl,
            generateClientId(),
            false,
            "",
            std::chrono::steady_clock::now()
        });
    }

    void handleClientData(Client& client) {
        char buffer[4096];
        int bytes_read;

        if (client.ssl) {
            bytes_read = SSL_read(client.ssl, buffer, sizeof(buffer));
        } else {
            bytes_read = recv(client.fd, buffer, sizeof(buffer), 0);
        }

        if (bytes_read <= 0) {
            if (client.ssl) {
                int ssl_error = SSL_get_error(client.ssl, bytes_read);
                if (ssl_error != SSL_ERROR_WANT_READ && ssl_error != SSL_ERROR_WANT_WRITE) {
                    closeClient(client);
                }
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                closeClient(client);
            }
            return;
        }

        client.last_activity = std::chrono::steady_clock::now();
        client.buffer.append(buffer, bytes_read);

        if (!client.is_websocket) {
            handleHTTPRequest(client);
        } else {
            handleWebSocketMessage(client);
        }
    }

    void handleHTTPRequest(Client& client) {
        // Check if we have a complete HTTP request
        size_t end_of_headers = client.buffer.find("\r\n\r\n");
        if (end_of_headers == std::string::npos) {
            return;
        }

        // Parse request
        std::istringstream request_stream(client.buffer.substr(0, end_of_headers));
        std::string line;
        
        // Parse request line
        std::getline(request_stream, line);
        std::istringstream line_stream(line);
        std::string method, path, version;
        line_stream >> method >> path >> version;

        // Parse headers
        std::unordered_map<std::string, std::string> headers;
        while (std::getline(request_stream, line) && line != "\r") {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                // Trim whitespace
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                headers[key] = value;
            }
        }

        // Check for WebSocket upgrade
        if (method == "GET" && headers["Upgrade"] == "websocket") {
            handleWebSocketUpgrade(client, headers);
            return;
        }

        // Create request object
        Request request{
            method,
            path,
            headers,
            client.buffer.substr(end_of_headers + 4),
            getClientIP(client.fd)
        };

        // Check rate limiting
        if (config_.enable_rate_limiting && isRateLimited(request.client_ip)) {
            sendErrorResponse(client, 429, "Too Many Requests");
            return;
        }

        // Update rate limit
        rate_limits_[request.client_ip].push_back(std::chrono::steady_clock::now());

        // Find and call handler
        auto it = endpoints_.find(method + ":" + path);
        if (it != endpoints_.end()) {
            Response response = it->second(request);
            sendResponse(client, response);
        } else {
            sendErrorResponse(client, 404, "Not Found");
        }

        // Clear buffer
        client.buffer.clear();
    }

    void handleWebSocketUpgrade(Client& client, const std::unordered_map<std::string, std::string>& headers) {
        // Verify WebSocket key
        std::string ws_key = headers.at("Sec-WebSocket-Key");
        std::string ws_version = headers.at("Sec-WebSocket-Version");

        if (ws_version != "13") {
            sendErrorResponse(client, 400, "Unsupported WebSocket Version");
            return;
        }

        // Generate accept key
        std::string accept_key = generateWebSocketAcceptKey(ws_key);

        // Send upgrade response
        std::string response = 
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: " + accept_key + "\r\n"
            "\r\n";

        if (client.ssl) {
            SSL_write(client.ssl, response.c_str(), response.length());
        } else {
            send(client.fd, response.c_str(), response.length(), 0);
        }

        // Mark as WebSocket client
        client.is_websocket = true;
        client.buffer.clear();

        // Call WebSocket handler
        auto it = ws_handlers_.find(headers.at("Sec-WebSocket-Protocol"));
        if (it != ws_handlers_.end()) {
            WebSocketMessage msg{"open", "", true};
            it->second(msg);
        }
    }

    void handleWebSocketMessage(Client& client) {
        while (client.buffer.length() >= 2) {
            // Parse WebSocket frame
            uint8_t first_byte = client.buffer[0];
            uint8_t second_byte = client.buffer[1];

            bool fin = (first_byte & 0x80) != 0;
            uint8_t opcode = first_byte & 0x0F;
            bool masked = (second_byte & 0x80) != 0;
            uint64_t payload_length = second_byte & 0x7F;

            size_t header_length = 2;
            if (payload_length == 126) {
                if (client.buffer.length() < 4) return;
                payload_length = (client.buffer[2] << 8) | client.buffer[3];
                header_length = 4;
            } else if (payload_length == 127) {
                if (client.buffer.length() < 10) return;
                payload_length = 0;
                for (int i = 0; i < 8; ++i) {
                    payload_length = (payload_length << 8) | client.buffer[2 + i];
                }
                header_length = 10;
            }

            if (masked) {
                header_length += 4;
            }

            if (client.buffer.length() < header_length + payload_length) {
                return;
            }

            // Extract payload
            std::string payload = client.buffer.substr(header_length, payload_length);
            if (masked) {
                // Unmask payload
                uint8_t mask[4];
                for (int i = 0; i < 4; ++i) {
                    mask[i] = client.buffer[header_length - 4 + i];
                }
                for (size_t i = 0; i < payload.length(); ++i) {
                    payload[i] ^= mask[i % 4];
                }
            }

            // Remove processed frame from buffer
            client.buffer.erase(0, header_length + payload_length);

            // Handle control frames
            if (opcode == 0x8) {  // Close
                sendWebSocketClose(client);
                closeClient(client);
                return;
            } else if (opcode == 0x9) {  // Ping
                sendWebSocketPong(client, payload);
                continue;
            } else if (opcode == 0xA) {  // Pong
                continue;
            }

            // Call WebSocket handler
            auto it = ws_handlers_.find(client.id);
            if (it != ws_handlers_.end()) {
                WebSocketMessage msg{
                    opcode == 0x1 ? "text" : "binary",
                    payload,
                    fin
                };
                it->second(msg);
            }
        }
    }

    void sendResponse(const Client& client, const Response& response) {
        std::string http_response = 
            "HTTP/1.1 " + std::to_string(response.status_code) + " " + getStatusMessage(response.status_code) + "\r\n";

        // Add headers
        for (const auto& header : response.headers) {
            http_response += header.first + ": " + header.second + "\r\n";
        }

        // Add content length
        http_response += "Content-Length: " + std::to_string(response.body.length()) + "\r\n";
        http_response += "\r\n";
        http_response += response.body;

        if (client.ssl) {
            SSL_write(client.ssl, http_response.c_str(), http_response.length());
        } else {
            send(client.fd, http_response.c_str(), http_response.length(), 0);
        }
    }

    void sendErrorResponse(const Client& client, int status_code, const std::string& message) {
        Response response{
            status_code,
            {{"Content-Type", "text/plain"}},
            message
        };
        sendResponse(client, response);
    }

    bool sendWebSocketFrame(const Client& client, const WebSocketMessage& message) {
        std::vector<uint8_t> frame;
        
        // First byte: FIN + opcode
        uint8_t first_byte = 0x80;  // FIN bit set
        if (message.type == "text") {
            first_byte |= 0x1;
        } else if (message.type == "binary") {
            first_byte |= 0x2;
        } else if (message.type == "ping") {
            first_byte |= 0x9;
        } else if (message.type == "pong") {
            first_byte |= 0xA;
        } else if (message.type == "close") {
            first_byte |= 0x8;
        }
        frame.push_back(first_byte);

        // Second byte: MASK + payload length
        size_t payload_length = message.data.length();
        if (payload_length <= 125) {
            frame.push_back(payload_length);
        } else if (payload_length <= 65535) {
            frame.push_back(126);
            frame.push_back((payload_length >> 8) & 0xFF);
            frame.push_back(payload_length & 0xFF);
        } else {
            frame.push_back(127);
            for (int i = 0; i < 8; ++i) {
                frame.push_back((payload_length >> ((7 - i) * 8)) & 0xFF);
            }
        }

        // Add payload
        frame.insert(frame.end(), message.data.begin(), message.data.end());

        // Send frame
        if (client.ssl) {
            return SSL_write(client.ssl, frame.data(), frame.size()) > 0;
        } else {
            return send(client.fd, frame.data(), frame.size(), 0) > 0;
        }
    }

    void sendWebSocketClose(const Client& client) {
        WebSocketMessage msg{"close", "", true};
        sendWebSocketFrame(client, msg);
    }

    void sendWebSocketPong(const Client& client, const std::string& payload) {
        WebSocketMessage msg{"pong", payload, true};
        sendWebSocketFrame(client, msg);
    }

    void closeClient(Client& client) {
        if (client.ssl) {
            SSL_free(client.ssl);
        }
        close(client.fd);
        
        std::lock_guard<std::mutex> lock(mutex_);
        clients_.erase(
            std::remove_if(clients_.begin(), clients_.end(),
                [&](const Client& c) { return c.fd == client.fd; }),
            clients_.end()
        );
    }

    void cleanupInactiveClients() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        
        clients_.erase(
            std::remove_if(clients_.begin(), clients_.end(),
                [&](const Client& client) {
                    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                        now - client.last_activity).count();
                    if (duration > config_.connection_timeout) {
                        if (client.ssl) {
                            SSL_free(client.ssl);
                        }
                        close(client.fd);
                        return true;
                    }
                    return false;
                }),
            clients_.end()
        );
    }

    std::string generateClientId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* hex = "0123456789abcdef";
        
        std::string uuid;
        uuid.reserve(36);
        
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                uuid += '-';
            } else {
                uuid += hex[dis(gen)];
            }
        }
        
        return uuid;
    }

    std::string generateWebSocketAcceptKey(const std::string& key) {
        std::string concat = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1(reinterpret_cast<const unsigned char*>(concat.c_str()), concat.length(), hash);
        return base64Encode(hash, SHA_DIGEST_LENGTH);
    }

    std::string base64Encode(const unsigned char* data, size_t length) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string result;
        result.reserve(((length + 2) / 3) * 4);
        
        for (size_t i = 0; i < length; i += 3) {
            unsigned char octet_a = i < length ? data[i] : 0;
            unsigned char octet_b = i + 1 < length ? data[i + 1] : 0;
            unsigned char octet_c = i + 2 < length ? data[i + 2] : 0;
            
            unsigned int triple = (octet_a << 16) + (octet_b << 8) + octet_c;
            
            result.push_back(base64_chars[(triple >> 18) & 0x3F]);
            result.push_back(base64_chars[(triple >> 12) & 0x3F]);
            result.push_back(base64_chars[(triple >> 6) & 0x3F]);
            result.push_back(base64_chars[triple & 0x3F]);
        }
        
        // Add padding
        switch (length % 3) {
            case 1:
                result[result.length() - 2] = '=';
                result[result.length() - 1] = '=';
                break;
            case 2:
                result[result.length() - 1] = '=';
                break;
        }
        
        return result;
    }

    std::string getClientIP(int fd) {
        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(addr);
        if (getpeername(fd, (struct sockaddr*)&addr, &addr_len) == 0) {
            return inet_ntoa(addr.sin_addr);
        }
        return "unknown";
    }

    std::string getStatusMessage(int status_code) {
        switch (status_code) {
            case 200: return "OK";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 429: return "Too Many Requests";
            case 500: return "Internal Server Error";
            default: return "Unknown";
        }
    }

    std::string getSSLError() {
        char err_buf[256];
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        return std::string(err_buf);
    }

    void setError(const std::string& error) {
        last_error_ = error;
        spdlog::error("API manager error: {}", error);
    }

    Config config_;
    std::atomic<bool> running_;
    int server_fd_;
    SSL_CTX* ssl_ctx_;
    std::vector<Client> clients_;
    std::unordered_map<std::string, RequestHandler> endpoints_;
    std::unordered_map<std::string, WebSocketHandler> ws_handlers_;
    std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> rate_limits_;
    std::thread server_thread_;
    mutable std::mutex mutex_;
    std::string last_error_;
};

APIManager::APIManager() : impl_(std::make_unique<Impl>()) {}
APIManager::~APIManager() = default;

bool APIManager::initialize(const Config& config) {
    return impl_->initialize(config);
}

bool APIManager::start() {
    return impl_->start();
}

void APIManager::stop() {
    impl_->stop();
}

bool APIManager::isRunning() const {
    return impl_->isRunning();
}

bool APIManager::registerEndpoint(const std::string& method, const std::string& path, RequestHandler handler) {
    return impl_->registerEndpoint(method, path, handler);
}

bool APIManager::unregisterEndpoint(const std::string& method, const std::string& path) {
    return impl_->unregisterEndpoint(method, path);
}

bool APIManager::registerWebSocketHandler(const std::string& path, WebSocketHandler handler) {
    return impl_->registerWebSocketHandler(path, handler);
}

bool APIManager::unregisterWebSocketHandler(const std::string& path) {
    return impl_->unregisterWebSocketHandler(path);
}

bool APIManager::sendWebSocketMessage(const std::string& client_id, const WebSocketMessage& message) {
    return impl_->sendWebSocketMessage(client_id, message);
}

bool APIManager::broadcastWebSocketMessage(const WebSocketMessage& message) {
    return impl_->broadcastWebSocketMessage(message);
}

bool APIManager::setRateLimit(int requests_per_minute) {
    return impl_->setRateLimit(requests_per_minute);
}

bool APIManager::isRateLimited(const std::string& client_ip) const {
    return impl_->isRateLimited(client_ip);
}

std::string APIManager::getLastError() const {
    return impl_->getLastError();
}

void APIManager::clearError() {
    impl_->clearError();
}

} // namespace core
} // namespace satox 