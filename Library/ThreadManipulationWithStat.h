/*
 * ThreadManipulationWithStat.h
 *
 *  Created on: Sep 6, 2013
 *      Author: Gaurav and Amit
 */
#ifndef THREADMANIPULATIONWITHSTAT_H_
#define THREADMANIPULATIONWITHSTAT_H_



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "ThreadQueueWithStat.h"

//value of time quantum
#define TIMEQUANTUM 10000

//Function declaration
int CreateThread(void (*f)(void));
void Go();
int GetMyId();
int DeleteThread(int thread_id);
void Dispatch(int sig);
void YieldCPU();
int SuspendThread(int thread_id);
int ResumeThread(int thread_id);
int GetStatus(int thread_id, status *stat);
void SleepThread(int sec);
void CleanUp();
int CreateThreadWithArgs(void *(*f)(void *), void *arg);
void *GetThreadResult(int tid);

void Clean();

//Functions and variables related to wrapper function
void * (*global_function_with_args)(void *);
void (*global_function_without_args)();
void wrapperForThread();

//semaphore operation function declaration
void semSignal (semaphore * s);
void semWait (semaphore * s);
semaphore * initSemaphore(int);

//Function related to sleep operation
void addTime();
void reduceTime();

//the main thread parent of all threads
void mainThread();


#endif /* THREADMANIPULATION_H_ */
