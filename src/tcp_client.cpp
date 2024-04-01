/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      tcp_client.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Implements Communication With Chat Server Thru TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp_client.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Implements Communication With Chat Server Thru TCP Protocol.
 * ****************************/

/************************************************/
/*                  Libraries                   */
/************************************************/
#include "../include/tcp_client.hpp"
/************************************************/
/*                  CLASS                       */
/************************************************/
TcpClient::TcpClient(std::string addr, int port, uint protocol) : Client(addr, port, protocol) 
{
    // Constructor
    fds[STDIN].fd = STDIN_FILENO;
    fds[STDIN].events = POLLIN;
    fds[SOCKET].fd = sock;
    fds[SOCKET].events = POLLIN;
}

TcpClient::~TcpClient() {
    // Destructor
}

void TcpClient::tcpHandleInterrupt(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {   
        /* Send Message */
        tcpMessage.sentByeMessage(sock);
        exit(SUCCESS);
    }
}

void TcpClient::checkAuthentication() 
{
    bool sendAuthMessage = false;
    int retVal = 0;

    while (!authConfirmed) 
    {
        retVal = poll(fds,NUM_FILE_DESCRIPTORS,UNLIMITED_TIMEOUT);
        if (FAIL == retVal)
        {
            fprintf(stderr,"ERR: poll() Failed\n");
            exit(FAIL);
        }

        if (fds[STDIN].revents & POLLIN)
        {
            if (NULL!= fgets(buf,BUFSIZE,stdin))
            {

                tcpMessage.readAndStoreContent(buf);
                retVal = tcpMessage.checkMessage();
                if (SUCCESS == retVal && TcpMessages::COMMAND_AUTH == tcpMessage.msg.type && !sendAuthMessage)
                {
                    tcpMessage.sendAuthMessage(sock);
                    sendAuthMessage = true;
                }
                else if (SUCCESS == retVal && TcpMessages::COMMAND_AUTH == tcpMessage.msg.type && sendAuthMessage)
                {
                    fprintf(stderr,"ERR: Authentication Message Already Send To Server, Wait For The Reply\n");
                }
                else if (NON_VALID_PARAM == retVal)
                {
                    fprintf(stderr,"ERR: Invalid Parameter/s In The Message\n");
                }

            }
            memset(buf,0,sizeof(buf));
        }

        if (fds[SOCKET].revents & POLLIN)
        {
            printf("RECEIVED\n");
            memset(buf,0,sizeof(buf));
            int bytesRx = recv(sock,buf,BUFSIZE-1,0);
            if (0 < bytesRx)
            {
                tcpMessage.readAndStoreContent(buf);

                tcpMessage.checkIfErrorOrBye(sock);

                retVal = tcpMessage.handleAuthReply();
                if(SUCCESS == retVal)
                {
                    authConfirmed = true;
                    break;
                }
                if (AUTH_FAILED == retVal)
                {
                    sendAuthMessage = false;
                }     
            }
            else
            {
                // TODO Message
                exit(AUTH_FAILED);
            }
            memset(buf,0,sizeof(buf));
        }


    }
}


int TcpClient::runTcpClient()
{
    /* Variables */
    bool expectReply = false;
    int retVal = 0;
    ClientState state = Authentication;
    checkAuthentication();
    state = Open;

    while (true)
    {
        // Process Message From Queue 
        if (!messageQueue.empty() && !expectReply)
        {
            TcpMessages frontMessage = messageQueue.front();
            messageQueue.pop();
            retVal = frontMessage.checkMessage();
            if (SUCCESS == retVal)
            {
                if (BaseMessages::COMMAND_JOIN == tcpMessage.msg.type)
                {
                    tcpMessage.sendJoinMessage(sock);
                    state = RecvReply;
                }                
                else if (BaseMessages::MSG == tcpMessage.msg.type)
                {
                    tcpMessage.sentUsersMessage(sock);
                    state = Open;
                }
                else if (BaseMessages::COMMAND_HELP == tcpMessage.msg.type)
                {
                    tcpMessage.printHelp();
                }
                else if (BaseMessages::COMMAND_AUTH == tcpMessage.msg.type)
                {
                    fprintf(stderr,"ERR: Authentication Already Processed - Not Possible Again\n");
                }
            }
            else if (NON_VALID_PARAM == retVal)
                fprintf(stderr,"ERR: Non Valid Parameter/s Of Message\n");

        }

        retVal = poll(fds,NUM_FILE_DESCRIPTORS,UNLIMITED_TIMEOUT);
        if (FAIL == retVal)
        {
            // Nejaka hlaska 
            exit(FAIL);
        }

        if (fds[STDIN].revents & POLLIN)
        {
            if (NULL!= fgets(buf,BUFSIZE,stdin))
            {
                tcpMessage.readAndStoreContent(buf);
                if (expectReply || !messageQueue.empty())                   // If We Wait For REPLY Message, Store The Message Into Queue, Message Will Be Send Later
                {
                        TcpMessages toQueueMessage = tcpMessage;
                        messageQueue.push(toQueueMessage);
                }
                else                                                        // Else Send The Message To The Server
                {
                    retVal = tcpMessage.checkMessage();
                    if (NON_VALID_PARAM == retVal)
                        fprintf(stderr,"Non Valid Parameters\n");
                    else if (BaseMessages::COMMAND_JOIN == tcpMessage.msg.type)
                    {
                        tcpMessage.sendJoinMessage(sock);
                        state = RecvReply;
                        expectReply = true;                                 // Block The Sending Messages From STDIN
                    }                
                    else if (BaseMessages::MSG == tcpMessage.msg.type)
                    {
                        tcpMessage.sentUsersMessage(sock);
                        state = Open;
                    }
                    else if (BaseMessages::COMMAND_HELP == tcpMessage.msg.type)
                    {
                        tcpMessage.printHelp();
                    }
                    else if (BaseMessages::COMMAND_AUTH == tcpMessage.msg.type)
                    {
                        fprintf(stderr,"ERR: Authentication Already Processed - Not Possible Again\n");
                    }
                }
            }
            memset(buf,0,sizeof(buf));
        }    

        if (fds[SOCKET].revents & POLLIN)
        {
            tcpMessage.cleanMessage();
            int bytesRx = recv(sock,buf,BUFSIZE-1,0);
            if (0 >= bytesRx)
            {
                fprintf(stderr,"ERR: Server Disconnected\n");
                exit(FAIL); 
            }
            printf("RECEIVED\n");
            tcpMessage.readAndStoreContent(buf);
            tcpMessage.checkIfErrorOrBye(sock);
            switch (state)
            {
                case RecvReply:
                    retVal = tcpMessage.checkJoinReply();
                    if (SUCCESS != retVal)
                    {
                        tcpMessage.sendErrorMessage(sock,BaseMessages::REPLY);
                        state = Error;
                    }
                    if (BaseMessages::REPLY == tcpMessage.msg.type)
                    {
                        state = Open;
                        expectReply = false;                            // Enable To Send Message From STDIN Or From Queue (In Case If Queue Is Not Empty)
                    }
                    break;
                case Open:
                    retVal = tcpMessage.parseMessage();
                    printf("Ret Val %d\n",retVal);
                    if (SUCCESS == retVal)
                    {
                        tcpMessage.printMessage();
                        state = Open;
                    }
                    else if (NON_VALID_PARAM == retVal)
                    {
                        tcpMessage.insertErrorMsgToContent("Non Valid Parameters");
                        tcpMessage.sendErrorMessage(sock,BaseMessages::UNKNOWN_MSG_TYPE);
                        state = Error;
                    }
                    else if (MSG_PARSE_FAILED == retVal)
                    {
                        state = Error;
                    }
                    break;
                case End:
                    exit(0);
                case Error:
                    tcpMessage.sentByeMessage(sock);
                    exit(FAIL);
                case Authentication:
                default:
                    break;
            }
            memset(buf,0,sizeof(buf));
        }       
    }
    return 0;
};


