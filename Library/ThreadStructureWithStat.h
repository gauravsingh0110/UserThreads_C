/*
 * ThreadStructureWithStat.h
 *
 *  Created on: Sep 6, 2013
 *      Author: Gaurav and Amit
 */
#ifndef THREADSTRUCTUREWITHSTAT_H_
#define THREADSTRUCTUREWITHSTAT_H_





#include <setjmp.h>

//Constants
#define SECOND 1000000
#define STACKSIZE 10240


typedef struct {
   enum { RUNNING , READY , SLEEPING , SUSPENDED , WAITING ,  DEAD } state ;
  long no_of_bursts;
   long total_exec_time;
  long total_sleep_time;
   long avr_wait_time;
} status;


typedef union func		//will hold start function
{
	void (*without_args)(void);
	void * (*with_args)(void *);
}fun_type;

//Thread Control Block structure
typedef struct AGThread
{
	int tid;
	status tstate;
	sigjmp_buf jb;
	char stack[STACKSIZE];
	struct semaphore_struct * th_semaphore;

	long sleep_time;						//in microseconds
	//long previous_time;						//to remember last reduction time
	int wait_no;							//no of semaphores for which thread is waiting

	fun_type start_function;
	void * argument;
	void * ret_val;
}AGThreadTCB;

//Node , will be used in different Queues
typedef struct AGThreadQueueNode
{
		AGThreadTCB * thread;
		struct AGThreadQueueNode * next;
}AGThreadQNode;

//typedef struct AGThreadAllQueueNode
//{
//		AGThreadQNode * threadNode;
//		struct AGThreadAllQueueNode * next;
//}AGThreadAllQNode;

//Structure of Semapohore
typedef struct semaphore_struct
{
	int value;
	int modified;
	AGThreadQNode * semHead;
	AGThreadQNode * semTail;
}semaphore;


#endif /* THREADSTRUCTURE_H_ */
