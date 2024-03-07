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

#ifndef BASE_MESSAGES_H
#define BASE_MESSAGES_H

#define DEBUG_MACRO 0
/************************************************/
/*                  Libraries                   */
/************************************************/
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <regex>
#include <sys/socket.h>
/************************************************/
/*                  Class                       */
/************************************************/
class BaseMessages 
{
    public:
    static constexpr int SUCCESS                = 0;
    static constexpr int AUTH_FAILED            = -1;
    static constexpr int JOIN_FAILED            = -2;
    static constexpr int MSG_FAILED             = -3;
    static constexpr int LENGHT_ID              = 20;
    static constexpr int LENGHT_SECRET          = 128;
    static constexpr int LENGHT_CONTENT         = 1400;
    static constexpr int LENGHT_DISPLAY_NAME    = 128;
    /* MESSAGE TYPES */
    enum MessageType_t
    {
        CONFIRM          = 0x00,     //!< Confirm - Template: CONFIRM {MessageID}\r\n
        REPLY            = 0x01,     //!< Reply - Template: REPLY IS {MessageContent}\r\n
        COMMAND_AUTH     = 0x02,     //!< Authentication - Template: AUTH {Username} USING {Secret}\r\n
        COMMAND_JOIN     = 0x03,     //!< Join - Template: JOIN {ChannelID} AS {DisplayName}\r\n
        MSG              = 0x04,     //!< Message - Template: MSG FROM {DisplayName} IS {MessageContent}\r\n
        ERROR            = 0xFE,     //!< Error - Template: ERROR FROM {DisplayName} IS {MessageContent}\r\n
        COMMAND_BYE      = 0xFF,     //!< Disconnect - Template: BYE\r\n
        UNKNOWN_MSG_TYPE = 0x99,     //!< Unknown Message Type

    };
    enum InputType_t
    {
        INPUT_UNKNOWN,            //!< Unknown Input
        INPUT_AUTH,       //!< Authentication - Template: AUTH {Username} USING {Secret}\r\n
        INPUT_JOIN,       //!< Join - Template: JOIN {ChannelID} AS {DisplayName}\r\n
        INPUT_RENAME,      //!< Rename - Template: RENAME {NewDisplayName}\r\n
        INPUT_MSG,        //!< Message - Template: MSG FROM {DisplayName} IS {MessageContent}\r\n
        INPUT_HELP        //!< Disconnect - Template: BYE\r\n
    };

    struct Message_t
    {
        /* data */
        MessageType_t type;             //!< Type of The Message
        std::vector<char> content;      //!< Content of The Message
        bool isCommand;                 //!< Is The Message Command
        std::vector<char> login;
        std::vector<char> secret;
        std::vector<char> displayName;
        std::vector<char> channelID;
        std::vector<char> displayNameOutside;
        std::vector<char> buffer;
        bool shouldReply;
    };
    

    MessageType_t msgType;
    Message_t msg;

    BaseMessages() : msgType(UNKNOWN_MSG_TYPE) {
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
    BaseMessages(MessageType_t type, Message_t content) : msgType(type), msg(content) {}

    /**
     * @brief Destructor of TcpMessages Class 
     * 
     * Destructor of TcpMessages Class.
     */
    ~BaseMessages() {}


    /**
     * @brief Compares Content Of Vector And String
     * @param vec Vector To Compare
     * @param str String To Compare
     * 
     * @return True If The Content Of Vector And String Are The Same, Otherwise False
    */
    bool compareVectorAndString(const std::vector<char>& vec, const std::string& str) {
        std::string vecAsString(vec.begin(), vec.end());
        return vecAsString == str;
    }


    void cleanMessage()
    {
        msg.content.clear();
        msg.login.clear();
        msg.secret.clear();
        msg.displayName.clear();
        msg.channelID.clear();
        msg.displayNameOutside.clear();
        msg.shouldReply = false;
    }
    /**
     * @brief Check If The Message Components Are Valid (ID, Display Name, Content, Secret Length)
     * 
     * 
     * @return 0 If The Message Is Valid, Otherwise Returns -1, -2, -3, -4
     */
    int checkLength()
    {
        if (msg.channelID.size() > LENGHT_ID)
        {
            return -1;
        }
        if (msg.displayName.size() > LENGHT_DISPLAY_NAME)
        {
            return -2;
        }
        if (msg.displayNameOutside.size() > LENGHT_DISPLAY_NAME)
        {
            return -2;
        }
        if (msg.content.size() > LENGHT_CONTENT)
        {
            return -3;
        }
        if (msg.secret.size() > LENGHT_SECRET)
        {
            return -4;
        }
        return 0;
    }


    /**
     * @brief Stores Chars From Buffer To Message Buffer
     * @param buffer Buffer
     *
     * Stores Chars From Buffer To Message Buffer
     * @return None
     */
    void readAndStoreContent(const char* buffer)
    {
        // Clear The Message Content
        msg.content.clear();
        // Find The Lenght Of Buffer
        size_t len = strlen(buffer);

        for (size_t i = 0; i < len; i++)
        {
            if (buffer[i] != '\r' && buffer[i] != '\n')
            {
                msg.content.push_back(buffer[i]);
            }
        }
        msg.content.push_back('\r'); //TODO: Check If It's Needed
        msg.content.push_back('\n'); //TODO: Check If It's Needed
    }

    /**
     * @brief Identifies Message Type & Parse Message Parts
     * 
     * 
     * @return 0 If The Message Is Valid, Otherwise Returns -1
     */
    int checkMessage()
    {
        int retVal = -1;
        InputType_t inputType = INPUT_UNKNOWN;
        if (msg.content.size() >= 5 && msg.content[0] == '/' && msg.content[1] == 'a' && msg.content[2] == 'u' 
        && msg.content[3] == 't' && msg.content[4] == 'h') {
            // delete first 5 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 6);
            inputType = INPUT_AUTH;
        }
        else if (msg.content.size() >= 4 && msg.content[0] == '/' && msg.content[1] == 'j' && msg.content[2] == 'o'
        && msg.content[3] == 'i' && msg.content[4] == 'n') {
            // delete first 5 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 6);
            inputType = INPUT_JOIN;
        }
        else if (msg.content.size() >= 4 && msg.content[0] == '/' && msg.content[1] == 'r' && msg.content[2] == 'e'
        && msg.content[3] == 'n' && msg.content[4] == 'a' && msg.content[5] == 'm' && msg.content[6] == 'e') {
            // delete first 7 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 8);
            inputType = INPUT_RENAME;
        }
        else if (msg.content.size() >= 3 && msg.content[0] == '/' && msg.content[1] == 'h' && msg.content[2] == 'e'
        && msg.content[3] == 'l' && msg.content[4] == 'p') {
            // delete first 5 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 6);
            inputType = INPUT_HELP;
        }
        else if (msg.content.size() >= 3 && msg.content[0] == 'B' && msg.content[1] == 'Y' && msg.content[2] == 'E'
        && msg.content[3] == '\r' && msg.content[4] == '\n') {
            // delete first 5 characters + 1 space
            msg.type = COMMAND_BYE;
            return SUCCESS;
        }
        else 
        {
            inputType = INPUT_MSG;
        }

        if (inputType == INPUT_UNKNOWN) {
            return -1;
        }
        else if (inputType == INPUT_AUTH) {
            size_t idx = 0;
            // Process Username
            while (idx < msg.content.size() && msg.content[idx] != ' ')
            {
                msg.login.push_back(msg.content[idx]);   
                idx++;
            }
            if (idx < msg.content.size()) {  
                // Clear The Username And Space 
                msg.content.erase(msg.content.begin(), msg.content.begin() + std::min(idx + 1, msg.content.size()));
            }    
#if (DEBUG_MACRO == 1)      
            printf("Username: %s\n", std::string(msg.login.begin(), msg.login.end()).c_str());
#endif
            // Process Secret
            idx = 0;
            while (idx < msg.content.size() && msg.content[idx] != ' ') {
                msg.secret.push_back(msg.content[idx]);   
                idx++;
            }
            if (idx < msg.content.size()) {  
                // Clear The Secret And Space 
                msg.content.erase(msg.content.begin(), msg.content.begin() + std::min(idx + 1, msg.content.size()));
            }
#if (DEBUG_MACRO == 1)
            printf("Secret: %s\n", std::string(msg.secret.begin(), msg.secret.end()).c_str());
#endif
            // Process Display Name
            idx = 0; // Reset idx if you're using it to iterate through the remaining content
            //while (idx < msg.content.size()) 
            //{
            while (idx < msg.content.size() && msg.content[idx] != '\n' && msg.content[idx] != '\r') {
                msg.displayName.push_back(msg.content[idx]);   
                idx++;
            }
#if (DEBUG_MACRO == 1)
            printf("Display Name: %s\n", std::string(msg.displayName.begin(), msg.displayName.end()).c_str());
#endif
            msg.type = COMMAND_AUTH;

        }
        else if (inputType == INPUT_JOIN) {
            size_t idx = 0;
            printf("Join Message Recognised\n");
            // Process Channel ID
            while (idx < msg.content.size() && msg.content[idx] != '\n' && msg.content[idx] != '\r') 
            {
                msg.channelID.push_back(msg.content[idx]);   
                idx++;
            }
            msg.type = COMMAND_JOIN;

        }
        else if (inputType == INPUT_MSG) {
            msg.type = MSG;

        }
        else if (inputType == INPUT_HELP) {
            //TODO: - Print Help Message
            msg.type = COMMAND_BYE;

        }
        else if (inputType == INPUT_RENAME)
        {
            size_t idx = 0;
            // Process New Display Name
            while (idx < msg.content.size() && msg.content[idx] != '\n' && msg.content[idx] != '\r')  // '\n' should not be in content
            {
                msg.displayName.push_back(msg.displayName[idx]);   
                idx++;
            }
        }
        // Otherwise Message Is Already Stored In The Content    
        else  if (compareVectorAndString(msg.content, "BYE\r\n")) {
            msg.type = COMMAND_BYE;

        }
        else {
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
    void parseMessage()
    {
    msg.displayNameOutside.clear();
    // Convert Vector To String
    std::string contentStr(msg.content.begin(), msg.content.end());
    std::regex msgInRegex("^IS");

        // Check if Content Is a Message 
        if (std::regex_search(contentStr, std::regex("^MSG FROM"))) 
        {
            size_t prefixLength = std::string("MSG FROM").length();
            msg.content.erase(msg.content.begin(), msg.content.begin() + prefixLength + 1); 
            
            size_t idx = 0;
            // Loop until the substring starting at the current idx does not start with "IN"
            // and ensure we're not at the last character
            while (idx < msg.content.size() - 1)
            {
                // Convert current substring to string for regex search
                std::string currentSubStr(msg.content.begin() + idx, msg.content.end());
                if (std::regex_search(currentSubStr, msgInRegex)) {
                    break; // Exit the loop if "IN" is found at the beginning of the current substring
                }
                
                msg.displayNameOutside.push_back(msg.content[idx]);
                idx++;
            }

            // Check and Remove the Last Character if Needed (It's a Space)
            if (!msg.displayNameOutside.empty()) {
                msg.displayNameOutside.pop_back();
            }

            /* Process The Content */
            int isPlusSpace = 3;                                                    // 1. Get Rid of "IS "
            msg.content.erase(msg.content.begin(), msg.content.begin() + idx + isPlusSpace);


            std::string messageContent(msg.content.begin(), msg.content.end());     // 2. Get Rid of "\r\n"
            if (msg.content.size() > 2 || messageContent == "\r\n") {
                msg.content.resize(msg.content.size() - 2);
            }
            else 
            {
                exit(1);
            }

            // Check The Message And User Name Length
            checkLength();
        }
    }
    /**
     * @brief Handles Reply From Server
     * 
     * @return 0 If The Reply Is OK, -1 If The Reply Is Not OK, -2 If Error Occurs 
    */
    int handleReply()
    {
        /* Preparation  */
        std::regex replyPrefix("^REPLY ");
        std::string contentAsStr(msg.content.begin(), msg.content.end());
        std::regex okReply("^OK ");
        size_t prefixLenght = (size_t)strlen("REPLY ");
        size_t okLenght = (size_t)strlen("OK ");

        /* Execution    */
        if (msg.content.size() >= prefixLenght && std::regex_search(contentAsStr,replyPrefix)) 
        {
            // delete first 5 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + prefixLenght);
            // Update Local String
            contentAsStr = std::string(msg.content.begin(), msg.content.begin());
        }

        if (compareVectorAndString(msg.content, "OK IS Authentication successful.\r\n")) 
        {
            msg.shouldReply = false;
            printf(" SUCCESS\n");
            return SUCCESS;
        }
        else if (std::regex_search(contentAsStr,okReply)) 
        {
            // Erase The Ok From The Message 
            msg.content.erase(msg.content.begin(), msg.content.begin() + okLenght);
            
            /* Prepaire Message Content -> Get Rid of '\n\r' */  
            contentAsStr = std::string(msg.content.begin(), msg.content.begin());
            if (msg.content.size() > 2 || contentAsStr == "\r\n") {
                msg.content.resize(msg.content.size() - 2);
            }         

            // Print Content 
            contentAsStr = std::string(msg.content.begin(), msg.content.begin());
            printf("Server: %s\n", contentAsStr.c_str());
            return SUCCESS;
        }
        else 
        {
            // Compare With Error Message Template
            std::string content(msg.content.begin(), msg.content.end());
            std::regex errorRegex("ˆNOK IS ");   
            if (std::regex_search(content,errorRegex))  
            {
                return AUTH_FAILED;
            }
            
        }
        return SUCCESS;
    }

};


#endif // BASE_MESSAGES_H