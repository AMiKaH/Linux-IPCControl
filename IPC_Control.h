#pragma once

#ifndef IPC_CONTROL_H
#define IPC_CONTROL_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

//=========================================
//Linux uncomment the line below
//#define MAXBUFFSIZE         (4096 - 16)
//MacOS uncomment the line below
#define MAXBUFFSIZE       (2048 - 16)
//=========================================

#define SOMEVALUE           1024
#define MKEY                66
#define SKEY                55
#define CLIENT_TO_SERVER    1

//Control codes
#define FILE_NOT_FOUND      -1
#define INVALID_PRIORITY    -2
#define TRANSFER_COMPLETE   -3


typedef struct
{
  long mtype;
  char mtext[MAXBUFFSIZE];
  int mesg_len;
}my_msg;

int sendMsg(int msg_qid,  my_msg* qbuf);
int readMsg(int msg_qid,  my_msg* qbuf);
int initsem (key_t key);
void P(int sid);
void V(int sid);

#endif

