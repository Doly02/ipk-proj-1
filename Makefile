#   Project:        IPK Project 1 - Client for Chat Servers
#   File Name:      base_messages.cpp
#   Author:         Tomas Dolak
#   Date:           21.02.2024
#   Description:    Makefile for ipk24chat-client Client Application.
 

# Program Name
TARGET = ipk24chat-client
# Program Name For Debug Configuration
DEBUG_TARGET = ipk24chat-client_debug
# Test Program Name
TEST_TARGET = ipk24chat-client_test

# Compiler
CC = clang++
# Compiler Flags
CFLAGS = -std=c++17 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic -Iinclude
DEBUG_CFLAGS = -fsanitize=address -g -std=c++17 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic

# Header Files
HEADERS = include/macros.hpp include/strings.hpp include/arguments.hpp include/base_messages.hpp include/base_client.hpp include/tcp_messages.hpp include/tcp_client.hpp 

# Source Files
SOURCES = src/arguments.cpp src/base_client.cpp src/base_messages.cpp src/tcp_messages.cpp src/udp_messages.cpp src/tcp_client.cpp src/udp_client.cpp src/main.cpp
# Object Files 
OBJECTS = $(SOURCES:.cpp=.o)

# Test Source Files
TEST_SOURCES = tests/test_parseMessage.cpp
# Test Object Files (Derived from TEST_SOURCES)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
# Google Test Flags
GTEST_FLAGS = -lgtest -lgtest_main -pthread



# Rule for Target
all: $(TARGET)

# Rule For Assemble of Final Executable File
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	rm -f $(OBJECTS)

# Rule For Assemble Object Files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@


# Rule For Cleaning Executable And Object Files
clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_OBJECTS) $(TEST_TARGET)

# Rule for Test Target
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Rule For Assemble Test Object Files and Linking Test Target
$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(GTEST_FLAGS)

# Pattern Rule For Assemble Test Object Files
tests/%.o: tests/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

debug: $(SOURCES)
	$(CC) $(DEBUG_CFLAGS) -o $(DEBUG_TARGET) $^
