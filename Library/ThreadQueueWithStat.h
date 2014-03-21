/*
 * ThreadQueueWithStat.h
 *
 *  Created on: Sep 6, 2013
 *      Author: Gaurav and Amit
 */

#ifndef THREADQUEUEWITHSTAT_H_
#define THREADQUEUEWITHSTAT_H_




#include <stdio.h>
#include <stdlib.h>
#include "ThreadStructureWithStat.h"

//declaration of functions 
void enqueue (AGThreadQNode ** head , AGThreadQNode ** tail , AGThreadQNode * node);
AGThreadQNode * dequeue (AGThreadQNode ** head , AGThreadQNode ** tail);
AGThreadQNode * insertNewNode(AGThreadQNode ** head , AGThreadQNode ** tail);
void traverse(AGThreadQNode ** head);
AGThreadQNode * getNode(AGThreadQNode ** head , AGThreadQNode ** tail , int t_id);
AGThreadQNode * searchNode(AGThreadQNode ** head , AGThreadQNode ** tail , int t_id);

#endif /* THREADQUEUE_H_ */

