/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      test_readAndStore.cpp
 *  Author:         Tomas Dolak
 *  Date:           26.02.2024
 *  Description:    Unit Tests For readAndStore Method.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           test_readAndStore.cpp
 *  @author         Tomas Dolak
 *  @date           26.02.2024
 *  @brief          Unit Tests For readAndStore Method.
 * ****************************/

#include <gtest/gtest.h>
#include "../src/tcp_messages.cpp" 

// Test Fixture
class TcpMessagesTest : public ::testing::Test {
protected:
    TcpMessages* tcpMessages;

    void SetUp() override {
        tcpMessages = new TcpMessages();
    }

    void TearDown() override {
        delete tcpMessages;
    }
};

/**
* @brief Test to verify that the readAndStoreContent method correctly reads and stores a simple string
*/
TEST_F(TcpMessagesTest, StoresSimpleStringCorrectly) {
    const char* testString = "Hello, World!";
    tcpMessages->readAndStoreContent(testString);

    std::string expectedContent(testString);
    std::string actualContent(tcpMessages->msg.content.begin(), tcpMessages->msg.content.end());

    EXPECT_EQ(expectedContent, actualContent);
}

/**
* @brief Test that the readAndStoreContent method correctly reads and stores an empty string.
*/
TEST_F(TcpMessagesTest, HandlesEmptyStringCorrectly) {
    const char* testString = "";
    tcpMessages->readAndStoreContent(testString);

    EXPECT_TRUE(tcpMessages->msg.content.empty());
}

/**
* @brief Test that the readAndStoreContent method correctly reads and stores a string containing special characters.
*/
TEST_F(TcpMessagesTest, StoresStringWithSpecialCharactersCorrectly) {
    const char* testString = "Hello, \nWorld!\r\n";
    tcpMessages->readAndStoreContent(testString);

    std::string expectedContent(testString);
    std::string actualContent(tcpMessages->msg.content.begin(), tcpMessages->msg.content.end());

    EXPECT_EQ(expectedContent, actualContent);
}

/**
* @brief Test that the readAndStoreContent method correctly reads and stores a string containing special characters
*       two time in the row.
*/
TEST_F(TcpMessagesTest, StoresStringWithSpecialCharactersCorrectly2) {
    const char* testString = "Hello, \nWorld!\r\n";
    tcpMessages->readAndStoreContent(testString);

    std::string expectedContent(testString);
    std::string actualContent(tcpMessages->msg.content.begin(), tcpMessages->msg.content.end());

    EXPECT_EQ(expectedContent, actualContent);

    const char* testString2 = "/auth Login Secret Display Name\r\n";
    std::string expectedContent2(testString2);
    tcpMessages->readAndStoreContent(testString2);
    std::string actualContent2(tcpMessages->msg.content.begin(), tcpMessages->msg.content.end());
    EXPECT_EQ(expectedContent2, actualContent2);

}