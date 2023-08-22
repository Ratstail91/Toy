/*
 * utils.h
 *
 *  Created on: 10 ago. 2023
 *      Original Author: Emiliano Augusto Gonzalez (egonzalez . hiperion @ gmail . com)
 *
 * Further modified by Kayne Ruse, and added to the Toy Programming Language tool repository.
 */

#ifndef UTILS_H_
#define UTILS_H_

struct Node {
	void *data;
	struct Node *next;
};

extern struct Node *queue_front, *queue_rear;

void enqueue(void *x);
void dequeue(void);
void* front(void);

#endif /* UTILS_H_ */
