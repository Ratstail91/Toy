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

struct disassembler_node_s {
	void *data;
	struct disassembler_node_s *next;
};

extern struct disassembler_node_s *queue_front, *queue_rear;

void disassembler_enqueue(void *x);
void disassembler_dequeue(void);
void* disassembler_front(void);

#endif /* UTILS_H_ */
