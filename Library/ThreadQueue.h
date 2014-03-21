/*
 * ThreadQueue.h
 *
 *  Created on: Sep 6, 2013
 *      Author: Gaurav
 */

#ifndef THREADQUEUE_H_
#define THREADQUEUE_H_




#include <stdio.h>
#include <stdlib.h>
#include "ThreadStructure.h"

//declaration of functions 
void enqueue (AGThreadQNode ** head , AGThreadQNode ** tail , AGThreadQNode * node);
AGThreadQNode * dequeue (AGThreadQNode ** head , AGThreadQNode ** tail);
AGThreadQNode * insertNewNode(AGThreadQNode ** head , AGThreadQNode ** tail);
void traverse(AGThreadQNode ** head);
AGThreadQNode * getNode(AGThreadQNode ** head , AGThreadQNode ** tail , int t_id);
AGThreadQNode * searchNode(AGThreadQNode ** head , AGThreadQNode ** tail , int t_id);

#endif /* THREADQUEUE_H_ */

