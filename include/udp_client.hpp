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

#include <csignal>  // For Signal Handling
#include <unistd.h>         // For close
#include <chrono>
#include <queue>
#include "base_client.hpp"
#include "udp_messages.hpp"
/*************************************************/
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Milliseconds = std::chrono::milliseconds;


class UdpClient : public Client {
public:





    UdpClient(const std::string& addr, int port, int retryCnt, int confirmTimeOut);
    virtual ~UdpClient();

    void udpHandleInterrupt(int signal);
    int processAuthetification();
    int runUdpClient();

private:
    struct pollfd fds[NUM_FILE_DESCRIPTORS];
    /* Timers */
    TimePoint startWatch;               //!< Contains the Initial Measurement Time
    TimePoint stopWatch;                //!< Contains the Final Measurement Time
    bool measureTime = false;           //!< Indicates That Time Should Be Measured
    
    std::queue<UdpMessages> messageQueue;

    static constexpr int BUFSIZE = 1536;
    char buf[BUFSIZE];

    int confirmationTimeout;
    int currentRetries = 0;
    int retryCount;
    
    bool receivedConfirm;
    UdpMessages udpMessage;
    UdpMessages udpBackUpMessage;
    struct sockaddr_in si_other;
    struct sockaddr_in newServerAddr;

    void setSocketOptions();
    void initializeConnection();
};

#endif // UDP_CLIENT_HPP
