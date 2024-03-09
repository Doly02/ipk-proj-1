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
    fd_set readfds;
    struct timeval tv;
    const int BUFSIZE = 1536;
    char buf[1536];
    int retryCount;
    int max_sd;
    int confirmationTimeout;
    bool sendAuth = false;
    bool authConfirmed = false;
    UdpMessages udpMessageTransmitter;
    UdpMessages udpMessageReceiver;
    struct sockaddr_in si_other;    

    int lastSentMessageID       = 0;                // ID Last Sended Message  
    int lastReceivedMessageID   = 0;                // ID Last Received Message
    std::unordered_set<int> receivedMessageIDs;     // Watchdog For Unique Received ID Messages

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
        /* Variables */
        udpMessageTransmitter.messageID = 1;
        bool checkReply = false;
        int retValue    = 0;
        int currentRetries = 0;
        const struct sockaddr_in& serverAddr = GetServerAddr();

        tv.tv_sec = 0;
        tv.tv_usec = 250000;        // 250ms TimeOut
        // While Not Authenticated
        while(!authConfirmed && currentRetries < retryCount)
        {
            if (!sendAuth) 
            {
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    size_t len = strlen(buf);
                    if (buf[len - 1] == '\n') 
                    {
                        buf[len - 1] = '\0';
                    }
                    udpMessageTransmitter.readAndStoreContent(buf);
                    retValue = udpMessageTransmitter.checkMessage();
                    if (retValue == 0 && udpMessageTransmitter.msg.type == UdpMessages::COMMAND_AUTH) 
                    {
                        udpMessageTransmitter.sendUdpAuthMessage(sock,serverAddr);
                        lastSentMessageID = udpMessageTransmitter.messageID;
                        sendAuth = true;
                        checkReply = true;
                        currentRetries++;
                    }
                }   
            }
            // Pripaire For Waiting With TimeOut
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            struct timeval timeout;
            timeout.tv_sec = 0;  // 250 ms timeout
            timeout.tv_usec = 250000;

            int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);
            if (activity == -1) 
            {
                perror("select() failed");
                exit(EXIT_FAILURE);
            }
            else if (activity == 0) {  // Run Out Of Timeout
                if (currentRetries < retryCount) 
                {
                    // If We Didn't Get a Answer Back Try To Send It Again
                    udpMessageTransmitter.sendUdpAuthMessage(sock,serverAddr);
                    currentRetries++;
                } 
                else 
                {
                    // Limit Overrun
                    return UdpMessages::AUTH_FAILED;
                }

            }
            else if (FD_ISSET(sock, &readfds)) 
            {
                // Máme nějakou aktivitu, pokusíme se přijmout zprávu
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

                    printf("RECEIVED %zu Bytes\n",udpMessageReceiver.msg.buffer.size());

                    retValue = udpMessageReceiver.recvUpdConfirm(lastSentMessageID);
                    if (!authConfirmed && retValue == UdpMessages::SUCCESS) 
                    {
                        printf(" AUTHENTICATION MESSAGE CONFIRMED\n");
                        authConfirmed = true;
                    } 
                    else if (checkReply && authConfirmed) 
                    {
                        retValue = udpMessageReceiver.recvUpdIncomingReply(lastSentMessageID);
                        if (UdpMessages::SUCCESS == retValue)
                        {
                            authConfirmed = true;
                            checkReply = false;
                            break;

                        }
                        else 
                        {
                            // Reply Failed -> Exit
                            exit(UdpMessages::AUTH_FAILED);
                        }
                    }
                } 
            }
        }
        return UdpMessages::SUCCESS;
    }

    int runUdpClient()
    {
        int currentRetries = 0;
        int retValue = 0;
        bool sendAgain = false;
        //bool sendConfirm = false;
        bool expectedConfirm = false;

        if (!Client::isConnected())
        {
            return NOT_CONNECTED;
        }

        /* Process Authentication */
        retValue = processAuthetification();
        if (BaseMessages::SUCCESS != retValue)
        {
            return retValue;
        }

        /* Main Loop */
        while (currentRetries < retryCount)
        {
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            FD_SET(STDIN_FILENO, &readfds);
            max_sd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

            // Timeout For recvfrom
            tv.tv_sec   = 0;        // s
            tv.tv_usec  = 250000;   // 100 ms

            // Waiting For An Activity
            int activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
            if (activity == 0 && sendAgain) {  // Run Out Of Timeout
                if (currentRetries < retryCount) 
                {
                    // If We Didn't Get a Answer Back Try To Send It Again
                    udpMessageTransmitter.SendUdpMessage(sock);
                    currentRetries++;
                    sendAgain = false;
                } 
                else 
                {
                    // Limit Overrun
                    return UdpMessages::AUTH_FAILED;
                }

            }
            else if ((activity < 0) && (errno != EINTR)) 
            {
                std::cerr << "Select error" << std::endl;
                break;
            }

            // Activity On Socket (Incoming Message From Server)
            if (FD_ISSET(sock, &readfds))
            {
                socklen_t slen = sizeof(si_other);
                memset(buf, 0, BUFSIZE);
                int bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                if (0 < bytesRx)
                {
                    // Ready To Handle The Message
                    buf[bytesRx] = '\0';
                    udpMessageReceiver.readAndStoreContent(buf);
                    if (true == expectedConfirm)
                    {
                        retValue = udpMessageReceiver.recvUpdConfirm(lastSentMessageID);
                        if (BaseMessages::SUCCESS == retValue)
                        {
                            expectedConfirm = false;
                        }
                        else 
                        {
                            // Send CONFIRM Again
                            sendAgain = true;
                        }
                    }
                    else
                    {
                        // Message With Data Was Send
                        retValue = udpMessageReceiver.RecvUdpMessage(lastSentMessageID);
                        if (BaseMessages::SUCCESS == retValue)
                        {
                            // Set Retries To Zero
                            currentRetries = 0;
                            sendAgain = false;
                            if (UdpMessages::COMMAND_JOIN == udpMessageReceiver.msgType)
                            {
                                // Command Join Should Not Be Sended To Client
                                exit(EXIT_FAILURE);
                            }
                            else if (UdpMessages::COMMAND_BYE == udpMessageReceiver.msgType)
                            {
                                // Stop The Loop
                                break;
                            }
                            else if (UdpMessages::MSG == udpMessageReceiver.msgType)
                            {
                                // Print The Message
                                std::string displayNameOutside(udpMessageReceiver.msg.displayNameOutside.begin(), udpMessageReceiver.msg.displayNameOutside.end());
                                std::string content(udpMessageReceiver.msg.content.begin(), udpMessageReceiver.msg.content.end());
                                printf("%s: %s\n", displayNameOutside.c_str(), content.c_str());
                            }
                            else if (UdpMessages::ERROR == udpMessageReceiver.msgType)
                            {
                                // Print The Error
                                std::string errMessage(udpMessageReceiver.msg.content.begin(), udpMessageReceiver.msg.content.end());
                                printf("Error: %s\n", errMessage.c_str());
                                exit(EXIT_FAILURE);
                            }
                            else
                            {
                                // Unknown Message Type
                                exit(EXIT_FAILURE);
                            }
                            // Send Confirmation
                            udpMessageReceiver.SendUdpConfirm(sock,lastReceivedMessageID);
                            // Store Message ID of Message That Has To Be Confirmed
                            lastReceivedMessageID = udpMessageReceiver.messageID;
                            // Message Is Processed -> Clear The Buffer
                            memset(buf, 0, BUFSIZE);

                        }
                    }
                } 
                // Capture Activity on STDIN
                if (FD_ISSET(STDIN_FILENO, &readfds)) 
                {
                    memset(buf, 0, BUFSIZE);
                    if (fgets(buf, BUFSIZE, stdin) != NULL) 
                    {
                        // Store Input From STDIN To Vector
                        udpMessageTransmitter.readAndStoreContent(buf);
                        // Check Message Validity
                        retValue = udpMessageTransmitter.checkMessage();
                        if(BaseMessages::SUCCESS == retValue)
                        {
                            if ((int)BaseMessages::COMMAND_JOIN == udpMessageTransmitter.msgType)
                            {
                                // Send Join Message
                                udpMessageTransmitter.SendUdpMessage(sock);
                                // Increment Retries
                                currentRetries++;
                                // Store Last Sent Message ID For Check
                                lastSentMessageID = udpMessageTransmitter.messageID;
                                // Confirm Is Expected
                                expectedConfirm = true;
                            }
                            else if ((int)BaseMessages::COMMAND_BYE == udpMessageTransmitter.msgType)
                            {
                                udpMessageTransmitter.SendUdpMessage(sock);
                                // Increment Retries
                                currentRetries++;
                                // Store Last Sent Message ID For Check
                                lastSentMessageID = udpMessageTransmitter.messageID;
                                // Confirm Is Expected
                                expectedConfirm = true;
                            }
                            else if ((int)BaseMessages::MSG == udpMessageTransmitter.msgType)
                            {
                                udpMessageTransmitter.SendUdpMessage(sock);
                                // Increment Retries
                                currentRetries++;
                                // Store Last Sent Message ID For Check
                                lastSentMessageID = udpMessageTransmitter.messageID;
                                // Confirm Is Expected
                                expectedConfirm = true;
                            }
                        }                                                
                    }
                }              
            }
        }


        return 0;
    }
    
};


/*
    TODOs
-----------------------------------------------
- Implementovat resand message (uzivatel ma zde tri pokusy)
- Mozna pouzit jeste kopii Transmit aby se poslala ta zprava kterou opavdu chceme

*/