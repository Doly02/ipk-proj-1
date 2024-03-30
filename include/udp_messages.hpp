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
    UdpMessages();
    UdpMessages(MessageType_t type, Message_t content);

    void appendContent(std::vector<uint8_t>& serialized, const std::vector<char>& contentBuffer);
    int checkTimer(std::chrono::high_resolution_clock::time_point startTime, std::chrono::high_resolution_clock::time_point endTime);
    void incrementUdpMsgId();
    void setUdpMsgId();
    uint16_t getUdpMsgId();
    void setUdpDisplayName(const std::vector<char>& displayNameVec);
    void setUdpChannelID(const std::vector<char>& channelIDVec);
    std::vector<uint8_t> serializeMessage();
    void deserializeMessage(const std::vector<char>& serializedMsg);
    int recvUpdIncomingReply();
    void sendUdpAuthMessage(int sock, const struct sockaddr_in& server);
    void sendUdpMessage(int sock, const struct sockaddr_in& server);
    int recvUdpMessage();
    void sendUdpConfirm(int sock, const struct sockaddr_in& server);
    int recvUpdConfirm(int socket, const struct sockaddr_in& server);
    void sendUdpError(int sock, const struct sockaddr_in& server, const std::string& errorMsg);
    void sendByeMessage(int sock,const struct sockaddr_in& server);

private:
    static constexpr int8_t NULL_BYTE = 0x00;
    uint16_t lastSentMessageID;
    uint16_t lastReceivedMessageID;
};

#endif // UDP_MESSAGES_HPP
