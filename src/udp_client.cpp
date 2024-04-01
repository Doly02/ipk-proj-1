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
        bool confirmed = false;
        currentRetries = 0;
        udpMessage.msg.type = UdpMessages::COMMAND_BYE;
        /* Send Message */
        udpMessage.sendUdpMessage(sock,newServerAddr);
        /* Receive CONFIRM */
        while ((currentRetries < retryCount) && !confirmed)
        {
            socklen_t slen = sizeof(si_other);
            ssize_t bytesRx = recvfrom(sock, buf, BUFSIZE, 0, (struct sockaddr *) &si_other, &slen);
            if (-1 == bytesRx) 
            {
                // Chyba při příjmu dat
                fprintf(stderr,"ERR: recvfrom() failed\n"); // FIXME
                exit(EXIT_FAILURE);
            }
            newServerAddr = si_other; 
            udpMessage.readAndStoreBytes(buf,bytesRx);
            retVal = udpMessage.recvUpdConfirm();
            if (SUCCESS != retVal)
            {
                udpMessage.msg.type = UdpMessages::COMMAND_BYE;
                udpMessage.sendUdpMessage(sock,newServerAddr);
                currentRetries++;
            }
            else
            {
                confirmed = true;
                exit(SUCCESS);
            }

        }
        exit(FAIL);
    }

}


int UdpClient::processAuthetification() 
{
    /* Variables */
    int retVal = 0;
    int watchDog = 0;
    const struct sockaddr_in& serverAddr = getServerAddr();
    udpMessage.msg.type = BaseMessages::UNKNOWN_MSG_TYPE;
    ClientState state = Authentication;


    while (currentRetries < retryCount) 
    {   
        retVal = poll(fds,NUM_FILE_DESCRIPTORS,UNLIMITED_TIMEOUT);
        if (FAIL == retVal)
        {
            // TODO Nejaka hlaska 
            exit(FAIL);
        }

        if ((fds[STDIN].revents & POLLIN))
        {
            // Capture Activity on STDIN
            // Note: Block The User's Input From STDIN To Be Send When Reply Is Expected
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
                    udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
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
            newServerAddr = si_other;                           // Set Server's New Port 
            buf[BUFSIZE - 1] = '\0'; 
            udpMessage.readAndStoreBytes(buf,bytesRx);
            BaseMessages::MessageType_t type = (BaseMessages::MessageType_t)buf[0];
            switch(state)
            {
                case Authentication:
                    if (BaseMessages::CONFIRM == type)
                    {
                        retVal = udpMessage.recvUpdConfirm();
                        if (retVal == SUCCESS) 
                        {
                            measureTime = false;   
                            currentRetries = 0;
                        }     
                    }
                    else if (BaseMessages::REPLY == type)
                    {
                        retVal = udpMessage.recvUpdIncomingReply();
                        if (SUCCESS == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                            return SUCCESS;
                        }
                        else if (FAIL == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again                          
                        }
                    }
                    else if (BaseMessages::COMMAND_BYE == type)
                    {
                        udpMessage.sendUdpConfirm(sock,newServerAddr);
                        udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                        exit(SUCCESS);
                    }
                    else if (BaseMessages::ERROR == type)
                    {
                        udpMessage.recvUdpError();
                        udpMessage.sendUdpConfirm(sock,newServerAddr);
                        udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                        state = Error;
                        watchDog = -1;
                    }
                    else if (BaseMessages::MSG == type)
                    {
                        /* MESSAGE */
                        retVal = udpMessage.recvUdpMessage();
                        if (SUCCESS == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                            break;
                        }
                        else
                        {
                            state = Error;
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpMessage.sendUdpError(sock,newServerAddr,"Invalid Messsage Params");
                            measureTime = true;
                            watchDog = -1;
                        }
                        break;
                    }
                    break;
                case Error:
                    retVal = udpMessage.recvUpdConfirm();                       // Receive Confirmation on Send Error Message
                    if (SUCCESS == retVal)
                    {
                        measureTime = false;
                        currentRetries = 0;
                        udpMessage.sendByeMessage(sock,newServerAddr);
                        udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                        measureTime = true;
                        state = End;
                    }
                    break;
                case End:
                    retVal =udpMessage.recvUpdConfirm();
                    if (SUCCESS == retVal)
                    {
                        return watchDog;
                    }
                    break;
                case Open:
                default:
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
                udpBackUpMessage.sendUdpAuthMessage(sock,newServerAddr);
                currentRetries++;
            }
        }

    }

    if (currentRetries >= retryCount) 
    {
        fprintf(stderr,"ERR: Attempts Overrun\n");
        // Attempts Overrun
        return AUTH_FAILED;
    }
    return SUCCESS;
}

int UdpClient::runUdpClient()
{
    /*** Variables ***/
    int watchDog            = -1;
    int retVal              = 0;
    bool expectedConfirm    = false;
    ClientState state       = Authentication;
    
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

    /* Main Loop */
    while (currentRetries < retryCount)
    {

        if (!messageQueue.empty() && !expectedConfirm)
        {
            UdpMessages frontMessage = messageQueue.front();
            messageQueue.pop();
            frontMessage.sendUdpMessage(sock,newServerAddr);
            startWatch = std::chrono::high_resolution_clock::now();
            expectedConfirm = true;
            udpBackUpMessage = frontMessage;                                    // Store Message For Case When Sending Again
        }

        retVal = poll(fds,NUM_FILE_DESCRIPTORS,UNLIMITED_TIMEOUT);
        if (FAIL == retVal)
        {
            // TODO Nejaka hlaska 
            exit(FAIL);
        }
        // Capture Activity on STDIN
        if ((fds[STDIN].revents & POLLIN))
        {
            if (fgets(buf, BUFSIZE, stdin) != NULL)
            {
                udpMessage.readAndStoreContent(buf);    
                retVal = udpMessage.checkMessage();
                if (SUCCESS != retVal)
                    fprintf(stderr,"ERR: Invalid Parameters\n");

                if (UdpMessages::COMMAND_HELP == udpMessage.msg.type)
                    udpMessage.printHelp();
                
                else if (UdpMessages::COMMAND_AUTH == udpMessage.msg.type)
                    fprintf(stderr,"ERR: Authentication Already Processed - Not Possible Again\n");
                
                else if (UdpMessages::COMMAND_RENAME != udpMessage.msg.type && SUCCESS == retVal)
                {
                    if (expectedConfirm || !messageQueue.empty())
                    {
                        UdpMessages toQueueMessage = udpMessage;
                        messageQueue.push(toQueueMessage);
                    }
                    else
                    {
                        udpMessage.sendUdpMessage(sock,newServerAddr);
                        udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                        expectedConfirm = true; 
                        startWatch = std::chrono::high_resolution_clock::now();
                    }
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
            BaseMessages::MessageType_t type = (BaseMessages::MessageType_t)buf[0];
            switch (state)
            {
                case Open:
                    /* CONFIRM MESSAGE */
                    if (BaseMessages::CONFIRM == type)
                    {
                        retVal = udpMessage.recvUpdConfirm();
                        if (SUCCESS == retVal)   
                        {
                            expectedConfirm = false;
                            currentRetries = 0;
                        }  
                    }  
                    else if (BaseMessages::ERROR == type)
                    {
                        udpMessage.recvUdpError();
                        udpMessage.sendUdpConfirm(sock,newServerAddr);
                        state = Error;
                        watchDog = -1;
                        break;
                    }
                    else if (BaseMessages::REPLY == type)
                    {
                        /* REPLY MESSAGE */
                        retVal = udpMessage.recvUpdIncomingReply();
                        if (SUCCESS == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                            break;
                        }
                        else if (FAIL == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                        }
                        else        // TODO Unexpected Message?
                        {
                            state = Error;
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpMessage.sendUdpError(sock,newServerAddr,"Invalid Messsage Params");
                            expectedConfirm = true;
                            watchDog = -1;
                        }
                        break;
                    }
                    else if (BaseMessages::MSG == type)
                    {
                        /* MESSAGE */
                        retVal = udpMessage.recvUdpMessage();
                        if (SUCCESS == retVal)
                        {
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
                            break;
                        }
                        else
                        {
                            state = Error;
                            udpMessage.sendUdpConfirm(sock,newServerAddr);
                            udpMessage.sendUdpError(sock,newServerAddr,"Invalid Messsage Params");
                            expectedConfirm = true;
                            watchDog = -1;
                        }
                        break;
                    }
                    else // Unknown Message
                    {
                        state = Error;
                        udpMessage.sendUdpConfirm(sock,newServerAddr);
                        udpMessage.sendUdpError(sock,newServerAddr,"Invalid Messsage Type");
                        expectedConfirm = true;
                        watchDog = -1;                        
                    }
                    break;
                case End:
                    retVal = udpMessage.recvUpdConfirm();
                    if (SUCCESS == retVal)
                    {
                        return watchDog;
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
                        udpBackUpMessage = udpMessage;                          // Store Message For Case When Sending Again
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
                currentRetries++;
                udpBackUpMessage.sendUdpMessage(sock,newServerAddr);
            }
            elapsedTime = 0;
        }

    }
    if (currentRetries >= retryCount) {
        // Attempts Overrun
        fprintf(stderr,"ERR: Attemp Overrun\n");
        return FAIL;
    }

    return SUCCESS;
}

