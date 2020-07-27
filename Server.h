#pragma once
#ifndef SERVER_H
#define SERVER_H

#include "IPC_Control.h"

int readFile(char* fileName);
void serveClient(char* clientParams, int msq_id);

#endif
