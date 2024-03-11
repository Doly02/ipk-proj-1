# Program Name
TARGET = ipk24chat-client
DEBUG_TARGET = ipk24chat-client
# Test Program Name
TEST_TARGET = client_test
# Compiler
CC = clang++
# Compiler Flags
CFLAGS = -std=c++17 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic -Iinclude
DEBUG_CFLAGS = -fsanitize=address -g -std=c++17 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic
# Header Files
HEADERS = macros.hpp
# Source Files
SOURCES = src/client.cpp src/base_messages.cpp src/tcp_messages.cpp src/udp_messages.cpp src/tcp.cpp src/udp.cpp src/main.cpp
# Test Source Files
TEST_SOURCES = tests/test_parseMessage.cpp
#tests/test_readAndStore.cpp
# tests/test_checkInout_CmdAuth_2.cpp
# tests/test_checkInput_CmdAuth.cpp
# tests/test_checkInput_CmdJoin.cpp
# tests/test_argumentsParsing.cpp
# Object Files 
OBJECTS = $(SOURCES:.cpp=.o)
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
