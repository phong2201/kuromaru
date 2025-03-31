#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

#define FIFO_NAME "abc_fifo_name"


int fifo_snd(const char *fifo_name) {
    const char signal = '1';

    // Open the FIFO for writing (O_WRONLY)
    int fd = open(fifo_name, O_WRONLY);
    write(fd, &signal, sizeof(signal));
    close(fd);
    return 0;
}

int rcv(const char *fifo_name) {
    char signal;

    // Open the FIFO for reading (O_RDONLY)
    int fd = open(fifo_name, O_RDONLY);
    read(fd, &signal, sizeof(signal));
    close(fd);
    return 0;
}

void *send_message(void *arg) {
    fifo_snd(FIFO_NAME);
    return NULL;
}

void *receive_message(void *arg) {
    rcv(FIFO_NAME);
    return NULL;
}

int main() {
    pthread_t send_thread, receive_thread;

    // Create two threads for sending and receiving
    pthread_create(&receive_thread, NULL, receive_message, NULL);
    sleep(1);
    pthread_create(&send_thread, NULL, send_message, NULL);

    // Wait for both threads to complete
    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    // Clean up
    unlink(FIFO_NAME);

    return 0;
}