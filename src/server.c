#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>    
#include <sys/shm.h>    
#include <sys/sem.h>    
#include <sys/types.h>
#include <sys/wait.h>   
#include <semaphore.h>  
#include "common.h"
#include "digest.h"

#define MAX_CHILDREN 4
#define SHM_SIZE MAX_DATA

int msgid, shmid, semid;
sem_t mutex;

int active_children = 0;
int max_children = MAX_CHILDREN;

void cleanup_and_exit(int signo) {
    printf("\n[server] Terminazione, pulizia risorse IPC...\n");

    if (msgctl(msgid, IPC_RMID, NULL) == -1)
        perror("[server] Errore rimozione coda");

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        perror("[server] Errore rimozione SHM");

    if (semctl(semid, 0, IPC_RMID) == -1)
        perror("[server] Errore rimozione semaforo");

    sem_destroy(&mutex);

    exit(EXIT_SUCCESS);
}

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

    if (sem_init(&mutex, 0, 1) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, cleanup_and_exit);

    msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    union semun {
        int val;
    } arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL");
        exit(EXIT_FAILURE);
    }

    // buffer union per ricevere tutti i tipi messaggi
    union {
        control_msg_t control;
        status_request_msg_t status_req;
        request_msg_t request;
    } msg_buf;

    while (1) {
        ssize_t res;
        do {
            // Passa la dimensione max dati del messaggio più grande (request)
            res = msgrcv(msgid, &msg_buf, sizeof(msg_buf.request) - sizeof(long), -3, 0);
        } while (res == -1 && errno == EINTR);

        if (res == -1) {
            perror("msgrcv");
            continue;
        }

        long mtype = msg_buf.control.mtype; // sempre primo campo

        if (mtype == CONTROL_MSG_TYPE) {
            sem_wait(&mutex);
            max_children = msg_buf.control.new_limit;
            sem_post(&mutex);
            printf("[server] Limite aggiornato a %d figli concorrenti\n", max_children);
            continue;
        }

        if (mtype == STATUS_MSG_TYPE) {
            status_response_msg_t status_res;
            status_res.mtype = msg_buf.status_req.pid;
            sem_wait(&mutex);
            status_res.active = active_children;
            status_res.limit = max_children;
            sem_post(&mutex);
            if (msgsnd(msgid, &status_res, sizeof(status_res) - sizeof(long), 0) == -1)
                perror("msgsnd status");
            else
                printf("[server] Stato inviato a %d\n", msg_buf.status_req.pid);
            continue;
        }

        if (mtype == REQUEST_MSG_TYPE) {
            while (1) {
                sem_wait(&mutex);
                if (active_children < max_children) {
                    active_children++;
                    sem_post(&mutex);
                    break;
                }
                sem_post(&mutex);
                usleep(100000);
            }

            pid_t child_pid = fork();
            if (child_pid < 0) {
                perror("fork");
                continue;
            }

            if (child_pid == 0) {
                printf("[server] Figlio PID %d elabora richiesta di %d (%zu byte)\n",
                    getpid(), msg_buf.request.pid, msg_buf.request.size);

                struct sembuf sb = {0, -1, 0};
                if (semop(semid, &sb, 1) == -1) {
                    perror("semop - P");
                    exit(EXIT_FAILURE);
                }

                void *shmaddr = shmat(shmid, NULL, 0);
                if (shmaddr == (void *)-1) {
                    perror("shmat");
                    sb.sem_op = 1;
                    semop(semid, &sb, 1);
                    exit(EXIT_FAILURE);
                }

                uint8_t hash[32];
                digest_buffer((uint8_t *)shmaddr, msg_buf.request.size, hash);

                if (shmdt(shmaddr) == -1)
                    perror("shmdt");

                sb.sem_op = 1;
                if (semop(semid, &sb, 1) == -1) {
                    perror("semop - V");
                    exit(EXIT_FAILURE);
                }

                char hex[65];
                for (int i = 0; i < 32; i++)
                    sprintf(hex + (i * 2), "%02x", hash[i]);
                hex[64] = '\0';

                response_msg_t res;
                res.mtype = msg_buf.request.pid;
                strncpy(res.hash, hex, MAX_HASH);
                res.hash[MAX_HASH - 1] = '\0';

                if (msgsnd(msgid, &res, sizeof(res.hash), 0) == -1)
                    perror("msgsnd");
                else
                    printf("[server] Figlio %d ha risposto a %d\n", getpid(), msg_buf.request.pid);

                exit(EXIT_SUCCESS);
            }
        }
    }

    sem_destroy(&mutex);
    return 0;
}
