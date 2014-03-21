/*
 * ThreadStructure.h
 *
 *  Created on: Sep 6, 2013
 *      Author: Gaurav
 */


#include <setjmp.h>

//Constants
#define SECOND 1000000
#define STACKSIZE 10240

//Architecture Dependent Code

#ifdef __x86_64__
// code for 64 bit architecture

typedef unsigned long enc_addr;

#define JB_SP 6		//location of stack pointer and Instruction Pointer in jump buffer
#define JB_IP 7

enc_addr convert_address(enc_addr addr)
{
	enc_addr ret , key;
	asm volatile("movq    %%fs:0x30,%0\n"		//getting the value of key which is used to mangle/encrypt the address
			: "=r" (key));
	ret = key ^ addr;							//xor the key with addr
	ret = (ret << 17) | (ret >> (64 - 17));		//left shift 17 times or 0x11 times
	return ret;
}

//Unused function was not working found workaround using wrapper function
//void insertArgument(void * arg , AGThreadQNode * th_node)
//{
//	void * arg1;
//	asm volatile 	("movq %1, %%rdi\n"
//					 "movq %%rdi, %0\n"
//					 :"=r" (arg1)
//					 :"r" (arg)
//					 :"%rdi");
//	printf("%p %p in insert argument \n",arg,arg1);
//}

#else

// code for 32 bit architecture

typedef unsigned int enc_addr;

#define JB_SP 4		//location of stack pointer and Instruction Pointer in jump buffer
#define JB_IP 5

enc_addr convert_address(enc_addr addr)
{
    enc_addr ret , key;
//    int size;
//    size = sizeof(enc_addr)*8;
    asm volatile("mov   %%gs:0x18,%0\n"
 	                 : "=r" (key));
    ret = key ^ addr;
    ret = (ret << 0x9) | (ret >> (32-0x9));
    return ret;
}

//Unused function was not working found workaround using wrapper function
//void insertArgument(void * arg , AGThreadQNode * th_node)
//{
//	char * temp;
//	int i = 0;
//	enc_addr t;
//	t = (enc_addr)arg;
//
//	temp = (char *)&t;
////	temp = (char *)arg;
//
//	while(i<8)
//		{
//			th_node->thread->stack[4088+i] = *temp;
//			temp++;
//			//printf("thread stack %d %x\n " ,4088+i , th_node->thread->stack[4088+i] );
//			i++;
//
//		}
//}

#endif

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

	int sleep_time;
	int wait_no;

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
	AGThreadQNode * semHead;
	AGThreadQNode * semTail;
}semaphore;

