/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      arguments.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Header File For Implementation Handling Program Arguments.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           arguments.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Header File For Implementation Handling Program Arguments.
 * ****************************/


#ifndef ARGUMENTS_HPP
#define ARGUMENTS_HPP

#include <string>
#include <cstdint>

/************************************************/
/*                  Class                       */
/************************************************/
class arguments 
{
    public:
        
        std::string transferProtocol;               // Transfer Protocol Used For Communication (1 - TCP, 2 - UDP)
        std::string hostName;                       // Host Name of The Server
        std::string ipAddress;                      // IP Address of The Server
        uint16_t port;                              // Port Number on Which The Server Is Listening
        uint16_t confirmTimeOutUDP = 250;           // Time Out For UDP Protocol
        uint8_t confirmRetriesUDP  = 3;             // Number of Retries For UDP Protocol

        arguments(int argc, char* argv[]);          // Constructor Declaration

        void printHelp();                           // Prints Help Message Declaration

    private:
        
        void processArguments(int argc, char* argv[]); // Processes Program Arguments Declaration
        
        bool parseArguments(int argc, char* argv[]);   // Parses Command Line Arguments Declaration
        
        void resolveHostName();                        // Resolves Host Name To IP Address Declaration
        
        bool isIpAddress(std::string& potentialAddress); // Checks If String Is IP Address Declaration
};

#endif // ARGUMENTS_HPP
