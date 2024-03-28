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
    int TcpMessages::checkJoinReply() 
    {
        bool confirmAcceppted = false;
        const std::string joinServerMsg(msg.buffer.begin(), msg.buffer.end());
        const std::string prefix = "^MSG FROM Server IS ";
        const std::string stdPrefixLenght = "MSG FROM Server IS ";
        const std::string errPrefixLenght = "ERR FROM Server IS ";
        
        /* Reply Join OK */
        if (compare(msg.buffer,"^REPLY OK IS Join success\n\r") & !confirmAcceppted)
        {
            confirmAcceppted = true;
            return SUCCESS;
        }
        /* Message From Server */
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
        /* Join Confirmed */


        /* Error From Server */
        else if (compare(msg.buffer,"^ERR FROM Server IS "))
        {
            if (msg.buffer.size() >= errPrefixLenght.length()) 
            {
                msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + errPrefixLenght.length());
                msg.content.clear();
                msg.content = msg.buffer;
                basePrintExternalError();
                exit(ERROR);
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
        msg.content.clear();
        
        if (msg.buffer.size() >= 9 && compare(msg.buffer, "^ERR FROM Server IS "))
        {
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 19);

            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.content.push_back(msg.buffer[idx]);   
                idx++;
            }
            /* PRINT ERROR MESSAGE */
            basePrintExternalError();
            exit(EXTERNAL_ERROR);            

        }
        else if (msg.buffer.size() < 6 && compare(msg.buffer, "^BYE\r\n"))
        {
            // FIXME Should Optimalize
            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.content.push_back(msg.buffer[idx]);   
                idx++;
            }
            PrintServeReply();
            exit(SUCCESS);
        }
        printf("SUCCESS\n");
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

    /**
     * @brief Handles Reply From Server
     * 
     * @return 0 If The Reply Is OK, -1 If The Reply Is Not OK, -2 If Error Occurs 
    */
    int TcpMessages::handleAuthReply()
    {
        /* Preparation  */
        std::string errorLenght     = "ERR FROM Server IS ";
        std::string prefixLenght    = "REPLY ";
        std::string okLenght        = "OK IS ";

        /* Execution    */
        if (compare(msg.buffer,"^REPLY OK IS Authentication successful.")) 
        {
            // Delete Prefix
            return SUCCESS;      
        }
        else if (compare(msg.buffer,"^OK IS ")) 
        {
            // Erase The Ok From The Message 
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + okLenght.length());
            // Print buffer 
            msg.content.clear();
            msg.content = msg.buffer;
            PrintServeReply();
            
            return SUCCESS;
        }

        /*  ERROR MESSAGE HANDLING  */
        else if (compare(msg.buffer,"^ERR FROM Server IS "))
        {
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + errorLenght.length());
            //bufferAsStr = std::string(msg.buffer.begin(), msg.buffer.end());
            //std::cerr << bufferAsStr << std::endl; /// FIXME
            return AUTH_FAILED;
        }
        else 
        {
            // Compare With Error Message Template   
            if (compare(msg.buffer,"ˆNOK IS "))  
            {
                std::cerr << "ERR: Authentication Failed" << std::endl;
                return AUTH_FAILED;
            }
            
        }
        return AUTH_FAILED;
    }
