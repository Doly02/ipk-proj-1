/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      main.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Implements Communication With Chat Server Thru TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           main.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Implements Communication With Chat Server Thru TCP Protocol.
 * ****************************/



/************************************************/
/*                  Libraries                   */
/************************************************/
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
/************************************************/
/*                  Constants                   */
/************************************************/



/************************************************/
/*                  CLASS                       */
/************************************************/
class TcpClient
{
private:
    int sock;                 //!<  File Descriptor of The Socket Used For Communication
    std::string serverAddress;  //!< IP Address of The Server
    int port;                   //!< Port Number on Which The Server Is Listening
    struct sockaddr_in server;  //!< Structure Containing Server's Address Information

public:
    static constexpr int NOT_CONNECTED = -1;
    /**
     * @brief Constructor of TcpClient Class 
     * @param addr Server's Address 
     * @param port Server's Port
     *
     * Constructor Initialize Client With Server's Address And Port.
     * Default State of Socket Is Set To NOT_CONNECTED.
     */
    TcpClient(std::string addr, int port) : serverAddress(addr), port(port), sock(NOT_CONNECTED) {}

    /**
     * @brief Destructor of TcpClient Class 
     * 
     * Constructor Initialize Client With Server's Address And Port.
     * Default State of Socket Is Set To NOT_CONNECTED.
     */
    ~TcpClient() 
    {
        if (isConnected()) {
            close(sock);
        }
    }
    /**
     * @brief Determine If The Client Is Connected To The Server
     * @return True If The Client Is Connected, False Otherwise
     */
    bool isConnected() {
        return sock != NOT_CONNECTED;
    }

    bool connectServer()
    {
        if (isConnected())
        {
            return true;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (NOT_CONNECTED == sock)
        {
            std::cerr << "Not Possible To Create Socket: " << strerror(errno) << std::endl;
            return false;
        }

        // Set Server's Address Information
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        if (inet_pton(AF_INET, serverAddress.c_str(), &server.sin_addr) <= 0)
        {
            std::cerr << "Invalid Address: " << serverAddress << std::endl;
            return false;
        }

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            std::cerr << "Connection Failed: " << strerror(errno) << std::endl;
            return false;
        }

        // Connection Was Successful
        return true;
    }
}