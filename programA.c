#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PIPE_NAME "/tmp/thread_mgmt_pipe"

void send_command(const char *command) {
    int fd = open(PIPE_NAME, O_WRONLY);
    if (fd == -1) {
        printf("[ERROR] Failed to open pipe.\n");
        return;
    }
    write(fd, command, strlen(command) + 1);
    close(fd);

    fd = open(PIPE_NAME, O_RDONLY);
    if (fd == -1) {
        printf("[ERROR] Failed to open pipe for reading.\n");
        return;
    }

    char response[256] = {0};
    read(fd, response, sizeof(response));
    printf("%s", response);
    close(fd);
}

int main() {
    while (1) {
        printf("\n1. Create Thread\n2. List Threads\n3. Kill Thread\n4. Exit\nChoose: ");
        int choice;
        scanf("%d", &choice);

        if (choice == 1) {
            send_command("CREATE_THREAD");
        } else if (choice == 2) {
            send_command("LIST_THREADS");
        } else if (choice == 3) {
            int id;
            printf("Enter Thread ID to Kill: ");
            scanf("%d", &id);
            char command[50];
            sprintf(command, "KILL_THREAD %d", id);
            send_command(command);
        } else if (choice == 4) {
            send_command("EXIT");
            break;
        }
    }
    return 0;
}
