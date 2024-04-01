/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      udp_messages.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Implements Functions That Handles Processing Of UDP Messages.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           udp_messages.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Implements Functions That Handles Processing Of UDP Messages.
 * ****************************/

#ifndef UDP_MESSAGES_HPP
#define UDP_MESSAGES_HPP

#include <iostream>
#include <string>
#include <unistd.h>             // For close
#include <unordered_set>
#include <chrono>
#include <thread>
#include <netinet/in.h>         // For sockaddr_in, AF_INET, SOCK_DGRAM
#include <arpa/inet.h>          // For Debug
#include <iomanip> 
#include "base_messages.hpp"

class UdpMessages : public BaseMessages {
public:


    uint16_t messageID;
    uint16_t refMessageID;
    uint8_t result;
    uint16_t internalMsgId;
    std::unordered_set<uint16_t> receivedMessageIDs;
   /**
     * @brief Construct a new Udp Messages object
     */
    UdpMessages();
    /**
     * @brief Construct a new Udp Messages object
     * @param type Type Of Message
     * @param content Content Of Message
    */
    UdpMessages(MessageType_t type, Message_t content);
    /**
     * @brief Apppend Vector To Serialized Message
     * @param serialized Serialized Message
     * @param contentBuffer Content Buffer
     */
    void appendContent(std::vector<uint8_t>& serialized, const std::vector<char>& contentBuffer);
    /**
     * @brief Checks Timer
     * @param startTime Start Time
     * @param endTime End Time
    */    
    int checkTimer(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime);
    /**
     * @brief Increment Udp Message ID
    */    
    void incrementUdpMsgId();
    /**
     * @brief Set Udp Message ID
    */    
    void setUdpMsgId();
    /**
     * @brief Get Udp Message ID
    */    
    uint16_t getUdpMsgId();
    /**
     * @brief Set Display Name
     * @param displayNameVec Display Name Vector
    */    
    void setUdpDisplayName(const std::vector<char>& displayNameVec);
    /**
     * @brief Set Channel ID
     * @param channelIDVec Channel ID Vector
    */    
    void setUdpChannelID(const std::vector<char>& channelIDVec);
    /**
     * @brief Serialize Message
     * @return std::vector<uint8_t> Serialized Message
     */    
    std::vector<uint8_t> serializeMessage();
    /**
     * @brief Deserialize Byte Array To Message
     * @param serialized Byte Array
     * 
     * Deserialize Byte Array To Message
     * @return Message
    */    
    void deserializeMessage(const std::vector<char>& serializedMsg);
    /**
     * @brief Checks if Message is Reply
     * @return int
    */    
    int recvUpdIncomingReply();
    /**
     * @brief Send UDP Auth Message
     * @param sock Socket
     * @param server Server
    */    
    void sendUdpAuthMessage(int sock, const struct sockaddr_in& server);
    /**
     * @brief Send UDP Message
     * @param sock Socket
     * @param server Server
    */    
    void sendUdpMessage(int sock, const struct sockaddr_in& server);
    /**
     * @brief Checks UDP Message
     * @return int
    */    
    int recvUdpMessage();
    /**
     * @brief Send UDP Confirm
     * @param sock Socket
     * @param server Server
    */    
    void sendUdpConfirm(int sock, const struct sockaddr_in& server);
    /**
     * @brief Checks UDP Confirm
     * @return int
    */
    int recvUpdConfirm();
    /**
     * @brief Send UDP Error
     * @param sock Socket
     * @param server Server
     * @param errorMsg Error Message
    */
    void sendUdpError(int sock, const struct sockaddr_in& server, const std::string& errorMsg);
    /**
     * @brief Send UDP Bye Message
     * @param sock Socket
     * @param server Server
    */
    void sendByeMessage(int sock,const struct sockaddr_in& server);
    /**
     * @brief Receives UDP Error
     * @return void
    */    
    void recvUdpError();

private:
    static constexpr int8_t NULL_BYTE = 0x00;
    uint16_t lastSentMessageID;
    uint16_t lastReceivedMessageID;
};

#endif // UDP_MESSAGES_HPP
