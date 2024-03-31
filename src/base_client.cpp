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

/************************************************/
/*                  Libraries                   */
/************************************************/
#include "../include/base_client.hpp"
/************************************************/
/*                  Constants                   */
/************************************************/



/************************************************/
/*                  CLASS                       */
/************************************************/
/**
 * @brief Constructor of TcpClient Class 
 * @param addr Server's Address 
 * @param port Server's Port
 *
 * Constructor Initialize Client With Server's Address And Port.
 * Default State of Socket Is Set To NOT_CONNECTED.
 */
Client::Client(const std::string& addr, int port, uint prot) 
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
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(this->_port);

    if (inet_pton(AF_INET, _serverAddress.c_str(), &server.sin_addr) <= 0)
    {
        throw std::runtime_error("Invalid address/ Address not supported");
    }

    if (TCP == _protocol)
    {
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
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
Client::~Client()
{
    if (sock != NOT_CONNECTED)
    {
        close(sock);
        std::cout << "Client destructor called -> Sock Closed" << std::endl;
    }
}

void Client::updateServerAddress(const std::string& newAddress) 
{
    _serverAddress = newAddress;
}
/**
 * @brief Determine If The Client Is Connected To The Server
 * @return True If The Client Is Connected, False Otherwise
 */
bool Client::isConnected() {
    return sock != NOT_CONNECTED;
}

// Return server's struct 
const struct sockaddr_in& Client::getServerAddr() const 
{
    return server;
}
/**
 * @brief Connects The Client To The Server
 * @return True If The Connection Was Successful, False Otherwise
 */


