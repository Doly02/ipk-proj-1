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
    static constexpr int SERVER_SAYS_BYE        = 1;
    static constexpr int SUCCESS                = 0;
    static constexpr int AUTH_FAILED            = -1;
    static constexpr int JOIN_FAILED            = -2;
    static constexpr int MSG_FAILED             = -3;
    static constexpr int MSG_PARSE_FAILED       = -4;
    static constexpr int FAIL                   = -5;
    static constexpr int EXTERNAL_ERROR         = -6;
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
        INPUT_UNKNOWN,    //!< Unknown Input
        INPUT_AUTH,       //!< Authentication - Template: AUTH {Username} USING {Secret}\r\n
        INPUT_JOIN,       //!< Join - Template: JOIN {ChannelID} AS {DisplayName}\r\n
        INPUT_RENAME,     //!< Rename - Template: RENAME {NewDisplayName}\r\n
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
        msg.buffer.clear();

        // Find The Lenght Of Buffer
        size_t len = strlen(buffer);

        for (size_t i = 0; i < len; i++)
        {
            if (buffer[i] != '\r' && buffer[i] != '\n')
            {
                msg.buffer.push_back(buffer[i]);
            }
        }
        msg.buffer.push_back('\r'); //TODO: Check If It's Needed
        msg.buffer.push_back('\n'); //TODO: Check If It's Needed

    }


    void readAndStoreBytes(const char* buffer, size_t bytesRx)
    {
        // Clear The Message Content
        msg.buffer.clear();
        printf("READING BYTES\n");
        for (size_t i = 0; i < bytesRx; i++)
        {
            
            msg.buffer.push_back(buffer[i]);

        }
    }

    /**
     * @brief Identifies Message Type & Parse Message Parts
     * 
     * 
     * @return 0 If The Message Is Valid, Otherwise Returns -1
     */
    int checkMessage()
    {
        size_t idx = 0;
        int retVal = FAIL;
        InputType_t inputType = INPUT_UNKNOWN;
        std::string bufferStr(msg.buffer.begin(),msg.buffer.end());

        /*                  INPUT TYPE RECOGNITION LOGIC                                */
        if (msg.buffer.size() >= 6 && std::regex_search(bufferStr, std::regex("^/auth"))) 
        {
            // delete first 5 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 6);
            inputType = INPUT_AUTH;
        }
        else if (msg.buffer.size() >= 6 && std::regex_search(bufferStr, std::regex("^/join"))) 
        {
            // delete first 5 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 6);
            inputType = INPUT_JOIN;
        }
        else if (msg.buffer.size() >= 7 && std::regex_search(bufferStr, std::regex("^/rename"))) 
        {
            // delete first 7 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 8);
            inputType = INPUT_RENAME;
        }
        else if (msg.buffer.size() >= 5 && std::regex_search(bufferStr, std::regex("^/help"))) 
        {
            // delete first 5 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + 6);
            inputType = INPUT_HELP;
        }
        else if (msg.buffer.size() >= 5 && std::regex_search(bufferStr, std::regex("^BYE\r\n"))) 
        {
            msg.type = COMMAND_BYE;
            return SUCCESS;
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
            //while (idx < msg.content.size()) 
            //{
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
        else if (inputType == INPUT_HELP) {
            //TODO: - Print Help Message
            msg.type = COMMAND_BYE;

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
        // Otherwise Message Is Already Stored In The Content    
        else if (compareVectorAndString(msg.buffer, "BYE\r\n")) 
        {
            idx = 0;
            while (idx < msg.buffer.size() && msg.buffer[idx] != '\n' && msg.buffer[idx] != '\r') 
            {
                msg.content.push_back(msg.buffer[idx]);   
                idx++;
            }            
            msg.type = COMMAND_BYE;

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
        printf("MSG TYPE: %d, retVal: %d (checkMessage)\n",msg.type,retVal);
        return retVal;   
    }


    /**
     * @brief Parse Messages From Incoming Packet
     * 
     * 
     * @return None, Exits If Something Goes Wrong
    */
    int parseMessage()
    {
        int retVal = 0;
        size_t idx = 0;

        // Prepaire Attributes
        msg.content.clear();
        msg.displayNameOutside.clear();

        // Convert Vector To String
        std::string bufferStr(msg.buffer.begin(), msg.buffer.end());
        std::regex msgIsRegex("^IS");

        // Check if Content Is a Message 
        if (std::regex_search(bufferStr, std::regex("^MSG FROM"))) 
        {
            size_t prefixLength = std::string("MSG FROM").length();
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + prefixLength + 1); 
            
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
            if (!msg.displayNameOutside.empty()) {
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
            std::string realContent(msg.content.begin(),msg.content.end());

            // Check The Message And User Name Length
            retVal = checkLength();
            msgType = MSG;
            return retVal;
        }
        else if (std::regex_search(bufferStr, std::regex("^BYE\r\n")))
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


    /**
     * @brief Handles Reply From Server
     * 
     * @return 0 If The Reply Is OK, -1 If The Reply Is Not OK, -2 If Error Occurs 
    */
    int handleReply()
    {
        /* Preparation  */
        std::regex replyPrefix("^REPLY ");
        std::string bufferAsStr(msg.buffer.begin(), msg.buffer.end());
        std::regex okReply("^OK IS ");
        std::regex errorPrefix("^ERR FROM Server IS ");
        
        std::string errorLenght     = "ERR FROM Server IS ";
        std::string prefixLenght    = "REPLY ";
        std::string okLenght        = "OK IS ";

        /* Execution    */
        if (msg.buffer.size() >= prefixLenght.length() && std::regex_search(bufferAsStr,replyPrefix)) 
        {
            printf("BEFORE PREFIX REMOVED\n");
            // delete first 5 characters + 1 space
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + prefixLenght.length());
            // Update Local String
            bufferAsStr = std::string(msg.buffer.begin(), msg.buffer.end());
            printf("Internal(1): %s\n", bufferAsStr.c_str());
            printf("PREFIX REMOVED\n");
        }

        if (compareVectorAndString(msg.buffer, "OK IS Authentication successful.\r\n")) 
        {
            msg.shouldReply = false;
            printf(" SUCCESS\n");
            return SUCCESS;
        }
        else if (std::regex_search(bufferAsStr,okReply)) 
        {
            // Erase The Ok From The Message 
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + okLenght.length());
            
            
            /* Prepaire Message buffer -> Get Rid of '\n\r' */  
            bufferAsStr = std::string(msg.buffer.begin(), msg.buffer.end());
            if (msg.buffer.size() > 2 || bufferAsStr == "\r\n") {
                msg.buffer.resize(msg.buffer.size() - 2);
            }         
            
            // Print buffer 
            bufferAsStr = std::string(msg.buffer.begin(), msg.buffer.end());
            printf("Server: %s\n", bufferAsStr.c_str());
            
            return SUCCESS;
        }

        /*  ERROR MESSAGE HANDLING  */
        else if (std::regex_search(bufferAsStr,errorPrefix))
        {
            msg.buffer.erase(msg.buffer.begin(), msg.buffer.begin() + errorLenght.length());
            bufferAsStr = std::string(msg.buffer.begin(), msg.buffer.end());
            std::cerr << bufferAsStr << std::endl;
            return AUTH_FAILED;
        }
        else 
        {
            // Compare With Error Message Template
            std::regex errorRegex("ˆNOK IS ");   
            if (std::regex_search(bufferAsStr,errorRegex))  
            {
                std::cerr << "Authentication Failed" << std::endl;
                return AUTH_FAILED;
            }
            
        }
        return AUTH_FAILED;
    }


    void printMessage()
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

    void printHelp()
    {
        printf("Commands:\n");
        printf("----------------------------------------------\n");
        printf("AUTHENTICATION CMD:  /auth [username] [password] [displayname]\n");
        printf("JOIN CMD:            /join [channel]\n");
        printf("RENAME CMD:          /rename [displayname]\n");
        printf("HELP CMD:            /help\n");
        printf("To Exit The Program Correctly, Type 'BYE' And Press ENTER\n");
    }


};


#endif // BASE_MESSAGES_H