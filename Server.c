/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Server.c - This is the server side of the application. It starts by creating the message queue and the
--                         semaphore then waits for clients to join.
--
-- PROGRAM: IPC Control
--
-- FUNCTIONS:
-- int main ()
-- void serveClient (char* clientParams, int msq_id)
--
-- DATE: February 23, 2020
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
-- As soon as a client joins a new process is forked to
-- serve that client. The server receives the Client's process ID to use as an identifier when sending
-- messages to that client. The server will also receive the client's priority and the file to open.
-- The highsest priority for this application is 1 and lowest priority is not bound but 400 is used as an upper
-- bound for priorities. The lower the priority the smaller the distinction in terms of speed. The server will
-- serve clients with the high priority with larger message size and lower priorities with smaller buffer sizes.
-- Priority 1 is a special priority, it receives full buffer size of 4096 while the next priority receives half of that.
-- Priorities 3 and lower (i.e 4,5,6...) receive much smaller buffer sizes in order for us to be able to make a distinction
-- in speed.
--
----------------------------------------------------------------------------------------------------------------------*/

#include "Server.h"

/*--- Globals -----------------*/
int sid;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: main
--
-- DATE: February 23, 2020
--
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE:
--
--      int main (int argc, char *argv[])
--
-- RETURN: int      - on success it returns a non-negative integer
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
-- The main function for the server takes no input. It creates the message queue and semaphore to be used by both the
-- client and the server. After creating the required IPCs the server will listen for new connections and when one join the
-- server will create a new process gto serve the client that just joined.
--------------------------------------------------------------------------------------------------------*/

int main ()
{
    key_t mkey;
    int msq_id;
    struct msqid_ds;
    int retval;
    my_msg sMsg;
    
    //Set the size
    sMsg.mtype = CLIENT_TO_SERVER;
    sMsg.mesg_len = 1;

    /*---- Get message queue identifier ------*/
    mkey = (key_t) MKEY;
    if ((msq_id = msgget (mkey, IPC_CREAT)) < 0)
    {
      perror ("msgget failed!");
      exit(2);
    }

    sid = initsem ((key_t)SKEY);
    V(sid);
    
    printf("Starting server...\n");
    while(1){
        //Get the message from the queue

        memset(sMsg.mtext, 0, MAXBUFFSIZE);

        retval = readMsg(msq_id, &sMsg);
        if (retval == -1){

        } else {
            //TODO
            //fork here and serve client
            if (fork() == 0){
                printf("Message received:%s\n", sMsg.mtext);
                printf("Length received: %lu\n", strlen(sMsg.mtext));
                printf("msq_id: %d\n", msq_id);
                serveClient(sMsg.mtext, msq_id);
            } else {
                continue;
            }
        }
    }
    
    //Remove semaphore and message queue
    //At this point the server keeps listening for clients so it doesn't reach the block
    /*This code is here to be used in different applications where clients can send
      temination messages which are not required for this assignment
      **Another file provided rm.c deletes the queue and semaphore when compiled and executed
    */
    semctl(sid,0,IPC_RMID,0);
    msgctl(msq_id, IPC_RMID, 0);
    printf("Leaving...\n");
    return 0 ;

    exit(0);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: serveClient
--
-- DATE: February 23, 2020
--
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE:
--
--      void serveClient(char* clientParams, int msq_id)
--
-- PARAMETERS:
--      char* clientParams - the parameters that the client pass such as filename and priority
--      int msq_id         - the ID of the message queue to send the file to
-- RETURN: void
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
-- This function deals with reading the input of the client including the file name and the priority of the client
-- The function will open the file and if the file is not found it will notify the client with an error code. If file is
-- found then the server will specify the buffer length to send to the client each time based on the priority of the client.
-- priority 1 is the highest and will receive the total size of the message queue buffer 4096 bytes or (2048 for MacOS), subsequent
-- prioriies will receive much smaller buffer size to make it possible for human testing. Upon completion, the function will notify the
-- client that the transfer is completed and terminate the process.
--
--------------------------------------------------------------------------------------------------------*/

void serveClient(char* clientParams, int msq_id){
    FILE *fp;
    char fileName[SOMEVALUE];
    char temp[SOMEVALUE];
    char* clientBuffer;
    long cPid = 0;
    int priority = 0, i = 0, j = 0, clientBufferSize = 0, read = 0;
    my_msg serveMsg;
    
    
    for (; clientParams[i] != ' '; ++i){
        fileName[j] = clientParams[i];
        j++;
    }
    fileName[j] = '\0';
    j = 0;
    ++i;
    
    for (; clientParams[i] != ' '; ++i){
        temp[j] = clientParams[i];
        j++;
    }
    
    temp[j] = '\0';
    priority = atoi(temp);
    
    j = 0;
    ++i;
    
    for (; clientParams[i] != '\0'; ++i){
        temp[j] = clientParams[i];
        j++;
    }
    
    
    temp[j] = '\0';
    cPid = atoi(temp);
    serveMsg.mtype = cPid;
    serveMsg.mesg_len = 1;
    
    if (priority <= 0){
         printf("Error!\nPriority cannot be %d!\n", priority);
         memcpy(serveMsg.mtext, "", MAXBUFFSIZE);
         serveMsg.mesg_len = INVALID_PRIORITY;
         while (1){
             if ((sendMsg(msq_id, &serveMsg)) == -1){
             } else {
                 break;
             }
         }
        exit(1);
    }
    
    if (priority == 1){
        clientBufferSize = MAXBUFFSIZE/priority;
    } else {
        clientBufferSize = MAXBUFFSIZE/(priority * 2);
    }
    clientBuffer = (char*) malloc (clientBufferSize);

    fp = fopen(fileName, "rb");

    
    if(fp == NULL)
     {
         printf("Error!\nCould not open the file %s!\n", fileName);
         memcpy(serveMsg.mtext, "", MAXBUFFSIZE);
         serveMsg.mesg_len = FILE_NOT_FOUND;
         while (1){
             if ((sendMsg(msq_id, &serveMsg)) == -1){
             } else {
                 break;
             }
         }
        exit(1);
     }
    

    
     while((read = fread(clientBuffer, 1, clientBufferSize-1, fp)) > 0)
     {
         clientBuffer[read] = '\0';
         memcpy(serveMsg.mtext, clientBuffer, MAXBUFFSIZE);
         
         P(sid);
         fflush(stdout);
         while (1){
             if ((sendMsg(msq_id, &serveMsg)) == -1){
                
             } else {
                 break;
             }
         }
         memset(clientBuffer, 0, clientBufferSize);
         memset(serveMsg.mtext, 0, clientBufferSize);

         V(sid);

     }
    
    memcpy(serveMsg.mtext, "", MAXBUFFSIZE);
    serveMsg.mesg_len = TRANSFER_COMPLETE;
    
    while (1){
        if ((sendMsg(msq_id, &serveMsg)) == -1){
           
        } else {
            break;
        }
    }

    

    struct msqid_ds msq_status;

    msgctl (msq_id, IPC_STAT, &msq_status);
    printf("\nFinished sending to process: %ld\n", cPid);
    printf ("%d messages on queue\n\n", (int)msq_status.msg_qnum);

    exit(0);
}
