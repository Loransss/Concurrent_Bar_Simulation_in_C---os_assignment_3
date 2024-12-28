# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -pthread

# Shared memory object file
SHARED_MEMORY_OBJ = shared_memory.o

# Executable names
MAIN_EXEC = main
MONITOR_EXEC = monitor
VISITOR_EXEC = visitor
RECEPTIONIST_EXEC = receptionist

# Source files
MAIN_SRC = main.c
MONITOR_SRC = monitor.c
VISITOR_SRC = visitor.c
RECEPTIONIST_SRC = receptionist.c
SHARED_MEMORY_SRC = shared_memory.c

# Header files
SHARED_MEMORY_HEADER = shared_memory.h

# Default target
all: $(MAIN_EXEC) $(MONITOR_EXEC) $(VISITOR_EXEC) $(RECEPTIONIST_EXEC)

# Shared memory object
$(SHARED_MEMORY_OBJ): $(SHARED_MEMORY_SRC) $(SHARED_MEMORY_HEADER)
	$(CC) $(CFLAGS) -c $(SHARED_MEMORY_SRC)

# Build main executable
$(MAIN_EXEC): $(MAIN_SRC) $(SHARED_MEMORY_OBJ)
	$(CC) $(CFLAGS) -o $(MAIN_EXEC) $(MAIN_SRC) $(SHARED_MEMORY_OBJ)

# Build monitor executable
$(MONITOR_EXEC): $(MONITOR_SRC) $(SHARED_MEMORY_OBJ)
	$(CC) $(CFLAGS) -o $(MONITOR_EXEC) $(MONITOR_SRC) $(SHARED_MEMORY_OBJ)

# Build visitor executable
$(VISITOR_EXEC): $(VISITOR_SRC) $(SHARED_MEMORY_OBJ)
	$(CC) $(CFLAGS) -o $(VISITOR_EXEC) $(VISITOR_SRC) $(SHARED_MEMORY_OBJ)

# Build receptionist executable
$(RECEPTIONIST_EXEC): $(RECEPTIONIST_SRC) $(SHARED_MEMORY_OBJ)
	$(CC) $(CFLAGS) -o $(RECEPTIONIST_EXEC) $(RECEPTIONIST_SRC) $(SHARED_MEMORY_OBJ)

# Clean target
clean:
	rm -f $(MAIN_EXEC) $(MONITOR_EXEC) $(VISITOR_EXEC) $(RECEPTIONIST_EXEC) $(SHARED_MEMORY_OBJ)
