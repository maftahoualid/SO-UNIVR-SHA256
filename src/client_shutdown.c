#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <limits.h>
#include "common.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <new_max_children>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    long val = strtol(argv[1], &endptr, 10);

    // Controllo conversione e validità valore (>0)
    if (*endptr != '\0' || val <= 0 || val > INT_MAX) {
        fprintf(stderr, "Limit must be a positive integer\n");
        exit(EXIT_FAILURE);
    }
    int new_limit = (int) val;

    int msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    control_msg_t ctrl;
    ctrl.mtype = CONTROL_MSG_TYPE;
    ctrl.new_limit = new_limit;

    if (msgsnd(msgid, &ctrl, sizeof(ctrl) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    printf("[client_control] Nuovo limite richiesto: %d\n", new_limit);
    return 0;
}
