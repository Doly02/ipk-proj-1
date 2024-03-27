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
/*************************************************/
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Milliseconds = std::chrono::milliseconds;

/************************************************/
/*                  Constants                   */
/************************************************/
    UdpClient::UdpClient(const std::string& addr, int port,int retryCnt,int confirmTimeOut) : Client(addr, port, UDP) // Inicialization By Contructor From Base Class
    {
        retryCount = retryCnt;
        confirmationTimeout = confirmTimeOut;
        newServerAddr = server;
        lastSentMessageID       = 0;               // ID Last Sended Message  
        lastReceivedMessageID   = 0;               // ID Last Received Message
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
            lastSentMessageID = udpMessage.getUdpMsgId();
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
            retVal = udpMessage.recvUpdConfirm(lastSentMessageID);
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
        bool checkReply = false;            //!< Indicates Whether It is Expected Reply Message
        const struct sockaddr_in& serverAddr = getServerAddr();
        udpMessage.msg.type = BaseMessages::UNKNOWN_MSG_TYPE;

        /* Timers */
        TimePoint startWatch;               //!< Contains the Initial Measurement Time
        TimePoint stopWatch;                //!< Contains the Final Measurement Time
        bool measureTime = false;           //!< Indicates That Time Should Be Measured

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

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
                        if (SUCCESS == retVal && udpMessage.msg.type == UdpMessages::COMMAND_AUTH) 
                        {
                            udpMessage.sendUdpAuthMessage(sock,serverAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            measureTime = true;
                            checkReply = true;
                            lastSentMessageID = udpMessage.getUdpMsgId();
                            currentRetries++;

                        }
                        else if (SUCCESS == retVal && udpMessage.msg.type == UdpMessages::COMMAND_HELP)
                        {
                            udpMessage.printHelp();
                        }                        
                    }   
                }

                // Check Activity On Socket
                if (FD_ISSET(sock, &readfds) && checkReply) 
                {
                    memset(buf, 0, BUFSIZE);
                    socklen_t slen = sizeof(si_other);
                    ssize_t bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
                    
                    if (bytesRx == -1) {
                        // Check If Any Other Fault Was Made Then EWOULDBLOCK/EAGAIN
                        if (errno != EWOULDBLOCK && errno != EAGAIN) 
                        {
                            perror("recvfrom() failed");
                        }
                        printf("DEBUG: bytesRx = -1\n");
                        break; 
                    }
                    else 
                    {   
                        newServerAddr = si_other; // Set Server's New Port  
                        buf[BUFSIZE - 1] = '\0'; 
                        udpMessage.readAndStoreBytes(buf,bytesRx);

                        retVal = udpMessage.recvUpdConfirm(lastSentMessageID);
                        if (!receivedConfirm && retVal == SUCCESS)      // First Confirm
                        {
                            receivedConfirm = true;
                            measureTime = false;
                            
                        }
                        else if (checkReply && receivedConfirm)         // Then Reply
                        {

                            retVal = udpMessage.recvUpdIncomingReply(lastSentMessageID);
                            if (SUCCESS == retVal)
                            {
                                printf("DEBUG INFO: messageID=%d of Incomming Reply\n",udpMessage.messageID);
                                lastReceivedMessageID = udpMessage.messageID;

                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                receivedConfirm = true;
                                checkReply = false;
                                break;

                            }
                            // If ALREADY_PROCESSED_MSG Do Nothing
                            // TODO UNEXPECTED_MESSAGE   
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
                    udpMessage.messageID = lastSentMessageID;
                    // TODO ukladat si docacne nekde odeslanou zpravu muze totiz prijit neco jineho 
                    udpMessage.sendUdpAuthMessage(sock,newServerAddr);
                    currentRetries++;
                }
            }

            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(sock, &readfds);
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
        bool lastMessage        = false;
        bool joinSend           = false;

        /* Timers */
        TimePoint startWatch;
        TimePoint stopWatch;

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
            udpMessage.insertErrorMsgToContent("Authentication Failed\n");
            udpMessage.basePrintInternalError(retVal);
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
    
            int activity = select(sock + 1, &readfds, NULL, NULL, NULL); 
            if (-1 == activity) 
            { 
                udpMessage.insertErrorMsgToContent("ERR: Select Error\n");
                exit(EXIT_FAILURE);
            }
            // Capture Activity on STDIN
            if (FD_ISSET(STDIN_FILENO, &readfds)) 
            {
                memset(buf, 0, BUFSIZE);
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
                            // Confirm Is Expected
                            expectedConfirm = true;
                            joinSend = true;
                            lastSentMessageID = udpMessage.getUdpMsgId();
                        }
                        else if ((int)BaseMessages::COMMAND_BYE == udpMessage.msg.type)
                        {
                            printf("DEBUG INFO: INPUT BYE\n");
                            udpMessage.sendUdpMessage(sock,newServerAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            // Increment Retries
                            currentRetries++;
                            // Confirm Is Expected
                            expectedConfirm = true;
                            lastMessage = true;
                            lastSentMessageID = udpMessage.getUdpMsgId();
                        }
                        else if ((int)BaseMessages::MSG == udpMessage.msg.type)
                        {
                            printf("DEBUG INFO: INPUT MSG\n");
                            udpMessage.sendUdpMessage(sock,newServerAddr);
                            // Set Timer
                            startWatch = std::chrono::high_resolution_clock::now();
                            // Increment Retries
                            currentRetries++;
                            // Confirm Is Expected
                            expectedConfirm = true;
                            lastSentMessageID = udpMessage.getUdpMsgId();
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
                            expectedConfirm = false; // Do not Expect Confirm Anymore
                            if (lastMessage)
                                return SUCCESS;
                            currentRetries = 0;
                            printf("DEBUG INFO: HANDLED CONFIRM SUCCESSFULY\n");
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
                    else if (true == joinSend)
                    {
                        retVal = udpMessage.recvUdpMessage(lastReceivedMessageID+1);
                        if (UdpMessages::MSG == udpMessage.msg.type)
                        {
                                // Print The Message
                                lastReceivedMessageID = udpMessage.messageID;
                                printf("DEBUG INFO: RECEIVED MESSAGE: messageID=%d\n",udpMessage.messageID);
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                printf("DEBUG INFO: CONFIRMATION SENT on messageID=%d\n",lastReceivedMessageID);
                        }
                        if (UdpMessages::REPLY == udpMessage.msg.type)
                        {
                            retVal = udpMessage.recvUpdIncomingReply(lastSentMessageID);
                            if (SUCCESS == retVal)
                            {
                                lastReceivedMessageID = udpMessage.messageID;
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                printf("DEBUG INFO: CONFIRMATION SENT on messageID=%d\n",lastReceivedMessageID);
                            }
                            udpMessage.printMessage();
                            joinSend = false;
                        }
                    }
                    else
                    {
                        // Message With Data Was Send
                        retVal = udpMessage.recvUdpMessage(lastReceivedMessageID+1);
                        if (SUCCESS == retVal)
                        {
                            currentRetries = 0;

                            if (UdpMessages::COMMAND_JOIN == udpMessage.msg.type)
                            {
                                // Command Join Should Not Be Sended To Client
                                lastReceivedMessageID = udpMessage.messageID;
                                exit(EXIT_FAILURE);
                            }
                            else if (UdpMessages::COMMAND_BYE == udpMessage.msg.type)
                            {
                                lastReceivedMessageID = udpMessage.messageID;
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                printf("DEBUG INFO: SERVER CLOSED THE CONNECTION\n");
                                break;  // Stop The Loop
                            }
                            else if (UdpMessages::MSG == udpMessage.msg.type)
                            {
                                // Print The Message
                                lastReceivedMessageID = udpMessage.messageID;
                                printf("DEBUG INFO: RECEIVED MESSAGE: messageID=%d\n",udpMessage.messageID);
                                udpMessage.sendUdpConfirm(sock,newServerAddr,lastReceivedMessageID);
                                printf("DEBUG INFO: CONFIRMATION SENT on messageID=%d\n",lastReceivedMessageID);
                                udpMessage.printMessage();
                            }
                            else if (expectedConfirm && lastMessage)
                            {
                                return SUCCESS;
                            }
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
                        udpMessage.messageID = lastSentMessageID;
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
/*
    TODOs
-----------------------------------------------
- Implementovat resand message (uzivatel ma zde tri pokusy)
- Mozna pouzit jeste kopii Transmit aby se poslala ta zprava kterou opavdu chceme
- Musim umet potvrdit i errror message
Nove otazky:
- Co mam delat pokud mi prijde reply s NOK a nebo s result = 0?
*/

