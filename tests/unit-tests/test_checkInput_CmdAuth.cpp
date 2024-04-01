/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      test_checkInput_CmdAuth
 *  Author:         Tomas Dolak
 *  Date:           25.02.2024
 *  Description:    Test For Processing Authentication Command.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp_messages.cpp
 *  @author         Tomas Dolak
 *  @date           25.02.2024
 *  @brief          Test For Processing Authentication Command.
 * ****************************/

#include "./../src/tcp_messages.cpp" 
#include <gtest/gtest.h>

class TcpMessagesTest_Auth : public ::testing::Test {
protected:
    TcpMessages::Message_t preparedMessage;

    void SetUp() override {
        //TODO: Look If The Command Is Ended By '\n'
        std::vector<char> content = {'/', 'a', 'u', 't', 'h', ' ', 'u', 's', 'e', 'r', ' ', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', ' ', 'D', 'i', 's', 'p', 'l', 'a', 'y', ' ', 'N', 'a', 'm', 'e', '\n'};
        preparedMessage.content = content;
    }
};

TEST_F(TcpMessagesTest_Auth, HandlesAuthCommandCorrectly) {
    // SetUp Message Type
    TcpMessages::MessageType_t type = TcpMessages::UNKNOWN_MSG_TYPE;
    // Create Instance Of TcpMessages With Prepaired Message 
    TcpMessages msg(type, preparedMessage);

    // Run method checkMessage
    int result = msg.checkMessage();

    // Check If The Method Returned 0
    EXPECT_EQ(result, 0); 
 

    // Convert vectors to strings
    std::string loginStr(msg.msg.login.begin(), msg.msg.login.end());
    std::string secretStr(msg.msg.secret.begin(), msg.msg.secret.end());
    std::string displayNameStr(msg.msg.displayName.begin(), msg.msg.displayName.end());


    // Test Extraction Of The Command Values 
    EXPECT_EQ(loginStr, "user");
    EXPECT_EQ(secretStr, "password");
    EXPECT_EQ(displayNameStr, "Display Name\n");
    EXPECT_EQ(preparedMessage.type, TcpMessages::COMMAND_AUTH);
}