#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "common.h"

int main(int argc, char *argv[]) {
    // Controllo numero argomenti: deve esserci esattamente un argomento (nuovo limite)
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <new_max_children>\n", argv[0]);
        exit(1);
    }

    // Conversione dell'argomento in intero
    int new_limit = atoi(argv[1]);
    if (new_limit <= 0) {
        fprintf(stderr, "Limit must be > 0\n");
        exit(1);
    }

    // Apertura coda messaggi con permessi 0666 (presuppone che esista)
    int msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }

    // Preparazione messaggio di controllo
    control_msg_t ctrl;
    ctrl.mtype = CONTROL_MSG_TYPE;  // tipo messaggio controllo (2)
    ctrl.new_limit = new_limit;      // nuovo limite figli

    // Invio messaggio di controllo (dimensione escluso mtype)
    if (msgsnd(msgid, &ctrl, sizeof(ctrl) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("[client_control] Nuovo limite richiesto: %d\n", new_limit);
    return 0;
}
