#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>  // per pid_t e size_t

// Definizioni chiave per IPC (Inter-Process Communication)
// Chiavi per identificare risorse IPC come code di messaggi, memoria condivisa e semafori
#define MSG_KEY 1234          // chiave per la coda di messaggi
#define SHM_KEY 5678          // chiave per la memoria condivisa
#define SEM_KEY 4242          // chiave per il semaforo

// Costanti per dimensioni e tipi messaggi
#define MAX_PATH 256          // dimensione massima per il nome/file path
#define MAX_HASH 65           // dimensione stringa hash (64 + terminatore '\0')
#define MAX_DATA 4096         // dimensione massima del dato (es. contenuto file)
#define CONTROL_MSG_TYPE 1    // tipo messaggio di controllo
#define STATUS_MSG_TYPE 2     // tipo messaggio per status
#define REQUEST_MSG_TYPE 3    //
#define SHUTDOWN_MSG_TYPE 99  // tipo messaggio per shutdown (terminazione)

// Strutture per i messaggi scambiati tramite la coda messaggi

// Messaggio per richiesta di stato, contenente tipo messaggio e PID del processo richiedente
typedef struct {
    long mtype;        // tipo messaggio, deve essere primo campo
    pid_t pid;
} status_request_msg_t;

// Messaggio di risposta allo status, con informazioni su processi attivi e limite
typedef struct {
    long mtype;  // tipo messaggio
    int active;  // numero di processi attivi
    int limit;   // limite massimo configurato per processi/risorse
} status_response_msg_t;

// Messaggio di risposta contenente l'hash calcolato
typedef struct {
    long mtype;        // tipo messaggio, deve essere primo campo
    char hash[MAX_HASH];  // stringa hash esadecimale (64 + '\0')
} response_msg_t;

// Messaggio di controllo per modifiche runtime (es. cambio limite)
typedef struct {
    long mtype;        // tipo messaggio, deve essere primo campo
    int new_limit;      // nuovo limite max figli concorrenti
} control_msg_t;

// Messaggio di richiesta per calcolo hash di un file
typedef struct {
    long mtype;        // tipo messaggio, deve essere primo campo
    pid_t pid;
    size_t size;
    char filename[MAX_PATH];
} request_msg_t;

#endif
