// © PCOM labs & Router-Dataplane HW1 QUEUE
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "list.h"

#include <stdlib.h>
#include <assert.h>

struct queue
{
	list head;
	list tail;
};

typedef struct queue *queue;

/* create an empty queue */
extern queue queue_create(void);

/* insert an element at the end of the queue */
extern void queue_enq(queue __q, void *__element);

/* delete the front element on the queue and return it */
extern void *queue_deq(queue __q);

/* return a true value if and only if the queue is empty */
extern int queue_empty(queue __q);

/* return the front element on the queue without removing it */
extern void *queue_peek(queue __q);

/* deallocate the memory used by the queue and all its elements */
void queue_destroy(queue __q);

#endif /* _QUEUE_H_ */
