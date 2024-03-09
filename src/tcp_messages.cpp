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
    TcpMessages(const Message_t& content) : BaseMessages(content) {}

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
    }


    /**
     * @brief Checks If Incomming Message Includes Join Pattern.
     * 
     * @return Returns BaseMessages::SUCCESS If Everything Went Well Otherwise Returns BaseMessages::JOIN_FAILED
    */
    int CheckJoinReply() {
        const std::string joinServerMsg(msg.buffer.begin(), msg.buffer.end());
        const std::string prefix = "^MSG FROM Server IS ";
        const std::string prefixForLenght = "MSG FROM Server IS ";
        if (std::regex_search(joinServerMsg, std::regex(prefix))) {
            if (msg.buffer.size() >= prefixForLenght.length()) {
                msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + prefixForLenght.length());
                return BaseMessages::SUCCESS;
            }
        }
        //TODO: Return Issue
        return BaseMessages::JOIN_FAILED;
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
        std::string msgToSend = "MSG FROM " + std::string(msg.displayName.begin(), msg.displayName.end()) + " IS " + std::string(msg.content.begin(), msg.content.end()) + "\r\n";
        ssize_t bytesTx = send(clientSocket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0)
            perror("ERROR in sendto");
    }

    int CheckIfErrorOrBye()
    {
        size_t idx = 0;
    
        /* HAS TO BE CLEANED -> WILL BE MODIFIED */
        msg.displayNameOutside.clear();
        msg.content.clear();

        std::string bufferStr(msg.buffer.begin(),msg.buffer.end());
        if (msg.buffer.size() >= 9 && std::regex_search(bufferStr, std::regex("^ERR FROM ")))
        {
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 9);

            /* PARSE NAME OF SENDER */
            while (idx < msg.buffer.size() && msg.buffer[idx] != ' ')
            {
                msg.displayNameOutside.push_back(msg.buffer[idx]);   
                idx++;
            }

            if (idx < msg.buffer.size()) 
            {  
                // Clear The Username And Space 
                msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + std::min(idx + 1, msg.buffer.size()));
            }  

            if (!std::regex_search(bufferStr, std::regex("^IS ")))
                return BaseMessages::MSG_PARSE_FAILED;

            /* PARSE MESSAGE CONTENT */
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 3);

            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.content.push_back(msg.buffer[idx]);   
                idx++;
            }
            msg.type = ERROR;

            /* PRINT ERROR MESSAGE */
            std::string sender(msg.displayNameOutside.begin(),msg.displayNameOutside.end());
            std::string msgContent(msg.content.begin(), msg.content.end());

            printf("%s: %s\n",sender.c_str(),msgContent.c_str());

            return BaseMessages::EXTERNAL_ERROR;            

        }
        else if (msg.buffer.size() < 6 && std::regex_search(bufferStr, std::regex("^BYE\r\n")))
        {
            printf("BYE");
            msg.type = COMMAND_BYE;

            return BaseMessages::SERVER_SAYS_BYE;
        }
        return BaseMessages::SUCCESS;
    }


    void SendErrorMessage(int clientSocket, MessageType_t type)
    {
        /* Variables */
        std::string errContent;

        /* Code */
        if (BaseMessages::REPLY == type)
            errContent = "Expected Reply";
        else if (BaseMessages::CONFIRM == type)
            errContent = "Expected Confirm";
        else
            errContent = "Unknown Error";

        std::string msgToSend = "ERR FROM " + std::string(msg.displayName.begin(), msg.displayName.end()) + " IS " +  errContent + "\r\n";
        ssize_t bytesTx = send(clientSocket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0)
            perror("ERROR in sendto");        
    }

};
