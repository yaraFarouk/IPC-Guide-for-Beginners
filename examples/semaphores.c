#include <sys/types.h> // For pid_t
#include <sys/ipc.h>   // For IPC_PRIVATE, etc.
#include <sys/sem.h>   // For semaphore functions
#include <stdio.h>     // For printf, perror
#include <stdlib.h>    // For exit
#include <unistd.h>    // For fork

union semun { int val; }; // Union for semctl arguments

// Function to perform a semaphore "down" (decrement) operation
void down(int semid) {
    struct sembuf op = {0, -1, 0}; // Operation: decrement semaphore 0 by 1
    if (semop(semid, &op, 1) == -1) { // Perform the operation
        perror("down failed");
        exit(1);
    }
}

// Function to perform a semaphore "up" (increment) operation
void up(int semid) {
    struct sembuf op = {0, 1, 0}; // Operation: increment semaphore 0 by 1
    if (semop(semid, &op, 1) == -1) { // Perform the operation
        perror("up failed");
        exit(1);
    }
}

int main() {
    // Create a semaphore set with 1 semaphore (IPC_PRIVATE means it's private to this process)
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }

    // Initialize the semaphore to 0
    union semun arg = {0}; // Set the initial value to 0
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl failed");
        exit(1);
    }

    // Create a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) { // Child
        printf("Child: A\n");
        // up() (Signal/V): Increments the semaphore. If another process is waiting, it unblocks.
        up(semid); // Increments the semaphore. This "signals" the parent that the child has reached a certain point.
        printf("Child: B\n");
    } else { // Parent
        // down() (Wait/P): Decrements the semaphore. If the semaphore is 0, the process blocks.
        down(semid); // Since the semaphore was initialized to 0, the parent process blocks here until the child increments it.
        printf("Parent: C\n");
        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("Cleaning up (semctl) failed");
            exit(1);
        }
    }
    return 0;
}