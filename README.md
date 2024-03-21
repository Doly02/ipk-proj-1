# IPK Project 1: IPK Chat Client
- Author: Tomáš Dolák 
- Login: [xdolak09](https://www.vut.cz/lide/tomas-dolak-247220)
- Email: <xdolak09@stud.fit.vutbr.cz>

The goal of first project for computer communications and networks project is to implement a client for chat server[ using IPK24-CHAT protocol ] (https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201/README.md) which can communicate with any server using  IPK24-CHAT protocol.

## Table of contents
-   [UML Diagram](#UML-diagram)
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

# Project Requirements 

### Installation


### Project Organization 

ipk-proj-1/
│
├── include/                # Header files for class declarations
│   ├── arguments.hpp
│   ├── baseMessages.hpp         # Header file for MyClass
│   ├── tcpMessages.hpp    # Header file for AnotherClass
│   ├── udpMessages.hpp
|   ├── baseClient.hpp
│   ├── tcpClient.hpp
│   └── udpClient.hpp
│
├── src/                    # Source files containing class definitions and main application logic
│   ├── arguments.cpp       # Implementation of Class For Argument Processing
│   ├── baseMessages.cpp    # Implementation of Base Class For Messages
│   ├── tcpMessages.cpp     # Implementation of Class For TCP Messages
│   ├── udpMessages.cpp     # Implementation of Class For UDP Messages
|   ├── baseClient.cpp      # Implementation of Base Class For Client
│   ├── tcpClient.cpp       # Implementation of Class For TCP Client
│   ├── udpClient.cpp       # Implementation of Class For UDP Client
│   └── main.cpp            # Main application entry point
│
├── test/                   # Test files
│   ├── unit-texts/         # Tests for routine operations over messages, inputs and arguments
│   │   └── text_parseMessages.cpp
│   └── tests-w-server/     # Tests of Communication With Server
│
├── doc/                    # Documentation files and resources
│   └── doxygen/            # Directory of Doxygen Documentation
│
├── Makefile                # Makefile for compiling the project
│
└── README.md               # Overview and documentation for the project

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

# Authentication Command 
Authenticates the client. 
```
/auth {Username} {Secret} {DisplayName}
```

# Join Command
Joins Client To Join Specific Channel.
```
/join {ChannelID}
```

# Rename Command 
Sets Different Local Display Name. 
```
/rename {DisplayName}
```

# Command Print Help
Prints Help Statement.
```
/help
```
# Messages
Everything Else As The Previous Commands Are Interpreted As Regular Message.

### TCP Client



### UDP Client