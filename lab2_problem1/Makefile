# Compiler and Flags
CC = gcc
CFLAGS = -Wall -std=c99

# Source and Object Files
SRC = problem1.c
OBJ = problem1.o

# Output executable
EXEC = Movie_rating

# Targets
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

problem1.o: problem1.c
	$(CC) $(CFLAGS) -c problem1.c

# Clean up
clean:
	rm -f $(OBJ) $(EXEC) output.txt