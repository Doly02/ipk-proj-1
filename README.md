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
│
├── src/                    # Source files containing class definitions and main application logic
│
├── test/                   # Test files
│   ├── unit-tests/         # Tests for routine operations over messages, inputs, and arguments
│   │   
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

**Note:** Be aware that there are limitations for `{ChannelID}`, `{DisplayName}`, and similar fields. For instance, packets should not exceed the default Ethernet MTU of 1500 octets as defined by [RFC 894](https://tools.ietf.org/html/rfc894). Exceeding this limit could result in packet fragmentation, potentially affecting communication efficiency and reliability.

### Unblocking Communication With poll()
In programming, when implementing chat client is better to use unblocking communication with poll() rather than busy-waiting for data on the socket. Client can fluently check activity on stadart input and socket using poll(), which is a more efficient and elegant solution. The benefits of poll() are that it allows the program to efficiently check multiple sockets for activity simultaneously without wasting CPU time. An alternative to the poll() function is the select() function, which is also commonly used for checking multiple sockets for activity, but may have some limitations in terms of scalability and performance. 

### TCP Client

#### Introduction To TCP Communication

TCP is a reliable and connection-oriented protocol that operates at the transport layer of the TCP/IP protocol suite (T, 2016). It provides reliable, ordered, and error-checked delivery of data packets over an IP network (Meghanathan, 2014). TCP works by establishing a reliable and ordered connection between two devices, typically a client and a server. During the connection establishment phase, a three-way handshake process is used, where the client and server exchange SYN (synchronize) and ACK (acknowledge) packets to confirm the connection (Postel et al., 1995). Once the connection is established, TCP segments the data into small packets and adds sequence numbers to each packet.

<p align="center">
  <img src="doc/pics/tcp_communication.png" alt="Ilustration of TCP Communication" width="600"/><br>
  <em>Ilustration of TCP Communication</em>
</p>

In scenarios involving text-based communication, encoding or decoding special strings isn't necessary. Monitoring ensures the exchange of `HELLO` and `BYE` messages. Prior to receiving server messages, the sent string is cleared. Should the `BYE` message remain unsent, the program autonomously sends it, simultaneously notifying the user of the omission through an error indication.

To conclude the session, the program invokes `shutdown` with the parameter `2`, signaling the cessation of both sending and receiving activities. On Windows systems, termination and shutdown actions are performed using `closesocket`. Following these steps, the close function is called to officially close the socket, thereby ending the interaction.


### UDP Client

#### Introduction To UDP Communication

UPD protocol provides a connectionless and unreliable communication service, meaning that it does not establish a dedicated connection between the sender and receiver and does not guarantee the delivery of data packets. Instead, UDP sends data packets, called datagrams, without any acknowledgment or error checking. These datagrams are transmitted independently and can arrive out of order, be duplicated, or even be lost during transmission. The lack of built-in reliability mechanisms in UDP allows fast, low-latency communication, making it ideal for that can tolerate lost packets, such as streaming audio or video, where speed is more crucial than perfect accuracy. [3] [4]

### Testing
This section is dedicated to testing the project, so what was tested?
- unit tests on individual class methods
- communication testing with fake server - NETCAT
- bilateral communication with personal local UDP server
- bilateral communication with reference server 
The following subsections will explain the individual parts of the testing.

#### Unit tests on individual class methods
The aim of the unit test was to guarantee the correctness and expected behavior of the individual class methods. Testing was performed on critical methods that interact with either STDIN/STDOUT/STDERR or manipulate messages.

**Note:** Not all methods were tasted by unit test, just the critical ones!

#### Communication testing with fake server - NETCAT
Netcat, often referred to as the "Swiss Army Knife of networking tools", is a computer tool used for networking. It allows reading from and writing to TCP or UDP network connections using the command line.

Netcat was used to confirm the correctness of the communication based on the IPK24 protocol. Both TCP and UDP were tested for a variety of key situations that can occur in reality during client-server communication. Such as:

    1. Client authentication with server rejection or acceptance
    2. The client sends a message other than the authentication message after start of the application
    3. Servers error occurs in authentication state
    4. Multiple messages sent from server to client (Client just as a listener)
    5. Client wants to join another channel and server rejects 
    6. Client wants to join another channel and server accepts
    7. Server in UDP variant sends error message instead of CONFIRM message
    8. Server in UDP variant sends error message instead of REPLY message

#### Bilateral communication with personal local UDP server
Testing of the UDP client was conducted on a local chat server to ensure the correct implementation of basic and more complex functionalities. The primary focus was on the clients' ability to handle various communication scenarios that are typical in client-server interactions.

#### Bilateral communication with reference server
Due to the complexity of simulating situations on the reference server using test scripts (because of the interaction with other users contents), only the manual tests with scenarios were tested on the reference server. The test scenarios were described in chapter [Communication testing with fake server - NETCAT](#communication-testing-with-fake-server---netcat). The interaction in all scenarios went as expected i.e. as [specified](https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/README.md)

## Resources 
[RFC791]: Information Sciences Institute, University of Southern California. "Internet Protocol" [online]. September 1981. [cited 2024-03-26]. DOI: 10.17487/RFC791. Available at [https://www.ietf.org/rfc/rfc793.txt](https://www.ietf.org/rfc/rfc793.txt).

James F. Kurose, Keith W. Ross: *Computer Networking: A Top Down Approach* (Eighth Edition). Figure 2.28 [cited 2024-03-20].

"Difference Between TCP/IP and OSI Model" [online]. [cited 2024-03-29]. Available at [https://techdifferences.com/difference-between-tcp-ip-and-osi-model.html](https://techdifferences.com/difference-between-tcp-ip-and-osi-model.html).

Natarajan Meghanathan: "A Tutorial on Network Security: Attacks and Controls" [online]. [cited 2024-03-29]. Available at [https://arxiv.org/pdf/1412.6017v1.pdf](https://arxiv.org/pdf/1412.6017v1.pdf)

Gorry Fairhurst: "The User Datagram Protocol (UDP)" [online]. 19.11.2008. [cited 2024-03-20]. Available at [https://www.erg.abdn.ac.uk/users/gorry/course/inet-pages/udp.html](https://www.erg.abdn.ac.uk/users/gorry/course/inet-pages/udp.html).

Marek Majkowski: "Everything you ever wanted to know about UDP sockets but were afraid to ask, part 1" [online]. 25.11.2021. [cited 2024-03-26]. Available at [https://blog.cloudflare.com/everything-you-ever-wanted-to-know-about-udp-sockets-but-were-afraid-to-ask-part-1](https://blog.cloudflare.com/everything-you-ever-wanted-to-know-about-udp-sockets-but-were-afraid-to-ask-part-1).

Andrew T. Campbell: "Socket Programming" [online]. December 2023. Available at [https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html](https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html).