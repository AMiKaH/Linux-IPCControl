#include "IPC_Control.h"

/**
 * Use this file to remove all message queues and semaphores from previous run
 */
int main (int argc, char *argv[])
{
    int mid, sid;
    
    mid = msgget(MKEY, IPC_CREAT);
    sid = semget((key_t)SKEY,1,0666|IPC_CREAT);
    
    if (argc > 1){
        struct msqid_ds   msq_status;

        msgctl (mid, IPC_STAT, &msq_status);
        printf ("%d messages on queue\n\n", (int)msq_status.msg_qnum);
        exit(0);
    }
    semctl(sid,0,IPC_RMID,0);
    msgctl (mid, IPC_RMID, 0);
    
    exit(0);
}
