#include <gtest/gtest.h>
#include "satox/network/network_manager.hpp"
#include "satox/network/letsencrypt_manager.hpp"

namespace satox {
namespace tests {

class NetworkManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        networkManager = std::make_unique<network::NetworkManager>("tcp", 8080);
        letsEncryptManager = std::make_unique<network::LetsEncryptManager>();
    }

    void TearDown() override {
        networkManager.reset();
        letsEncryptManager.reset();
    }

    std::unique_ptr<network::NetworkManager> networkManager;
    std::unique_ptr<network::LetsEncryptManager> letsEncryptManager;
};

TEST_F(NetworkManagerTest, InitializationTest) {
    EXPECT_TRUE(networkManager->initialize());
}

TEST_F(NetworkManagerTest, LetsEncryptIntegrationTest) {
    network::LetsEncryptManager::Config config;
    config.domain = "test.example.com";
    config.email = "test@example.com";
    config.webroot_path = "/tmp/webroot";
    config.cert_path = "/tmp/certs";
    
    EXPECT_TRUE(letsEncryptManager->initialize(config));
    EXPECT_TRUE(networkManager->initialize());
}

} // namespace tests
} // namespace satox 