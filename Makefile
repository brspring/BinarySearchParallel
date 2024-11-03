# Compiler
CC = gcc

# Target executable
TARGET = binary_search_parallel

# Source files
SRCS = partA.c geraInput.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the object files to create the executable, with -lpthread
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ -lpthread

# Compile source files to object files
%.o: %.c
	$(CC) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

run:
	./binary_search_parallel

.PHONY: all clean
