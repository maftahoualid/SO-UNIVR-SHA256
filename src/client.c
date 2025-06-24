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
        exit(1);
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    char *data = malloc(MAX_DATA);
    size_t read_size = fread(data, 1, MAX_DATA, fp);
    fclose(fp);

    if (read_size == 0) {
        fprintf(stderr, "File vuoto o non leggibile\n");
        exit(1);
    }

    int shmid = shmget(SHM_KEY, MAX_DATA, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1); // P (wait)

    void *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    memcpy(shmaddr, data, read_size);
    shmdt(shmaddr);

    sb.sem_op = 1;
    semop(semid, &sb, 1); // V (signal)

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }

    request_msg_t req;
    req.mtype = 1;
    req.pid = getpid();
    req.size = read_size;

    if (msgsnd(msgid, &req, sizeof(req) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    response_msg_t res;
    if (msgrcv(msgid, &res, sizeof(res.hash), getpid(), 0) == -1) {
        perror("msgrcv");
        exit(1);
    }

    printf("SHA-256: %s\n", res.hash);

    free(data);
    return 0;
}