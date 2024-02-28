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
    TcpClient(std::string addr, int port) : sock(NOT_CONNECTED), serverAddress(addr), port(port) {}
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

    void updateServerAddress(const std::string& newAddress) {
        serverAddress = newAddress;
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
            printf("SRV ADDR: %s\n", serverAddress.c_str());
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
        bool checkReply = false;
        int bytesRx = 0;
        const int BUFSIZE = 1024;
        char buf[BUFSIZE];
        int RetValue = -1;

        while (fgets(buf, BUFSIZE, stdin)) {

            size_t len = strlen(buf);
#if defined (_WIN64) || defined (_WIN32) 
            if (len > 1 && buffer[len - 2] == '\r' && buffer[len - 1] == '\n') {    // Windows
                buffer[len - 2] = '\0';
            }
#else
            if (buf[len - 1] == '\n') {                                             // Unix
                buf[len - 1] = '\0';
            }
#endif
            // Vytvoření obsahu zprávy jako std::vector<char>
            std::vector<char> contentStdIn(buf, buf + strlen(buf));

            //TODO: Create New Instance -> Should Be Deleted After Use
            TcpMessages tcpMessage;     
            tcpMessage.readAndStoreContent(buf);                       // Content Of The Message -> Content of Buffer

            RetValue = tcpMessage.checkMessage();
            if (RetValue == 0) {

                if ((int)TcpMessages::COMMAND_AUTH == tcpMessage.msg.type && !sendAuth)
                {
                    // Sent To Server Authentication Message
                    printf("Sending AUTH Message: %s\n", buf);
                    tcpMessage.SendAuthMessage(sock);
                    sendAuth = true;
                    checkReply = true;
                    
                }

                if ((int)TcpMessages::COMMAND_JOIN == tcpMessage.msg.type)
                {
                    // Sent To Server Join Message
                    tcpMessage.SendJoinMessage(sock);
                }

                if ((int)TcpMessages::BYE == tcpMessage.msg.type)
                {
                    // Sent To Server Bye Message
                    tcpMessage.SentByeMessage(sock,buf);
                }
                // TODO: Missing Some Message Types
                if ((int)TcpMessages::MSG == tcpMessage.msg.type)
                {
                    // Sent User's Message To Server
                    tcpMessage.SentUsersMessage(sock); 
                }

                // Clear The Content Of The Buffer
                memset(buf, 0, sizeof(buf));


                bytesRx = recv(sock, buf, BUFSIZE, 0);
                if (bytesRx < 0) 
                {
                    std::perror("ERROR: recvfrom");
                }
                else {
                    std::cout << buf;
                }

                if (strcmp(buf, "BYE\r\n") == 0) {
                    printf("%s", buf);
                    tcpMessage.SentByeMessage(sock,buf); // Should Send BYE Message Back To Server
                    exit(0);
                }
                // Check If Is Alles Gute
                if (true == checkReply)
                {
                    // Store The Content Of The Buffer Into Internal Vector
                    tcpMessage.readAndStoreContent(buf);       
                    // Check If The Message Is REPLY
                    int retVal = tcpMessage.handleReply();
                    if (retVal == 0)
                    {
                        std::string displayNameOutside(tcpMessage.msg.displayNameOutside.begin(), tcpMessage.msg.displayNameOutside.end());
                        std::string content(tcpMessage.msg.content.begin(), tcpMessage.msg.content.end());
                        //printf("%s: %s", displayNameOutside.c_str(), content.c_str());
                        printf("REPLY OK\n");
                        
                        checkReply = false;
                    }
                    else{
                        std::cerr << "Error: " << strerror(errno) << std::endl;
                        return -1;
                    }
                }
                // Print The Content Of The Buffer
                //printf("%s", buf);
            }
            
        }
        return 0;
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


