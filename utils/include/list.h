// © PCOM labs & Router-Dataplane HW1 LIST
#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

typedef struct cell *list;

// Linked lit
struct cell
{
  void *element;  /* pointer to a value */
  list next;      /* pointer to next cell in list */
};

/* Createing a new cell in linked list with 
 * the given element, head of the list */
extern list cons(void *__element, list __l);

/* Removing head of the list and returns the new list */
extern list cdr_and_free(list __l);

#endif
