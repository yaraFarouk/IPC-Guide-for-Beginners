#include <sys/types.h> // For pid_t
#include <sys/ipc.h>   // For IPC_PRIVATE, etc.
#include <sys/shm.h>   // For shared memory functions
#include <stdio.h>     // For printf, perror
#include <string.h>    // For strcpy
#include <stdlib.h>    // For exit
#include <unistd.h>    // For fork, sleep

int main() {
    // Create a shared memory segment
    int shmid = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0666); // 4096 bytes, created if it doesn't exist.

    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Create a child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) { // Child: Writer
        // Attach the shared memory segment to the child's address space
        void *shmaddr = shmat(shmid, NULL, 0); // Attach for read/write.
        if (shmaddr == (void *)-1) {
            perror("shmat failed");
            exit(1);
        }
        // Write data to the shared memory
        strcpy((char *)shmaddr, "Hello from child!");
        printf("Child wrote: %s\n", (char *)shmaddr);
        // Detach the shared memory
        if (shmdt(shmaddr) == -1) {
            perror("shmdt failed");
            exit(1);
        }
    } else { // Parent: Reader
        sleep(1); // Wait for writer
        // Attach the shared memory segment to the parent's address space (read-only)
        void *shmaddr = shmat(shmid, NULL, SHM_RDONLY); // Attach for read only.
        if (shmaddr == (void *)-1) {
            perror("shmat failed");
            exit(1);
        }
        // Read data from the shared memory
        printf("Parent read: %s\n", (char *)shmaddr);
        // Detach the shared memory
        if (shmdt(shmaddr) == -1) {
            perror("shmdt failed");
            exit(1);
        }
        // Remove the shared memory segment (cleanup)
        // Notice the differnce between this and the previous code block
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("Cleaning up (shmctl) failed");
            exit(1);
        }
    }
    return 0;
}