# Compiler and Flags
CC = gcc
CFLAGS = -Wall -std=c99 -pthread

# Source and Object Files
SRC = problem2.c
OBJ = problem2.o

# Output executable
EXEC = Movie_rating

# Targets
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

problem2.o: problem2.c
	$(CC) $(CFLAGS) -c problem2.c

# Clean up
clean:
	rm -f $(OBJ) $(EXEC) output.txt