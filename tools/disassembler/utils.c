/*
 * utils.c
 *
 *  Created on: 10 ago. 2023
 *      Original Author: Emiliano Augusto Gonzalez (egonzalez . hiperion @ gmail . com)
 *
 * Further modified by Kayne Ruse, and added to the Toy Programming Language tool repository.
 */

#include "stdio.h"
#include "stdlib.h"

#include "utils.h"

struct Node *queue_front, *queue_rear;

void enqueue(void *x) {
	struct Node *temp;

	temp = (struct Node*) malloc(sizeof(struct Node));
	temp->data = x;
	temp->next = NULL;

	if (queue_front == NULL && queue_rear == NULL) {
		queue_front = queue_rear = temp;
		return;
	}
	queue_rear->next = temp;
	queue_rear = temp;

}

void dequeue(void) {
	struct Node *temp = queue_front;

	if (queue_front == NULL) {
		printf("Error : QUEUE is empty!!");
		return;
	}
	if (queue_front == queue_rear)
		queue_front = queue_rear = NULL;

	else
		queue_front = queue_front->next;

	free(temp->data);
	free(temp);
}

void* front(void) {
	return queue_front->data;
}
