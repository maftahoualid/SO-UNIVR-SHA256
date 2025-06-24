#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "common.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <new_max_children>\n", argv[0]);
        exit(1);
    }

    int new_limit = atoi(argv[1]);
    if (new_limit <= 0) {
        fprintf(stderr, "Limit must be > 0\n");
        exit(1);
    }

    int msgid = msgget(MSG_KEY, 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }

    control_msg_t ctrl;
    ctrl.mtype = CONTROL_MSG_TYPE;
    ctrl.new_limit = new_limit;

    if (msgsnd(msgid, &ctrl, sizeof(ctrl) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("[client_control] Nuovo limite richiesto: %d\n", new_limit);
    return 0;
}
