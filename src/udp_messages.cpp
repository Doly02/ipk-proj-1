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

    UdpMessages(Message_t content) : BaseMessages(content) 
    {
        refMessageID = 1;
        result = 0;
        messageID = 1;
    }

    /*----------------------------------------------------------------------*/
    /*                          Supporting Methods                          */
    /*----------------------------------------------------------------------*/
    /**
     * @brief Sets the UDP Message ID.
     * 
     * Sets the UDP Message ID.
     */
    void SetUdpMsgId()
    {
        messageID = 1;
    }

    /**
     * @brief Increments UDP Message ID.
     * 
     * Increments UDP Message ID.
     */
    void IncrementUdpMsgId()
    {
        messageID++;
    }

    /**
     * @brief Returns Time Difference In Milliseconds Between Two Events.
     * 
     * @param startTime Start Event
     * @param endTime End Event
     * @return Time Differecnce In Milliseconds Between Two Events.
     * 
     * Returns Time Difference In Milliseconds Between Two Events.
     */
    int CheckTimer(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        return duration.count();
    }

    /**
     * @brief Appends Content To Serialized Message With NULL Byte At The End.
     * 
     * @param serialized Serialized Message
     * @param contentBuffer Source Buffer
     * Appends Content To Serialized Message With NULL Byte At The End.
     */
    void AppendContent(std::vector<uint8_t>& serialized, const std::vector<char>& sourceBuffer) 
    {
        // Serialize The Message Content To UDP Message 
        for (const auto& byte : sourceBuffer) 
        {
            serialized.push_back(static_cast<uint8_t>(byte));
        }
        serialized.push_back(NULL_BYTE);
    }

    /*----------------------------------------------------------------------*/
    /*                          Operational Functions                       */
    /*----------------------------------------------------------------------*/

    /**
     * @brief Serialize The Message To Byte Array
     * @param message Message To Serialize
     * 
     * Serialize The Message To Byte Array
     * @return Byte Array
    */
    std::vector<uint8_t> SerializeMessage() {
        std::vector<uint8_t> serialized;

        /*  MESSAGE TYPE */
        serialized.push_back(msg.type);
        /*  MESSAGE ID   */
        serialized.push_back(messageID & 0xFF);
        serialized.push_back((messageID >> 8) & 0xFF);
        printf("Sends: MSG ID: %d\n",messageID);

        switch (msg.type)
        {
            
            case REPLY:
                /*  RESULT      */
                serialized.push_back(result);
                /*  REF. MESSAGE ID */
                serialized.push_back(refMessageID & 0xFF);
                serialized.push_back((refMessageID >> 8) & 0xFF);
                printf("Ref.Sends: MSG ID: %d\n",refMessageID);
                /*  MESSAGE CONTENT */
                AppendContent(serialized, msg.content);
                break;
            case COMMAND_AUTH:
                /*  USERNAME        */
                AppendContent(serialized, msg.login);
                /*  DISPLAY NAME    */
                AppendContent(serialized, msg.displayName);
                /*  SECRET          */
                AppendContent(serialized, msg.secret);
                break;
            case COMMAND_JOIN:
                /*  CHANNEL ID      */
                AppendContent(serialized, msg.channelID);
                /*  DISPLAY NAME    */
                AppendContent(serialized, msg.displayName);
                break;
            case MSG:
                /*  DISPLAY NAME    */
                AppendContent(serialized, msg.displayName);
                /*  MESSAGE CONTENT */
                AppendContent(serialized, msg.content);
                break;
            case ERROR:
                /*  DISPLAY NAME    */
                AppendContent(serialized, msg.displayName);
                /*  MESSAGE CONTENT */
                AppendContent(serialized, msg.content);
                break;
            case COMMAND_BYE:
                /* MSG TYPE & IN ALREADY THERE */
                break;
            case CONFIRM:
            case COMMAND_HELP:
                break;
            case UNKNOWN_MSG_TYPE:
                exit(1);
        }
        return serialized;
    }


    /**
     * @brief Deserialize Byte Array To Message
     * @param serialized Byte Array
     * 
     * Deserialize Byte Array To Message
     * @return Message
    */
    int DeserializeMessage(const std::vector<char>& serializedMsg)
    {
        BaseMessages::MessageType_t typeOfMsg = BaseMessages::UNKNOWN_MSG_TYPE;


        if (serializedMsg.size() < 3)
        {
            std::cerr << "ERR: Invalid Message Length" << std::endl;
            return BaseMessages::EXTERNAL_ERROR;
        }

        // Store Packet Into The UDP Message Struct
        typeOfMsg = (BaseMessages::MessageType_t)serializedMsg[0];
        if (CONFIRM != msg.type)
            messageID = static_cast<uint16_t>(serializedMsg[1]) | (static_cast<uint16_t>(serializedMsg[2]) << 8);
        else
            refMessageID = static_cast<uint16_t>(serializedMsg[1]) | (static_cast<uint16_t>(serializedMsg[2]) << 8);

        size_t offset = 3;

        switch (typeOfMsg)
        {
            case CONFIRM:
                break;
            case REPLY:
                if (serializedMsg.size() < offset + 3)
                {
                    std::cerr << "ERR: Invalid Message Length" << std::endl;
                    return BaseMessages::EXTERNAL_ERROR;
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
                std::cerr << "ERR: Invalid Message Length" << std::endl;
                return BaseMessages::EXTERNAL_ERROR;
            
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
                exit(EXIT_FAILURE);
            
        
        }
        msg.type = typeOfMsg;
        return BaseMessages::SUCCESS;

    }

    /**
     * @brief Handles The Incommed UDP Message.
     * 
     * @param internalId Internally Stored Message ID.
     * @return If The Message Was Successfully Received Returns 'SUCCESS' (0) Otherwise Returns 'MSG_FAILED'.
     * 
     * Stores Message Parts (Type, ID, Result, Ref. ID, Content) Into The Message Struct And Checks The Message IDs.
     * @warning In The Begging Clears The Message Struct.
     */
    int RecvUdpMessage(int internalId)
    {
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        CleanMessage();
        DeserializeMessage(serialized);
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


    /**
     * @brief Handles The Incoming UDP 'Reply' Message.
     * 
     * @param internalId Internally Stored Message ID.
     * @return If The Message Was Successfully Received Returns 'SUCCESS' (0) Otherwise Returns 'AUTH_FAILED'.
     * 
     * Stores Message Parts (Type, ID, Result, Ref. ID) Into The Message Struct And Checks The Message IDs.
     * @warning In The Begging Clears The Message Struct. 
     */
    int recvUpdIncomingReply(int internalId)
    {
        int retVal = EXTERNAL_ERROR;
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        
        CleanMessage();
        retVal = DeserializeMessage(serialized);
        if (SUCCESS != retVal)
            return retVal;

        if (REPLY == msg.type)
        {
            printf("recvUpdIncomingReply -> REPLY MESSAGE, refMessageID=%d,internalId=%d,result = %d\n",refMessageID,internalId,result);
            // Check With Internal Message ID
            if (refMessageID == internalId && result == SUCCESS)
            {
                printf("recvUpdIncomingReply -> refMessageID == internalId && result == SUCCESS\n");
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

    /**
     * @brief Handles The Incoming UDP 'Confirm' Message.
     * 
     * @param internalId Internally Stored Message ID.
     * @return If The Message Was Successfully Received Returns 'SUCCESS' (0) Otherwise Returns 'CONFIRM_FAILED'.
     * 
     * Stores Message Parts (Type, ID, Result) Into The Message Struct And Checks The Message IDs.
     * @warning In The Begging Clears The Message Struct. 
     */
    int recvUpdConfirm(int internalId)
    {
        int retVal = EXTERNAL_ERROR;
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        
        CleanMessage();
        retVal = DeserializeMessage(serialized);
        if (SUCCESS != retVal)
            return retVal;

        if (CONFIRM == msg.type)
        {
            printf("recvUpdConfirm -> CONFIRM MESSAGE, refMessageID=%d,internalId=%d,result = %d\n",refMessageID,internalId,result);
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
                printf("recvUdpConfirm -> CONFIRM_FAILED\n");
            }
        }
        else if (ERROR == msg.type)
        {
            if (messageID == internalId) // TODO: Schvalne co to udela
            {
                //Print Error On STDOUT
                std::string errDisplayName(msg.displayNameOutside.begin(),msg.displayNameOutside.end());
                std::string errContext(msg.content.begin(),msg.content.end());

                fprintf(stderr,"ERR FROM %s: %s\n",errDisplayName.c_str(),errContext.c_str());                
                return FAIL; //TODO: 
            }
        }
        return CONFIRM_FAILED;
    }

    /**
     * @brief Serializes And Sends UDP Message (Except 'Confirm' Message)
     * 
     * @param sock Socket Where To Send The Message
     * @param server Server Where To Send The Message
     * 
     * Sends UDP Message (Except 'Confirm' Message)
     */
    void SendUdpMessage(int sock,const struct sockaddr_in& server)
    {
        std::vector<uint8_t> serialized = SerializeMessage();
        ssize_t bytesTx = sendto(sock, serialized.data(), serialized.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
    }

    /**
     * @brief Serializes And Sends UDP 'Confirm' Message
     * 
     * @param sock Socket Where To Send The Message
     * @param server Server Where To Send The Message
     * 
     * Sends UDP 'Confirm' Message
     */
    void SendUdpConfirm(int sock,   const struct sockaddr_in& server, int internalId)
    {
        std::vector<uint8_t> serialized;

        /*  MESSAGE TYPE */
        serialized.push_back(CONFIRM);
        /*  MESSAGE ID   */
        serialized.push_back(internalId & 0xFF);
        serialized.push_back((internalId >> 8) & 0xFF);
        ssize_t bytesTx = sendto(sock, serialized.data(), serialized.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
    }
};


