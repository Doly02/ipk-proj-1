/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      udp.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Implements Communication With Chat Server Thru UDP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           udp.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Implements Communication With Chat Server Thru UDP Protocol.
 * ****************************/

#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP

#include <string>
#include <csignal>  // For Signal Handling
#include <unistd.h>         // For close
#include <chrono>
#include "base_client.hpp"
#include "udp_messages.hpp"

class UdpClient : public Client {
public:
    UdpClient(const std::string& addr, int port, int retryCnt, int confirmTimeOut);
    virtual ~UdpClient();

    void udpHandleInterrupt(int signal);
    int processAuthetification();
    int runUdpClient();

private:
    fd_set readfds;
    static constexpr int BUFSIZE = 1536;
    char buf[BUFSIZE];
    int retryCount;
    int confirmationTimeout;
    int currentRetries;
    bool receivedConfirm;
    UdpMessages udpMessage;
    struct sockaddr_in si_other;
    struct sockaddr_in newServerAddr;
    uint16_t lastSentMessageID;
    uint16_t lastReceivedMessageID;

    void setSocketOptions();
    void initializeConnection();
};

#endif // UDP_CLIENT_HPP
