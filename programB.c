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
    int buffer[BUFFER_SIZE];  // Bộ nhớ chia sẻ giữa các thread
    int index;
    pthread_mutex_t mutex;
} SharedData;

ThreadInfo threads[MAX_THREADS];
SharedData shared_data;
int thread_count = 0;
int running = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Danh sách ID có thể tái sử dụng
int available_ids[MAX_THREADS];
int available_count = 0;

// ✅ Worker thread: Ghi dữ liệu vào buffer chia sẻ
void *worker_task(void *arg) {
    int id = *(int *)arg;
    printf("[THREAD %d] Started, writing to shared buffer...\n", id);
    
    for (int i = 0; i < 1000; i++) {
        sleep(1);
        pthread_mutex_lock(&shared_data.mutex);
        if (shared_data.index < BUFFER_SIZE) {
            shared_data.buffer[shared_data.index++] = id;
        }
        pthread_mutex_unlock(&shared_data.mutex);
        pthread_testcancel();  // Kiểm tra tín hiệu hủy
    }
    
    printf("[THREAD %d] Done.\n", id);
    return NULL;
}

// ✅ Lấy ID trống hoặc tạo ID mới
int get_available_id() {
    if (available_count > 0) {
        return available_ids[--available_count];  // Lấy ID từ danh sách ID trống
    }
    return thread_count++;  // Nếu không có ID cũ, tăng thread_count
}

// ✅ Tạo thread mới
void create_thread(int client_fd) {
    pthread_mutex_lock(&mutex);
    if (thread_count >= MAX_THREADS && available_count == 0) {
        write(client_fd, "[ERROR] Cannot create more threads. Max limit reached.\n", 55);
        pthread_mutex_unlock(&mutex);
        return;
    }

    int id = get_available_id();
    threads[id].id = id;
    threads[id].running = 1;
    pthread_create(&threads[id].thread, NULL, worker_task, &threads[id].id);

    char response[50];
    sprintf(response, "[SERVER] Created thread %d\n", id);
    write(client_fd, response, strlen(response));
    pthread_mutex_unlock(&mutex);
}

// ✅ Liệt kê các thread đang chạy
void list_threads(int client_fd) {
    pthread_mutex_lock(&mutex);
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
    pthread_mutex_unlock(&mutex);
}

// ✅ Hủy một thread theo ID
void kill_thread(int id, int client_fd) {
    pthread_mutex_lock(&mutex);
    if (id < 0 || id >= thread_count || !threads[id].running) {
        write(client_fd, "[ERROR] Invalid thread ID.\n", 27);
    } else {
        pthread_cancel(threads[id].thread);
        pthread_join(threads[id].thread, NULL);  // Chờ thread kết thúc
        threads[id].running = 0;

        // Thêm ID vào danh sách tái sử dụng
        available_ids[available_count++] = id;

        char response[50];
        sprintf(response, "[SERVER] Killed thread %d\n", id);
        write(client_fd, response, strlen(response));
    }
    pthread_mutex_unlock(&mutex);
}

// ✅ Đóng tất cả thread trước khi thoát
void cleanup_threads() {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < thread_count; i++) {
        if (threads[i].running) {
            pthread_cancel(threads[i].thread);
            pthread_join(threads[i].thread, NULL);
            threads[i].running = 0;
        }
    }
    pthread_mutex_unlock(&mutex);
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
        cleanup_threads();  // Đóng tất cả thread trước khi thoát
    } else {
        write(client_fd, "[ERROR] Unknown command.\n", 26);
    }
}

int main() {
    mkfifo(PIPE_NAME, 0666);
    printf("[SERVER] Listening on FIFO...\n");

    // Khởi tạo bộ nhớ chia sẻ
    shared_data.index = 0;
    pthread_mutex_init(&shared_data.mutex, NULL);

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

    unlink(PIPE_NAME);  // Xóa FIFO khi kết thúc
    pthread_mutex_destroy(&shared_data.mutex);
    return 0;
}
