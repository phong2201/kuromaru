#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid;

    // Gọi hàm fork()
    pid = fork();

    if (pid < 0) {
        // fork() thất bại
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        // Đây là tiến trình con
        printf("Tiến trình con (PID: %d), tiến trình cha (PPID: %d)\n", getpid(), getppid());
    } else {
        // Đây là tiến trình cha
        printf("Tiến trình cha (PID: %d), tiến trình con (PID: %d)\n", getpid(), pid);
    }

    return 0;
}
