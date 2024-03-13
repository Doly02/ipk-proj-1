/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      main.cpp
 *  Author:         Tomas Dolak
 *  Date:           21.02.2024
 *  Description:    Main file of the project. Implements Communication With Chat Server Using TCP & UDP Protocols
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           main.cpp
 *  @author         Tomas Dolak
 *  @date           21.02.2024
 *  @brief          Main file of the project. Implements Communication With Chat Server Using TCP & UDP Protocols
 * ****************************/



/************************************************/
/*                  Libraries                   */
/************************************************/
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "arguments.cpp"
#include "tcp.cpp"
#include "udp.cpp"
/************************************************/
/*                  Functions                   */
/************************************************/
UdpClient* globalUdpClientInstance = nullptr;
TcpClient* globalTcpClientInstance = nullptr;

void globalUdpHandleInterrupt(int signal) 
{
    if (globalUdpClientInstance) 
    {
        globalUdpClientInstance->udpHandleInterrupt(signal);
    }
}


void globalTcpHandleInterrupt(int signal) 
{
    if (globalTcpClientInstance) 
    {
        globalTcpClientInstance->tcpHandleInterrupt(signal);
    }
}


/************************************************/
/*                  Main                        */
/************************************************/
int main(int argc, char *argv[])
{
    try {
        int retVal = FAIL;

        // Parse Arguments
        arguments args(argc, argv); 

        if ("tcp" == args.transferProtocol)
        {
            TcpClient client(args.ipAddress, args.port, Client::TCP);
            // Set Global Instance Of TcpClient For Signal Handling
            globalTcpClientInstance = &client;
            // Set Signal Handler
            signal(SIGINT, globalTcpHandleInterrupt);
            // Run Tcp Client
            retVal = client.runTcpClient();
            return retVal;   
        }
        else if ("udp" == args.transferProtocol)
        {
            UdpClient client(args.ipAddress, args.port, args.confirmRetriesUDP, args.confirmTimeOutUDP);
            // Set Global Instance Of UdpClient For Signal Handling
            globalUdpClientInstance = &client;          
            // Set Signal Handler
            signal(SIGINT, globalUdpHandleInterrupt);
            // Run Udp Client
            retVal = client.runUdpClient();
            return retVal;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}