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


/************************************************/
/*                  Libraries                   */
/************************************************/
#include "client.cpp"
#include <iostream>
#include <string>
#include <unistd.h>     // For close
#include <unordered_set>
#include <chrono>
#include "udp_messages.cpp"
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
    UdpMessages udpMessageTransmitter;
    UdpMessages udpMessageReceiver;
    struct sockaddr_in si_other;    // Struct For Storing Server (Sender) Address

    int lastSentMessageID       = 0;                // ID Last Sended Message  
    int lastReceivedMessageID   = 0;                // ID Last Received Message
    std::unordered_set<int> receivedMessageIDs;     // Watchdog For Unique Received ID Messages

    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Milliseconds = std::chrono::milliseconds;

public:
    UdpClient(const std::string& addr, int port,int retryCnt,int confirmTimeOut) : Client(addr, port, UDP) // Inicialization By Contructor From Base Class
    {
        retryCount = retryCnt;
        confirmationTimeout = confirmTimeOut;
    }

    virtual ~UdpClient() {
        // Destructor From Base Class [Calls close(sock)]
    }

    int processAuthetification() 
    {
        /* Variable */
        bool checkReply = false; 
        int retVal = 0;
        bool expectedReply = false;
        int currentRetries = 0;
        const struct sockaddr_in& serverAddr = getServerAddr();
        /* Timers */
        TimePoint startWatch;
        TimePoint stopWatch;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        timeout.tv_sec = 0; 
        timeout.tv_usec = 250000;   // 250ms 


        while (currentRetries < retryCount) 
        {    
            int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);
            if (activity == -1) {
                perror("select() failed");
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
                        udpMessageTransmitter.readAndStoreContent(buf);
                        printf("RESEVED MESSAGE TO SEND\n");
                        // Check The Message 
                        retVal = udpMessageTransmitter.checkMessage();
                        printf("MSG TYPE: %d",udpMessageTransmitter.msg.type);
                        if (retVal == 0 && udpMessageTransmitter.msg.type == UdpMessages::COMMAND_AUTH) 
                        {
                            udpMessageTransmitter.sendUdpAuthMessage(sock,serverAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            lastSentMessageID = udpMessageTransmitter.messageID;
                            sendAuth = true;
                            expectedReply = true;
                            checkReply = true;
                            printf("INCREMENT currentRetries\n");
                            currentRetries++;
                        }
                        else if (retVal == 0 && udpMessageTransmitter.msg.type == UdpMessages::COMMAND_BYE)
                        {
                            return SUCCESS;;
                        }
                        else if (retVal == 0 && udpMessageTransmitter.msg.type == UdpMessages::COMMAND_HELP)
                        {
                            udpMessageReceiver.printHelp();
                        }
                        else
                        {
                            // TODO: Treba osetrit co vsechno muze byt spatne (fce checkMessage)
                            return retVal;
                        }
                    }   
                }
    //            std::this_thread::sleep_for(std::chrono::milliseconds(250)); // Čekáme 250ms

                // Check Activity On Socket
                if (FD_ISSET(sock, &readfds)) 
                {
                    memset(buf, 0, BUFSIZE);
                    socklen_t slen = sizeof(si_other);
                    ssize_t bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                    if (-1 == bytesRx) 
                    {
                        // Chyba při příjmu dat
                        perror("recvfrom() failed");
                        exit(EXIT_FAILURE);
                    }
                    else 
                    {   
                        buf[BUFSIZE - 1] = '\0'; 
                        udpMessageReceiver.readAndStoreBytes(buf,bytesRx);
                        retVal = udpMessageReceiver.recvUpdConfirm(lastSentMessageID);
                    
                        if (!receivedConfirm && retVal == SUCCESS) 
                        {
                            receivedConfirm = true;
                            expectedReply = false;
                            
                        }
                        else if (checkReply && receivedConfirm) 
                        {
                            retVal = udpMessageReceiver.recvUpdIncomingReply(lastSentMessageID);
                            if (SUCCESS == retVal)
                            {
                                receivedConfirm = true;
                                checkReply = false;
                                break;

                            }
                            if (EXTERNAL_ERROR == retVal)
                            {
                                exit(EXTERNAL_ERROR);
                            }
                            else 
                            {
                                // Reply Failed -> Exit
                                exit(AUTH_FAILED);
                            }
                        }
                    } 

                } 
            }
            // Check The Receive TimeOut
            if (expectedReply)
            {
                stopWatch = std::chrono::high_resolution_clock::now();
                int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(stopWatch - startWatch).count();
                if (elapsedTime > confirmationTimeout) 
                {
                    udpMessageTransmitter.sendUdpAuthMessage(sock,serverAddr);
                    currentRetries++;
                }
            }

            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(sock, &readfds);
        }

        udpMessageReceiver.incrementUdpMsgId();
        udpMessageTransmitter.incrementUdpMsgId();
        
        if (currentRetries >= retryCount) {
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

        udpMessageReceiver.setUdpMsgId();
        udpMessageTransmitter.setUdpMsgId();

        /* Process Authentication */
        retVal = processAuthetification();
        if (SUCCESS != retVal)
        {
            printf("AUTHENTICATION FAILED (return code: %d)\n",retVal);
            return retVal;
        }
        else if (SUCCESS && BaseMessages::COMMAND_AUTH == udpMessageTransmitter.msg.type)
        {
            // Propagate 
            return SUCCESS;;
        }
        printf("AUTHENTICATION DONE (runUdpClient)\n");
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

            int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);
            if (activity == -1) {
                perror("Select error");
                exit(EXIT_FAILURE);
            }
            // Capture Activity on STDIN
            if (FD_ISSET(STDIN_FILENO, &readfds)) 
            {
                //memset(buf, 0, BUFSIZE);
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    // Store Input From STDIN To Vector
                    udpMessageTransmitter.readAndStoreContent(buf);
                    // Check Message Validity
                    size_t len = strlen(buf);
                    if (buf[len - 1] == '\n') 
                    {
                        buf[len - 1] = '\0';
                    }

                    retVal = udpMessageTransmitter.checkMessage();
                    if(SUCCESS == retVal)
                    {
                        if ((int)BaseMessages::COMMAND_JOIN == udpMessageTransmitter.msg.type)
                        {
                            // Send Join Message
                            udpMessageTransmitter.sendUdpMessage(sock,serverAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();                        
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID++;
                            // Confirm Is Expected
                            expectedConfirm = true;
                        }
                        else if ((int)BaseMessages::COMMAND_BYE == udpMessageTransmitter.msg.type)
                        {
                            printf("COMMAND BYE\n");
                            udpMessageTransmitter.sendUdpMessage(sock,serverAddr);
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
                        else if ((int)BaseMessages::MSG == udpMessageTransmitter.msg.type)
                        {
                            printf("RECOGNISED MSG\n");
                            udpMessageTransmitter.sendUdpMessage(sock,serverAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID++;
                            // Confirm Is Expected
                            expectedConfirm = true;
                        }
                        else if ((int)BaseMessages::COMMAND_HELP == udpMessageTransmitter.msg.type)
                        {
                            udpMessageTransmitter.printHelp();
                        }
                    }
                    else
                    {
                        // TODO: Treba osetrit co vsechno muze funkce (checkMessage) vratit
                    }                                              
                }
            }              
            // Activity On Socket (Incoming Message From Server)
            if (FD_ISSET(sock, &readfds))
            {
                memset(buf, 0, BUFSIZE);
                socklen_t slen = sizeof(si_other);
                int bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                if (-1 == bytesRx)
                {
                    perror("recvfrom() failed");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    int bufferLen = bytesRx;
                    buf[bytesRx-1] = '\0';
                    udpMessageReceiver.readAndStoreBytes(buf,bytesRx);
                    printf("RECEIVED BYTES: %zu but BUFFER IS: %d\n",udpMessageReceiver.msg.buffer.size(),bufferLen);
                    if (true == expectedConfirm)
                    {
                        retVal = udpMessageReceiver.recvUpdConfirm(lastSentMessageID);
                        if (SUCCESS == retVal)
                        {
                            printf("RECEIVED CONFIRM\n");
                            expectedConfirm = false; // Do not Expect Confirm Anymore
                            udpMessageReceiver.incrementUdpMsgId();
                            udpMessageTransmitter.incrementUdpMsgId();
                            if (lastMessage)
                                return SUCCESS;
                            currentRetries = 0;
                        }
                        else if (EXTERNAL_ERROR == retVal)
                        {
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
                        retVal = udpMessageReceiver.recvUdpMessage(lastSentMessageID);
                        if (SUCCESS == retVal)
                        {
                            // Set Retries To Zero
                            currentRetries = 0;
                            if (UdpMessages::COMMAND_JOIN == udpMessageReceiver.msg.type)
                            {
                                // Command Join Should Not Be Sended To Client
                                exit(EXIT_FAILURE);
                            }
                            else if (UdpMessages::COMMAND_BYE == udpMessageReceiver.msg.type)
                            {
                                // Stop The Loop
                                break;
                            }
                            else if (UdpMessages::MSG == udpMessageReceiver.msg.type)
                            {
                                // Print The Message
                                std::string displayNameOutside(udpMessageReceiver.msg.displayNameOutside.begin(), udpMessageReceiver.msg.displayNameOutside.end());
                                std::string content(udpMessageReceiver.msg.content.begin(), udpMessageReceiver.msg.content.end());
                                printf("%s: %s\n", displayNameOutside.c_str(), content.c_str());
                            }
                            else if (UdpMessages::ERROR == udpMessageReceiver.msgType)
                            {
                                // Print The Error
                                udpMessageReceiver.printError();
                                exit(EXTERNAL_ERROR);
                            }
                            else if (expectedConfirm && lastMessage)
                            {
                                return SUCCESS;
                            }
                            else
                            {
                                //TODO: Unknown Message Type
                                exit(EXIT_FAILURE);
                            }
                            // Store Message ID of Message That Has To Be Confirmed
                            lastReceivedMessageID = udpMessageReceiver.messageID;
                            // Send Confirmation
                            udpMessageReceiver.sendUdpConfirm(sock,serverAddr,lastReceivedMessageID);
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
                    printf("OUT OF TIMEOUT!\n");
                    udpMessageTransmitter.sendUdpMessage(sock,serverAddr);
                    currentRetries++;
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