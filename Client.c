/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Client.c - This file is the client side of an appltion that has a server and multiple clients. This
--                         Client has two functions, main, which will ask for user input and send information to the
--                         server about this Client's priority, the file name for the server to open and read to the
--                         Client as well as the Client's processes ID so that the Server can send the messages to the
--                         Message Queue using the clients prociess ID. In addition there's a second function running
--                         on a separate thread to calculate the time it takes to read the file.
--
-- PROGRAM: IPC Control

--
-- FUNCTIONS:
-- int main (int argc, char *argv[])
-- void *calcTime(void *vargp)
--
-- DATE: February 23, 2020
--
--
-- RETURN: int       - on success it returns a non-negative integer
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES: This file only contains one function; however, multiple instances can be run simultaneously with different priorities
-- and file names to read
--
--
----------------------------------------------------------------------------------------------------------------------*/
#include <pthread.h>
#include <time.h>
#include "IPC_Control.h"

#define PIDSIZE 6
int sid;
int isDone;
int time_taken;

void *calcTime(void *vargp);

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
-- PARAMETERS:
--      int argc     - the count of input arguments
--      char *argv[] - the input arguments delimited by white space
--
-- RETURN: int       - on success it returns a non-negative integer
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
-- This main function requires exactly 4 inputs: 1) the program name 2) filename, 3) Priority level, and 4) process ID.
-- The function will call the getsem() function to access the semaphore that is previously created by the server with Key ID 66,
-- the function will then parse the user's input (getting rid of the program name) and send it to the server with message type:
-- CLIENT_TO_SERVER which the server will recognize as a request from a client to join the message queue. Once the message is sent
-- the client will wait for server sending message of the type of client PID, if the file is opened successfully the message contains
-- text, otherwise the message is NULL. When reading the file is complete the server will send a termination message which is of size 0
-- to signify the end of the file to the client.
--------------------------------------------------------------------------------------------------------*/

int main (int argc, char *argv[])
{
    key_t mkey;
    int msq_id;
    struct msqid_ds;
    int retval;
    pid_t pid = getpid();
    char pidText[PIDSIZE];
    my_msg message;
    pthread_t thread_id;

    isDone = 0;
    
    sid = semget((key_t)SKEY,1,IPC_CREAT);
    
    if (argc < 3){
        printf("Usage: filename, priority [1-10]\n");
        return -1;
    }
    
      
    //Populate the struct
    message.mtype = CLIENT_TO_SERVER;
    message.mesg_len = 1;
    
    strncpy(message.mtext, argv[1], MAXBUFFSIZE);
    strcat(message.mtext, " ");
    strcat(message.mtext, argv[2]);
    strcat(message.mtext, " ");
    sprintf(pidText, "%d", pid);
    strcat(message.mtext, pidText);
        
      
    /*---- Get message queue identifier ------*/
    mkey = (key_t) MKEY;
    
    
    if ((msq_id = msgget (mkey, IPC_CREAT)) < 0)
    {
      perror ("msgget failed!");
      exit(2);
    }
    
    P(sid);
    while (1){
        retval = sendMsg (msq_id, &message);
        if (retval > -1)
            break;
    }
    V(sid);
    
    message.mtype = pid;
    pthread_create(&thread_id, NULL, calcTime, NULL);

    while(1){
        //Get the message from the queue
        if (readMsg(msq_id, &message) == -1){
        } else {
            if (message.mesg_len == TRANSFER_COMPLETE){
                printf("\nFile transfer completed!\n");
                break;
            } else if (message.mesg_len == FILE_NOT_FOUND) {
                printf("\n%s is not a valid file!\n", argv[1]);
                exit(0);
            } else if (message.mesg_len == INVALID_PRIORITY) {
                printf("\nPriority cannot be: %s\n", argv[2]);
                exit(0);
            } else {
                printf("%s", message.mtext);
            }
        }
    }
    
    isDone = 1;
    
    while (isDone != 0){
        
    }
        
    printf("Transfer took %dms\n", time_taken);
    return 0 ;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: calcTime
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
--      void calcTime (void *varp)
--
-- PARAMETERS:
--      void *varp   -- void pointer to arguments passed to thread function, in this case none or NULL
--
-- RETURN: void
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- NOTES:
-- This function is only used to calculate the time it takes to receive the file from the server
--------------------------------------------------------------------------------------------------------*/
void *calcTime(void *vargp)
{
    clock_t t;
    t = clock();
    while(isDone == 0){
        
    }
    t = clock() - t;
    time_taken = (((double)t)/CLOCKS_PER_SEC) * 1000;
    isDone = 0;
    return NULL;
}
