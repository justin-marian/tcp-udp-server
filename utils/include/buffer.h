#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "helper.h"
#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Buffer structure
struct buff
{
    int len, cap; /* length, capacity */
    size_t size;  /* size elements */
    void **data;  /* array of pointers to elements */
};

typedef struct buff *buff;

// Topics with Store-Forwards Client Subscriptions
typedef struct TopicSF
{
    buff topics; /* subscribed topics */
    buff sfs;    /* store/forward news about topic */
} topic_sfs;

/**
 * Initialize a new buffer with the given capacity and element size
 * @param cap capacity of the buffer
 * @param size size of each element in bytes
 * @return pointer to the newly initialized buffer
 */
buff init(int cap, size_t size);

/**
 * Get the length of the buffer (nr of elements stored)
 * @param buffer pointer to the buffer
 * @return length of the buffer
 */
int len(buff buffer);

/**
 * Get the element stored at the given position in the buffer
 * @param buffer a pointer to the buffer
 * @param pos the position of the element to retrieve
 * @return pointer to the element at the specified position
 */
void *get_pos(buff buffer, int pos);

/**
 * Add a new element to the buffer at the given position
 * @param buffer pointer to the buffer
 * @param data pointer to the data to add
 * @param pos position at which to insert the data
 */
void add_pos(buff buffer, void *data, int pos);

/**
 * Delete the element at the given position from the buffer
 * @param buffer pointer to the buffer
 * @param pos position of the element to delete
 */
void del_pos(buff buffer, int pos);

#endif /* _BUFFER_H_ */