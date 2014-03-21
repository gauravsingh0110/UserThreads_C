/*
 * ThreadManipulation.c
 *
 *  Created on: Sep 6, 2013
 *      Author: Gaurav & Amit
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "ThreadQueue.h"
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


//value of time quantum , always less than 1 second , in microseconds
#define TIMEQUANTUM 1000

AGThreadQNode * readyHead = NULL , * readyTail  = NULL ;	//will store ready threads , will be used by scheduler
AGThreadQNode * allHead = NULL , * allTail  = NULL;			//will store all threads
AGThreadQNode * otherHead = NULL , * otherTail  = NULL;		//will store other than ready threads
AGThreadQNode * deadHead = NULL , * deadTail  = NULL;		//will store dead nodes

//will point to the node of the current running thread
AGThreadQNode * runningThreadNode = NULL;

//for statistics
static int thread_ids = 0 , no_of_sleeping_threads = 0;
static int total_no_of_threads = 0, no_of_threads_deleted = 0 , no_of_threads_ready = 0;

//sleep realted global variables
static long time_usec = 0 , previous_time = 0;
struct timeval tv;

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
void * global_argument = NULL;
//typeOfFunction global_type_of_function;
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

// Wrapper function , this function wraps the thread function , helps to end the thread successfully and give arguments to threads and get results
void wrapperForThread()
{
	void * ret_val=NULL;
	if(global_argument == NULL)
		global_function_without_args();
	else
	{
		ret_val=global_function_with_args(global_argument);
		runningThreadNode->thread->ret_val = ret_val;
	}

	semSignal(runningThreadNode->thread->th_semaphore);
	DeleteThread(runningThreadNode->thread->tid);
	//printf("%d in wrapper\n",*(int*)ret_val);
//	ualarm(TIMEQUANTUM , TIMEQUANTUM);
//	Dispatch(SIGALRM);
	YieldCPU();
}


//semaphores

//Function to initialize semaphore
semaphore * initSemaphore(int value)
{
	semaphore * temp;
	if((temp = malloc(sizeof(semaphore))) == NULL)
			return NULL;

	temp->value = value;
	temp->semHead = NULL;
	temp->semTail = NULL;

	return temp;
}

//function to signal semaphore
void semSignal (semaphore * s)
{
	AGThreadQNode * node;
	s->value++;
	if((node = dequeue(&(s->semHead) , &(s->semTail))) == NULL)
		return;
	node->thread->wait_no--;
	if(node->thread->wait_no == 0)
	{
		node->thread->state = READY;
		no_of_threads_ready ++;
		enqueue(&readyHead , &readyTail , node);

	}
}

//function to wait on semaphore
void semWait(semaphore * s)
{
	AGThreadQNode * node;
	s->value--;
	if(s->value >= 0)
		return;
	node = runningThreadNode;
	node->thread->wait_no++;
	enqueue(&(s->semHead) , &(s->semTail) , node);
	node->thread->state = WAITING;
	YieldCPU();
}

//end semaphore

//function to add constant time to all the sleeping threads //wont be used anymore since new method of sleep function is used now more accurate
//void addTime()
//{
//	AGThreadQNode *node;
//	node = otherHead;
//	while(node != NULL)
//	{
//		if(node->thread->state == SLEEPING)
//			node->thread->sleep_time = node->thread->sleep_time +1;
//		node = node -> next;
//	}
//}

//function to reduce constant time from all the sleeping threads
void reduceTime()
{
	AGThreadQNode * node , * temp;
	long second;
	node = otherHead;
	gettimeofday(&tv, NULL);
	second = (tv.tv_sec%10)*SECOND;
	second = second + tv.tv_usec;

	if(previous_time > second)
		time_usec = second  - previous_time + 10000000;
	else
		time_usec = second - previous_time;
	previous_time = second;
	//printf("%ld previous time \n",time_usec);

	while (node != NULL)
	{
		if (node->thread->state == SLEEPING)
		{
			if ((node->thread->sleep_time = node->thread->sleep_time - time_usec) <= 0)
			{
				//printf("inside job\n");
				temp = node;
				node = node->next;
				temp = getNode(&otherHead, &otherTail, temp->thread->tid);
			//	printf("inside job1\n");
				temp->thread->state = READY;
				no_of_threads_ready++;
				enqueue(&readyHead, &readyTail, temp);
			//	printf("%ld time left in inside of the node \n",temp->thread->sleep_time);
				temp->thread->sleep_time=0;
				no_of_sleeping_threads--;
				continue;
			}
		//	printf("%ld time left in node \n",node->thread->sleep_time);
		}
		node = node->next;
	}
}

//function of main thread , this thread will always be in the ready queue ,this will yield control if there is anyother thread in the ready queue else
//it will just waste time waiting for other threads to become ready , it avoids termination of program
void mainThread()

{
	while(total_no_of_threads != (no_of_threads_deleted+1))
	{
	//	printf("%d no of threads\n",no_of_threads_ready);
		//if(no_of_threads_ready >= 1)
		//{
		//	printf("in mainThread\n");
			//addTime();
			//YieldCPU();
		//}
		//printf("just waiting\n");
		//usleep(TIMEQUANTUM);
		//waste time
		YieldCPU();
	}
	printf("\nAll Threads Completed .... ");
	CleanUp();
	//exit(0);
}


//function to create a new thread and return the thread id
int CreateThread(void (*f)(void))
{

	AGThreadQNode * th_node , * all_th;
	if((all_th = malloc(sizeof(AGThreadQNode)))== 0)
			return -1;

	if((th_node = insertNewNode(&readyHead , &readyTail)) == NULL)
	{
		free(all_th);
		return -1;
	}
	no_of_threads_ready ++;
	th_node->thread->tid = thread_ids++;
	th_node->thread->state = READY;

	th_node->thread->sleep_time = 0;
	th_node->thread->wait_no = 0;
	th_node->thread->argument = NULL;
	th_node->thread->ret_val = NULL;
	th_node->thread->start_function.without_args=f;
	th_node->thread->th_semaphore = initSemaphore(0);
	total_no_of_threads++;
	sigsetjmp(th_node->thread->jb , 1);

	th_node->thread->jb[0].__jmpbuf[JB_SP] = convert_address((enc_addr)th_node->thread->stack+STACKSIZE);
//	th_node->thread->jb[0].__jmpbuf[JB_IP] = convert_address((enc_addr)f); //old method without wrapper function
	th_node->thread->jb[0].__jmpbuf[JB_IP] = convert_address((enc_addr)wrapperForThread);
	//enqueueAll(&allHead , &allTail , th_node);
	all_th->thread = th_node->thread;
	enqueue(&allHead , &allTail , all_th);
	return th_node->thread->tid;
}

//Scheduler // disaptcher selects a thread and passes the control to that thread
void Dispatch(int sig)
{
	if(sigsetjmp(runningThreadNode->thread->jb , 1) == 1)
		return;
	if(no_of_sleeping_threads > 0)
		reduceTime();

	//printf("Traversing in Dispatch\n");

	//printf("old running thread id %d\n" , runningThreadNode->thread->tid);
	if(runningThreadNode->thread->state == RUNNING)
	{
	//	printf("inside special loop\n");
	runningThreadNode->thread->state = READY;
	no_of_threads_ready ++;
	enqueue(&readyHead , &readyTail , runningThreadNode);
	}

	if((runningThreadNode = dequeue(&readyHead , &readyTail))==NULL)
	{
		printf("ALL Threads finished status 0\n");
		exit(0); //call CleanUp here after it is declared
	}
	no_of_threads_ready --;
	runningThreadNode->thread->state = RUNNING;
	//if(runningThreadNode->thread->argument != NULL)
	//	insertArgument(runningThreadNode);
	//printf("new running thread id %d\n" , runningThreadNode->thread->tid);
//	traverse(&readyHead);
	//traverse(&readyHead);
	if(runningThreadNode->thread->argument == NULL)
		global_function_without_args = runningThreadNode->thread->start_function.without_args;
	else
		global_function_with_args = runningThreadNode->thread->start_function.with_args;
	global_argument =runningThreadNode->thread->argument;
	siglongjmp(runningThreadNode->thread->jb , 1);
}

//this function will call the scheduler for the first time and set the timer and other settings
void Go()
{

	CreateThread(mainThread);
	runningThreadNode = dequeue(&readyHead , &readyTail);
	no_of_threads_ready --;
	runningThreadNode->thread->state = RUNNING;
	//traverse(&allHead);
	signal(SIGALRM , Dispatch);
	signal(SIGINT , Clean);
	if(runningThreadNode->thread->argument == NULL)
		global_function_without_args = runningThreadNode->thread->start_function.without_args;
	else
		global_function_with_args = runningThreadNode->thread->start_function.with_args;
	global_argument =runningThreadNode->thread->argument;
	//global_function_with_args = runningThreadNode->thread->fun.start_function_with_args;
	//global_argument =runningThreadNode->thread->argument;
	ualarm(TIMEQUANTUM , TIMEQUANTUM);
	siglongjmp(runningThreadNode->thread->jb , 1);

}

//this function will return the thread id of the caller thread
int GetMyId()
{
	return runningThreadNode->thread->tid;
}

//this function will delete the thread and the thread will no longer be scheduled
int DeleteThread(int id)
{
	AGThreadQNode *node;
	if(runningThreadNode->thread->tid == id)
	{
		runningThreadNode->thread->state = DEAD;
		enqueue(&deadHead , &deadTail , runningThreadNode);
		no_of_threads_deleted++;
		YieldCPU();
	}
	if((node = getNode(&readyHead , &readyTail , id))!=NULL )
	{
		no_of_threads_ready --;
		node->thread->state = DEAD;
		enqueue(&deadHead , &deadTail , node);
		no_of_threads_deleted++;
		return id;
	}
	else 	if((node = getNode(&otherHead , &otherTail , id))!=NULL )
	{
		node->thread->state = DEAD;
		enqueue(&deadHead , &deadTail , node);
		no_of_threads_deleted++;
		return id;
	}
	return -1;

		//return deleteNode(&readyHead , &readyTail , id);
}

//on calling this function the thread yields the cpu and the dispather is called
void YieldCPU()
{
	//addTime();
	ualarm(TIMEQUANTUM , TIMEQUANTUM);
	Dispatch(SIGALRM);
}

//this function suspends the thread
int SuspendThread(int id)
{
	//ualarm(10 , TIMEQUANTUM);
	AGThreadQNode * node;
//	traverse(&readyHead);
	if(runningThreadNode->thread->tid == id)
	{
		runningThreadNode->thread->state = SUSPENDED;
		enqueue(&otherHead , &otherTail , runningThreadNode);
		YieldCPU();
	}
	else
	{	//traverse(&readyHead);
		if((node = searchNode(&otherHead , &otherTail ,id)) != NULL)
		{
			if(node->thread->state == SUSPENDED)
				return id;
			else
				return -1;
		}

//		if((node = getNode(&sleepHead , &sleepTail ,id)) != NULL)
//			node->thread->sleep_time = 0;
//		else
		if((node = getNode(&readyHead , &readyTail ,id)) == NULL)
			return -1;
		no_of_threads_ready --;
//		else
//		if((node = getNode(&waitHead , &waitTail ,id)) != NULL)
//					return -1;

		node->thread->state=SUSPENDED;
	//	printf("%d hulalalalal \n" , node->thread->tid);
	//	return 1;
	//	traverse(&readyHead);
		enqueue(&otherHead , &otherTail , node);
		//traverse(&readyHead);
	}
	return id;
}

//this function resumes the thread
int ResumeThread(int id)
{
	AGThreadQNode * node;
	if(runningThreadNode->thread->tid == id)
		return id;
	if(searchNode(&readyHead , &readyTail , id) != NULL)
		return id;
	node = getNode(&otherHead , &otherTail , id);
	if(node == NULL)
		return -1;
	if(node->thread->state != SUSPENDED)
		return -1;
//	printf("resumed thread node  %d \n" , node->thread->tid);
	no_of_threads_ready ++;
	enqueue(&readyHead , &readyTail , node);
	node->thread->state = READY;
	//traverse(&readyHead);
	return id;

}

//this function gives the status of the  given thread
int GetStatus(int id, status * stat)
{
	AGThreadQNode * node;
	if((node = searchNode(&allHead , &allTail ,id)) == NULL)
	{
		return -1;
	}
	*stat = node->thread->state;
	return id;
}

//this method makes the thread to sleep for given micro seconds
void uSleepThread(int usec)
{
	int sec;
	if(usec>=1000000 || usec<=10)
		return;
	if(usec <= TIMEQUANTUM)
		usleep(usec);
	else
	{
		sec = usec % TIMEQUANTUM;
		runningThreadNode->thread->sleep_time = usec - sec;
		runningThreadNode->thread->state = SLEEPING;
		if(no_of_sleeping_threads>0)
				reduceTime();
		else
		{
			gettimeofday(&tv, NULL);
			previous_time = (tv.tv_sec%10)*SECOND + tv.tv_usec;
		}
		enqueue(&otherHead , &otherTail , runningThreadNode);
		no_of_sleeping_threads++;
		YieldCPU();
		usleep(sec);
	}
}

//this function makes the calling thread to sleep for given seconds
void SleepThread(int sec)
{
	runningThreadNode->thread->sleep_time = sec*SECOND;
	runningThreadNode->thread->state = SLEEPING;
	if(no_of_sleeping_threads>0)
		reduceTime();
	else
	{
		gettimeofday(&tv, NULL);
		previous_time = (tv.tv_sec%10)*SECOND + tv.tv_usec;
	}
	enqueue(&otherHead , &otherTail , runningThreadNode);
	no_of_sleeping_threads++;
	YieldCPU();
}

//this is a temporary function ..will be used if user interrupts the process
void Clean(int sig)
{
	printf("\nInterrupted by User \n");
	CleanUp();
}

//this is the cleanup function , it is used to stop the multithreading and exit the program
void CleanUp()
{
	AGThreadQNode *node , *temp;
	node = readyHead;
	while(node != NULL)
	{
		temp = node;
		node = node -> next;
		free(temp);
	}
	readyHead = readyTail = NULL;
	node = otherHead;
	while(node != NULL)
	{
		temp = node;
		node = node -> next;
		free(temp);
	}
	otherHead = otherTail = NULL;
	node = deadHead;
	while(node != NULL)
	{
		temp = node;
		node = node -> next;
		free(temp);
	}
	deadHead = deadTail = NULL;
	node = allHead;
	while(node != NULL)
	{
		temp = node;
		node = node -> next;
		//printf("%d \n",temp->thread->tid);
		free(temp->thread->th_semaphore);
		free(temp->thread);
		free(temp);
	}
	allHead = allTail =NULL;
	//traverse(&allHead);
	printf("\nExitting ....\n");
	exit(0);
}

//this function creates a new thread with arguments and return value
int CreateThreadWithArgs(void * (*f)(void *), void *arg)
{

	AGThreadQNode * th_node , *all_th;
	if((all_th = malloc(sizeof(AGThreadQNode)))== 0)
				return -1;
	if(arg == NULL)
		arg=(void *)-1;
	if ((th_node = insertNewNode(&readyHead, &readyTail)) == NULL)
	{
		free(all_th);
		return -1;
	}
	no_of_threads_ready ++;
	th_node->thread->tid = thread_ids++;
	th_node->thread->state = READY;
	th_node->thread->sleep_time = 0;
	th_node->thread->wait_no = 0;
	th_node->thread->argument = arg;
	th_node->thread->ret_val = NULL;
	th_node->thread->start_function.with_args = f;
	th_node->thread->th_semaphore = initSemaphore(0);
	total_no_of_threads++;
	sigsetjmp(th_node->thread->jb, 1);

	th_node->thread->jb[0].__jmpbuf[JB_SP] = convert_address((enc_addr) th_node->thread->stack + STACKSIZE);
	//	th_node->thread->jb[0].__jmpbuf[JB_IP] = convert_address((enc_addr)f); //old method without wrapper function
	th_node->thread->jb[0].__jmpbuf[JB_IP] = convert_address((enc_addr) wrapperForThread);
	//enqueueAll(&allHead , &allTail , th_node);
	all_th->thread = th_node->thread;
	enqueue(&allHead , &allTail , all_th);
	return th_node->thread->tid;

}

//this function waits until the thread finishes , it will only wait for ready threads and sleeping threads , so as to avoid deadlock
int WaitForThread(int id)
{
	AGThreadQNode * node;
	if (runningThreadNode->thread->tid == id)
		return -1;

	if ((node = searchNode(&readyHead, &readyTail, id)) != NULL)
	{
		semWait(node->thread->th_semaphore);
		semSignal(node->thread->th_semaphore);
		return id;
	}
	if ((node = searchNode(&otherHead, &otherTail, id)) != NULL)
	{
		if (node->thread->state == SLEEPING)
		{
			semWait(node->thread->th_semaphore);
			semSignal(node->thread->th_semaphore);
			return id;
		}
	}
	return -1;

}

//this thread waits for the thread to complete and then returns the result
void * GetThreadResult(int id)
{
	AGThreadQNode * node=NULL;
	node =searchNode(&allHead , &allTail ,id);
	semWait(node->thread->th_semaphore);
	semSignal(node->thread->th_semaphore);
	return node->thread->ret_val;
}




