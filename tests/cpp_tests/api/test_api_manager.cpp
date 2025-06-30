#include <gtest/gtest.h>
#include "satox/api/api_manager.hpp"
#include <fstream>
#include <sstream>

using namespace satox::api;

class APIManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary config file
        std::ofstream configFile("test_config.json");
        configFile << R"({
            "rest": {
                "host": "localhost",
                "port": 8080
            },
            "websocket": {
                "host": "localhost",
                "port": 8081
            },
            "graphql": {
                "host": "localhost",
                "port": 8082
            }
        })";
        configFile.close();
    }

    void TearDown() override {
        if (manager.isInitialized()) {
            manager.shutdown();
        }
        std::remove("test_config.json");
    }

    APIManager& manager = APIManager::getInstance();
};

TEST_F(APIManagerTest, Initialization) {
    EXPECT_TRUE(manager.initialize("test_config.json"));
    EXPECT_TRUE(manager.isInitialized());
    EXPECT_EQ(manager.getConfigPath(), "test_config.json");
}

TEST_F(APIManagerTest, DoubleInitialization) {
    EXPECT_TRUE(manager.initialize("test_config.json"));
    EXPECT_TRUE(manager.initialize("test_config.json")); // Should return true but not reinitialize
}

TEST_F(APIManagerTest, Shutdown) {
    EXPECT_TRUE(manager.initialize("test_config.json"));
    EXPECT_TRUE(manager.shutdown());
    EXPECT_FALSE(manager.isInitialized());
}

TEST_F(APIManagerTest, RegisterEndpoint) {
    ASSERT_TRUE(manager.initialize("test_config.json"));

    bool handlerCalled = false;
    auto handler = [&handlerCalled](const crow::request& req, crow::response& res) {
        handlerCalled = true;
        res = "test response";
    };

    EXPECT_TRUE(manager.registerEndpoint("/test", "GET", handler));
}

TEST_F(APIManagerTest, RegisterWebSocketHandler) {
    ASSERT_TRUE(manager.initialize("test_config.json"));

    bool handlerCalled = false;
    auto handler = [&handlerCalled](const std::string& message) {
        handlerCalled = true;
    };

    EXPECT_TRUE(manager.registerWebSocketHandler("/ws", handler));
}

TEST_F(APIManagerTest, RegisterGraphQLResolver) {
    ASSERT_TRUE(manager.initialize("test_config.json"));

    bool resolverCalled = false;
    auto resolver = [&resolverCalled](const nlohmann::json& args) -> nlohmann::json {
        resolverCalled = true;
        return {{"result", "test"}};
    };

    EXPECT_TRUE(manager.registerGraphQLResolver("Query", "test", resolver));
}

TEST_F(APIManagerTest, RegisterGraphQLMutation) {
    ASSERT_TRUE(manager.initialize("test_config.json"));

    bool mutationCalled = false;
    auto mutation = [&mutationCalled](const nlohmann::json& args) -> nlohmann::json {
        mutationCalled = true;
        return {{"result", "test"}};
    };

    EXPECT_TRUE(manager.registerGraphQLMutation("testMutation", mutation));
}

TEST_F(APIManagerTest, RegisterGraphQLSubscription) {
    ASSERT_TRUE(manager.initialize("test_config.json"));

    bool subscriptionCalled = false;
    auto subscription = [&subscriptionCalled](const std::string& message) {
        subscriptionCalled = true;
    };

    EXPECT_TRUE(manager.registerGraphQLSubscription("testSubscription", subscription));
}

TEST_F(APIManagerTest, StartStop) {
    ASSERT_TRUE(manager.initialize("test_config.json"));

    EXPECT_TRUE(manager.start());
    EXPECT_TRUE(manager.stop());
}

TEST_F(APIManagerTest, InvalidConfig) {
    EXPECT_FALSE(manager.initialize("nonexistent_config.json"));
    EXPECT_FALSE(manager.isInitialized());
}

TEST_F(APIManagerTest, OperationsBeforeInitialization) {
    EXPECT_FALSE(manager.registerEndpoint("/test", "GET", [](const crow::request&, crow::response&) {}));
    EXPECT_FALSE(manager.registerWebSocketHandler("/ws", [](const std::string&) {}));
    EXPECT_FALSE(manager.registerGraphQLResolver("Query", "test", [](const nlohmann::json&) { return nlohmann::json(); }));
    EXPECT_FALSE(manager.registerGraphQLMutation("test", [](const nlohmann::json&) { return nlohmann::json(); }));
    EXPECT_FALSE(manager.registerGraphQLSubscription("test", [](const std::string&) {}));
    EXPECT_FALSE(manager.start());
    EXPECT_TRUE(manager.stop()); // Should return true when not initialized
}

TEST_F(APIManagerTest, InvalidConfigFormat) {
    // Create an invalid config file
    std::ofstream configFile("invalid_config.json");
    configFile << "invalid json";
    configFile.close();

    EXPECT_FALSE(manager.initialize("invalid_config.json"));
    EXPECT_FALSE(manager.isInitialized());

    std::remove("invalid_config.json");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 