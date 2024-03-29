/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      tcp_client.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Implements Communication With Chat Server Thru TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp_client.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Implements Communication With Chat Server Thru TCP Protocol.
 * ****************************/

#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <cstdlib>
#include <netinet/in.h>
#include <unistd.h> 
#include <csignal>
#include <string>
#include <vector>
#include <csignal>     
// ---
#include "base_client.hpp"
#include "tcp_messages.hpp"


class TcpClient : public Client 
{
private:
        struct pollfd fds[NUM_FILE_DESCRIPTORS];
        bool authConfirmed = false;
        const int BUFSIZE = 1536;
        char buf[1536];
        TcpMessages tcpMessage;
        

    public:
        
        TcpClient(std::string addr, int port, uint protocol);
        virtual ~TcpClient();

        void tcpHandleInterrupt(int signal);
        void checkAuthentication();
        int runTcpClient();
};

#endif // TCP_CLIENT_HPP
