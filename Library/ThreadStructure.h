/*
 * ThreadStructure.h
 *
 *  Created on: Sep 6, 2013
 *     Author: Gaurav & Amit
 */
#ifndef THREADSTRUCTURE_H_
#define THREADSTRUCTURE_H_





#include <setjmp.h>

//Constants
#define SECOND 1000000
#define STACKSIZE 10240

typedef enum { RUNNING , READY , SLEEPING , SUSPENDED , WAITING ,  DEAD } status ;

typedef union func		//will hold start function
{
	void (*without_args)(void);
	void * (*with_args)(void *);
}fun_type;

//Thread Control Block structure
typedef struct AGThread
{
	int tid;
	status state;
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


