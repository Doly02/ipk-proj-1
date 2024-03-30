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
#include "../include/udp_client.hpp"
/************************************************/
/*                  Constants                   */
/************************************************/
    UdpClient::UdpClient(const std::string& addr, int port,int retryCnt,int confirmTimeOut) : Client(addr, port, UDP) // Inicialization By Contructor From Base Class
    {
        retryCount = retryCnt;
        confirmationTimeout = confirmTimeOut;
        newServerAddr = server;

        fds[STDIN].fd = STDIN_FILENO;
        fds[STDIN].events = POLLIN;
        fds[SOCKET].fd = sock;
        fds[SOCKET].events = POLLIN;
    }

    UdpClient::~UdpClient() {
        // Destructor From Base Class [Calls close(sock)]
    }

    void UdpClient::udpHandleInterrupt(int signal)
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
            newServerAddr = si_other; 
            udpMessage.readAndStoreBytes(buf,bytesRx);
            retVal = udpMessage.recvUpdConfirm();
            if (SUCCESS != retVal)
            {
                udpMessage.insertErrorMsgToContent("ERR: CONFIRMATION RECEIVED\n");
                udpMessage.basePrintInternalError(retVal);
                exit(retVal);
            }
            exit(SUCCESS);
        }

    }


    int UdpClient::processAuthetification() 
    {
        /* Variables */
        int retVal = 0;
        const struct sockaddr_in& serverAddr = getServerAddr();
        udpMessage.msg.type = BaseMessages::UNKNOWN_MSG_TYPE;

        while (currentRetries < retryCount) 
        {   
            retVal = poll(fds,NUM_FILE_DESCRIPTORS,UNLIMITED_TIMEOUT);
            if (FAIL == retVal)
            {
                // TODO Nejaka hlaska 
                exit(FAIL);
            }

            if (fds[STDIN].revents & POLLIN)
            {
                // Capture Activity on STDIN
                if (fgets(buf, BUFSIZE, stdin) != NULL) 
                {
                    udpMessage.readAndStoreContent(buf);
                    retVal = udpMessage.checkMessage();

                    if (SUCCESS != retVal)
                        fprintf(stderr,"ERR: Invalid Parameters\n");

                    if (udpMessage.msg.type == UdpMessages::COMMAND_AUTH)       // Authentication Message
                    {
                        udpMessage.sendUdpAuthMessage(sock,serverAddr);
                        // Set Timer
                        startWatch = std::chrono::high_resolution_clock::now();
                        measureTime = true;
                        currentRetries++;
                    }
                    else if (udpMessage.msg.type == UdpMessages::COMMAND_HELP)  // Help Command
                    {
                        udpMessage.printHelp();
                    }    
                }

            }

            // Check Activity On Socket
            if (fds[SOCKET].revents & POLLIN)
            {
                memset(buf, 0, BUFSIZE);
                socklen_t slen = sizeof(si_other);
                ssize_t bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                if (bytesRx <= 0)
                {
                    return FAIL;
                }
                newServerAddr = si_other;   // Set Server's New Port 
                buf[BUFSIZE - 1] = '\0'; 
                udpMessage.readAndStoreBytes(buf,bytesRx);
                
                retVal = udpMessage.recvUpdConfirm();
                if (retVal == SUCCESS)      
                    measureTime = false;   
                
                retVal = udpMessage.recvUpdIncomingReply();
                if (SUCCESS == retVal)
                {
                    printf("DEBUG INFO: messageID=%d of Incomming Reply\n",udpMessage.messageID);
                    udpMessage.sendUdpConfirm(sock,newServerAddr);
                    break;
                }
            }

            // Check The Receive TimeOut
            if (measureTime)
            {
                stopWatch = std::chrono::high_resolution_clock::now();
                int elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(stopWatch - startWatch).count();
                if (elapsedTime > confirmationTimeout) 
                {
                    // TODO GetLastSentMessageID: udpMessage.messageID = lastSentMessageID;
                    // TODO ukladat si docacne nekde odeslanou zpravu muze totiz prijit neco jineho 
                    udpMessage.sendUdpAuthMessage(sock,newServerAddr);
                    currentRetries++;
                }
            }

        }

        if (currentRetries >= retryCount) 
        {
            printf("DEBUG: Attempts Overrun\n");
            // Attempts Overrun
            return AUTH_FAILED;
        }
        return SUCCESS;
    }

    int UdpClient::runUdpClient()
    {
        /*** Variables ***/
        int retVal              = 0;
        bool expectedConfirm    = false;
        ClientState state       = Authentication;

        printf("Set Attempts: %d, Current Attempts: %d, Set Timeout: %d\n",currentRetries,retryCount,confirmationTimeout);
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
            udpMessage.insertErrorMsgToContent("Authentication Failed\n");
            udpMessage.basePrintInternalError(retVal);
            return retVal;
        }
        state = Open;
        printf("DEBUG INFO: AUTHENTICATION DONE (runUdpClient)\n");
        printf("------------------------------------------------\n");

        /* Main Loop */
        while (currentRetries < retryCount)
        {
            retVal = poll(fds,NUM_FILE_DESCRIPTORS,UNLIMITED_TIMEOUT);
            if (FAIL == retVal)
            {
                // TODO Nejaka hlaska 
                exit(FAIL);
            }
            // Capture Activity on STDIN
            if (fds[STDIN].revents & POLLIN)
            {
                if (fgets(buf, BUFSIZE, stdin) != NULL)
                {
                    udpMessage.readAndStoreContent(buf);    
                    retVal = udpMessage.checkMessage();
                    if (SUCCESS != retVal)
                        printf("ERR: Invalid Parameters\n");

                    if (udpMessage.msg.type == UdpMessages::COMMAND_HELP)
                        udpMessage.printHelp();
                    else
                    {
                        udpMessage.sendUdpMessage(sock,newServerAddr);
                        expectedConfirm = true;
                        startWatch = std::chrono::high_resolution_clock::now();
                        currentRetries++;
                    }

                    if (BaseMessages::COMMAND_BYE == udpMessage.msg.type)
                    {
                        state = End;
                    }
                    memset(buf, 0, BUFSIZE);
                }
            }


            // Capture Activity on Socket
            if (fds[SOCKET].revents & POLLIN)
            {
                memset(buf, 0, BUFSIZE);
                socklen_t slen = sizeof(si_other);
                ssize_t bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                if (bytesRx <= 0)
                {
                    return FAIL;
                }
                newServerAddr = si_other;   // Set Server's New Port 
                buf[BUFSIZE - 1] = '\0'; 
                udpMessage.readAndStoreBytes(buf,bytesRx);
                switch (state)
                {
                    case Open:
                        /* CONFIRM MESSAGE */
                        retVal = udpMessage.recvUpdConfirm();
                        if (SUCCESS == retVal)   
                        {
                            expectedConfirm = false;
                            currentRetries = 0;
                            break;
                        }  
                        else if (NON_VALID_MSG_TYPE == retVal)
                        {
                            udpMessage.sendUdpError(sock,newServerAddr,"Unexpected Message Type");
                            expectedConfirm = true;
                            state = Error;
                            break;
                        }
                        else if (EXTERNAL_ERROR == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            // Error Message Was Send From Server And Client Just Send BYE Message So Just Wait For Confirmation
                            state = End;
                            break;
                        }

                        /* REPLY MESSAGE */
                        retVal = udpMessage.recvUpdIncomingReply();
                        if (SUCCESS == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            break;
                        }
                        else if (EXTERNAL_ERROR == retVal)
                        {
                            udpMessage.basePrintInternalError(retVal);
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            break;
                        }

                        /* MESSAGE */
                        retVal = udpMessage.recvUdpMessage();
                        if (SUCCESS == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            break;
                        }

                        break;
                    case End:
                        retVal =udpMessage.recvUpdConfirm();
                        if (SUCCESS == retVal)
                        {
                            return SUCCESS;
                        }
                        break;
                    case Authentication:
                    case Error:

                        retVal = udpMessage.recvUpdConfirm();       // Receive Confirmation on Send Error Message
                        if (SUCCESS == retVal)
                        {
                            expectedConfirm = false;
                            currentRetries = 0;
                            udpMessage.sendByeMessage(sock,newServerAddr);
                            expectedConfirm = true;
                            state = End;
                        }
                        break;
                    default:
                        break;

                memset(buf, 0, BUFSIZE);
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
                        //TODO Same As In AUTH! udpMessage.messageID = lastSentMessageID;
                        udpMessage.sendUdpMessage(sock,newServerAddr);
                        currentRetries++;
                    }
                }
                elapsedTime = 0;
            }

        }
        if (currentRetries >= retryCount) {
            // Attempts Overrun
            printf("Attemp Overrun -> runUdpClient()\n");
            return FAIL;
        }

        return SUCCESS;
    }
/*
    TODOs
-----------------------------------------------
- Implementovat resand message (uzivatel ma zde tri pokusy)
- Mozna pouzit jeste kopii Transmit aby se poslala ta zprava kterou opavdu chceme
- Musim umet potvrdit i errror message
Nove otazky:
- Co mam delat pokud mi prijde reply s NOK a nebo s result = 0?
*/

