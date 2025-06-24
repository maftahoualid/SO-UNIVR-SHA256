#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "common.h"

int main() {
    // Apertura coda messaggi (presuppone che il server l'abbia creata)
    int msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Preparazione messaggio di richiesta stato
    status_request_msg_t req;
    req.mtype = STATUS_MSG_TYPE;  // tipo messaggio interrogazione stato
    req.pid = getpid();            // identifica il client con PID

    // Invio messaggio richiesta (dimensione senza mtype)
    if (msgsnd(msgid, &req, sizeof(req) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    // Ricezione risposta dal server: filtro con tipo = PID client
    status_response_msg_t res;
    if (msgrcv(msgid, &res, sizeof(res) - sizeof(long), getpid(), 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    // Stampa stato ricevuto
    printf("[client_status] Stato server:\n");
    printf(" - Processi figli attivi: %d\n", res.active);
    printf(" - Limite massimo:        %d\n", res.limit);

    return 0;
}
