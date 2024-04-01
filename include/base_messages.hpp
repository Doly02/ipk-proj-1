/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      base_messages.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Header File For Messages Base Class. 
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           base_messages.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Header File For Messages Base Class. 
 * ****************************/

#ifndef BASE_MESSAGES_HPP
#define BASE_MESSAGES_HPP

#include "macros.hpp"
#include "strings.hpp"
#include <string>
#include <vector>
#include <cstdint>

class BaseMessages {
public:
    enum MessageType_t : uint8_t 
    {
        CONFIRM             = 0x00,
        REPLY               = 0x01,
        COMMAND_AUTH        = 0x02,
        COMMAND_JOIN        = 0x03,
        MSG                 = 0x04,
        COMMAND_HELP        = 0x05,
        COMMAND_RENAME      = 0x06,
        ERROR               = 0xFE,
        COMMAND_BYE         = 0xFF,
        UNKNOWN_MSG_TYPE    = 0x99,
    };

    enum InputType_t 
    {
        INPUT_UNKNOWN,
        INPUT_AUTH,
        INPUT_JOIN,
        INPUT_RENAME,
        INPUT_MSG,
        INPUT_HELP
    };

    struct Message_t 
    {
        MessageType_t type;
        std::vector<char> content;
        bool isCommand;
        std::vector<char> login;
        std::vector<char> secret;
        std::vector<char> displayName;
        std::vector<char> channelID;
        std::vector<char> displayNameOutside;
        std::vector<char> buffer;
    };

    MessageType_t msgType;
    Message_t msg;


    BaseMessages();
    /**
     * @brief Constructor of TcpMessages Class 
     * @param type Type of The Message
     * @param content Content of The Message
     *
     * Constructor Initialize Message With Type And Content.
     */
    BaseMessages(MessageType_t type, Message_t content);
    /**
     * @brief Destructor of TcpMessages Class 
     * 
     * Destructor of TcpMessages Class.
     */ 
    ~BaseMessages();
    /**
     * @brief Inserts Error Message To The Content
     * @param inputString Error Message
     * 
     * Inserts Error Message To The Content Of The Message.
    */
    void insertErrorMsgToContent(const std::string& inputString);
    /**
     * @brief Creates Message
    */    
    void cleanMessage();
    /**
     * @brief Check If The Message Components Are Valid (ID, Display Name, Content, Secret Length)
     * 
     * @return SUCCESS If The Message Is Valid, Otherwise Returns NON_VALID_PARAM
     */    
    int checkLength();
    /**
     * @brief Store String Into Vector Buffer
     * @param buffer String To Be Stored
     * 
     * Store String Into Vector Buffer.
    */    
    void readAndStoreContent(const char* buffer);
    /**
     * @brief Store Incomming Bytes Into Vector Buffer
     * @param buffer Character Buffer
     * @param bytesRx Number of Bytes To Be Stored
     * 
     * Store String Into Vector Buffer.
    */    
    void readAndStoreBytes(const char* buffer, size_t bytesRx);
    /**
     * @brief Check If The Message Is Valid
     * @return 0 If The Message Is Valid, -1 Otherwise
     * 
     * Check If The Message Is Valid.
    */    
    int checkMessage();
    /**
     * @brief Parse Message
     * @return 0 If The Message Is Valid, -1 Otherwise
     * 
     * Parse Message.
    */    
    int parseMessage();
    /**
     * @brief Prints Message to The Standard Output
    */    
    void printMessage();
    /**
     * @brief Prints Servers Reply to The Standard Output
    */    
    void PrintServerOkReply();
  
    void PrintServerNokReply();
    /**
     * @brief Prints External Error Message to The Standard Output
    */      
    void basePrintExternalError();
    /**
     * @brief Prints Internal Error Message to The Standard Output
     * @param retVal Return Code
    */    
    void basePrintInternalError(int retVal);
    
    void printHelp();
};

#endif // BASE_MESSAGES_HPP
