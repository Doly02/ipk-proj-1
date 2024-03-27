/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      base_messages.cpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Implements Serialization & Deserialization of Messages For TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           base_messages.cpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Implements Serialization & Deserialization of Messages For TCP Protocol.
 * ****************************/

#ifndef BASE_MESSAGES_HPP
#define BASE_MESSAGES_HPP

#include "../include/macros.hpp"
#include "strings.hpp"
#include <string>
#include <vector>
#include <cstdint>

class BaseMessages {
public:
    enum MessageType_t : uint8_t {
        CONFIRM = 0x00,
        REPLY = 0x01,
        COMMAND_AUTH = 0x02,
        COMMAND_JOIN = 0x03,
        MSG = 0x04,
        COMMAND_HELP = 0x05,
        ERROR = 0xFE,
        COMMAND_BYE = 0xFF,
        UNKNOWN_MSG_TYPE = 0x99,
    };

    enum InputType_t {
        INPUT_UNKNOWN,
        INPUT_AUTH,
        INPUT_JOIN,
        INPUT_RENAME,
        INPUT_MSG,
        INPUT_HELP
    };

    struct Message_t {
        MessageType_t type;
        std::vector<char> content;
        bool isCommand;
        std::vector<char> login;
        std::vector<char> secret;
        std::vector<char> displayName;
        std::vector<char> channelID;
        std::vector<char> displayNameOutside;
        std::vector<char> buffer;
        bool shouldReply;
    };

    MessageType_t msgType;
    Message_t msg;

    BaseMessages();
    BaseMessages(MessageType_t type, Message_t content);
    ~BaseMessages();
    
    void insertErrorMsgToContent(const std::string& inputString);
    void cleanMessage();
    int checkLength();
    void readAndStoreContent(const char* buffer);
    void readAndStoreBytes(const char* buffer, size_t bytesRx);
    int checkMessage();
    int parseMessage();
    int handleReply();
    void printMessage();
    void PrintServeReply();
    void basePrintExternalError();
    void basePrintInternalError(int retVal);
    void printHelp();
};

#endif // BASE_MESSAGES_HPP
