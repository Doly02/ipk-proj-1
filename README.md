# IPK Project 1: IPK Chat Client
- Author: Tomáš Dolák 
- Login: [xdolak09](https://www.vut.cz/lide/tomas-dolak-247220)
- Email: <xdolak09@stud.fit.vutbr.cz>

The goal of first project for computer communications and networks project is to implement a client for chat server[ using IPK24-CHAT protocol ] (https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201/README.md) which can communicate with any server using  IPK24-CHAT protocol.

## Table of contents
-   [Requirements](#Requirements)
-   [Project organization](#Project-organization)
- [Implementation](#Implementation)
-   [Arguments](#Arguments)
-   [Users Possiblities](#Users-possibilities)
-   [TCP Client](#TCP-Client)
-   [UDP Client](#Udp-Client)
-  [Implementation Details](#Implementation-details)
-   [Windows/Linux/MacOs portability ](#Windows-and-Linux-portability)
-   [Extra functionality ](#Extra-functionality )
-   [Resources](#Resources)

## Requirements
To build and run `ipk24chat-client`, you will need the following:

### Compiler
- **Clang++** with support for **C++17** standard. This project uses specific compiler flags to enforce code quality and standards. Make sure your compiler version supports `-std=c++17` along with the flags `-Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic`.

### Libraries
- **Google Test (gtest)**: Required for compiling and running the unit tests. Ensure you have Google Test installed on your system as it uses `-lgtest -lgtest_main -pthread` flags for linking.

### Build Tools
- **Make**: This project uses a `Makefile` for easy building and testing. Ensure you have Make installed on your system.

### Operating System
- The Makefile and C++ code were designed with Unix-like environments in mind (Linux, MacOS). While it may be possible to compile and run the project on Windows, using a Unix-like environment (or WSL for Windows users) is recommended.

### Debugging (Optional)
- For debugging, this project uses Clang++'s AddressSanitizer which requires `-fsanitize=address` flag. Ensure your development environment supports AddressSanitizer if you intend to use the debug build.

## Installation
1. Clone the repository to your local machine.
2. Navigate to the project directory.
3. Run `make` to build the client application. This will create the `ipk24chat-client` executable.
4. (Optional) Run `make test` to build and run the unit tests. Ensure you have Google Test installed.
5. (Optional) Run `make debug` to build the application with debug flags enabled.

Please refer to the Makefile for additional targets and commands.


### Project Organization 
```
ipk-proj-1/
│
├── include/                # Header files for class declarations
│   ├── arguments.hpp       # Header file for Argument Processing Class
│   ├── baseMessages.hpp    # Header file for Base Message Class
│   ├── tcpMessages.hpp     # Header file for TCP Message Class
│   ├── udpMessages.hpp     # Header file for UDP Message Class
│   ├── baseClient.hpp      # Header file for Base Client Class
│   ├── tcpClient.hpp       # Header file for TCP Client Class
│   └── udpClient.hpp       # Header file for UDP Client Class
│
├── src/                    # Source files containing class definitions and main application logic
│   ├── arguments.cpp       # Implementation of Class For Argument Processing
│   ├── baseMessages.cpp    # Implementation of Base Class For Messages
│   ├── tcpMessages.cpp     # Implementation of Class For TCP Messages
│   ├── udpMessages.cpp     # Implementation of Class For UDP Messages
│   ├── baseClient.cpp      # Implementation of Base Class For Client
│   ├── tcpClient.cpp       # Implementation of Class For TCP Client
│   ├── udpClient.cpp       # Implementation of Class For UDP Client
│   └── main.cpp            # Main application entry point
│
├── test/                   # Test files
│   ├── unit-tests/         # Tests for routine operations over messages, inputs, and arguments
│   │   └── test_parseMessages.cpp
│   └── tests-with-server/  # Tests of Communication With Server
│
├── doc/                    # Documentation files and resources
│   └── doxygen/            # Directory of Doxygen Documentation
│
├── Makefile                # Makefile for compiling the project
│
└── README.md               # Overview and documentation for the project
```

## Implementation 

### Arguments

Arguments that are checked and verified:

| Argument | Value           | Possible Values            | Meaning or Expected Program Behavior                        |
|----------|-----------------|----------------------------|-------------------------------------------------------------|
| `-t`     | User provided   | `tcp` or `udp`             | Transport protocol used for connection                      |
| `-s`     | User provided   | IP address or hostname     | Server IP or hostname                                       |
| `-p`     | `4567`          | 0 to 65535                 | Server port                                                 |
| `-d`     | `250`           | 0 to 65535                 | UDP confirmation timeout in milliseconds                    |
| `-r`     | `3`             | 0 to 255                   | Maximum number of UDP retransmissions                       |
| `-h`     |                 |                            | Prints program help output and exits                        |

**Note:** The arguments `-t`, `-s` are mandatory. Arguments `-p`, `-d`, `-r` have default values, so they are optional. Using `-h` immediately terminates the program with a help statement, regardless of the other arguments provided. If the count of arguments or the count of identifiers (-h,-m,-p, -help) are invalid the program will send error message to standard error output.

### User's possibilities 

#### Authentication Command 
Authenticates the client. 
```
/auth {Username} {Secret} {DisplayName}
```

#### Join Command
Joins Client To Join Specific Channel.
```
/join {ChannelID}
```

#### Rename Command 
Sets Different Local Display Name. 
```
/rename {DisplayName}
```

#### Command Print Help
Prints Help Statement.
```
/help
```
#### Messages
Everything Else As The Previous Commands Are Interpreted As Regular Message.

### TCP Client

#### Introduction To TCP Communication
To establish a `TCP connection`, the initial step involves creating a socket, specifying it as a TCP socket with the `SOCK_STREAM` parameter within the socket function. Following socket creation, a three-way handshake is executed, setting up a dedicated connection socket on the server side for the client, a process that, though invisible to the client, occurs server-side and within the transport layer. Upon completing this setup, the connect function is engaged, allowing data exchange to commence.

<p align="center">
  <img src="doc/pics/tcp_communication.png" alt="Ilustration of TCP Communication" width="600"/><br>
  <em>Ilustration of TCP Communication</em>
</p>

In scenarios involving text-based communication, encoding or decoding special strings isn't necessary. Monitoring ensures the exchange of `HELLO` and `BYE` messages. Prior to receiving server messages, the sent string is cleared. Should the `BYE` message remain unsent, the program autonomously sends it, simultaneously notifying the user of the omission through an error indication.

To conclude the session, the program invokes `shutdown` with the parameter `2`, signaling the cessation of both sending and receiving activities. On Windows systems, termination and shutdown actions are performed using `closesocket`. Following these steps, the close function is called to officially close the socket, thereby ending the interaction.


### UDP Client

#### Introduction To UDP Communication

UPD protocol provides a connectionless and unreliable communication service, meaning that it does not establish a dedicated connection between the sender and receiver and does not guarantee the delivery of data packets. Instead, UDP sends data packets, called datagrams, without any acknowledgment or error checking. These datagrams are transmitted independently and can arrive out of order, be duplicated, or even be lost during transmission. The lack of built-in reliability mechanisms in UDP allows fast, low-latency communication, making it ideal for that can tolerate lost packets, such as streaming audio or video, where speed is more crucial than perfect accuracy. [3] [4]


## Resources 
[RFC791]: Information Sciences Institute, University of Southern California. "Internet Protocol" [online]. September 1981. [cited 2024-03-26]. DOI: 10.17487/RFC791. Available at [https://www.ietf.org/rfc/rfc793.txt](https://www.ietf.org/rfc/rfc793.txt).

James F. Kurose, Keith W. Ross: *Computer Networking: A Top Down Approach* (Eighth Edition). Figure 2.28 [cited 2024-03-20].

Gorry Fairhurst: "The User Datagram Protocol (UDP)" [online]. 19.11.2008. [cited 2024-03-20]. Available at [https://www.erg.abdn.ac.uk/users/gorry/course/inet-pages/udp.html](https://www.erg.abdn.ac.uk/users/gorry/course/inet-pages/udp.html).

Marek Majkowski: "Everything you ever wanted to know about UDP sockets but were afraid to ask, part 1" [online]. 25.11.2021. [cited 2024-03-26]. Available at [https://blog.cloudflare.com/everything-you-ever-wanted-to-know-about-udp-sockets-but-were-afraid-to-ask-part-1](https://blog.cloudflare.com/everything-you-ever-wanted-to-know-about-udp-sockets-but-were-afraid-to-ask-part-1).

Andrew T. Campbell: "Socket Programming" [online]. December 2023. Available at [https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html](https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html).