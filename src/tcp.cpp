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
#include "client.cpp"
/************************************************/
/*                  Constants                   */
/************************************************/



/************************************************/
/*                  CLASS                       */
/************************************************/

class TcpClient : public Client {
private:
    fd_set readfds;
    int max_sd;
    struct timeval tv;
    bool sendAuth = false;
    bool checkReply = false;
    bool authConfirmed = false;
    const int BUFSIZE = 1536;
    char buf[1536];
    TcpMessages tcpMessage;

public:
    TcpClient(std::string addr, int port, uint protocol) : Client(addr, port, protocol) {
        // Constructor
    }

    ~TcpClient() {
        // Destructor
    }

    void checkAuthentication() 
    {
        int RetValue = 0;
        while (!authConfirmed) {
            // Get User's Authentication Message
            if (!sendAuth) {
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    size_t len = strlen(buf);
                    if (buf[len - 1] == '\n') {
                        buf[len - 1] = '\0';
                    }
                    tcpMessage.readAndStoreContent(buf);    // Store Content To Vector                    
                    RetValue = tcpMessage.checkMessage();   // Check Message
                    if (RetValue == 0) 
                    {
                        if ((int)TcpMessages::COMMAND_AUTH == tcpMessage.msg.type && !sendAuth)
                        {
                            // Sent To Server Authentication Message
                            tcpMessage.SendAuthMessage(sock);
                            sendAuth = true;
                            checkReply = true;
                        }
                    }
                }
            }

            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            max_sd = sock;

            // Set Timeout
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            // Waiting For An Activity
            int activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
            if ((activity < 0) && (errno != EINTR)) {
                exit(1);
            }

            if (FD_ISSET(sock, &readfds)) {
                memset(buf, 0, BUFSIZE);
                int bytesRx = recv(sock, buf, BUFSIZE - 1, 0);
                if (bytesRx > 0) {
                    buf[BUFSIZE - 1] = '\0';
                    tcpMessage.readAndStoreContent(buf);

                    if (checkReply) {
                        int retVal = tcpMessage.handleReply();
                        if (retVal == 0) 
                        {
                            printf("Handled Reply");
                            authConfirmed = true;
                            checkReply = false;
                            break;
                        } else {
                            // Reply Failed -> Exit
                            exit(TcpMessages::AUTH_FAILED); //TODO: Budes se uz muset podivat na ty navratove hodnoty, je v tom border
                        }
                    }
                } else if (bytesRx == 0) {
                    break; // Server closed the connection
                } else {
                    std::cerr << "recv failed" << std::endl;
                }
            }
        }
    }


    int runTcpClient()
    {
        int retVal = 0;
        bool joinSend = false;
        bool joinServerMsgSend = false;
        bool debugJoin = false;
        int RetValue = 0;
        /* Code */
        if (!Client::isConnected())
        {
            return NOT_CONNECTED;
        }

        // Set Socket To Non-blocking Mode
        fcntl(sock, F_SETFL, O_NONBLOCK);
        // Set Standard Input To Non-blocking Mode
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
        // First Handle Authentication
        checkAuthentication();


        while (true) 
        {
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            FD_SET(STDIN_FILENO, &readfds);
            max_sd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

            // Set Timeout
            tv.tv_sec = 0;
            tv.tv_usec = 500;

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
                memset(buf, 0, sizeof(buf));
                // Receive A Packet From Server
                int bytesRx = recv(sock, buf, BUFSIZE - 1, 0);
                if (bytesRx > 0) 
                {
                    //buf[bytesRx]    = '\0'; //TODO: Check If This Is Right ("\r\n")
                    buf[BUFSIZE-1]  = '\0';
                    tcpMessage.readAndStoreContent(buf);  
                    
                    // Print Content For Debug Information 
                    std::string contentFromServer(tcpMessage.msg.content.begin(),tcpMessage.msg.content.end());
                    printf("Receive Message: %s\n", contentFromServer.c_str());

                    /* Print The Message To STDOUT */
                    if (strcmp(buf, "BYE\r\n") == 0) 
                    {
                        printf("%s", buf);
                        tcpMessage.SentByeMessage(sock); // Should Send BYE Message Back To Server TODO: Do I Have To Send It Back?
                        exit(0);
                    }


                    // Check If Is Alles Gute
                    if (true == checkReply && true == joinSend)
                    {
                        if (!joinServerMsgSend)
                        {
                            retVal = tcpMessage.checkJoinReply();
                            if (BaseMessages::SUCCESS != retVal)
                            {
                                return retVal;
                            }
                        }

                        if (joinServerMsgSend)
                        {
                            retVal = tcpMessage.handleReply();
                            if (BaseMessages::SUCCESS != retVal)
                                return BaseMessages::JOIN_FAILED;
                            checkReply = false;
                            joinSend = false;
                            debugJoin = true; 
                        }
                        joinServerMsgSend = true;
                    }
                    else if (true == checkReply && false == joinSend) 
                    {
                        // Store The Content Of The Buffer Into Internal Vector
                        // tcpMessage.readAndStoreContent(buf);       
                        // Check If The Message Is REPLY
                        retVal = tcpMessage.handleReply();
                        if (retVal == 0)
                        {
                            std::string displayNameOutside(tcpMessage.msg.displayNameOutside.begin(), tcpMessage.msg.displayNameOutside.end());
                            std::string content(tcpMessage.msg.content.begin(), tcpMessage.msg.content.end());
                            printf("REPLY OK\n");
                            
                            checkReply = false;
                        }
                        else
                        {
                            std::cerr << "Error: " << strerror(errno) << std::endl;
                            return -1;
                        }
                    }
                    else
                    {
                        // Print The Content Of The Buffer
                        tcpMessage.parseMessage();
                        std::string displayNameOutside(tcpMessage.msg.displayNameOutside.begin(), tcpMessage.msg.displayNameOutside.end());
                        std::string content(tcpMessage.msg.content.begin(), tcpMessage.msg.content.end());
                        if (!displayNameOutside.empty() && !content.empty())
                        {
                            printf("%s: %s\n", displayNameOutside.c_str(), content.c_str());
                        }
                    }
                    
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
                    if (errno != EAGAIN && errno != EWOULDBLOCK) 
                    {
                        std::cerr << "recv failed: " << strerror(errno) << std::endl;
                    }
                }
            }
            if (debugJoin)
            {
                printf("Back To Main\n");
                debugJoin = false;
            }
            // Check Activity On STDIN
            if (FD_ISSET(STDIN_FILENO, &readfds)) 
            {
                memset(buf, 0, sizeof(buf));
                // Wait For User's Input
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    size_t len = strlen(buf);

#if defined (_WIN64) || defined (_WIN32) 
                    if (len > 1 && buffer[len - 2] == '\r' && buffer[len - 1] == '\n') 
                    {    // Windows
                        buffer[len - 2] = '\0';
                        buffer[len - 1] = '\0';
                    }
#else
                    if (buf[len - 1] == '\n') 
                    {                                             // Unix
                        buf[len - 1] = '\0';
                    }
#endif

                    tcpMessage.msgType = TcpMessages::UNKNOWN_MSG_TYPE;
                    tcpMessage.readAndStoreContent(buf);    // Store Content To Vector                    
                    RetValue = tcpMessage.checkMessage();   // Check Message
                    if (RetValue == 0) 
                    {
                        if ((int)TcpMessages::COMMAND_JOIN == tcpMessage.msg.type && sendAuth)
                        {
                            // Sent To Server Join Message
                            printf("JOIN MESSAGE WILL BE SEND\n");
                            tcpMessage.SendJoinMessage(sock);
                            printf("JOIN MESSAGE WILL WAS SEND\n");
                            joinSend = true;
                            checkReply = true;
                        }
                        if (debugJoin)
                        {
                            printf("Back To Main2\n");
                            debugJoin = false;
                        }
                        if ((int)TcpMessages::COMMAND_BYE == tcpMessage.msg.type )
                        {
                            // Sent To Server Bye Message
                            tcpMessage.SentByeMessage(sock);
                            exit(EXIT_SUCCESS);
                        }
                        // TODO: Missing Some Message Types
                        if ((int)TcpMessages::MSG == tcpMessage.msg.type)
                        {
                            // Sent User's Message To Server
                            tcpMessage.SentUsersMessage(sock); 
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
