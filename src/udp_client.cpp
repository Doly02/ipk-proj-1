/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      udp.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Implements Communication With Chat Server Thru UDP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           udp.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Implements Communication With Chat Server Thru UDP Protocol.
 * ****************************/

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

/************************************************/
/*                  Libraries                   */
/************************************************/
#include <iostream>
#include <string>
#include <csignal>          // For Signal Handling
#include <unistd.h>         // For close
#include <unordered_set>
#include <chrono>
#include "udp_messages.cpp"
#include "base_client.cpp"
/*************************************************/
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Milliseconds = std::chrono::milliseconds;


/************************************************/
/*                  Constants                   */
/************************************************/
class UdpClient : public Client {

private: 
    static constexpr int AUTHENTICATION_BYE        = 74;
    fd_set readfds;
    const int BUFSIZE = 1536;
    char buf[1536];
    int retryCount = 3;
    int confirmationTimeout;
    bool sendAuth = false;
    bool receivedConfirm = false;
    UdpMessages udpMessage;
    struct sockaddr_in si_other;    // Struct For Storing Server (Sender) Address
    struct sockaddr_in newServerAddr;

    int lastSentMessageID       = 0;                // ID Last Sended Message  
    int lastReceivedMessageID   = 0;                // ID Last Received Message
    std::unordered_set<int> receivedMessageIDs;     // Watchdog For Unique Received ID Messages


public:
    UdpClient(const std::string& addr, int port,int retryCnt,int confirmTimeOut) : Client(addr, port, UDP) // Inicialization By Contructor From Base Class
    {
        retryCount = retryCnt;
        confirmationTimeout = confirmTimeOut;
        newServerAddr = server;
    }

    virtual ~UdpClient() {
        // Destructor From Base Class [Calls close(sock)]
    }

    void udpHandleInterrupt(int signal)
    {
        if (SIGINT == signal || SIGTERM == signal)
        {
            int retVal = 0;
            udpMessage.msg.type = UdpMessages::COMMAND_BYE;
            /* Send Message */
            udpMessage.sendUdpMessage(sock,newServerAddr);
            printf("DEBUG INFO: BYE MESSAGE SENT\n");
            /* Receive CONFIRM */
            socklen_t slen = sizeof(si_other);
            ssize_t bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
            if (-1 == bytesRx) 
            {
                // Chyba při příjmu dat
                perror("WARNING: recvfrom() failed"); // FIXME
                exit(EXIT_FAILURE);
            }
            newServerAddr = si_other;   //TODO  Needs To Be Here?
            udpMessage.readAndStoreBytes(buf,bytesRx);
            retVal = udpMessage.recvUpdConfirm(lastSentMessageID);
            if (SUCCESS != retVal)
            {
                printf("DEBUG INFO: CONFIRMATION RECEIVED\n");
                // FIXME Add ERROR STRING
                exit(retVal);
            }
            exit(SUCCESS);
        }

    }



    int processAuthetification() 
    {
        /* Variables */
        int retVal = 0;
        int currentRetries = 0;             //!< Actual Number Of Retries
        bool checkReply = false;            //!< Indicates Whether It is Expected Reply Message
        const struct sockaddr_in& serverAddr = getServerAddr();

        /* Timers */
        TimePoint startWatch;               //!< Contains the Initial Measurement Time
        TimePoint stopWatch;                //!< Contains the Final Measurement Time
        bool measureTime = false;           //!< Indicates That Time Should Be Measured
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        timeout.tv_sec = 0; 
        timeout.tv_usec = 250000;   // 250ms 

        while (currentRetries < retryCount) 
        {   
            int activity = select(sock + 1, &readfds, NULL, NULL, NULL); 
            if (activity == -1) {
                perror("select() failed"); // FIXME
                exit(EXIT_FAILURE);
            }
            else if (activity > 0)
            {

                // Capture Activity on STDIN
                if (FD_ISSET(STDIN_FILENO, &readfds)) 
                {
                    if (fgets(buf, BUFSIZE, stdin) != NULL) 
                    {
                        
                        size_t len = strlen(buf);
                        if (buf[len - 1] == '\n') 
                        {
                            buf[len - 1] = '\0';
                        }

                        // Store The Input Into Internal Buffer
                        udpMessage.readAndStoreContent(buf);
                        // Check The Message 
                        retVal = udpMessage.checkMessage();
                        printf("DEBUG INFO: MSG TYPE: %d\n",udpMessage.msg.type);
                        if (retVal == 0 && udpMessage.msg.type == UdpMessages::COMMAND_AUTH) 
                        {
                            udpMessage.sendUdpAuthMessage(sock,serverAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            lastSentMessageID = udpMessage.messageID;
                            sendAuth = true;
                            measureTime = true;
                            checkReply = true;
                            currentRetries++;
                        }
                        else if (retVal == 0 && udpMessage.msg.type == UdpMessages::COMMAND_BYE)
                        {
                            return SERVER_SAYS_BYE;
                        }
                        else if (retVal == 0 && udpMessage.msg.type == UdpMessages::COMMAND_HELP)
                        {
                            udpMessage.printHelp();
                        }
                        else
                        {
                            // TODO: Treba osetrit co vsechno muze byt spatne (fce checkMessage)
                            return retVal;
                        }
                    }   
                }

                // Check Activity On Socket
                if (FD_ISSET(sock, &readfds)) 
                {
                    memset(buf, 0, BUFSIZE);
                    socklen_t slen = sizeof(si_other);
                    ssize_t bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                    if (bytesRx == -1) {
                        // Zkontrolovat, zda došlo k chybě jiné než EWOULDBLOCK/EAGAIN
                        if (errno != EWOULDBLOCK && errno != EAGAIN) {
                            perror("recvfrom() failed");
                            // Ošetřit chybu
                        }
                        break; // Žádná další data k dispozici
                    }
                    else 
                    {   
                        newServerAddr = si_other;  
                        buf[BUFSIZE - 1] = '\0'; 
                        udpMessage.readAndStoreBytes(buf,bytesRx);
                        retVal = udpMessage.recvUpdConfirm(lastSentMessageID);
                        if (!receivedConfirm && retVal == SUCCESS) 
                        {
                            receivedConfirm = true;
                            measureTime = false;
                            
                        }
                        else if (checkReply && receivedConfirm) 
                        {

                            retVal = udpMessage.recvUpdIncomingReply(lastSentMessageID);
                            if (SUCCESS == retVal)
                            {
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                receivedConfirm = true;
                                checkReply = false;
                                break;

                            }
                            if (EXTERNAL_ERROR == retVal)
                            {
                                exit(EXTERNAL_ERROR);
                            }
                            // TODO Finish The Program?
                        }
                    } 

                } 
            }
            // Check The Receive TimeOut
            if (measureTime)
            {
                stopWatch = std::chrono::high_resolution_clock::now();
                int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(stopWatch - startWatch).count();
                if (elapsedTime > confirmationTimeout) 
                {
                    udpMessage.sendUdpAuthMessage(sock,newServerAddr);
                    currentRetries++;
                }
            }

            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(sock, &readfds);
        }

        /* Prepare Objects For Main Loop*/
        udpMessage.incrementUdpMsgId();
        

        if (currentRetries >= retryCount) 
        {
            // Attempts Overrun
            return AUTH_FAILED;
        }
        return SUCCESS;
    }

    int runUdpClient()
    {
        /*** Variables ***/
        int currentRetries      = 0;
        int retVal              = 0;
        bool expectedConfirm    = false;
        bool lastMessage        = false;

        /* Timers */
        TimePoint startWatch;
        TimePoint stopWatch;
        struct timeval timeout;
        timeout.tv_sec          = 0; 
        timeout.tv_usec         = 250000;   // 250ms

        const struct sockaddr_in& serverAddr = getServerAddr();

        /*** Code ***/
        if (!Client::isConnected())
        {
            return NOT_CONNECTED;
        }

        udpMessage.setUdpMsgId();

        /* Process Authentication */
        retVal = processAuthetification();
        if (SERVER_SAYS_BYE == retVal)
        {
            // Finish The Program 
            return SUCCESS;;
        }
        else if (SUCCESS != retVal)
        {
            printf("DEBUG INFO: AUTHENTICATION FAILED (return code: %d)\n",retVal);
            return retVal;
        }
        printf("DEBUG INFO: AUTHENTICATION DONE (runUdpClient)\n");
        printf("------------------------------------------------\n");

        /* Main Loop */
        while (currentRetries < retryCount)
        {
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(sock, &readfds);

            // Nastavení timeout pro select
            timeout.tv_sec = 0; // 0 sekund
            timeout.tv_usec = 250000; // 250 milisekund
            int activity = select(sock + 1, &readfds, NULL, NULL, NULL); 
            if (-1 == activity) 
            { 
                perror("WARNING: Select error"); // FIXME
                exit(EXIT_FAILURE);
            }
            // Capture Activity on STDIN
            if (FD_ISSET(STDIN_FILENO, &readfds)) 
            {
                //memset(buf, 0, BUFSIZE);
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    // Store Input From STDIN To Vector
                    udpMessage.readAndStoreContent(buf);
                    // Check Message Validity
                    size_t len = strlen(buf);
                    if (buf[len - 1] == '\n') 
                    {
                        buf[len - 1] = '\0';
                    }

                    retVal = udpMessage.checkMessage();
                    if(SUCCESS == retVal)
                    {
                        if ((int)BaseMessages::COMMAND_JOIN == udpMessage.msg.type)
                        {
                            // Send Join Message
                            udpMessage.sendUdpMessage(sock,newServerAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();                        
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID++;
                            // Confirm Is Expected
                            expectedConfirm = true;
                        }
                        else if ((int)BaseMessages::COMMAND_BYE == udpMessage.msg.type)
                        {
                            printf("DEBUG INFO: COMMAND BYE\n");
                            udpMessage.sendUdpMessage(sock,newServerAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID++;
                            // Confirm Is Expected
                            expectedConfirm = true;
                            lastMessage = true;

                        }
                        else if ((int)BaseMessages::MSG == udpMessage.msg.type)
                        {
                            printf("DEBUG INFO: RECOGNISED MSG\n");
                            udpMessage.sendUdpMessage(sock,newServerAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID++;
                            // Confirm Is Expected
                            expectedConfirm = true;
                        }
                        else if ((int)BaseMessages::COMMAND_HELP == udpMessage.msg.type)
                        {
                            udpMessage.printHelp();
                        }
                    }
                    // TODO: Treba osetrit co vsechno muze funkce (checkMessage) vratit
                                                          
                }
            }              
            // Activity On Socket (Incoming Message From Server)
            if (FD_ISSET(sock, &readfds))
            {
                memset(buf, 0, BUFSIZE);

                socklen_t slen = sizeof(si_other);
                int bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                if (bytesRx == -1) {
                    // Zkontrolovat, zda došlo k chybě jiné než EWOULDBLOCK/EAGAIN
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        perror("recvfrom() failed");
                        // Ošetřit chybu
                    }
                    break; // Žádná další data k dispozici
                }
                else
                {
                    buf[bytesRx-1] = '\0';
                    udpMessage.readAndStoreBytes(buf,bytesRx);
                    if (true == expectedConfirm)
                    {
                        printf("DEBUG INFO: RECEIVED CONFIRM MESSAGE\n");
                        retVal = udpMessage.recvUpdConfirm(lastSentMessageID);
                        if (SUCCESS == retVal)
                        {
                            printf("DEBUG INFO: RECEIVED CONFIRM\n");
                            expectedConfirm = false; // Do not Expect Confirm Anymore
                            udpMessage.incrementUdpMsgId();
                            if (lastMessage)
                                return SUCCESS;
                            currentRetries = 0;
                        }
                        else if (EXTERNAL_ERROR == retVal)
                        {
                            // TODO 
                            udpMessage.basePrintExternalError();
                            exit(EXTERNAL_ERROR);
                        }
                        else 
                        {
                            // Zde bych asi pak bylo treba dodelat prehazovani packetu
                        }
                    }
                    else
                    {
                        // Message With Data Was Send
                        retVal = udpMessage.recvUdpMessage(lastSentMessageID);
                        if (SUCCESS == retVal)
                        {
                            // Set Retries To Zero
                            currentRetries = 0;
                            if (UdpMessages::COMMAND_JOIN == udpMessage.msg.type)
                            {
                                // Command Join Should Not Be Sended To Client
                                lastReceivedMessageID = udpMessage.messageID;
                                exit(EXIT_FAILURE);
                            }
                            else if (UdpMessages::COMMAND_BYE == udpMessage.msg.type)
                            {
                                // Stop The Loop
                                lastReceivedMessageID = udpMessage.messageID;
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                printf("DEBUG INFO: SERVER CLOSED THE CONNECTION\n");
                                break;
                            }
                            else if (UdpMessages::MSG == udpMessage.msg.type)
                            {
                                // Print The Message
                                lastReceivedMessageID = udpMessage.messageID;
                                printf("DEBUG INFO: RECEIVED MESSAGE: messageID=%d\n",udpMessage.messageID);
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                printf("DEBUG INFO: CONFIRMATION SENT on messageID=%d\n",lastReceivedMessageID);
                                std::string displayNameOutside(udpMessage.msg.displayNameOutside.begin(), udpMessage.msg.displayNameOutside.end());
                                std::string content(udpMessage.msg.content.begin(), udpMessage.msg.content.end());
                                printf("%s: %s\n", displayNameOutside.c_str(), content.c_str());
                            }
                            else if (UdpMessages::ERROR == udpMessage.msgType)
                            {
                                // Print The Error
                                lastReceivedMessageID = udpMessage.messageID;
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                printf("DEBUG INFO: SHOULD PRINT ERROR\n");
                                udpMessage.basePrintExternalError();
                                
                            }
                            else if (expectedConfirm && lastMessage)
                            {
                                return SUCCESS;
                            }
                            
                            // Store Message ID of Message That Has To Be Confirmed
                            // lastReceivedMessageID = udpMessage.messageID;
                            // Send Confirmation
                            // udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                            // Message Is Processed -> Clear The Buffer
                            memset(buf, 0, BUFSIZE);
                        }
                    }
                } 
            }

            // Check The Receive TimeOut
            if (expectedConfirm)
            {
                stopWatch = std::chrono::high_resolution_clock::now();
                int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(stopWatch - startWatch).count();
                if (elapsedTime > confirmationTimeout) 
                {   
                    printf("DEBUG INFO: OUT OF TIMEOUT!\n");
                    if (UdpMessages::REPLY != udpMessage.msg.type)
                    {
                        udpMessage.sendUdpMessage(sock,serverAddr);
                        currentRetries++;
                    }
                }
                elapsedTime = 0;

            }

        }
        if (currentRetries >= retryCount) {
            // Attempts Overrun
            return FAIL;
        }

        return SUCCESS;
    }
    
};


/*
    TODOs
-----------------------------------------------
- Implementovat resand message (uzivatel ma zde tri pokusy)
- Mozna pouzit jeste kopii Transmit aby se poslala ta zprava kterou opavdu chceme
- Musim umet potvrdit i errror message
*/

#endif
