/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      test_checkInput_CmdAuth
 *  Author:         Tomas Dolak
 *  Date:           25.02.2024
 *  Description:    Test For Processing Authentication Command.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           tcp_messages.cpp
 *  @author         Tomas Dolak
 *  @date           25.02.2024
 *  @brief          Test For Processing Authentication Command.
 * ****************************/

#include <gtest/gtest.h>
#include "../src/arguments.cpp"

class ArgumentsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code, pokud je potřeba
    }

    void TearDown() override {
        // Cleanup code, pokud je potřeba
    }
};

TEST_F(ArgumentsTest, ParsesValidArgumentsCorrectly) {
    const char* argv[] = {"program", "-t", "tcp", "-s", "localhost", "-p", "4567"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    arguments args(argc, argv);
    
    EXPECT_EQ(args.transferProtocol, "tcp");
    EXPECT_EQ(args.hostName, "localhost");
    EXPECT_EQ(args.confirmTimeOutUDP, 250);
    EXPECT_EQ(args.confirmRetriesUDP, 3);
    EXPECT_EQ(args.port, 4567);
}

TEST_F(ArgumentsTest, ResolvesHostNameToIPAddress) {
    const char* argv[] = {"program", "-t", "udp", "-s", "localhost"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    arguments args(argc, argv);
    args.resolveHostName();

    // Tento test předpokládá, že localhost se rozliší na 127.0.0.1
    // Pozor, tato hodnota se může lišit v závislosti na konfiguraci systému
    EXPECT_EQ(inet_ntoa(args.ipAddress), std::string("127.0.0.1"));
    EXPECT_EQ(args.transferProtocol, "udp");
    EXPECT_EQ(args.confirmTimeOutUDP, 250);
    EXPECT_EQ(args.confirmRetriesUDP, 3);
}

TEST_F(ArgumentsTest, HandlesInvalidHostNameGracefully) {
    const char* argv[] = {"program", "-t", "tcp", "-s", "nonexistenthostname"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    arguments args(argc, argv);
    // Očekáváme, že pokus o rozlišení neexistujícího hostname skončí chybou
    // Tento test by měl ověřit, že program správně zpracuje chybu
    // Můžete použít EXPECT_THROW nebo EXPECT_ANY_THROW, pokud je chyba ošetřena vyjímkou
    EXPECT_THROW(args.resolveHostName(), std::runtime_error);
}
