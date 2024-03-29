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
#include "../include/udp_messages.hpp"
/************************************************/
/*                  Constants                   */
/************************************************/
    UdpMessages::UdpMessages() : BaseMessages() {}

    UdpMessages::UdpMessages(MessageType_t type, Message_t content) : BaseMessages(type, content) 
    {
        refMessageID = 1;
        result = 0;
        messageID = 1;
    }

    void UdpMessages::appendContent(std::vector<uint8_t>& serialized, const std::vector<char>& contentBuffer) {
        
        // Serialize The Message Content To UDP Message 
        for (const auto& byte : contentBuffer) 
        {
            serialized.push_back(static_cast<uint8_t>(byte));
        }
        serialized.push_back(NULL_BYTE);
    }


    int UdpMessages::checkTimer(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime)
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        return duration.count();
    }

    void UdpMessages::incrementUdpMsgId()
    {
        messageID++;
    }

    void UdpMessages::setUdpMsgId()
    {
        messageID = 1;
    }

    uint16_t UdpMessages::getUdpMsgId()
    {
        return messageID;
    }

    void UdpMessages::setUdpDisplayName(const std::vector<char>& displayNameVec)
    {
        msg.displayName.assign(displayNameVec.begin(), displayNameVec.end());
    }

    void UdpMessages::setUdpChannelID(const std::vector<char>& channelIDVec)
    {
        msg.channelID.assign(channelIDVec.begin(), channelIDVec.end());
    }

    /**
     * @brief Serialize The Message To Byte Array
     * @param message Message To Serialize
     * 
     * Serialize The Message To Byte Array
     * @return Byte Array
    */
    std::vector<uint8_t> UdpMessages::serializeMessage() {
        std::vector<uint8_t> serialized;

        std::string disp(msg.displayName.begin(),msg.displayName.end());

        /*  MESSAGE TYPE */
        serialized.push_back((uint8_t)msg.type);
        /*  MESSAGE ID   */
        serialized.push_back((messageID >> 8) & 0xFF); // (MSB)
        serialized.push_back(messageID & 0xFF);        // (LSB)


        switch (msg.type)
        {
            
            case REPLY:
                /*  RESULT      */
                serialized.push_back(result);
                /*  REF. MESSAGE ID */
                serialized.push_back((refMessageID >> 8) & 0xFF);   // (MSB)
                serialized.push_back(refMessageID & 0xFF);          // (LSB)
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
                printf("DEBUG INFO: DisplayName->|%s|\n",disp.c_str());
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
            case COMMAND_HELP:
            case UNKNOWN_MSG_TYPE:
                exit(1);
        }
        std::string login(msg.login.begin(),msg.login.end());           // FIXME
        std::string cont(msg.content.begin(),msg.content.end());
        std::string displ(msg.displayName.begin(),msg.displayName.end());
        if (msg.type == MSG)
        {
            printf("DEBUG INFO: SER.MSG: %02x %d %s 0x00 %s 0x00\n",static_cast<unsigned char>(msg.type),messageID,displ.c_str(),cont.c_str());
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
    void UdpMessages::deserializeMessage(const std::vector<char>& serializedMsg)
    {
        if (serializedMsg.size() < 3)
        {
            throw std::runtime_error("WARNING: Invalid Message Length"); //FIXME:
        }

        // Store Packet Into The UDP Message Struct
        msgType = (BaseMessages::MessageType_t)serializedMsg[0];

        if (CONFIRM != msgType)
            messageID = static_cast<uint16_t>(serializedMsg[2]) | (static_cast<uint16_t>(serializedMsg[1]) << 8);
        else
            refMessageID = static_cast<uint16_t>(serializedMsg[2]) | (static_cast<uint16_t>(serializedMsg[1]) << 8);

        size_t offset = 3;
        printf("DEBUG INFO: DESERIALIZED: type = %d, messageID = %d\n",msgType,messageID);
        switch (msgType)
        {
            case CONFIRM:
                break;
            case REPLY:
                if (serializedMsg.size() < offset + 3)
                {
                    throw std::runtime_error("Invalid Message Length"); //FIXME:
                }
                result = serializedMsg[offset++];
                refMessageID = static_cast<uint16_t>(serializedMsg[offset+1]) | (static_cast<uint16_t>(serializedMsg[offset]) << 8);
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
                break;
            case MSG: 
            case ERROR:
                /* DISPLAY NAME */
                while (offset < serializedMsg.size())
                {
                    if (serializedMsg[offset] == NULL_BYTE)
                    {
                        offset++;
                        break;
                    } 
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
            case COMMAND_HELP:
            case UNKNOWN_MSG_TYPE:  /* Unused */
            default:
                exit(1);    // Tadu to mozna opravit tak at program pokracuje akorat nic nedela 
            
        
        }
        msg.type = msgType;

    }

    /**
     * @brief Handle The Processing Of The Incoming UDP Message. If EXTERNAL ERROR Occurs, It Print's ERR Message To STDERR.
     * 
     * @param internalId ID Of The Message To Which The Reply Message Replies
     * @return int Returns SUCCESS If Everything Went Well, Otherwise It Returns Error Code
     */
    int UdpMessages::recvUpdIncomingReply()
    {
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        cleanMessage();
        deserializeMessage(serialized);
        
        std::string contentReply(msg.content.begin(),msg.content.end());
        printf("DEBUG INFO: recvUpdIncomingReply -> contentReply: %s\n",contentReply.c_str());

        if (REPLY == msg.type)
        {
            // Check With Internal Message ID
            printf("DEBUG INFO: recvUpdIncomingReply -> refMessageID: %d, result: %d\n",refMessageID,result);
            if (refMessageID == lastSentMessageID && result == 1)
            {
                if (receivedMessageIDs.find(messageID) != receivedMessageIDs.end()) {
                    // Message With This ID Was Already Received
                    return ALREADY_PROCESSED_MSG;  // TODO
                }
                printf("DEBUG INFO: recvUpdIncomingReply -> REPLY SUCCESS (MessageID = %d)\n",messageID);
                receivedMessageIDs.insert(messageID);
                lastReceivedMessageID = messageID;
                PrintServeReply();
                return SUCCESS;         
            }
            else if (refMessageID == lastSentMessageID && result == 0)
            {
                return EXTERNAL_ERROR;
            }
        }
        else if (ERROR == msg.type)
        {
            basePrintExternalError();
            exit(EXTERNAL_ERROR);
        }
        printf("DEBUG INFO: recvUpdIncomingReply -> UNEXPECTED_MESSAGE\n");
        return UNEXPECTED_MESSAGE; 
    }


    void UdpMessages::sendUdpAuthMessage(int sock,const struct sockaddr_in& server)
    {
        lastSentMessageID = messageID;
        std::vector<uint8_t> serialized = serializeMessage();
        ssize_t bytesTx = sendto(sock, serialized.data(), serialized.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
    }

    void UdpMessages::sendUdpMessage(int sock,const struct sockaddr_in& server)
    {
        incrementUdpMsgId();
        std::vector<uint8_t> serialized = serializeMessage();
        ssize_t bytesTx = sendto(sock, serialized.data(), serialized.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
        printf("DEBUG INFO: sendUdpMessage -> messageID=%d\n",messageID);
        lastSentMessageID = messageID;
    }

    int UdpMessages::recvUdpMessage()
    {
        
        uint16_t internalId = lastReceivedMessageID + 1;
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        cleanMessage();
        deserializeMessage(serialized);
        printf("DEBUG INFO: recvUdpMessage -> messageID=%d, internalId=%d\n",messageID,internalId);
        if (REPLY == msg.type)
            return MSG_FAILED;

        if (receivedMessageIDs.find(messageID) != receivedMessageIDs.end()) {
            // Message With This ID Was Already Received
            return ALREADY_PROCESSED_MSG;
        }

        if (ERROR == msg.type)
        {
            // TODO udpMessage.sendUdpConfirm(sock,newServerAddr);
            basePrintExternalError();
            exit(EXTERNAL_ERROR);
        }
        // Přidání messageID do seznamu přijatých ID
        receivedMessageIDs.insert(messageID);
        printf("DEBUG INFO: recvUdpMessage -> RECEIVED SUCCESS\n");
        lastReceivedMessageID = messageID;
        return SUCCESS;
    }

    void UdpMessages::sendUdpConfirm(int sock, const struct sockaddr_in& server)
    {
        std::vector<uint8_t> serialized;
        printf("DEBUG INFO: sendUdpConfirm -> lastReceivedMessageID = %d\n",lastReceivedMessageID);
        /*  MESSAGE TYPE */
        serialized.push_back(CONFIRM);
        /*  MESSAGE ID   */
        serialized.push_back((lastReceivedMessageID >> 8) & 0xFF); // (MSB)
        serialized.push_back(lastReceivedMessageID & 0xFF);        // (LSB)
        ssize_t bytesTx = sendto(sock, serialized.data(), serialized.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (bytesTx < 0) 
        {
            perror("sendto failed");
        }
    }

    int UdpMessages::recvUpdConfirm()
    {
        std::vector<char> serialized(msg.buffer.begin(), msg.buffer.end());
        cleanMessage();
        deserializeMessage(serialized);
        if (CONFIRM == msg.type)
        {
            printf("DEBUG INFO: recvUdpConfirm -> CONFIRM MESSAGE (refMessageID = %d,lastSentMessageID = %d) \n",refMessageID,lastSentMessageID);
            // Check With Internal Message ID
            if (refMessageID == lastSentMessageID)
            {
                printf("DEBUG INFO: recvUdpConfirm -> refMessageID == internalId\n");
                incrementUdpMsgId();
                return SUCCESS;
            }
        }
        else if (ERROR == msg.type)
        {
            basePrintExternalError();
            // TODO Musim na to odpovidat?
            exit(EXTERNAL_ERROR);
        }

        printf("DEBUG INFO: recvUdpConfirm -> CONFIRM_FAILED\n");
        return CONFIRM_FAILED;
    }

