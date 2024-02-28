/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      arguments.cpp
 *  Author:         Tomas Dolak
 *  Date:           26.02.2024
 *  Description:    Implements Handling Program Arguments.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           arguments.cpp
 *  @author         Tomas Dolak
 *  @date           26.02.2024
 *  @brief          Implements Handling Program Arguments.
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
    std::string ipAddress;                   //!< Structure For IP Address of The Server
    uint16_t port                   = 4567u;    //!< Port Number on Which The Server Is Listening
    uint16_t confirmTimeOutUDP      = 250u;     //!< Time Out For UDP Protocol
    uint8_t confirmRetriesUDP       = 3u;       //!< Number of Retries For UDP Protocol

    arguments(int argc, char* argv[]) {
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


    void processArguments(int argc, char* argv[]) {
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
    bool parseArguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) { // Změna typu na int pro kompatibilitu s argc
        std::string flag(argv[i]);

        if ("-h" == flag) {
            printHelp();
            exit(0);
        } else if (i + 1 < argc) { // Opravená podmínka pro zpracování argumentů
            if ("-t" == flag) {
                transferProtocol = argv[++i]; // Přeskočení na hodnotu flagu
            } else if ("-s" == flag) {
                hostName = argv[++i];
            } else if ("-p" == flag) {
                port = static_cast<uint16_t>(std::stoi(argv[++i]));
            } else if ("-d" == flag) {
                confirmTimeOutUDP = static_cast<uint16_t>(std::stoi(argv[++i]));
            } else if ("-r" == flag) {
                confirmRetriesUDP = static_cast<uint8_t>(std::stoi(argv[++i]));
            } else {
                std::cerr << "Unknown flag: " << flag << std::endl;
                return false; // Vrátí false, pokud narazí na neznámý flag
            }
        } else {
            std::cerr << "Missing value for flag: " << flag << std::endl;
            return false; // Vrátí false, pokud flag nemá hodnotu
        }
    }
    return true; // Všechny argumenty byly zpracovány úspěšně
}

private:

    /**
     * @brief Resolves Host Name
     * 
     * Resolves Host Name To IP Address
    */
    void resolveHostName()
    {
        if (isIpAddress(hostName))
        {
            // Transfer IP Address From String To 'in_addr' Structure
            inet_pton(AF_INET, hostName.c_str(), &ipAddress);
            ipAddress = hostName;
        } else 
        {
            // Transfer Host Name To IP Address
            struct hostent *host;
            host = gethostbyname(hostName.c_str());
            if (nullptr == host)
            {
                std::cerr << "Host Not Found" << std::endl;
                exit(1);    //TODO: Error Code CHECK!
            }
            // Convert the First IP Address to String
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, host->h_addr_list[0], ipStr, sizeof(ipStr));
            ipAddress = ipStr;
            printf("IP Address: %s\n", ipAddress.c_str());
        }
    }

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
        return inet_pton(AF_INET, potentialAddress.c_str(), &(sa.sin_addr)) != 0;
    }

};


