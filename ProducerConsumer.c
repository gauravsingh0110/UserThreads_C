#include<stdio.h>
#include"AGThreadWithStat.h"

semaphore * empty , * full , * s;
int cd[10];


void producer()
{
	int i=-1;
	int time;
	empty = initSemaphore(10);
	full = initSemaphore(0);
	while(1)
	{
		i++;
		semWait(empty);
		time = rand() % 1000000 ;
		printf("Produced %d\n",(cd[i%10] = i));
		uSleepThread(time);
		//uSleepThread(500000);
		//usleep(999999);
		semSignal(full);
	}

}

void consumer()
{
	int i=-1;
	int time;
	while(1)
	{
		i++;
		semWait(full);
		time = rand() % 1000000 ;
		printf("%d Consumed\n",cd[i%10]);
		uSleepThread(time);
		//uSleepThread(500000);
		//usleep(500000);
		semSignal(empty);
	}

}

int main()
{
	CreateThread(producer);
	CreateThread(consumer);
	Go();
	return 0;
}
