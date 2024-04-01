#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <regex>
#include "./../src/tcp_messages.cpp"  



class MessageParserTest : public ::testing::Test {
protected:
    TcpMessages::Message_t preparedMessage;
    TcpMessages::Message_t preparedMessage2;
    TcpMessages::Message_t preparedMessage3;

    void SetUp() override {
        std::string testMessage = "MSG FROM John IS Hello, world!\r\n";
        std::string emptyMessage = "MSG FROM John Travolta IS \r\n";
        std::string longMessage = "MSG FROM John Travolta IS Hey There Lets Have a Group Message Together!\r\n";

        // Directly use the string's begin and end iterators to initialize the vector.
        preparedMessage.content = std::vector<char>(testMessage.begin(), testMessage.end());
        preparedMessage2.content = std::vector<char>(emptyMessage.begin(), emptyMessage.end());
        preparedMessage3.content = std::vector<char>(longMessage.begin(), longMessage.end());
    }
};

TEST_F(MessageParserTest, HandlesMsgFromCorrectly) {
    TcpMessages msg(TcpMessages::UNKNOWN_MSG_TYPE, preparedMessage);
    msg.parseMessage();
    // Validate displayNameOutside and content after parsing
    std::string expectedDisplayName = "John";
    std::string expectedContent = "Hello, world!";
    EXPECT_EQ(std::string(msg.msg.displayNameOutside.begin(), msg.msg.displayNameOutside.end()), expectedDisplayName);
    EXPECT_EQ(std::string(msg.msg.content.begin(), msg.msg.content.end()), expectedContent);
}

TEST_F(MessageParserTest, HandlesEmptyMsg) {
    TcpMessages msg(TcpMessages::UNKNOWN_MSG_TYPE, preparedMessage2);
    msg.parseMessage();
    // Validate displayNameOutside and content after parsing
    std::string expectedDisplayName = "John Travolta";
    std::string expectedContent = "";
    EXPECT_EQ(std::string(msg.msg.displayNameOutside.begin(), msg.msg.displayNameOutside.end()), expectedDisplayName);
    EXPECT_EQ(std::string(msg.msg.content.begin(), msg.msg.content.end()), expectedContent);
}

TEST_F(MessageParserTest, HandlesLongMsg) {
    TcpMessages msg(TcpMessages::UNKNOWN_MSG_TYPE, preparedMessage3);
    msg.parseMessage();
    // Validate displayNameOutside and content after parsing
    std::string expectedDisplayName = "John Travolta";
    std::string expectedContent = "Hey There Lets Have a Group Message Together!";
    EXPECT_EQ(std::string(msg.msg.displayNameOutside.begin(), msg.msg.displayNameOutside.end()), expectedDisplayName);
    EXPECT_EQ(std::string(msg.msg.content.begin(), msg.msg.content.end()), expectedContent);
}



// Add more tests as needed for different scenarios

