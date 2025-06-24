#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>

#define MSG_KEY 1234      // chiave per coda di messaggi
#define SHM_KEY 5678      // chiave per memoria condivisa
#define MAX_PATH 256
#define MAX_HASH 65
#define MAX_DATA 4096     // max dimensione file in questo esempio
#define CONTROL_MSG_TYPE 2
#define SEM_KEY 4242  // chiave del semaforo


// struttura del messaggio client -> server
typedef struct {
    long mtype; // deve essere >0
    pid_t pid;  // PID del client
    size_t size;
} request_msg_t;

// struttura del messaggio server -> client
typedef struct {
    long mtype;
    char hash[MAX_HASH];
} response_msg_t;

typedef struct {
    long mtype;
    int new_limit;
} control_msg_t;

#endif
