/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      tcp_messages.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Implements Serialization & Deserialization of Messages For TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp_messages.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Implements Serialization & Deserialization of Messages For TCP Protocol.
 * ****************************/

#ifndef CLIENT_H
#define CLIENT_H


/************************************************/
/*                  Libraries                   */
/************************************************/
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
/************************************************/
/*                  Constants                   */
/************************************************/



/************************************************/
/*                  CLASS                       */
/************************************************/
class Client
{
private:

    std::string _serverAddress;  //!< IP Address of The Server
    int _port;                   //!< Port Number on Which The Server Is Listening
    struct sockaddr_in _server;  //!< Structure Containing Server's Address Information
    uint _protocol;

public:
    int sock;                 //!<  File Descriptor of The Socket Used For Communication
    static constexpr uint TCP = 99u;
    static constexpr uint UDP = 100u;

    static constexpr int NOT_CONNECTED = -1;
    /**
     * @brief Constructor of TcpClient Class 
     * @param addr Server's Address 
     * @param port Server's Port
     *
     * Constructor Initialize Client With Server's Address And Port.
     * Default State of Socket Is Set To NOT_CONNECTED.
\     */
    Client(const std::string& addr, int port, uint prot) 
        : _serverAddress(addr), _port(port), _protocol(prot), sock(NOT_CONNECTED)
    {

        if (TCP == _protocol)
        {
            sock = socket(AF_INET, SOCK_STREAM, 0);
        }
        else if (UDP == _protocol)
        {
            sock = socket(AF_INET, SOCK_DGRAM, 0);
        }
    
        if (NOT_CONNECTED == sock)
        {
            std::cerr << "Not Possible To Create Socket: " << strerror(errno) << std::endl;
        }

        // Set Server's Address Information
        memset(&_server, 0, sizeof(_server));
        _server.sin_family = AF_INET;
        _server.sin_port = htons(this->_port);

        if (inet_pton(AF_INET, _serverAddress.c_str(), &_server.sin_addr) <= 0)
        {
            throw std::runtime_error("Invalid address/ Address not supported");
        }

        if (TCP == _protocol)
        {
            if (connect(sock, (struct sockaddr *)&_server, sizeof(_server)) < 0)
            {
                throw std::runtime_error("Socket creation failed");
            }            
        }

        // Connection Was Successful
    }
    /**
     * @brief Destructor of TcpClient Class 
     * 
     * Constructor Of Client With Server's Address And Port.
     * Default State of Socket Is Set To NOT_CONNECTED.
     */
    virtual ~Client()
    {
        if (sock != NOT_CONNECTED)
        {
            close(sock);
        }
    }

    void updateServerAddress(const std::string& newAddress) 
    {
        _serverAddress = newAddress;
    }
    /**
     * @brief Determine If The Client Is Connected To The Server
     * @return True If The Client Is Connected, False Otherwise
     */
    bool isConnected() {
        return sock != NOT_CONNECTED;
    }

    // Return server's struct 
    const struct sockaddr_in& getServerAddr() const 
    {
        return _server;
    }
    /**
     * @brief Connects The Client To The Server
     * @return True If The Connection Was Successful, False Otherwise
     */


};


#endif // CLIENT_H