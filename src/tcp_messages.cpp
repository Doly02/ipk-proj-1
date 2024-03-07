/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      base_messages.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Implements Serialization & Deserialization of Messages For TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           base_messages.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Implements Serialization & Deserialization of Messages For TCP Protocol.
 * ****************************/

#define DEBUG_MACRO 0
/************************************************/
/*                  Libraries                   */
/************************************************/
#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include "base_messages.cpp"
/************************************************/
/*                  Class                       */
/************************************************/

class TcpMessages : public BaseMessages
{
public:
    TcpMessages() : BaseMessages() {}
    TcpMessages(MessageType_t type, const Message_t& content) : BaseMessages(type, content) {}

    /**
    * @brief Sends Authentication Message.
    * @param client_socket Client Socket
    * @return None
    */
    void SendAuthMessage(int client_socket)
    {
        std::string msgToSend = "AUTH " + std::string(msg.login.begin(), msg.login.end()) + " AS " + std::string(msg.displayName.begin(), msg.displayName.end()) + " USING " + std::string(msg.secret.begin(), msg.secret.end()) + "\r\n";
        ssize_t bytesTx = send(client_socket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0) {
            std::perror("ERROR: sendto");
        }
        msg.shouldReply = true;
        printf("Sended MSG: %s\n",msgToSend.c_str());
    }

    /**
     * @brief Sends 'Join' Message
     * @param client_socket Client Socket
     * 
     * Sends 'Join' Message
     * @return None
    */
    void SendJoinMessage(int client_socket)
    {
        std::string msgToSend = "JOIN " + std::string(msg.channelID.begin(), msg.channelID.end()) + " AS " + std::string(msg.displayName.begin(), msg.displayName.end()) + "\r\n";
        printf("JOIN MSG: %s",msgToSend.c_str());
        ssize_t bytesTx = send(client_socket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0) {
            std::perror("ERROR: sendto");
        }
        msg.shouldReply = true;
        printf("Sended MSG: %s\n",msgToSend.c_str());
    }

    /**
     * @brief Sends 'Rename' Message
     * @param client_socket Client Socket
     * 
     * Sends 'Rename' Message
     * @return None
    */
    void SendRenameMessage(int client_socket)
    {
        std::string msgToSend = "RENAME " + std::string(msg.displayName.begin(), msg.displayName.end()) + "\r\n";
        ssize_t bytesTx = send(client_socket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0) {
            std::perror("ERROR: sendto");
        }
        printf("Sended MSG: %s",msgToSend.c_str());
    }

    /**
     * @brief Sends 'Bye' Message
     * @param clientSocket Client Socket
     * @param buffer Buffer
     * 
     * Sends 'Bye' Message
     * @return None
    */
    void SentByeMessage(int clientSocket)
    {
        std::string msgToSend = std::string(msg.content.begin(), msg.content.end()) + "\r\n"; //TODO:  "\r\n" WARNING!
        ssize_t bytesTx = send(clientSocket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0)
            perror("ERROR in sendto");
        printf("Sended MSG: %s\n",msgToSend.c_str());
    }
    
    /**
     * @brief Sends User's Message To Server
     * @param clientSocket Client Socket
     * 
     * Sends User's Message To Server
     * @return None
    */
    void SentUsersMessage(int clientSocket)
    {
        std::string msgToSend = "MSG FROM " + std::string(msg.displayName.begin(), msg.displayName.end()) + " IS " + std::string(msg.content.begin(), msg.content.end());
        ssize_t bytesTx = send(clientSocket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0)
            perror("ERROR in sendto");
        printf("Sended MSG: %s\n",msgToSend.c_str());
    }

    int checkJoinReply()
    {
        std::string joinServerMsg(msg.content.begin(),msg.content.end());
        if (std::regex_search(joinServerMsg,std::regex("^MSG FROM Server IS")))
        {
            ssize_t lenOfMsgPrefix = strlen("MSG FROM Server IS ");
            msg.content.erase(msg.content.begin(),msg.content.begin() + lenOfMsgPrefix);
            return BaseMessages::SUCCESS;
        }
        else
        {
            //TODO: Return Issue
            return BaseMessages::JOIN_FAILED;
        }
    }


};
