/*
 * ThreadQueueWithStat.h
 *
 *  Created on: Sep 6, 2013
 *     Author: Gaurav & Amit
 */


#include <stdio.h>
#include <stdlib.h>
#include "ThreadStructure.h"

//Current Running Node declared in ThreadManipulation.h
extern AGThreadQNode * runningThreadNode;

//code to enqueue a node in the given list
void enqueue (AGThreadQNode ** head , AGThreadQNode ** tail , AGThreadQNode * node)
{
	if(*head == NULL)
	{
		*head = *tail = node;
		node->next = NULL;
	}
	else
	{
		(*tail) -> next = node;
		*tail = node;
		node -> next = NULL;
	}
}

//code to dequeue a node from the given list and return it
AGThreadQNode * dequeue (AGThreadQNode ** head , AGThreadQNode ** tail)
{
	AGThreadQNode * node = NULL;
	if(*head == NULL)
	{
		return NULL;
	}
	else
	{

		node = *head;
		*head = (*head)->next;
		if(*head == NULL)
			*tail = NULL;
		node -> next = NULL;
	}
	return node;
}

//code to insert a new node in the list
AGThreadQNode * insertNewNode(AGThreadQNode ** head , AGThreadQNode ** tail)
{
	AGThreadQNode * node;
	if((node = malloc(sizeof(AGThreadQNode))) == 0)
			return NULL;
	if((node->thread = malloc(sizeof(AGThreadTCB))) == 0)
	{
		free(node);
		return NULL;
	}
//	if((node->thread->jb = malloc(sizeof(sigjmp_buf))) == 0)
//	{
//		free(node);
//		return NULL;
//	}

	if(*tail == NULL)
	{
		*head = node;
		*tail = node;
	}
	else
	{
		(*tail)->next = node;
		*tail = node;
	}
	node->next = NULL;
	return node;
}

//code to traverse given list
void traverse(AGThreadQNode ** head)
{
	AGThreadQNode * node = *head;
		while(node != NULL)
	{
		printf("%d \n",node->thread->tid);
		node = node -> next;
	}
}

//code to extract a node with the given tid in the given list and return it
AGThreadQNode * getNode(AGThreadQNode ** head , AGThreadQNode ** tail , int t_id)
{
	AGThreadQNode * node , * temp;
	node = *head;
	//traverse(head);
	if(*tail == NULL)
	{
		return NULL;
	}

	if(node->thread->tid == t_id)
	{
		*head = node->next;
		if(*head == NULL)
			*tail = *head;
		return node;
	}
	while(node->next != NULL)
	{
		if(node->next->thread->tid == t_id)
		{
			temp = node->next;
			node->next = temp->next;
			if(*tail == temp)
				*tail = node;
				//	return 1;
		//	traverse(head);
			return temp;
		}
		node = node->next;
	}

	return NULL;
}

//code to search a node with the given tid in the given list
AGThreadQNode * searchNode(AGThreadQNode ** head , AGThreadQNode ** tail , int t_id)
{
	AGThreadQNode * node , * temp;
	node = *head;
	//traverse(head);
	if(*tail == NULL)
	{
		return NULL;
	}

	if(node->thread->tid == t_id)
	{
		return node;
	}
	while(node->next != NULL)
	{
		if(node->next->thread->tid == t_id)
		{
			temp = node->next;

		//	traverse(head);
			return temp;
		}
		node = node->next;
	}

	return NULL;
}





