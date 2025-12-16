#include "http_handler.h"

#include <gtest/gtest.h>
#include <httplib.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

class HttpHandlerTest : public ::testing::Test {
protected:
    int port = 19090;
    std::shared_ptr<std::atomic<bool>> ready_flag = std::make_shared<std::atomic<bool>>(true);
    std::unique_ptr<HttpHandler> handler;

    void SetUp() override {
        handler = std::make_unique<HttpHandler>(port, [ready_flag = ready_flag]() {
            return ready_flag->load();
        });
        handler->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    void TearDown() override {
        if (handler) {
            handler->stop();
        }
    }
};

TEST_F(HttpHandlerTest, HealthPingReturnsOk) {
    httplib::Client client("localhost", port);
    auto response = client.Get("/health/ping");
    ASSERT_TRUE(response);
    EXPECT_EQ(response->status, 200);
    EXPECT_NE(response->body.find("metrics-service"), std::string::npos);
}

TEST_F(HttpHandlerTest, ReadyEndpointReflectsDatabaseStatus) {
    httplib::Client client("localhost", port);

    ready_flag->store(true);
    auto ready_response = client.Get("/health/ready");
    ASSERT_TRUE(ready_response);
    EXPECT_EQ(ready_response->status, 200);

    ready_flag->store(false);
    auto not_ready = client.Get("/health/ready");
    ASSERT_TRUE(not_ready);
    EXPECT_EQ(not_ready->status, 503);
}

TEST_F(HttpHandlerTest, SimplePingEndpointResponds) {
    httplib::Client client("localhost", port);
    auto response = client.Get("/ping");
    ASSERT_TRUE(response);
    EXPECT_EQ(response->status, 200);
    EXPECT_EQ(response->body, "pong");
}
