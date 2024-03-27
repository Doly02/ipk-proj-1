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
