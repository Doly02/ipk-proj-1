/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      constants.hpp
 *  Author:         Tomas Dolak
 *  Date:           11.03.2024
 *  Description:    Defines Used Constants Such As Return Codes In Project.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           constatns.hpp
 *  @author         Tomas Dolak
 *  @date           11.03.2024
 *  @brief          Defines Used Constants Such As Return Codes In Project.
 * ****************************/
#include <iostream>

/*****************************************************/
/*                  Return Codes                     */
/*****************************************************/
static constexpr int    SUCCESS             = 0;    //!< Indicates That Everything Went Well
static constexpr int8_t CONFIRM_FAILED      = 0x55;
static constexpr int    OUT_OF_TIMEOUT      = 0x77;
static constexpr int    SERVER_SAYS_BYE     = 1;    //!< Indicates That Server Sent BYE Message
static constexpr int AUTHENTICATION_BYE     = 74;   //!< Indicates That User Send Bye During Authetication Process To Server
static constexpr int ALREADY_PROCESSED_MSG  = 2;   //!< Indicates That Message Was Already Processed
static constexpr int UNEXPECTED_MESSAGE     = 3;    //!< Indicates That Command Is Invalid

static constexpr int JUST_A_MESSAGE         = 50;   //!< Indicates That Received Message Is Just A Normal Message

static constexpr int FAIL                   = -1;   //!< Indicates That Operation FAILED During Run-Time
static constexpr int JOIN_FAILED            = -2;   //!< Indicates That An Error Occurred During The Processing of The Join Command
static constexpr int MSG_FAILED             = -3;   //!< Indicates That An Error Occurred During The Processing of The Standard Message
static constexpr int MSG_PARSE_FAILED       = -4;   //!< Indicates That Parsing of Incomming Message Failed
static constexpr int AUTH_FAILED            = -5;   //!< Indicates That Authentication Did Not Proceed Properly
static constexpr int EXTERNAL_ERROR         = -6;   //!< Indicates That An External Error Occurred
static constexpr int NON_VALID_PARAM        = -7;   //!< Indicates That String Contains Non-Alphanumeric Characters
static constexpr int NON_VALID_MSG_TYPE     = -8;   //!< Indicates That Command Is Invalid
/*****************************************************/
/*                  Message Limits                   */
/*****************************************************/
static constexpr int LENGHT_CHANNEL_ID      = 20;
static constexpr int LENGHT_USERNAME        = 20;
static constexpr int LENGHT_SECRET          = 128;
static constexpr int LENGHT_CONTENT         = 1400;
static constexpr int LENGHT_DISPLAY_NAME    = 20;


static constexpr int STDIN                  = 0;
static constexpr int SOCKET                 = 1;
static constexpr int NUM_FILE_DESCRIPTORS   = 2;
static constexpr int UNLIMITED_TIMEOUT      = -1;