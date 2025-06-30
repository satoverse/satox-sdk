#include <gtest/gtest.h>
#include "satox/api/graphql_api.hpp"
#include <fstream>
#include <sstream>

namespace satox {
namespace api {
namespace test {

class GraphQLAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary configuration file
        std::ofstream configFile("test_config.json");
        configFile << R"({
            "host": "localhost",
            "port": 8080
        })";
        configFile.close();
    }

    void TearDown() override {
        // Clean up the temporary configuration file
        std::remove("test_config.json");
    }
};

TEST_F(GraphQLAPITest, Initialization) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));
    EXPECT_TRUE(api.isInitialized());
    EXPECT_EQ(api.getHost(), "localhost");
    EXPECT_EQ(api.getPort(), 8080);
}

TEST_F(GraphQLAPITest, DoubleInitialization) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));
    EXPECT_FALSE(api.initialize("test_config.json"));
}

TEST_F(GraphQLAPITest, InvalidConfig) {
    GraphQLAPI api;
    EXPECT_FALSE(api.initialize("nonexistent_config.json"));
}

TEST_F(GraphQLAPITest, RegisterResolver) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    auto resolver = [](const nlohmann::json& args) -> nlohmann::json {
        return {{"value", "test"}};
    };

    EXPECT_TRUE(api.registerResolver("Query", "test", resolver));
}

TEST_F(GraphQLAPITest, RegisterMutation) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    auto mutation = [](const nlohmann::json& args) -> nlohmann::json {
        return {{"success", true}};
    };

    EXPECT_TRUE(api.registerMutation("testMutation", mutation));
}

TEST_F(GraphQLAPITest, RegisterSubscription) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    auto subscription = [](const std::string& data) {
        // Subscription handler
    };

    EXPECT_TRUE(api.registerSubscription("testSubscription", subscription));
}

TEST_F(GraphQLAPITest, ExecuteQuery) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    // Register a test resolver
    auto resolver = [](const nlohmann::json& args) -> nlohmann::json {
        return {{"value", "test"}};
    };
    api.registerResolver("Query", "test", resolver);

    // Execute a test query
    std::string query = R"({
        test {
            value
        }
    })";

    auto result = api.executeQuery(query, nlohmann::json::object());
    EXPECT_TRUE(result.contains("data"));
    EXPECT_TRUE(result["data"].contains("test"));
    EXPECT_EQ(result["data"]["test"]["value"], "test");
}

TEST_F(GraphQLAPITest, ExecuteMutation) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    // Register a test mutation
    auto mutation = [](const nlohmann::json& args) -> nlohmann::json {
        return {{"success", true}};
    };
    api.registerMutation("testMutation", mutation);

    // Execute a test mutation
    std::string query = R"(mutation {
        testMutation {
            success
        }
    })";

    auto result = api.executeQuery(query, nlohmann::json::object());
    EXPECT_TRUE(result.contains("data"));
    EXPECT_TRUE(result["data"].contains("testMutation"));
    EXPECT_TRUE(result["data"]["testMutation"]["success"]);
}

TEST_F(GraphQLAPITest, ExecuteSubscription) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    // Register a test subscription
    auto subscription = [](const std::string& data) {
        // Subscription handler
    };
    api.registerSubscription("testSubscription", subscription);

    // Execute a test subscription
    std::string query = R"(subscription {
        testSubscription {
            status
        }
    })";

    auto result = api.executeQuery(query, nlohmann::json::object());
    EXPECT_TRUE(result.contains("data"));
    EXPECT_TRUE(result["data"].contains("testSubscription"));
    EXPECT_EQ(result["data"]["testSubscription"]["status"], "subscribed");
}

TEST_F(GraphQLAPITest, InvalidQuery) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    // Execute an invalid query
    std::string query = "invalid query";
    auto result = api.executeQuery(query, nlohmann::json::object());
    EXPECT_TRUE(result.contains("errors"));
}

TEST_F(GraphQLAPITest, StartStop) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    EXPECT_TRUE(api.start());
    api.stop();
}

TEST_F(GraphQLAPITest, GetStatus) {
    GraphQLAPI api;
    EXPECT_TRUE(api.initialize("test_config.json"));

    auto status = api.getStatus();
    EXPECT_TRUE(status.contains("initialized"));
    EXPECT_TRUE(status.contains("host"));
    EXPECT_TRUE(status.contains("port"));
    EXPECT_TRUE(status.contains("resolvers"));
    EXPECT_TRUE(status.contains("mutations"));
    EXPECT_TRUE(status.contains("subscriptions"));
}

} // namespace test
} // namespace api
} // namespace satox 