# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -g

# Source files
SRCS = main.cpp Ethernet.cpp parser.cpp

# Object files (generated from source files)
OBJS = $(SRCS:.cpp=.o)

# Output executable (Windows uses .exe for executables)
TARGET = main.exe

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove compiled files
clean:
	del /Q $(OBJS) $(TARGET)

# Run the program after building (PowerShell requires ./ to run executables)
run: $(TARGET)
	./$(TARGET)
