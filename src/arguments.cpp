/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      tcp_messages.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Implements Serialization & Deserialization of Messages For TCP Protocol.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp_messages.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Implements Serialization & Deserialization of Messages For TCP Protocol.
 * ****************************/


/************************************************/
/*                  Libraries                   */
/************************************************/
#include <iostream>
#include <netdb.h>
#include <ctype.h>
#include <cstring>
#include <arpa/inet.h>
#include <cstdlib>
/************************************************/
/*                  Class                       */
/************************************************/
class arguments
{
public:
    /*      Arguments             */
    std::string transferProtocol;               //!< Transfer Protocol Used For Communication (1 - TCP, 2 - UDP)
    std::string hostName;                       //!< Host Name of The Server
    struct in_addr ipAddress;                   //!< Structure For IP Address of The Server
    uint16_t port                   = 4567u;    //!< Port Number on Which The Server Is Listening
    uint16_t confirmTimeOutUDP      = 250u;     //!< Time Out For UDP Protocol
    uint8_t confirmRetriesUDP       = 3u;       //!< Number of Retries For UDP Protocol

    arguments(int argc, const char* argv[]) {
        processArguments(argc, argv);
    }

    /**
     * @brief Prints Help Message
     * 
     * Prints Help Message With Information About Arguments
    */
    void printHelp()
    {
        printf("Usage: ./client -t <Protocol> -s <HostName/IP> -p <Port> -d <Timeout> -r <MaxRetransmits> \n");
        printf("-help, help message, can not be combined with other arguments\n");
        printf("-t [tcp, udp] Mandatory Argument Specifying Transport Protocol Used For Connection\n");
        printf("-s, Mandatory Argument Specifying Host Name or IP Address\n");
        printf("-p, Optional Argument Specifying Server Port                            (Default Value: 4567)\n");
        printf("-d, Optional Argument Specifying UDP Confirmation Timeout               (Default Value: 250)\n");
        printf("-r, Optional Argument Specifying Maximum Number of UDP Retransmits      (Default Value: 3)\n");
        printf("Order of the arguments can be changed\n");

        printf("Example: ./client -t tcp -s localhost -p 1234\n\n");
        printf("Error Codes:\n");
        printf("0 - Success\n");                //TODO:
        printf("1 - Invalid Arguments\n");      //TODO:
        printf("2 - Connection Error\n");       //TODO:

        printf("Author: Tomas Dolak\n");
        printf("Thank You For Using My Application\n");
    }


    void processArguments(int argc, const char* argv[]) {
        if(false == parseArguments(argc, argv))
        {
            std::cerr << "Invalid Arguments" << std::endl;
            printHelp();
            exit(1);
        }
        // Resolve Host Name
        resolveHostName();
    }

    /**
     * @brief Parses Arguments
     * @param argc Number of Arguments
     * @param argv Array of Arguments
     * 
     * Parses Arguments From Command Line.
     * @return Arguments Are Not Valid,  Exits With Error Code 1, Otherwise Returns 0.
    */
    bool parseArguments(int argc, const char* argv[]) {
        for (uint8_t i = 1; i < argc; i++) {
            std::string flag(argv[i]);
            if(i + 1 < argc)
            {
                /* Transfer Protocol    */
                if ("-t" == flag)
                {
                    transferProtocol = argv[i + 1];
                }
                /* Host Name            */
                else if ("-s" == flag)
                {
                    hostName = argv[i + 1];
                }
                /* Port                 */
                else if ("-p" == flag)
                {
                    // Transfer String To Number
                    port = static_cast<uint16_t>(std::stoi(argv[i + 1]));
                }
                /* Confirmation Timeout */
                else if ("-d" == flag)
                {
                    confirmTimeOutUDP = static_cast<uint16_t>(std::stoi(argv[i + 1]));
                }
                /* Confirmation Retries */
                else if ("-r" == flag)
                {
                    confirmRetriesUDP = static_cast<uint8_t>(std::stoi(argv[i + 1]));
                }
                /* Help                 */
                else if ("-h" == flag)
                {
                    printHelp();
                    exit(0);
                }

            }
            else
            {
                if ("-h" == flag)
                {
                    printHelp();
                    exit(0);
                }
                else
                {
                    // TODO: Update Please Error Message
                    std::cerr << "Invalid Arguments" << std::endl;
                    exit(1);
                }
            }
        }
        return true;
    }

    void resolveHostName()
    {
        if (isIpAddress(hostName))
        {
            // Transfer IP Address From String To 'in_addr' Structure
            inet_pton(AF_INET, hostName.c_str(), &ipAddress);
        }
        else 
        {
            // Transfer Host Name To IP Address
            struct hostent *host;
            host = gethostbyname(hostName.c_str());
            if (nullptr == host)
            {
                std::cerr << "Host Not Found" << std::endl;
                exit(1);    //TODO: Error Code CHECK!
            }
            // Copy First IP Address To ipAddress Structure
            memcpy(&ipAddress, host->h_addr_list[0], sizeof(ipAddress));
        }
    }

private:
    /**
     * @brief Checks If The String Is IP Address
     * @param potentialAddress String With Potential IP Address
     * 
     * Checks If The String Is IP Address
     * @return True If The String Is IP Address, Otherwise False
    */
    bool isIpAddress(std::string& potentialAddress)
    {
        struct sockaddr_in sa;
        int result = inet_pton(AF_INET, potentialAddress.c_str(), &(sa.sin_addr));
        return (result != 0);
    }

};


