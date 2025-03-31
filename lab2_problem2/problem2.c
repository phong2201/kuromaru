#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TOTAL_MOVIES 1682
#define NUM_THREADS 2

typedef struct {
    float sumOfrating;
    int count;
} Movie;

Movie shared_data[TOTAL_MOVIES];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to compute average ratings from a given file
void* compute_average(void* arg) {
    const char* filename = (const char*)arg;
    FILE* file = fopen(filename, "r");

    int userID, itemID, rating;
    long timestamp;

    while (fscanf(file, "%d\t%d\t%d\t%ld", &userID, &itemID, &rating, &timestamp) == 4) {
        pthread_mutex_lock(&mutex);     // Locking the shared data before updating
        shared_data[itemID].sumOfrating += rating;
        shared_data[itemID].count += 1;
        pthread_mutex_unlock(&mutex);   // Unlocking the shared data
    }

    fclose(file);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    // Initialize shared memory
    for (int i = 0; i <= TOTAL_MOVIES; i++) {
        shared_data[i].sumOfrating = 0.0;
        shared_data[i].count = 0;
    }

    // 2 files to be processed by threads
    const char* files[NUM_THREADS] = {"movie-100k_1.txt", "movie-100k_2.txt"};

    // Create threads to process the two files
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, compute_average, (void*)files[i]) != 0) {
            return 1;
        }
    }

    // Wait for both threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Write results to the output file and close
    FILE *output = fopen("output.txt", "w");

    for (int i = 1; i <= TOTAL_MOVIES; i++) {
        if (shared_data[i].count > 0) {
            float average = shared_data[i].sumOfrating / shared_data[i].count;
            fprintf(output, "%d\t%.2f\n", i, average);
        }
    }

    fclose(output);
    return 0;
}