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

#define DEBUG_MACRO 0
/************************************************/
/*                  Libraries                   */
/************************************************/
#include "../include/tcp_messages.hpp"
/************************************************/
/*                  Class                       */
/************************************************/

    TcpMessages::TcpMessages() : BaseMessages() {}

    /**
    * @brief Sends Authentication Message.
    * @param client_socket Client Socket
    * @return None
    */
    void TcpMessages::sendAuthMessage(int client_socket)
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
    int TcpMessages::checkJoinReply() {
        const std::string joinServerMsg(msg.buffer.begin(), msg.buffer.end());
        const std::string prefix = "^MSG FROM Server IS ";
        const std::string stdPrefixLenght = "MSG FROM Server IS ";
        const std::string errPrefixLenght = "ERR FROM Server IS ";

        if (compare(msg.buffer, "^MSG FROM Server IS ")) {
            if (msg.buffer.size() >= stdPrefixLenght.length()) 
            {
                msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + stdPrefixLenght.length());
                msg.content.clear();
                msg.content = msg.buffer;
                PrintServeReply();
                return SUCCESS;
            }
        }
        else if (compare(msg.buffer,"^ERR FROM Server IS "))
        {
            if (msg.buffer.size() >= errPrefixLenght.length()) 
            {
                msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + errPrefixLenght.length());
                msg.content.clear();
                msg.content = msg.buffer;
                basePrintExternalError();
                return SUCCESS;
            }
        }
        //TODO: Return Issue
        return JOIN_FAILED;
    }


    /**
     * @brief Sends 'Join' Message
     * @param client_socket Client Socket
     * 
     * Sends 'Join' Message
     * @return None
    */
    void TcpMessages::sendJoinMessage(int client_socket)
    {
        std::string msgToSend = "JOIN " + std::string(msg.channelID.begin(), msg.channelID.end()) + " AS " + std::string(msg.displayName.begin(), msg.displayName.end()) + "\r\n";
        printf("DEBUG INFO: JOIN MSG: %s",msgToSend.c_str());
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
    void TcpMessages::sentByeMessage(int clientSocket)
    {
        std::string msgToSend = "BYE\r\n"; //TODO:  "\r\n" WARNING!
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
    void TcpMessages::sentUsersMessage(int clientSocket)
    {
        std::string msgToSend = "MSG FROM " + std::string(msg.displayName.begin(), msg.displayName.end()) + " IS " + std::string(msg.content.begin(), msg.content.end()) + "\r\n";
        ssize_t bytesTx = send(clientSocket, msgToSend.c_str(), msgToSend.length(), 0);
        if (bytesTx < 0)
            perror("ERROR in sendto");
    }

    int TcpMessages::checkIfErrorOrBye()
    {
        size_t idx = 0;
    
        /* HAS TO BE CLEANED -> WILL BE MODIFIED */
        msg.displayNameOutside.clear();
        msg.content.clear();
        
        std::string bufferStr(msg.buffer.begin(),msg.buffer.end());
        printf("DEBUG INFO: BUFFER: %s\n",bufferStr.c_str());
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
                return MSG_PARSE_FAILED;

            /* PARSE MESSAGE CONTENT */
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 3);

            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.content.push_back(msg.buffer[idx]);   
                idx++;
            }
            msgType = ERROR;

            /* PRINT ERROR MESSAGE */
            basePrintExternalError();

            return EXTERNAL_ERROR;            

        }
        else if (msg.buffer.size() < 6 && std::regex_search(bufferStr, std::regex("^BYE\r\n")))
        {
            // FIXME Should Optimalize
            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.content.push_back(msg.buffer[idx]);   
                idx++;
            }
            PrintServeReply();
            msgType = COMMAND_BYE;

            return SERVER_SAYS_BYE;
        }
        return SUCCESS;
    }


    void TcpMessages::sendErrorMessage(int clientSocket, MessageType_t type)
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

