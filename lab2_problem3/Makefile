CC = gcc
CFLAGS = -pthread
TARGET = problem3
FIFO_NAME = abc_fifo_name

all: $(TARGET)

$(TARGET): problem3.o
	$(CC) $(CFLAGS) -o $(TARGET) problem3.o

problem3.o: problem3.c
	$(CC) $(CFLAGS) -c problem3.c

clean:
	rm -f $(TARGET) *.o
	rm -f $(FIFO_NAME)