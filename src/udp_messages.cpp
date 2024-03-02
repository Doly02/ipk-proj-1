/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      udp_messages.cpp
 *  Author:         Tomas Dolak
 *  Date:           02.03.2024
 *  Description:    Implements Functions That Handles Processing Of UDP Messages.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           udp_messages.cpp
 *  @author         Tomas Dolak
 *  @date           02.03.2024
 *  @brief          Implements Functions That Handles Processing Of UDP Messages.
 * ****************************/


/************************************************/
/*                  Libraries                   */
/************************************************/
#include <iostream>
#include <string>
#include <unistd.h>     // For close
#include <unordered_set>
#include <chrono>
#include "base_messages.cpp"
/************************************************/
/*                  Constants                   */
/************************************************/
class UdpMessages : public BaseMessages {
private:
    static constexpr int8_t NULL_BYTE   = 0x00;
    static constexpr int8_t CONFIRM     = 0x00;
    static constexpr int8_t REPLY       = 0x01;
    static constexpr int8_t AUTH        = 0x02;
    static constexpr int8_t JOIN        = 0x03;
    static constexpr int8_t MSG         = 0x04;
    static constexpr int8_t ERR         = 0xFE;
    static constexpr int8_t BYE         = 0xFF;

public:
    struct UdpMessage
    {
        uint8_t type;           //!< Type of The Message
        uint16_t messageID;     //!< ID of The Message
        uint16_t refMessageID;  //!< ID of The Referenced Message
        uint8_t result;     //!< Result of The Message
        Message_t msgContent;      //!< Content of The Message
    };

    UdpMessages() : BaseMessages() {}

    UdpMessages(MessageType_t type, const Message_t& content) : BaseMessages(type, content) {}



    void appendContent(std::vector<uint8_t>& serialized, const std::vector<char>& contentBuffer) {
        // Serialize The Message Content To UDP Message 
        for (const auto& byte : contentBuffer) 
        {
            serialized.push_back(static_cast<uint8_t>(byte));
        }
        serialized.push_back(NULL_BYTE);
    }

    /**
     * @brief Serialize The Message To Byte Array
     * @param message Message To Serialize
     * 
     * Serialize The Message To Byte Array
     * @return Byte Array
    */
    std::vector<uint8_t> serializeMessage(const UdpMessage& message) {
        std::vector<uint8_t> serialized;

        /*  MESSAGE TYPE */
        serialized.push_back(message.type);
        /*  MESSAGE ID   */
        serialized.push_back(message.messageID & 0xFF);
        serialized.push_back((message.messageID >> 8) & 0xFF);

        switch (message.type)
        {
            
            case REPLY:
                /*  RESULT      */
                serialized.push_back(message.result);
                /*  REF. MESSAGE ID */
                serialized.push_back(message.refMessageID & 0xFF);
                serialized.push_back((message.refMessageID >> 8) & 0xFF);
                /*  MESSAGE CONTENT */
                appendContent(serialized, message.msgContent.content);
                break;
            case AUTH:
                /*  USERNAME        */
                appendContent(serialized, message.msgContent.login);
                /*  DISPLAY NAME    */
                appendContent(serialized, message.msgContent.displayName);
                /*  SECRET          */
                appendContent(serialized, message.msgContent.secret);
                break;
            case JOIN:
                /*  CHANNEL ID      */
                appendContent(serialized, message.msgContent.channelID);
                /*  DISPLAY NAME    */
                appendContent(serialized, message.msgContent.displayName);
                break;
            case MSG:
                /*  DISPLAY NAME    */
                appendContent(serialized, message.msgContent.displayName);
                /*  MESSAGE CONTENT */
                appendContent(serialized, message.msgContent.content);
                break;
            case ERR:
                /*  DISPLAY NAME    */
                appendContent(serialized, message.msgContent.displayName);
                /*  MESSAGE CONTENT */
                appendContent(serialized, message.msgContent.content);
                break;
            case BYE:
                /* MSG TYPE & IN ALREADY THERE */
                break;
        }
        return serialized;
    }

    
};

