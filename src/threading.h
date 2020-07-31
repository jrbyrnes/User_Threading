#ifndef THREADING_H
#define THREADING_H

#include <stdio.h>
#include <csetjmp>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <iostream>

#define SECOND 500000
#define TIME_QUANTUM 1*SECOND
#define STACK_SIZE 4096
#define MAX_THREADS 100
#define MAX_SWITCHES 50

enum State{Running = 0, Ready = 1, Asleep = 2, Done = 3};
enum Schedule{uniform = 0, randomWeight = 1, assignWeight = 2};

sigjmp_buf jbuf[MAX_THREADS];
sigjmp_buf mainbuf;

int nRuns = 0;


#include "target.h"
#include "thread.h"
#include "node.h"
#include "linkedList.h"


#endif