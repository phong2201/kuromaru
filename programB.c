#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>

#define PIPE_NAME "/tmp/thread_mgmt_pipe"
#define MAX_THREADS 10
#define BUFFER_SIZE 100

typedef struct {
    pthread_t thread;
    int id;
    int running;
} ThreadInfo;

typedef struct {
    int buffer[BUFFER_SIZE];  
    int index;
} SharedData;

ThreadInfo threads[MAX_THREADS];
SharedData shared_data;
int thread_count = 0;
int running = 1;

int available_ids[MAX_THREADS];
int available_count = 0;

// ✅ Hàm đếm đến 1000
int count_to_1000() {
    for (int i = 0; i < 1000; i++) {
        sleep(1);
    }
    return 0;
}

// ✅ Worker thread: Ghi dữ liệu vào buffer chia sẻ
void *worker_task(void *arg) {
    int id = *(int *)arg;
    printf("[THREAD %d] Started, writing to shared buffer...\n", id);
    
    count_to_1000(); // Thay thế vòng lặp cũ

    printf("[THREAD %d] Done.\n", id);
}

// ✅ Lấy ID trống hoặc tạo ID mới
int get_available_id() {
    if (available_count > 0) {
        return available_ids[--available_count];  
    }
    return thread_count++;  
}

// ✅ Tạo thread mới
void create_thread(int client_fd) {
    if (thread_count >= MAX_THREADS && available_count == 0) {
        write(client_fd, "[ERROR] Cannot create more threads. Max limit reached.\n", 55);
        return;
    }

    int id = get_available_id();
    threads[id].id = id;
    threads[id].running = 1;
    pthread_create(&threads[id].thread, NULL, worker_task, &threads[id].id);

    char response[50];
    sprintf(response, "[SERVER] Created thread %d\n", id);
    write(client_fd, response, strlen(response));
}

// ✅ Liệt kê các thread đang chạy
void list_threads(int client_fd) {
    char response[256] = "[SERVER] Active threads: ";
    int found = 0;

    for (int i = 0; i < thread_count; i++) {
        if (threads[i].running) {
            char temp[10];
            sprintf(temp, "%d ", threads[i].id);
            strcat(response, temp);
            found = 1;
        }
    }

    if (!found) strcat(response, "None");

    strcat(response, "\n");
    write(client_fd, response, strlen(response));
}

// ✅ Hủy một thread theo ID
void kill_thread(int id, int client_fd) {
    if (id < 0 || id >= thread_count || !threads[id].running) {
        write(client_fd, "[ERROR] Invalid thread ID.\n", 27);
    } else {
        pthread_cancel(threads[id].thread);
        pthread_join(threads[id].thread, NULL);  
        threads[id].running = 0;

        available_ids[available_count++] = id;

        char response[50];
        sprintf(response, "[SERVER] Killed thread %d\n", id);
        write(client_fd, response, strlen(response));
    }
}

// ✅ Xử lý lệnh từ client
void process_command(char *command, int client_fd) {
    if (strncmp(command, "CREATE_THREAD", 13) == 0) {
        create_thread(client_fd);
    } else if (strncmp(command, "LIST_THREADS", 12) == 0) {
        list_threads(client_fd);
    } else if (strncmp(command, "KILL_THREAD", 11) == 0) {
        int id;
        if (sscanf(command, "KILL_THREAD %d", &id) == 1) {
            kill_thread(id, client_fd);
        } else {
            write(client_fd, "[ERROR] Invalid KILL_THREAD format.\n", 36);
        }
    } else if (strncmp(command, "EXIT", 4) == 0) {
        write(client_fd, "[SERVER] Exiting...\n", 20);
        running = 0;
    } else {
        write(client_fd, "[ERROR] Unknown command.\n", 26);
    }
}

int main() {
    mkfifo(PIPE_NAME, 0666);
    printf("[SERVER] Listening on FIFO...\n");

    shared_data.index = 0;

    while (running) {
        int fd = open(PIPE_NAME, O_RDONLY);
        if (fd < 0) break;

        char command[256];
        read(fd, command, sizeof(command));
        close(fd);

        fd = open(PIPE_NAME, O_WRONLY);
        process_command(command, fd);
        close(fd);
    }

    unlink(PIPE_NAME);
    return 0;
}
