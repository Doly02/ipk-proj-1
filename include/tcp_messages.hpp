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
        TcpMessages();
        TcpMessages(MessageType_t type, const Message_t& content);

        void sendAuthMessage(int client_socket);
        int checkJoinReply();
        void sendJoinMessage(int client_socket);
        void sentByeMessage(int clientSocket);
        void sentUsersMessage(int clientSocket);
        int checkIfErrorOrBye();
        void sendErrorMessage(int clientSocket, MessageType_t type);
};

#endif // TCP_MESSAGES_HPP
