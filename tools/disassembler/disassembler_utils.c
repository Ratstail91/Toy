/*
 * utils.c
 *
 *  Created on: 10 ago. 2023
 *      Original Author: Emiliano Augusto Gonzalez (egonzalez . hiperion @ gmail . com)
 *
 * Further modified by Kayne Ruse, and added to the Toy Programming Language tool repository.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "disassembler_utils.h"

void dis_enqueue(void *x, queue_node_t **queue_front, queue_node_t **queue_rear, uint32_t *len) {
    queue_node_t *temp;

    temp = (queue_node_t*) malloc(sizeof(struct queue_node_s));
    temp->data = x;
    temp->next = NULL;

    if ((*queue_front) == NULL && (*queue_rear) == NULL) {
        (*queue_front) = (*queue_rear) = temp;
        ++(*len);
        return;
    }
    (*queue_rear)->next = temp;
    (*queue_rear) = temp;

    ++(*len);
}

void dis_dequeue(queue_node_t **queue_front, queue_node_t **queue_rear, uint32_t *len) {
    struct queue_node_s *temp = (*queue_front);

    if ((*queue_front) == NULL) {
        printf("Error : QUEUE is empty!!");
        return;
    }
    if ((*queue_front) == (*queue_rear))
        (*queue_front) = (*queue_rear) = NULL;

    else
        (*queue_front) = (*queue_front)->next;

    --(*len);
    free(temp->data);
    free(temp);
}

///

void str_append(char **str, const char *app) {
    if ((*str) == NULL)
        return;

    *str = realloc(*str, (strlen(*str) + strlen(app) + 1) * sizeof(char));
    memcpy((*str) + strlen(*str), app, strlen(app) + 1);
}
