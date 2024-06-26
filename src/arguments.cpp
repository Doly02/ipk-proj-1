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
#include "../include/arguments.hpp"
/************************************************/
/*                  Class                       */
/************************************************/
arguments::arguments(int argc, char* argv[]) 
{
    processArguments(argc, argv);
}

/**
 * @brief Prints Help Message
 * 
 * Prints Help Message With Information About Arguments
*/
void arguments::printHelp()
{
    fprintf(stdout,"Usage: ./ipk24chat-client -t <Protocol> -s <HostName/IP> -p <Port> -d <Timeout> -r <MaxRetransmits> \n");
    fprintf(stdout,"-help, help message, can not be combined with other arguments\n");
    fprintf(stdout,"-t [tcp, udp] Mandatory Argument Specifying Transport Protocol Used For Connection\n");
    fprintf(stdout,"-s, Mandatory Argument Specifying Host Name or IP Address\n");
    fprintf(stdout,"-p, Optional Argument Specifying Server Port                            (Default Value: 4567)\n");
    fprintf(stdout,"-d, Optional Argument Specifying UDP Confirmation Timeout               (Default Value: 250)\n");
    fprintf(stdout,"-r, Optional Argument Specifying Maximum Number of UDP Retransmits      (Default Value: 3)\n");
    fprintf(stdout,"Order of the arguments can be changed\n");

    fprintf(stdout,"Example: ./ipk24chat-client -t tcp -s localhost -p 1234\n\n");
    fprintf(stdout,"Return Codes:\n");
    fprintf(stdout,"0 - Success\n");                
    fprintf(stdout,"Other Then 0 - ERRORS (Look In To macros.hpp To See Exact Meaning of Return Value\n");      
    fprintf(stdout,"\n");      

    fprintf(stdout,"Author: Tomas Dolak\n");
    fprintf(stdout,"Thank You For Using My Application\n");
}


void arguments::processArguments(int argc, char* argv[]) {
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
bool arguments::parseArguments(int argc, char* argv[]) {
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

/**
 * @brief Resolves Host Name
 * 
 * Resolves Host Name To IP Address
*/
void arguments::resolveHostName() {
    struct sockaddr_in sa;
    if (isIpAddress(hostName)) {
        // Korektní použití inet_pton
        int result = inet_pton(AF_INET, hostName.c_str(), &(sa.sin_addr));
        if(result != 1) {
            std::cerr << "Invalid IP address format." << std::endl;
            exit(1);
        }
        // ipAddress již není potřeba převádět, hostName je již ve správném formátu
        ipAddress = hostName;
        } 
        else 
        {
            // Transfer Host Name To IP Address
            struct hostent *host;
            host = gethostbyname(hostName.c_str());
            if (nullptr == host)
            {
                std::cerr << "Host Not Found" << std::endl;
                exit(1);    
            }
            // Convert the First IP Address to String
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, host->h_addr_list[0], ipStr, sizeof(ipStr));
            ipAddress = ipStr;
        }
}

/**
 * @brief Checks If The String Is IP Address
 * @param potentialAddress String With Potential IP Address
 * 
 * Checks If The String Is IP Address
 * @return True If The String Is IP Address, Otherwise False
*/
bool arguments::isIpAddress(std::string& potentialAddress)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, potentialAddress.c_str(), &(sa.sin_addr)) != 0;
}

