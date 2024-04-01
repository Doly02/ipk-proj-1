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

class TcpMessagesTest_Join : public ::testing::Test {
protected:
    TcpMessages::Message_t preparedMessage;

    void SetUp() override {
        //TODO: Look If The Command Is Ended By '\n'
        std::vector<char> content = {'/', 'j', 'o', 'i', 'n', ' ', 'c', 'h', 'a', 'n', 'n', 'e', 'l', '1', '\n'};
        preparedMessage.content = content;
    }
};

TEST_F(TcpMessagesTest_Join, HandlesJoinCommandCorrectly) {
    // SetUp Message Type
    TcpMessages::MessageType_t type = TcpMessages::UNKNOWN_MSG_TYPE;
    // Create Instance Of TcpMessages With Prepaired Message 
    TcpMessages msg(type, preparedMessage);

    // Run method checkMessage
    int result = msg.checkMessage();

    // Check If The Method Returned 0
    EXPECT_EQ(result, 0); 
 

    // Convert vectors to strings
    std::string channelStr(msg.msg.channelID.begin(), msg.msg.channelID.end());

    // Test Extraction Of The Command Values 
    EXPECT_EQ(channelStr, "channel1");
    EXPECT_EQ(preparedMessage.type, TcpMessages::COMMAND_JOIN);
}