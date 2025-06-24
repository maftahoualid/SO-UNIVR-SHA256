#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>    
#include <sys/msg.h>    
#include <sys/shm.h>    
#include <sys/sem.h>    
#include <sys/types.h>
#include <unistd.h>
#include "common.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char *data = malloc(MAX_DATA);
    if (!data) {
        perror("malloc");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    size_t read_size = fread(data, 1, MAX_DATA, fp);
    fclose(fp);

    if (read_size == 0) {
        fprintf(stderr, "File vuoto o non leggibile\n");
        free(data);
        exit(EXIT_FAILURE);
    }

    int shmid = shmget(SHM_KEY, MAX_DATA, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        free(data);
        exit(EXIT_FAILURE);
    }

    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        free(data);
        exit(EXIT_FAILURE);
    }

    struct sembuf sb = {0, -1, 0};
    if (semop(semid, &sb, 1) == -1) {
        perror("semop wait");
        free(data);
        exit(EXIT_FAILURE);
    }

    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("shmat");
        sb.sem_op = 1;
        semop(semid, &sb, 1);
        free(data);
        exit(EXIT_FAILURE);
    }

    memcpy(shmaddr, data, read_size);

    if (shmdt(shmaddr) == -1)
        perror("shmdt");

    sb.sem_op = 1;
    if (semop(semid, &sb, 1) == -1) {
        perror("semop signal");
        free(data);
        exit(EXIT_FAILURE);
    }

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("msgget");
        free(data);
        exit(EXIT_FAILURE);
    }

    request_msg_t req;
    req.mtype = REQUEST_MSG_TYPE;
    req.pid = getpid();
    req.size = read_size;
    strncpy(req.filename, argv[1], MAX_PATH - 1);
    req.filename[MAX_PATH - 1] = '\0';

    if (msgsnd(msgid, &req, sizeof(req) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        free(data);
        exit(EXIT_FAILURE);
    }

    response_msg_t res;
    if (msgrcv(msgid, &res, sizeof(res.hash), getpid(), 0) == -1) {
        perror("msgrcv");
        free(data);
        exit(EXIT_FAILURE);
    }

    printf("SHA-256: %s\n", res.hash);

    free(data);
    return 0;
}
