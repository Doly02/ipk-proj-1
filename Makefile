# Program Name
TARGET = client
# Compiler
CC = clang++
# Compiler Flags
CFLAGS = -std=c++17 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic
# Source Files
SOURCES = main.cpp
# Object Files 
OBJECTS = $(SOURCES:.cpp=.o)

# Rule for Target
all: $(TARGET)

# Rule For Assemble of Final Executable File
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule For Assemble Object Files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule For Cleaning Executable And Object Files
clean:
	rm -f $(OBJECTS) $(TARGET)
