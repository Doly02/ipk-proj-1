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

    unsigned long lastSentMessageID = 0;                    // ID Last Sended Message  
    std::unordered_set<unsigned long> receivedMessageIDs;   // Watchdog For Unique Received ID Messages

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
        bool checkReply = false;
        bool sendAgain  = false;
        int retValue    = 0;
        int currentRetries = 0;
        
        // While Not Authenticated
        while(!authConfirmed)
        {
            // Get Authentication Command From User
            if(fgets(buf, BUFSIZE, stdin) != NULL)
            {
                size_t len = strlen(buf);
                if (buf[len - 1] == '\n') 
                {
                    buf[len - 1] = '\0';
                }

                // Store Content To Vector
                udpMessageTransmitter.readAndStoreContent(buf);    
                // Check User's Input Is Valid Message 
                retValue = udpMessageTransmitter.checkMessage();   
                if (retValue == 0) 
                {
                    // Check If Message Is AUTH Command
                    if ((int)UdpMessages::COMMAND_AUTH == udpMessageTransmitter.msgType)
                    {
                        // Sent To Server Authentication Message
                        udpMessageTransmitter.sendUdpAuthMessage(sock);
                        lastSentMessageID = udpMessageTransmitter.messageID;
                        currentRetries++;
                        sendAuth = true;
                        checkReply = true;
                    }
                }
                memset(buf, 0, sizeof(buf));
            }
            else if ((true == sendAgain) && (currentRetries < retryCount))
            {
                udpMessageTransmitter.sendUdpAuthMessage(sock);
                lastSentMessageID = udpMessageTransmitter.messageID;
                currentRetries++;
                    
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
                socklen_t slen = sizeof(si_other);
                if (recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen) == -1) {
                    exit(1);
                }
                else {
                    buf[BUFSIZE - 1] = '\0';
                    udpMessageReceiver.readAndStoreContent(buf);
                    retValue = udpMessageReceiver.revcConfirmWTimeOut(250, lastSentMessageID);
                    if (!authConfirmed && UdpMessages::SUCCESS == retValue)
                    {
                        authConfirmed = true;
                    }
                    else if (!authConfirmed && UdpMessages::OUT_OF_TIMEOUT == retValue)
                    {
                        sendAgain = true;
                    }
                    if (checkReply && authConfirmed)
                    {   
                        retValue = udpMessageReceiver.revcReplyWithTimeOut(250, lastSentMessageID);
                        if (0 == retValue)
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

        if (!Client::isConnected())
        {
            return NOT_CONNECTED;
        }

        // Timeout For recvfrom
        tv.tv_sec = 1;  // s
        tv.tv_usec = 0; // ms


        /* Process Authentication */
        retValue = processAuthetification();
        if (BaseMessages::SUCCESS != retValue)
        {
            return retValue;
        }

        /* Main Loop */
        while (true)
        {
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            FD_SET(STDIN_FILENO, &readfds);
            max_sd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

            // Set Timeout
            tv.tv_sec = 0;
            tv.tv_usec = 50;

            // Waiting For An Activity
            int activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);
            if ((activity < 0) && (errno != EINTR)) 
            {
                std::cerr << "Select error" << std::endl;
                break;
            }

            // Check Activity On STDIN
            if (FD_ISSET(STDIN_FILENO, &readfds)) 
            {
                // Clear Input Buffer
                memset(buf, 0, sizeof(buf));
                // Wait For User's Input
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    // Store Input From STDIN To Vector
                    udpMessageTransmitter.readAndStoreContent(buf);
                    // Check Message Validity
                    retValue = udpMessageTransmitter.checkMessage();
                    if (0 == retValue)
                    {
                        // Decide Reactions Based On Message Type
                        if ((int)BaseMessages::COMMAND_JOIN == udpMessageTransmitter.msgType)
                        {
                            // Send Join Message
                            udpMessageTransmitter.SendUdpMessage(sock);
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID = udpMessageTransmitter.messageID;
                        }
                        else if ((int)BaseMessages::COMMAND_BYE == udpMessageTransmitter.msgType)
                        {
                            udpMessageTransmitter.SendUdpMessage(sock);
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID = udpMessageTransmitter.messageID;
                        }
                        else if ((int)BaseMessages::MSG == udpMessageTransmitter.msgType)
                        {
                            udpMessageTransmitter.SendUdpMessage(sock);
                            // Increment Retries
                            currentRetries++;
                            // Store Last Sent Message ID For Check
                            lastSentMessageID = udpMessageTransmitter.messageID;
                        }

                    }

                }
            }
            
            // If Send Failed -> Send Again
            if ((true == sendAgain) && (currentRetries <= retryCount))
            {
                // Send Message Again
                udpMessageTransmitter.SendUdpMessage(sock);
                // Increment Retries
                currentRetries++;
                // Store Last Sent Message ID For Check
                lastSentMessageID = udpMessageTransmitter.messageID;
            }



            // Check Socket's Activity
            if (FD_ISSET(sock, &readfds)) 
            {
                memset(buf, 0, sizeof(buf));
                socklen_t slen = sizeof(si_other);
                int bytesRx = sendto(sock, buf, BUFSIZE+2 , 0, (struct sockaddr *) &si_other, slen);
                if (bytesRx > 0) 
                {
                    // Secure The End Of String
                    buf[BUFSIZE-1]  = '\0';
                    // Store Received Message To Vector
                    udpMessageReceiver.readAndStoreContent(buf);

                    // Check If Message Is Just Confirmation
                    /* Function Will Wait For 250ms For Confirmation */
                    /* Param lastSendMessageID Is Checked Message ID */
                    retValue = udpMessageReceiver.revcConfirmWTimeOut(confirmationTimeout, lastSentMessageID);
                    if (BaseMessages::SUCCESS == retValue)
                    {
                        // Set Retries To 0
                        currentRetries = 0;
                        // Set Send Again To False
                        sendAgain = false;
                    }
                    else if (UdpMessages::OUT_OF_TIMEOUT == retValue)
                    {
                        // Set Send Again To True
                        sendAgain = true;
                    }
                    else
                    {   // Should Be Normal Message
                        retValue = udpMessageReceiver.RecvUdpMessageWTimeOut(confirmationTimeout, lastSentMessageID);
                        if (BaseMessages::SUCCESS == retValue)
                        {
                            // Set Retries To 0
                            currentRetries = 0;
                            // Set Send Again To False
                            sendAgain = false;
                            if (UdpMessages::COMMAND_JOIN == udpMessageReceiver.msgType)
                            {
                                // Maybe Store It In Some Local Variable
                                std::string displayName(udpMessageTransmitter.msg.displayName.begin(), udpMessageTransmitter.msg.displayName.end());
                                std::string channel(udpMessageReceiver.msg.channelID.begin(), udpMessageReceiver.msg.channelID.end());
                                printf("Server: %s has joined channel %s\n", displayName.c_str(), channel.c_str());
                            }
                            else if (UdpMessages::COMMAND_BYE == udpMessageReceiver.msgType)
                            {
                                break;
                            }
                            else
                            {
                                std::string displayName(udpMessageReceiver.msg.displayNameOutside.begin(), udpMessageReceiver.msg.displayNameOutside.end());
                                std::string content(udpMessageReceiver.msg.content.begin(), udpMessageReceiver.msg.content.end());
                                printf("%s: %s\n", displayName.c_str(), content.c_str());
                            }
                        }
                        else if (UdpMessages::OUT_OF_TIMEOUT == retValue)
                        {
                            // Set Send Again To True
                            sendAgain = true;
                        }
                        else if (UdpMessages::MSG_FAILED)
                        {
                            // Exit
                            return UdpMessages::MSG_FAILED;
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