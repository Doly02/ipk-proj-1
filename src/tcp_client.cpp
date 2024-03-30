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
        int retVal = 0;
    
        while (!authConfirmed) 
        {
            retVal = poll(fds,NUM_FILE_DESCRIPTORS,UNLIMITED_TIMEOUT);
            if (FAIL == retVal)
            {
                // TODO Nejaka hlaska 
                exit(FAIL);
            }

            if (fds[STDIN].revents & POLLIN)
            {
                if (NULL!= fgets(buf,BUFSIZE,stdin))
                {

                    tcpMessage.readAndStoreContent(buf);
                    retVal = tcpMessage.checkMessage();
                    if (SUCCESS == retVal && TcpMessages::COMMAND_AUTH == tcpMessage.msg.type)
                    {
                        tcpMessage.sendAuthMessage(sock);
                    }
                    else if (NON_VALID_PARAM == retVal)
                    {
                        printf("ERR: Invalid Parameter/s In The Message\n");
                    }

                }
                memset(buf,0,sizeof(buf));
            }

            if (fds[SOCKET].revents & POLLIN)
            {
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
                    else
                    {
                        // TODO Message
                        exit(AUTH_FAILED);
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
        int retVal = 0;
        ClientState state = Authentication;
        checkAuthentication();
        state = Open;

        while (true)
        {
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
                    retVal = tcpMessage.checkMessage();
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
                }
                memset(buf,0,sizeof(buf));
            }    

            if (fds[SOCKET].revents & POLLIN)
            {
                tcpMessage.cleanMessage();
                int bytesRx = recv(sock,buf,BUFSIZE-1,0);
                if (0 >= bytesRx)
                {
                    printf("Server Disconnected\n");
                    exit(bytesRx); // TODO
                }
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
                            state = Open;
                        break;
                    case Open:
                        retVal = tcpMessage.parseMessage();
                        if (SUCCESS == retVal)
                        {
                            tcpMessage.printMessage();
                            state = Open;
                        }
                        break;
                    case End:
                        printf("End State\n");
                        exit(0);
                    case Error:
                        printf("Error State\n");
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


// NOTES:
/* Potreba dodelat -> Momentalne se pracuje se dvemi buffery je treba pouzit jen jeden
 *   -> pouziva se char Buf[1024] a <vector> Content -> Nejlepsi bude pouzivat Pouze Content a kdyz budes potrebovat tak si to prekonvertujes na Buf "ale jakoby pouze virtualne"
 */
/* Co je potreba dodelat?
 * - Osetrit chyby pri odesilani zprav                  -> SHOULD BE DONE
 * - Osetrit chyby pri prijimani zprav                  -> ??? (Muze ti prijit ERROR, ktery rovnou posilas)
 * - Dopracovat odesilani vsech moznych zprav v TCP     -> ???
 * - Rozpracovat a dodelat UDP                          -> ???
 * - Testovat Testovat                                  -> ???
 * 
 * TODO: Zbavit se vsech TODOs                          -> ???
 * -> Co se ma dit pokud ti podruhe prijde autentifikace-> ???
 * -> rozdelit CheckIfErrorOrBye
 */

