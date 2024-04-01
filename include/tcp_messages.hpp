/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      tcp_messages.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Implements Functions That Handles Processing Of TCP Messages.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp_messages.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Implements Functions That Handles Processing Of TCP Messages.
 * ****************************/
#ifndef TCP_MESSAGES_HPP
#define TCP_MESSAGES_HPP

#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include "base_messages.hpp" 

class TcpMessages : public BaseMessages 
{
    public:
        /**
         * @brief Default Constructor
         */        
        TcpMessages();
        /**
         * @brief Construct a new Tcp Messages object
         * @param type Type Of Message
         * @param content Content Of Message
         * 
        */        
        TcpMessages(MessageType_t type, const Message_t& content);
        /**
         * @brief Sends Authentication Message.
         * @param server_socket Client Socket
         */
        void sendAuthMessage(int client_socket);
        /**
         * @brief Checks If Incomming Message Includes Join Pattern.
        */        
        int checkJoinReply();
        /**
         * @brief Sends Join Message To Server
         * @param server_socket Server Socket
         * 
         * @return None
        */        
        void sendJoinMessage(int client_socket);
        /**
         * @brief Sends Bye Message To Server
         * @param server_socket Server Socket
         * 
         * @return None
        */        
        void sentByeMessage(int clientSocket);
        /**
         * @brief Sends Users Message To Server
         * @param server_socket Server Socket
        */        
        void sentUsersMessage(int clientSocket);
        /**
         * @brief Checks If Error Or Bye Message Was Received
         * @param server_socket Server Socket
         * 
         * @return 0 If Everything Went Well, -1 If The Reply Is Not OK, -2 If Error Occurs
        */        
        int checkIfErrorOrBye(int clientSocket);
        /**
         * @brief Sends Error Message To Server
         * @param server_socket Server Socket
         * @param type Type Of Error
         * 
         * @return None
        */        
        void sendErrorMessage(int clientSocket, MessageType_t type);
        /**
         * @brief Handles Reply From Server
         * 
         * @return 0 If The Reply Is OK, -1 If The Reply Is Not OK, -2 If Error Occurs 
        */        
        int handleAuthReply();
};

#endif // TCP_MESSAGES_HPP
