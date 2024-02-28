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
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
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

        /* Variables */
        fd_set readfds;
        int max_sd;
        struct timeval tv;
        bool sendAuth = false;
        bool checkReply = false;
        const int BUFSIZE = 1536;  
        char buf[BUFSIZE];
        int RetValue = -1;
        TcpMessages tcpMessageIncoming; 
        TcpMessages tcpMessageOutcoming;     
        /* Code */
        if (!connectServer())
        {
            return NOT_CONNECTED;
        }


        // Set Socket To Non-blocking Mode
        fcntl(sock, F_SETFL, O_NONBLOCK);
        // Set Standard Input To Non-blocking Mode
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

        while (true) 
        {

            TcpMessages tcpMessage;     
            tcpMessage.readAndStoreContent(buf);  

            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            FD_SET(STDIN_FILENO, &readfds);
            max_sd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

            // Set Timeout
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            // Waiting For An Activity
            int activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
            if ((activity < 0) && (errno != EINTR)) 
            {
                std::cerr << "Select error" << std::endl;
                break;
            }

            // Check Socket's Activity
            if (FD_ISSET(sock, &readfds)) 
            {
                tcpMessageOutcoming.readAndStoreContent(buf);  
                // Receive A Packet From Server
                int bytesRx = recv(sock, buf, BUFSIZE - 1, 0);
                if (bytesRx > 0) 
                {
                    buf[bytesRx] = '\0'; //TODO: Check If This Is Right ("\r\n")

                    /* Print The Message To STDOUT */
                    if (strcmp(buf, "BYE\r\n") == 0) 
                    {
                        printf("%s", buf);
                        tcpMessageOutcoming.SentByeMessage(sock); // Should Send BYE Message Back To Server TODO: Do I Have To Send It Back?
                        exit(0);
                    }
                    // Check If Is Alles Gute
                    if (true == checkReply)
                    {
                        // Store The Content Of The Buffer Into Internal Vector
                        tcpMessageOutcoming.readAndStoreContent(buf);       
                        // Check If The Message Is REPLY
                        int retVal = tcpMessageOutcoming.handleReply();
                        if (retVal == 0)
                        {
                            std::string displayNameOutside(tcpMessageOutcoming.msg.displayNameOutside.begin(), tcpMessageOutcoming.msg.displayNameOutside.end());
                            std::string content(tcpMessageOutcoming.msg.content.begin(), tcpMessageOutcoming.msg.content.end());
                            //printf("%s: %s", displayNameOutside.c_str(), content.c_str());
                            printf("REPLY OK\n");
                            
                            checkReply = false;
                        }
                        else
                        {
                            std::cerr << "Error: " << strerror(errno) << std::endl;
                            return -1;
                        }
                    }

                    // Print The Content Of The Buffer
                    //printf("%s", buf);

                    // Clear The Buffer After All
                    memset(buf, 0, sizeof(buf));
                } 
                else if (bytesRx == 0) 
                {
                    /* Server Closed The Connection */
                    break;
                } 
                else 
                {
                    std::cerr << "recv failed" << std::endl;
                }
            }
        
            // Check Activity On STDIN
            if (FD_ISSET(STDIN_FILENO, &readfds)) 
            {
                // Wait For User's Input
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    //printf("STDIN\n");
                    size_t len = strlen(buf);

#if defined (_WIN64) || defined (_WIN32) 
                    if (len > 1 && buffer[len - 2] == '\r' && buffer[len - 1] == '\n') 
                    {    // Windows
                        buffer[len - 2] = '\0';
                    }
#else
                    if (buf[len - 1] == '\n') 
                    {                                             // Unix
                        buf[len - 1] = '\0';
                    }
#endif

                        
                    tcpMessageIncoming.readAndStoreContent(buf);    // Store Content To Vector
                    RetValue = tcpMessageIncoming.checkMessage();   // Check Message
                    if (RetValue == 0) 
                    {

                        if ((int)TcpMessages::COMMAND_AUTH == tcpMessageIncoming.msg.type && !sendAuth)
                        {
                            // Sent To Server Authentication Message
                            printf("Sending AUTH Message: %s\n", buf);
                            tcpMessageIncoming.SendAuthMessage(sock);
                            sendAuth = true;
                            checkReply = true;
                            
                        }
        
                        if ((int)TcpMessages::COMMAND_JOIN == tcpMessageIncoming.msg.type)
                        {
                            // Sent To Server Join Message
                            tcpMessageIncoming.SendJoinMessage(sock);
                        }

                        if ((int)TcpMessages::BYE == tcpMessageIncoming.msg.type)
                        {
                            // Sent To Server Bye Message
                            tcpMessageIncoming.SentByeMessage(sock);
                            break;
                        }
                        // TODO: Missing Some Message Types
                        if ((int)TcpMessages::MSG == tcpMessageIncoming.msg.type)
                        {
                            // Sent User's Message To Server
                            tcpMessageIncoming.SentUsersMessage(sock); 
                            memset(buf, 0, sizeof(buf));
                        }

                    }
                }
            }

        }
        return 0;
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

};
