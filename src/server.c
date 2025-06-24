#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/sem.h>
#include "common.h"
#include "digest.h"

#define MAX_CHILDREN 4

int active_children = 0;
int max_children = MAX_CHILDREN;
sem_t mutex;

// Handler per evitare zombie e decrementare il conteggio figli
void sigchld_handler(int signo) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        sem_wait(&mutex);
        active_children--;
        sem_post(&mutex);
    }
    errno = saved_errno;
}

int main() {
    printf("[server] Avvio...\n");

    // Inizializza mutex e handler
    sem_init(&mutex, 0, 1);
    signal(SIGCHLD, sigchld_handler);

    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("msgget");
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

    union semun {
        int val;
    } arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    while (1) {
        char raw_msg[sizeof(control_msg_t)];
        ssize_t res;
        do {
            res = msgrcv(msgid, &raw_msg, sizeof(raw_msg), -2, 0);
        } while (res == -1 && errno == EINTR);

        if (res == -1) {
            perror("msgrcv");
            continue;
        }

        long *mtype_ptr = (long *)raw_msg;
        if (*mtype_ptr == CONTROL_MSG_TYPE) {
            control_msg_t *ctrl = (control_msg_t *)raw_msg;
            sem_wait(&mutex);
            max_children = ctrl->new_limit;
            sem_post(&mutex);
            printf("[server] Limite aggiornato a %d figli concorrenti\n", max_children);
            continue;
        }

        request_msg_t *req_ptr = (request_msg_t *)raw_msg;

        // Attendi se troppi figli
        while (1) {
            sem_wait(&mutex);
            if (active_children < max_children) {
                active_children++;
                sem_post(&mutex);
                break;
            }
            sem_post(&mutex);
            usleep(100000); // aspetta 100ms
        }

        pid_t child_pid = fork();
        if (child_pid < 0) {
            perror("fork");
            continue;
        }

        if (child_pid == 0) {
            // === PROCESSO FIGLIO ===
            printf("[server] Figlio PID %d elabora richiesta di %d (%zu byte)\n", getpid(), req_ptr->pid, req_ptr->size);

            // P (wait) sul semaforo SHM
            struct sembuf sb = {0, -1, 0};
            semop(semid, &sb, 1);

            void *shmaddr = shmat(shmid, NULL, 0);
            if (shmaddr == (void *)-1) {
                perror("shmat");
                exit(1);
            }

            uint8_t hash[32];
            digest_buffer((uint8_t *)shmaddr, req_ptr->size, hash);
            shmdt(shmaddr);

            // V (signal) sul semaforo SHM
            sb.sem_op = 1;
            semop(semid, &sb, 1);

            char hex[65];
            for (int i = 0; i < 32; ++i)
                sprintf(hex + (i * 2), "%02x", hash[i]);
            hex[64] = '\0';

            response_msg_t res;
            res.mtype = req_ptr->pid;
            strncpy(res.hash, hex, MAX_HASH);

            if (msgsnd(msgid, &res, sizeof(res.hash), 0) == -1) {
                perror("msgsnd");
            } else {
                printf("[server] Figlio %d ha risposto a %d\n", getpid(), req_ptr->pid);
            }

            exit(0);
        }

        // Il padre continua
    }

    sem_destroy(&mutex);
    return 0;
}