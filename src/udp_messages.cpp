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
#include <unistd.h>             // For close
#include <unordered_set>
#include <chrono>
#include <thread>
#include "base_messages.cpp"
#include <netinet/in.h>         // For sockaddr_in, AF_INET, SOCK_DGRAM
#include <arpa/inet.h>          // For Debug
/************************************************/
/*                  Constants                   */
/************************************************/
class UdpMessages : public BaseMessages {
private:
    static constexpr int8_t NULL_BYTE           = 0x00;

public:
    static constexpr int8_t CONFIRM_FAILED      = 0x55;
    static constexpr int    OUT_OF_TIMEOUT      = 0x77;

    uint16_t messageID;         //!< ID of The Message
    uint16_t refMessageID;      //!< ID of The Referenced Message
    uint8_t  result;            //!< Result of The Message
    uint16_t internalMsgId;

    UdpMessages() : BaseMessages() {}

    UdpMessages(MessageType_t type, Message_t content) : BaseMessages(type, content) 
    {
        refMessageID = 1;
        result = 0;
        messageID = 1;
    }

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
    std::vector<uint8_t> serializeMessage() {
        std::vector<uint8_t> serialized;

        /*  MESSAGE TYPE */
        serialized.push_back(msg.type);
        /*  MESSAGE ID   */
        serialized.push_back(messageID & 0xFF);
        serialized.push_back((messageID >> 8) & 0xFF);

        switch (msg.type)
        {
            
            case REPLY:
                /*  RESULT      */
                serialized.push_back(result);
                /*  REF. MESSAGE ID */
                serialized.push_back(refMessageID & 0xFF);
                serialized.push_back((refMessageID >> 8) & 0xFF);
                /*  MESSAGE CONTENT */
                appendContent(serialized, msg.content);
                break;
            case COMMAND_AUTH:
                /*  USERNAME        */
                appendContent(serialized, msg.login);
                /*  DISPLAY NAME    */
                appendContent(serialized, msg.displayName);
                /*  SECRET          */
                appendContent(serialized, msg.secret);
                break;
            case COMMAND_JOIN:
                /*  CHANNEL ID      */
                appendContent(serialized, msg.channelID);
                /*  DISPLAY NAME    */
                appendContent(serialized, msg.displayName);
                break;
            case MSG:
                /*  DISPLAY NAME    */
                appendContent(serialized, msg.displayName);
                /*  MESSAGE CONTENT */
                appendContent(serialized, msg.content);
                break;
            case ERROR:
                /*  DISPLAY NAME    */
                appendContent(serialized, msg.displayName);
                /*  MESSAGE CONTENT */
                appendContent(serialized, msg.content);
                break;
            case COMMAND_BYE:
                /* MSG TYPE & IN ALREADY THERE */
                break;
            case CONFIRM:
                break;
            case UNKNOWN_MSG_TYPE:
                exit(1);
        }
        std::string login(msg.login.begin(),msg.login.end());
        std::string display(msg.displayName.begin(),msg.displayName.end());
        std::string secret(msg.secret.begin(),msg.secret.end());
        printf("MSG_ID: %d,LOG: %s,DISPLAY: %s,SECRET: %s\n",messageID,login.c_str(),display.c_str(),secret.c_str());
        return serialized;
    }


    /**
     * @brief Deserialize Byte Array To Message
     * @param serialized Byte Array
     * 
     * Deserialize Byte Array To Message
     * @return Message
    */
    void deserializeMessage(const std::vector<char>& serializedMsg)
    {
        printf("RECEIVED MESSAGE SIZE: %zu\n",serializedMsg.size());
        if (serializedMsg.size() < 3)
        {
            throw std::runtime_error("Invalid Message Length");
        }

        // Store Packet Into The UDP Message Struct
        msgType = (BaseMessages::MessageType_t)serializedMsg[0];
        if (CONFIRM != msgType)
            messageID = static_cast<uint16_t>(serializedMsg[1]) | (static_cast<uint16_t>(serializedMsg[2]) << 8);
        else
            refMessageID = static_cast<uint16_t>(serializedMsg[1]) | (static_cast<uint16_t>(serializedMsg[2]) << 8);

        size_t offset = 3;

        switch (msgType)
        {
            case CONFIRM:
                break;
            case REPLY:
                if (serializedMsg.size() < offset + 3)
                {
                    throw std::runtime_error("Invalid Message Length");
                }
                result = serializedMsg[offset++];
                refMessageID = static_cast<uint16_t>(serializedMsg[offset]) | (static_cast<uint16_t>(serializedMsg[offset + 1]) << 8);
                offset = offset + 2; // Used Two Bytes
                
                while (offset < serializedMsg.size() && serializedMsg[offset] != NULL_BYTE)
                {
                    msg.content.push_back(static_cast<char>(serializedMsg[offset]));
                    offset++;
                }
                msg.type = REPLY;
                break;
            case COMMAND_AUTH: /* Should Not Be Sended To Client */
            case COMMAND_JOIN: /* Should Not Be Sended To Client */
                printf("Invalid Message Type\n"); //TODO:
                break;
            case MSG: 
            case ERROR:
                /* DISPLAY NAME */
                while (offset < serializedMsg.size() && serializedMsg[offset] != NULL_BYTE)
                {
                    msg.displayNameOutside.push_back(static_cast<char>(serializedMsg[offset]));
                    offset++;
                }
                /* MESSAGE CONTENT */
                while (offset < serializedMsg.size() && serializedMsg[offset] != NULL_BYTE)
                {
                    msg.content.push_back(static_cast<char>(serializedMsg[offset]));
                    offset++;
                }
                break;
            case COMMAND_BYE:
                break;
            case UNKNOWN_MSG_TYPE:  /* Unused */
            default:
                exit(1);
            
        
        }
        msg.type = msgType;

    }

    int RecvUdpMessage(int internalId)
    {
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        cleanMessage();
        deserializeMessage(serialized);
        if (refMessageID == internalId && result == SUCCESS)
        {
            // Check With Global Message ID
            if (refMessageID == messageID)
            {
                messageID++;
                return SUCCESS;
            }
        }
        return MSG_FAILED;
    }

    int recvUpdIncomingReply(int internalId)
    {
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        cleanMessage();
        deserializeMessage(serialized);
        if (REPLY == msg.type)
        {
            printf("recvUpdIncomingReply -> REPLY MESSAGE\n");
            // Check With Internal Message ID
            printf("recvUpdIncomingReply -> refMessageID: %d, result: %d\n",refMessageID,result);
            if (refMessageID == internalId && result == SUCCESS)
            {
                printf("recvUpdIncomingReply -> refMessageID == internalId\n");
                // Check With Global Message ID
                if (refMessageID == messageID)
                {
                    printf("recvUpdIncomingReply -> refMessageID == messageID\n");
                    return SUCCESS;
                }
            }
        }
        return AUTH_FAILED;

    }


    void sendUdpAuthMessage(int sock,const struct sockaddr_in& server)
    {
        std::vector<uint8_t> serialized = serializeMessage();
    
        ssize_t bytesTx = sendto(sock, serialized.data(), serialized.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
    }

    int recvUpdConfirm(int internalId)
    {
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        cleanMessage();
        deserializeMessage(serialized);
        if (CONFIRM == msg.type)
        {
            printf("recvUdpConfirm -> CONFIRM MESSAGE\n");
            // Check With Internal Message ID
            if (refMessageID == internalId)
            {
                printf("recvUdpConfirm -> refMessageID == internalId\n");
                // Check With Global Message ID
                if (refMessageID == messageID)
                {
                    printf("recvUdpConfirm -> refMessageID == messageID\n");
                    return SUCCESS;
                }
            }
        }
        printf("recvUdpConfirm -> CONFIRM_FAILED\n");
        return CONFIRM_FAILED;
    }

    void SendUdpMessage(int sock)
    {
        std::vector<uint8_t> serialized = serializeMessage();
        ssize_t bytesTx = send(sock, serialized.data(), serialized.size(), 0);
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
    }

    void SendUdpConfirm(int sock, int internalId)
    {
        std::vector<uint8_t> serialized;

        /*  MESSAGE TYPE */
        serialized.push_back(CONFIRM);
        /*  MESSAGE ID   */
        serialized.push_back(internalId & 0xFF);
        serialized.push_back((internalId >> 8) & 0xFF);
        ssize_t bytesTx = send(sock, serialized.data(), serialized.size(), 0);
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
    }


    int CheckTimer(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        return duration.count();
    }

    void IncrementUdpMsgId()
    {
        messageID++;
    }

    void SetUdpMsgId()
    {
        messageID = 1;
    }

};


