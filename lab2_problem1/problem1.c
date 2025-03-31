#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define TOTAL_MOVIES 1682

typedef struct {
    float sumOfrating;
    int count;
} Movie;

void compute_average(const char* filename, Movie* shared_data) {
    FILE* file = fopen(filename, "r");

    int userID, itemID, rating;
    long timestamp;

    while (fscanf(file, "%d\t%d\t%d\t%ld", &userID, &itemID, &rating, &timestamp) == 4) {
        shared_data[itemID].sumOfrating += rating;
        shared_data[itemID].count += 1;
    }

    fclose(file);
}

int main() {
    key_t key = ftok("shmfile", 38);

    // Create a shared memory segment
    int shmid = shmget(key, TOTAL_MOVIES * sizeof(Movie), 0666 | IPC_CREAT);
    // Attaches the shared memory segment
    Movie* shared_data = (Movie*) shmat(shmid, NULL, 0);

    // Initialize shared memory
    for (int i = 1; i <= TOTAL_MOVIES; i++) {
        shared_data[i].sumOfrating = 0.0;
        shared_data[i].count = 0;
    }

    // First child proces
    pid_t pid1 = fork();
    if (pid1 == 0) {
        compute_average("movie-100k_1.txt", shared_data);
        shmdt(shared_data);
        exit(0);
    }

    // Second child proces
    pid_t pid2 = fork();
    if (pid2 == 0) {
        compute_average("movie-100k_2.txt", shared_data);
        shmdt(shared_data);
        exit(0);
    }

    // Waits for both child processes to finish
    wait(NULL);
    wait(NULL);

    // Write results to the ouput file and close
    FILE *output = fopen("output.txt", "w");
    for (int i = 1; i <= TOTAL_MOVIES; i++) {
        if (shared_data[i].count > 0) {
            float average = shared_data[i].sumOfrating / shared_data[i].count;
            fprintf(output, "%d\t%.2f\n", i, average);
        }
    }

    // Detaches the shared memory segment
    shmdt(shared_data);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}