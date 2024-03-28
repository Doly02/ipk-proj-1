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
    TcpClient::TcpClient(std::string addr, int port, uint protocol) : Client(addr, port, protocol) {
        // Constructor
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
        struct pollfd fds[NUM_FILE_DESCRIPTORS];
        
        fds[STDIN].fd = STDIN_FILENO;
        fds[STDIN].events = POLLIN;
        fds[SOCKET].fd = sock;
        fds[SOCKET].events = POLLIN;


        while (!authConfirmed) 
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

                    tcpMessage.checkIfErrorOrBye();

                    retVal = tcpMessage.handleReply();
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
        bool joinSend = false;
        bool joinServerMsgSend = false;
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
                
                // Receive A Packet From Server
                int bytesRx = recv(sock, buf, BUFSIZE - 1, 0);
                if (bytesRx > 0) 
                {
                    //buf[bytesRx]    = '\0'; //TODO: Check If This Is Right ("\r\n")
                    buf[BUFSIZE-1]  = '\0';
                    tcpMessage.readAndStoreContent(buf);  
                    /* Check If Error Was Send */
                    retVal = tcpMessage.checkIfErrorOrBye();
                    if (EXTERNAL_ERROR == retVal)
                    {
                        // Upravit Tak ze se ERR Message Pridava uz ve funkci checkIfErrorOrBye
                        return retVal;
                    }
                    else if (SERVER_SAYS_BYE == retVal)
                    {
                        return SUCCESS;
                    }
                    // Check If Is Alles Gute
                    if (true == checkReply && true == joinSend)
                    {
                        if (!joinServerMsgSend)
                        {
                            retVal = tcpMessage.checkJoinReply();
                            if (SUCCESS != retVal)
                            {

                                tcpMessage.basePrintInternalError(retVal); //FIXME -> Zkotrolovat 
                                tcpMessage.sendErrorMessage(sock,BaseMessages::REPLY);
                                return retVal;
                            }
                        }

                        if (joinServerMsgSend)
                        {

                            retVal = tcpMessage.handleReply();
                            if (SUCCESS != retVal)
                            {
                                tcpMessage.sendErrorMessage(sock,BaseMessages::REPLY);
                                tcpMessage.basePrintExternalError();    // FIXME -> Zkotrolovat
                                return JOIN_FAILED;
                            }
                            checkReply = false;
                            joinSend = false;
                            joinServerMsgSend = false;
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
                            checkReply = false;
                        }
                        else
                        {   // FIXME Tady je to treba rozlisovat podle navratovych hodnot
                            tcpMessage.sendErrorMessage(sock,BaseMessages::REPLY);      // Send Error Message
                            //tcpMessage.printError();
                            return -1;
                        }
                    }
                    else
                    {

                        // Print The Content Of The Buffer
                        retVal = tcpMessage.parseMessage();
                        if (SUCCESS == retVal)
                            tcpMessage.printMessage();
                        else
                        {
                            // TODO: Dve moznosti co to muze vratit -> je treba to osetrit!
                        }
                    }
                    
                    // Clear The Buffer After All
                    memset(buf, 0, sizeof(buf));
                } 
                else if (bytesRx == 0) 
                {
                    /* Server Closed The Connection */
                    tcpMessage.insertErrorMsgToContent("Server Closed The Connection");
                    tcpMessage.basePrintInternalError(0); //TODO:
                    break;
                } 
                else 
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) 
                    {
                        tcpMessage.insertErrorMsgToContent("recv failed (EAGAIN/EWOULDBLOCK)");
                        tcpMessage.basePrintInternalError(0); 
                    }
                }
            memset(buf, 0, sizeof(buf));
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
                    if (0 == RetValue) 
                    {
                        if ((int)TcpMessages::COMMAND_JOIN == tcpMessage.msg.type && sendAuth)
                        {
                            // Sent To Server Join Message
                            tcpMessage.sendJoinMessage(sock);
                            joinSend = true;
                            joinServerMsgSend = false;
                            checkReply = true;
                        }
                        if ((int)TcpMessages::COMMAND_BYE == tcpMessage.msg.type )
                        {
                            // Sent To Server Bye Message
                            tcpMessage.sentByeMessage(sock);
                            exit(EXIT_SUCCESS);
                        }
                        // TODO: Missing Some Message Types
                        if ((int)TcpMessages::MSG == tcpMessage.msg.type)
                        {
                            // Sent User's Message To Server
                            tcpMessage.sentUsersMessage(sock); 
                            memset(buf, 0, sizeof(buf));
                        }
                        /// TODO: Missing ERRORMSG
                    }
                    else
                    {
                        return RetValue;
                    }
                }
            memset(buf, 0, sizeof(buf));
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

