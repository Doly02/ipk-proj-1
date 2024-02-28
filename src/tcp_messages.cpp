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
#include <vector>
#include <iostream>
#include <regex>
#include <sys/socket.h>
/************************************************/
/*                  Class                       */
/************************************************/
class TcpMessages 
{
    public:
    static constexpr int LENGHT_ID              = 20;
    static constexpr int LENGHT_SECRET          = 20;
    static constexpr int LENGHT_CONTENT         = 1400;
    static constexpr int LENGHT_DISPLAY_NAME    = 128;
    /* MESSAGE TYPES */
    enum MessageType_t
    {
        UNKNOWN_MSG_TYPE, //!< Unknown Message Type
        ERROR,      //!< Error - Template: ERROR FROM {DisplayName} IS {MessageContent}\r\n
        REPLY,      //!< Reply - Template: REPLY IS {MessageContent}\r\n
        AUTH,       //!< Authentication - Template: AUTH {Username} USING {Secret}\r\n
        JOIN,       //!< Join - Template: JOIN {ChannelID} AS {DisplayName}\r\n
        MSG,        //!< Message - Template: MSG FROM {DisplayName} IS {MessageContent}\r\n
        BYE,        //!< Disconnect - Template: BYE\r\n
        COMMAND_AUTH,
        COMMAND_JOIN,
        COMMAND_MSG,
        COMMAND_BYE

    };
    enum InputType_t
    {
        UNKNOWN,            //!< Unknown Input
        AUTH_COMMAND,       //!< Authentication - Template: AUTH {Username} USING {Secret}\r\n
        JOIN_COMMAND,       //!< Join - Template: JOIN {ChannelID} AS {DisplayName}\r\n
        RENAME_COMMAND,      //!< Rename - Template: RENAME {NewDisplayName}\r\n
        INPUT_MSG,        //!< Message - Template: MSG FROM {DisplayName} IS {MessageContent}\r\n
        HELP_COMMAND        //!< Disconnect - Template: BYE\r\n
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
    };
    

    MessageType_t msgType;
    Message_t msg;

    TcpMessages() : msgType(UNKNOWN_MSG_TYPE) {
        // Zde můžete inicializovat výchozí hodnoty, pokud je to potřeba
    }
    /**
     * @brief Constructor of TcpMessages Class 
     * @param type Type of The Message
     * @param content Content of The Message
     *
     * Constructor Initialize Message With Type And Content.
     */
    TcpMessages(MessageType_t type, Message_t content) : msgType(type), msg(content) {}

    /**
     * @brief Destructor of TcpMessages Class 
     * 
     * Destructor of TcpMessages Class.
     */
    ~TcpMessages() {}


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
     * @brief Identifies Message Type & Parse Message Parts
     * 
     * 
     * @return 0 If The Message Is Valid, Otherwise Returns -1
     */
    int checkMessage()
    {
        int retVal = -1;
        InputType_t inputType = UNKNOWN;
        if (msg.content.size() >= 5 && msg.content[0] == '/' && msg.content[1] == 'a' && msg.content[2] == 'u' 
        && msg.content[3] == 't' && msg.content[4] == 'h') {
            // delete first 5 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 6);
            inputType = AUTH_COMMAND;
        }
        else if (msg.content.size() >= 4 && msg.content[0] == '/' && msg.content[1] == 'j' && msg.content[2] == 'o'
        && msg.content[3] == 'i' && msg.content[4] == 'n') {
            // delete first 5 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 6);
            inputType = JOIN_COMMAND;
        }
        else if (msg.content.size() >= 4 && msg.content[0] == '/' && msg.content[1] == 'r' && msg.content[2] == 'e'
        && msg.content[3] == 'n' && msg.content[4] == 'a' && msg.content[5] == 'm' && msg.content[6] == 'e') {
            // delete first 7 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 8);
            inputType = RENAME_COMMAND;
        }
        else if (msg.content.size() >= 3 && msg.content[0] == '/' && msg.content[1] == 'h' && msg.content[2] == 'e'
        && msg.content[3] == 'l' && msg.content[4] == 'p') {
            // delete first 5 characters + 1 space
            msg.content.erase(msg.content.begin(), msg.content.begin() + 6);
            inputType = HELP_COMMAND;
        }
        else 
        {
            inputType = INPUT_MSG;
        }

        if (inputType == UNKNOWN) {
            return -1;
        }
        else if (inputType == AUTH_COMMAND) {
            size_t index = 0;
            // Process Username
            while (index < msg.content.size() && msg.content[index] != ' ')
            {
                msg.login.push_back(msg.content[index]);   
                index++;
            }
            if (index < msg.content.size()) {  
                // Clear The Username And Space 
                msg.content.erase(msg.content.begin(), msg.content.begin() + index + 1);
            }    
#if (DEBUG_MACRO == 1)      
            printf("Username: %s\n", std::string(msg.login.begin(), msg.login.end()).c_str());
#endif
            // Process Secret
            index = 0;
            while (index < msg.content.size() && msg.content[index] != ' ') {
                msg.secret.push_back(msg.content[index]);   
                index++;
            }
            if (index < msg.content.size()) {  
                // Clear The Secret And Space 
                msg.content.erase(msg.content.begin(), msg.content.begin() + index + 1);
            }
#if (DEBUG_MACRO == 1)
            printf("Secret: %s\n", std::string(msg.secret.begin(), msg.secret.end()).c_str());
#endif
            // Process Display Name
            index = 0; // Reset index if you're using it to iterate through the remaining content
            msg.displayName.clear(); // Ensure displayName is empty before filling
            while (index < msg.content.size()) {
                msg.displayName.push_back(msg.content[index]);   
                index++;
            }
#if (DEBUG_MACRO == 1)
            printf("Display Name: %s\n", std::string(msg.displayName.begin(), msg.displayName.end()).c_str());
#endif
            msg.type = COMMAND_AUTH;

        }
        else if (inputType == JOIN_COMMAND) {
            size_t index = 0;
            // Process Channel ID
            // while (index < msg.content.size() && (msg.content[index] != EOF || msg.content[index] != '\n'))
            while (index < msg.content.size() && msg.content[index] != '\n' && msg.content[index] != '\r') 
            {
                msg.channelID.push_back(msg.content[index]);   
                index++;
            }
            msg.type = COMMAND_JOIN;

        }
        else if (inputType == RENAME_COMMAND) {
            size_t index = 0;
            // Process New Display Name
            while (index < msg.content.size() && msg.content[index] != '\n' && msg.content[index] != '\r')  // '\n' should not be in content
            {
                msg.displayName.push_back(msg.content[index]);   
                index++;
            }
            msg.type = COMMAND_MSG;

        }
        else if (inputType == HELP_COMMAND) {
            //TODO: - Print Help Message
            msg.type = COMMAND_BYE;

        }
        // Otherwise Message Is Already Stored In The Content    
        else  if (compareVectorAndString(msg.content, "BYE")) {
            msg.type = BYE;

        }
        else {
            printf("MSG\n");
            msg.type = MSG;

        }
        retVal = checkLength();
        return retVal;   
    }

};