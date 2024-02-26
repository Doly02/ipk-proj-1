/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      tcp.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Implements Communication With Chat Server Thru TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Implements Communication With Chat Server Thru TCP Protocol.
 * ****************************/



/************************************************/
/*                  Libraries                   */
/************************************************/
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio> 
#include "tcp_messages.cpp"
/************************************************/
/*                  Constants                   */
/************************************************/



/************************************************/
/*                  CLASS                       */
/************************************************/
class TcpClient
{
private:
    int sock;                 //!<  File Descriptor of The Socket Used For Communication
    std::string serverAddress;  //!< IP Address of The Server
    int port;                   //!< Port Number on Which The Server Is Listening
    struct sockaddr_in server;  //!< Structure Containing Server's Address Information

public:
    static constexpr int NOT_CONNECTED = -1;
    /**
     * @brief Constructor of TcpClient Class 
     * @param addr Server's Address 
     * @param port Server's Port
     *
     * Constructor Initialize Client With Server's Address And Port.
     * Default State of Socket Is Set To NOT_CONNECTED.
     */
    TcpClient(std::string addr, int port) : serverAddress(addr), port(port), sock(NOT_CONNECTED) {}

    /**
     * @brief Destructor of TcpClient Class 
     * 
     * Constructor Initialize Client With Server's Address And Port.
     * Default State of Socket Is Set To NOT_CONNECTED.
     */
    ~TcpClient() 
    {
        if (isConnected()) {
            close(sock);
        }
    }
    /**
     * @brief Determine If The Client Is Connected To The Server
     * @return True If The Client Is Connected, False Otherwise
     */
    bool isConnected() {
        return sock != NOT_CONNECTED;
    }

    /**
     * @brief Connects The Client To The Server
     * @return True If The Connection Was Successful, False Otherwise
     */
    bool connectServer()
    {
        if (isConnected())
        {
            return true;
        }

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (NOT_CONNECTED == sock)
        {
            std::cerr << "Not Possible To Create Socket: " << strerror(errno) << std::endl;
            return false;
        }

        // Set Server's Address Information
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        if (inet_pton(AF_INET, serverAddress.c_str(), &server.sin_addr) <= 0)
        {
            std::cerr << "Invalid Address: " << serverAddress << std::endl;
            return false;
        }

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            std::cerr << "Connection Failed: " << strerror(errno) << std::endl;
            return false;
        }

        // Connection Was Successful
        return true;
    }


    int runTcpClient()
    {

        if (!connectServer())
        {
            return NOT_CONNECTED;
        }

        // Two-Way Handshake

        bool sendAuth = false;
        const int BUFSIZE = 1024;
        char buf[BUFSIZE];
        int RetValue = -1;

        while (fgets(buf, BUFSIZE, stdin)) {
            // Check If Loaded Message Includes if "\r\n"
            if (strstr(buf, "\r\n")) {
                // Vytvoření obsahu zprávy jako std::vector<char>
                std::vector<char> contentStdIn(buf, buf + strlen(buf));

                //TODO: Create New Instance -> Should Be Deleted After Use
                TcpMessages::Message_t messageContent;
                messageContent.type = TcpMessages::UNKNOWN_MSG_TYPE;    // Default Message Type
                messageContent.content = contentStdIn;                       // Content Of The Message -> Content of Buffer
                TcpMessages tcpMessage(TcpMessages::UNKNOWN_MSG_TYPE, messageContent);

                RetValue = tcpMessage.checkMessage();
                if (RetValue == 0) {
                    
                    if ((int)TcpMessages::AUTH_COMMAND == tcpMessage.msg.type)
                    {
                        // Sent To Server Authentication Message
                        tcpMessage.SendAuthMessage(sock);
                        sendAuth = true;
                    }

                    if ((int)TcpMessages::BYE == tcpMessage.msg.type)
                    {
                        // Sent To Server Bye Message
                        tcpMessage.SentByeMessage(sock,buf);
                        break;
                    }
                    
                }


            }
        return 0;
        }
    }
};



// NOTES:
/* Melo by fungovat odesilani zpravy AUTH 
 */
/* Potreba dodelat -> Momentalne se pracuje se dvemi buffery je treba pouzit jen jeden
 *   -> pouziva se char Buf[1024] a <vector> Content -> Nejlepsi bude pouzivat Pouze Content a kdyz budes potrebovat tak si to prekonvertujes na Buf "ale jakoby pouze virtualne"
 */
/* Co je potreba dodelat?
 * - Osetrit chyby pri odesilani zprav
 * - Osetrit chyby pri prijimani zprav
 * - Dopracovat odesilani vsech moznych zprav v TCP 
 * - Rozpracovat a dodelat UDP
 * - Testovat Testovat
 */


