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
        
        std::string transferProtocol;               //!< Transfer Protocol Used For Communication (1 - TCP, 2 - UDP)
        std::string hostName;                       //!< Host Name of The Server
        std::string ipAddress;                      //!< IP Address of The Server
        uint16_t port               = 4567;         //!< Port Number on Which The Server Is Listening
        uint16_t confirmTimeOutUDP  = 250;          //!< Time Out For UDP Protocol
        uint8_t confirmRetriesUDP   = 3;            //!< Number of Retries For UDP Protocol
        /**
         * @brief Constructor
         * @param argc Number of Arguments
         * @param argv Array of Arguments
         * 
         * Constructor For Arguments Class
        */
        arguments(int argc, char* argv[]);          // Constructor Declaration
        /**
         * @brief Prints Help Message
         * 
         * Prints Help Message With Information About Arguments
        */
        void printHelp();                           // Prints Help Message Declaration

    private:
        /**
         * @brief Processes Arguments
         * @param argc Number of Arguments
         * @param argv Array of Arguments
         * 
         * Processes Arguments From Command Line.
        */        
        void processArguments(int argc, char* argv[]); // Processes Program Arguments Declaration
        /**
         * @brief Parses Arguments
         * @param argc Number of Arguments
         * @param argv Array of Arguments
         * 
         * Parses Arguments From Command Line.
         * @return Arguments Are Not Valid,  Exits With Error Code 1, Otherwise Returns 0.
        */         
        bool parseArguments(int argc, char* argv[]);   // Parses Command Line Arguments Declaration
        /**
         * @brief Resolves Host Name
         * 
         * Resolves Host Name To IP Address
        */        
        void resolveHostName();                        // Resolves Host Name To IP Address Declaration
        /**
         * @brief Checks If The String Is IP Address
         * @param potentialAddress String With Potential IP Address
         * 
         * Checks If The String Is IP Address
         * @return True If The String Is IP Address, Otherwise False
        */          
        bool isIpAddress(std::string& potentialAddress); // Checks If String Is IP Address Declaration
};

#endif // ARGUMENTS_HPP
