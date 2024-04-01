/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      base_client.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Header File For Client Base Class. 
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           base_client.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Header File For Client Base Class. 
 * ****************************/
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <poll.h>
#include <iostream>
#include <queue>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

class Client 
{
    private:
        std::string _serverAddress;   //!< IP Address of The Server
        int _port;                    //!< Port Number on Which The Server Is Listening
        uint _protocol;               //!< Protocol used by the client (TCP/UDP)
    public:

        int sock;                     //!< File Descriptor of The Socket Used For Communication
        struct sockaddr_in server;    //!< Structure Containing Server's Address Information

        static constexpr uint TCP = 99u;
        static constexpr uint UDP = 100u;
        static constexpr int NOT_CONNECTED = -1;

        enum ClientState 
        {
            Authentication,
            Open,
            RecvReply,
            End,
            Error
        };
        /**
         * @brief Constructor of TcpClient Class 
         * @param addr Server's Address 
         * @param port Server's Port
         * @param protocol Protocol Used For Communication
         *
         * Constructor Initialize Client With Server's Address And Port.
         * Default State of Socket Is Set To NOT_CONNECTED.
         */
        Client(const std::string& addr, int port, uint protocol);
        /**
         * @brief Destructor of TcpClient Class 
         * 
         * Constructor Of Client With Server's Address And Port.
         * Default State of Socket Is Set To NOT_CONNECTED.
         */
        virtual ~Client();
        /**
         * @brief Updates Server Address
         * @param newAddress New Address of The Server
         * 
         * Updates Server's Address To New Address.
        */
        void updateServerAddress(const std::string& newAddress);
        /**
         * @brief Determine If The Client Is Connected To The Server
         * @return True If The Client Is Connected, False Otherwise
         */
        bool isConnected();
        /**
         * @brief Returns Server's Address
         * @return Server's Address Information
         * 
         * Returns Server's Address Information.
        */
        const struct sockaddr_in& getServerAddr() const;

};

#endif // CLIENT_HPP
