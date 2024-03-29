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
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cctype> // For isdigit and isalpha
#include <iostream>
#include <regex>
#include <sys/socket.h>
#include "../include/base_messages.hpp"

//#include "strings.cpp"
/************************************************/
/*                  Class                       */
/************************************************/

    BaseMessages::BaseMessages() : msgType(UNKNOWN_MSG_TYPE) {
        // Initialize Attributes
        msg.shouldReply = false;
    }
    /**
     * @brief Constructor of TcpMessages Class 
     * @param type Type of The Message
     * @param content Content of The Message
     *
     * Constructor Initialize Message With Type And Content.
     */
    BaseMessages::BaseMessages(MessageType_t type, Message_t content) : msgType(type), msg(content) {}

    /**
     * @brief Destructor of TcpMessages Class 
     * 
     * Destructor of TcpMessages Class.
     */
    BaseMessages::~BaseMessages() {}


    void BaseMessages::insertErrorMsgToContent(const std::string& inputString)
    {
        msg.content.clear();
        msg.content.assign(inputString.begin(), inputString.end());
    }

    void BaseMessages::cleanMessage()
    {
        msg.content.clear();
        //msg.login.clear();
        msg.secret.clear();
        //msg.displayName.clear();
        msg.channelID.clear();
        msg.displayNameOutside.clear();
        msg.shouldReply = false;
    }
    /**
     * @brief Check If The Message Components Are Valid (ID, Display Name, Content, Secret Length)
     * 
     * 
     * @return SUCCESS If The Message Is Valid, Otherwise Returns NON_VALID_PARAM
     */
    int BaseMessages::checkLength()
    {
        if (msg.type == COMMAND_AUTH)
        {
            // Check Username
            if (msg.login.size() > LENGHT_USERNAME || (!areAllDigitsOrLettersOrDash(msg.login)))
            {
                insertErrorMsgToContent("Username Is Too Long Or Contains Non-Alphanumeric Characters");
                basePrintInternalError(NON_VALID_PARAM);
                return NON_VALID_PARAM;
            }
        }
        if (msg.type == COMMAND_JOIN)
        {
            // Check Channel ID
            if (msg.channelID.size() > LENGHT_CHANNEL_ID || (!areAllDigitsOrLettersOrDashOrDot(msg.channelID)))
            {
                insertErrorMsgToContent("Channel ID Is Too Long Or Contains Non-Alphanumeric Characters");
                basePrintInternalError(NON_VALID_PARAM);
                return NON_VALID_PARAM;
            }
        }
        if (msg.type == COMMAND_AUTH)
        {
            // Check Secret
            if (msg.secret.size() > LENGHT_SECRET || (!areAllDigitsOrLettersOrDash(msg.secret)))
            {
                insertErrorMsgToContent("Secret Is Too Long Or Contains Non-Alphanumeric Characters");
                basePrintInternalError(NON_VALID_PARAM);
                return NON_VALID_PARAM;
            }
        }
        if (msg.type == MSG || msg.type == COMMAND_AUTH || msg.type == COMMAND_JOIN || msg.type == ERROR)
        {
            // Check Display Name
            if (msg.displayName.size() > LENGHT_DISPLAY_NAME || (!areAllPrintableCharacters(msg.displayName)))
            {
                insertErrorMsgToContent("Display Name Is Too Long Or Contains Non-Alphanumeric Characters");
                basePrintInternalError(NON_VALID_PARAM);
                return NON_VALID_PARAM;
            }
        }
        if (!msg.displayNameOutside.empty()) //FIXME: Should Findout Valid Condition
        {
            // Check Display Name From Outside (Another Client)
            if (msg.displayNameOutside.size() > LENGHT_DISPLAY_NAME || (!areAllPrintableCharacters(msg.displayNameOutside)))
            {
                insertErrorMsgToContent("Display Name From Another User Is Too Long Or Contains Non-Alphanumeric Characters");
                basePrintInternalError(NON_VALID_PARAM);
                return NON_VALID_PARAM;
            }
        }
        if (msg.type == MSG || msg.type == ERROR || msg.type == REPLY)
        {
            if (msg.content.size() > LENGHT_CONTENT || (!areAllPrintableCharactersOrSpace(msg.content)))
            {
                insertErrorMsgToContent("Message Is Too Long Or Contains Non-Alphanumeric Characters");
                basePrintInternalError(NON_VALID_PARAM);
                return NON_VALID_PARAM;
            }

        }
        return SUCCESS;
    }


    /**
     * @brief Stores Chars From Buffer To Message Buffer
     * @param buffer Buffer
     *
     * Stores Chars From Buffer To Message Buffer
     * @return None
     */
    void BaseMessages::readAndStoreContent(const char* buffer)
    {
        // Clear The Message Content
        msg.buffer.clear();

        // Find The Lenght Of Buffer
        size_t len = strlen(buffer);

        printf("DEBUG INFO: RECEIVED: ");
        for (size_t i = 0; i < len; i++)
        {
            if (buffer[i] != '\r' && buffer[i] != '\n')
            {
                msg.buffer.push_back(buffer[i]);
                printf("%c",buffer[i]);
            }
        }
        printf("\n");
        msg.buffer.push_back('\r'); //TODO: Check If It's Needed
        msg.buffer.push_back('\n'); 
    }


    void BaseMessages::readAndStoreBytes(const char* buffer, size_t bytesRx)
    {
        // Clear The Message Content
        msg.buffer.clear();
        printf("DEBUG INFO: READING BYTES: ");
        for (size_t i = 0; i < bytesRx; i++)
        {
            
            msg.buffer.push_back(buffer[i]);
            if (i > 2)
            {
                printf("%c",buffer[i]);
            }

        }
        printf("\nDEBUG INFO: BYTES READ: %zu byte[0]=%02x\n", bytesRx, static_cast<unsigned char>(buffer[0]));
    }

    /**
     * @brief Identifies Message Type & Parse Message Parts
     * 
     * 
     * @return 0 If The Message Is Valid, Otherwise Returns -1
     */
    int BaseMessages::checkMessage()
    {
        size_t idx = 0;
        int retVal = FAIL;
        InputType_t inputType = INPUT_UNKNOWN;
        std::string bufferStr = convertToString(msg.buffer);

        /*                  INPUT TYPE RECOGNITION LOGIC                                */
        if (msg.buffer.size() >= 6 && compare(msg.buffer,"^/auth")) 
        {
            // delete first 5 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 6);
            inputType = INPUT_AUTH;
        }
        else if (msg.buffer.size() >= 6 && compare(msg.buffer,"^/join"))
        {
            // delete first 5 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 6);
            inputType = INPUT_JOIN;
        }
        else if (msg.buffer.size() >= 7 && compare(msg.buffer,"^/rename")) 
        {
            // delete first 7 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 8);
            inputType = INPUT_RENAME;
        }
        else if (msg.buffer.size() >= 5 && compare(msg.buffer,"^/help")) 
        {
            // delete first 5 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 6);
            inputType = INPUT_HELP;
        }
        else 
        {
            inputType = INPUT_MSG;
        }

        /*      PROCES INPUT TYPE        */
        if (inputType == INPUT_UNKNOWN) 
        {
            return FAIL;
        }
        else if (inputType == INPUT_AUTH) 
        {
            idx = 0;
            // Clear The Message Contente That Will Be Stored
            msg.login.clear();
            msg.secret.clear();
            msg.displayName.clear();

            // Process Username
            while (idx < msg.buffer.size() && msg.buffer[idx] != ' ')
            {
                msg.login.push_back(msg.buffer[idx]);   
                idx++;
            }
            if (idx < msg.buffer.size()) {  
                // Clear The Username And Space 
                msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + std::min(idx + 1, msg.buffer.size()));
            }    

            // Process Secret
            idx = 0;
            while (idx < msg.buffer.size() && msg.buffer[idx] != ' ') {
                msg.secret.push_back(msg.buffer[idx]);   
                idx++;
            }
            if (idx < msg.buffer.size()) {  
                // Clear The Secret And Space 
                msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + std::min(idx + 1, msg.buffer.size()));
            }
            // Process Display Name
            idx = 0; // Reset idx if you're using it to iterate through the remaining content
            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') {
                msg.displayName.push_back(msg.buffer[idx]);   
                idx++;
            }
            msg.type = COMMAND_AUTH;
        }
        else if (inputType == INPUT_JOIN) 
        {
            idx = 0;
            msg.channelID.clear();
            
            // Process Channel ID
            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.channelID.push_back(msg.buffer[idx]);   
                idx++;
            }
            msg.type = COMMAND_JOIN;

        }
        else if (inputType == INPUT_HELP) 
        {
            msg.type = COMMAND_HELP;
        }
        else if (inputType == INPUT_RENAME)
        {
            idx = 0;
            msg.displayName.clear();
            // Process New Display Name
            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r')  // '\n' should not be in content
            {
                msg.displayName.push_back(msg.displayName[idx]);   
                idx++;
            }
        }
        else 
        {
            msg.content.clear();
            idx = 0;
            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.content.push_back(msg.buffer[idx]);   
                idx++;
            }
            msg.type = MSG;

        }
        retVal = checkLength();
        return retVal;   
    }


    /**
     * @brief Parse Messages From Incoming Packet
     * 
     * 
     * @return None, Exits If Something Goes Wrong
    */
    int BaseMessages::parseMessage()
    {
        int retVal = 0;
        size_t idx = 0;
        std::regex msgIsRegex("^IS");

        // Prepaire Attributes
        msg.content.clear();
        msg.displayNameOutside.clear();

        // Check if Content Is a Message 
        if (compare(msg.buffer,"^MSG FROM")) 
        {
            size_t prefixLength = std::string("MSG FROM ").length();
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + prefixLength); 
            
            idx = 0;
            // Loop until the substring starting at the current idx does not start with "IN"
            // and ensure we're not at the last character
            while (idx < msg.buffer.size() - 1)
            {
                // Convert current substring to string for regex search
                std::string currentSubStr(msg.buffer.begin() + idx, msg.buffer.end());
                if (std::regex_search(currentSubStr, msgIsRegex)) {
                    break; // Exit the loop if "IN" is found at the beginning of the current substring
                }
                
                msg.displayNameOutside.push_back(msg.buffer[idx]);
                idx++;
            }

            // Check and Remove the Last Character if Needed (It's a Space)
            if (!msg.displayNameOutside.empty()) 
            {
                msg.displayNameOutside.pop_back();
            }
            /* Process The Content */
            int isPlusSpace = 3;                                                    // 1. Get Rid of "IS "
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + idx + isPlusSpace);

            // Store The Content From Buffer
            idx = 0;
            while (idx < msg.buffer.size()) 
            {   
                if (msg.buffer[idx] == '\n' || msg.buffer[idx] == '\r')
                    break;
                msg.content.push_back(msg.buffer[idx]);
                idx++;
            }
            // Check The Message And User Name Length
            retVal = checkLength();
            msgType = MSG;

            std::string msgContent(msg.content.begin(), msg.content.end());
            std::string displayNameOutside(msg.displayNameOutside.begin(), msg.displayNameOutside.end());
            printf("DEBUG INFO: MSG FROM: %s: %s\n", displayNameOutside.c_str(), msgContent.c_str());

            return retVal;
        }
        else if (compare(msg.buffer, "^BYE\r\n"))
        {
            idx = 0;
            while (idx < msg.buffer.size()) 
            {   
                if (msg.buffer[idx] == '\n' || msg.buffer[idx] == '\r')
                    break;
                msg.content.push_back(msg.buffer[idx]);
                idx++;
            }
            msgType = COMMAND_BYE;
            return SUCCESS;
        }
        return MSG_PARSE_FAILED;
    }

    void BaseMessages::printMessage()
    {
        std::string content(msg.content.begin(), msg.content.end());
        std::string displayNameOutside(msg.displayNameOutside.begin(), msg.displayNameOutside.end());
        if (MSG == msgType)
        {
            if (!displayNameOutside.empty() && !content.empty())
            {
                printf("%s: %s\n", displayNameOutside.c_str(), content.c_str());
            }

        }
        else if (COMMAND_BYE == msgType)
        {
            if (!content.empty())
            {
                printf("%s\n", content.c_str());
            }
        }
    }

    void BaseMessages::PrintServeReply()
    {
        std::string serverSay(msg.content.begin(),msg.content.end());
        fprintf(stdout,"server: %s\n",serverSay.c_str());
    }

    void BaseMessages::basePrintExternalError()
    {
        std::string errDisplayName(msg.displayNameOutside.begin(),msg.displayNameOutside.end());
        std::string errContent(msg.content.begin(),msg.content.end());
        fprintf(stderr,"ERR FROM: %s: %s\n",errDisplayName.c_str(),errContent.c_str());
    }

    void BaseMessages::basePrintInternalError(int retVal)
    {
        std::string errDisplayName(msg.displayNameOutside.begin(),msg.displayNameOutside.end());
        std::string errContent(msg.content.begin(),msg.content.end());
        fprintf(stderr,"ERR: %s (Return Value: %d)\n",errContent.c_str(),retVal);
    }

    void BaseMessages::printHelp()
    {
        printf("Commands:\n");
        printf("----------------------------------------------\n");
        printf("AUTHENTICATION CMD:  /auth [username] [password] [displayname]\n");
        printf("JOIN CMD:            /join [channel]\n");
        printf("RENAME CMD:          /rename [displayname]\n");
        printf("HELP CMD:            /help\n");
        printf("To Exit The Program Correctly, Type 'BYE' And Press ENTER\n");
    }


