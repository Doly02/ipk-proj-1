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


};