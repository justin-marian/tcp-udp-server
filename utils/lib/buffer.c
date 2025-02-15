#include "../include/buffer.h"

buff init(int cap, size_t size)
{
    // Check for invalid capacity / data size
    DIE(cap <= 0, "ERROR: CAPACITY\n");
    DIE(size <= 0, "ERROR: DATA SIZE\n");

    // Allocate buff structure
    buff buffer = malloc(sizeof(buff));
    DIE(buffer == NULL, "ERROR: MALLOC BUFFER\n");

    // Allocate data inside the buffer
    buffer->data = calloc(cap, sizeof(void *));
    DIE(buffer->data == NULL, "ERROR: MALLOC DATA BUFFER\n");

    // Initialize the buffer structure with the given: 
    // capacity, data size, and length
    buffer->size = size;
    buffer->cap = cap;
    buffer->len = 0;

    return buffer;
}

int len(buff buffer)
{
    return buffer ? buffer->len : 0;
}

void *get_pos(buff buffer, int pos)
{
    // Check for empty buffer / invalid position
    DIE(buffer == NULL, "ERROR: Empty Buffer\n");
    DIE(pos > len(buffer), "ERROR: Invalid Position in Buffer\n");
    // Return the element at the given position
    return buffer->data[pos];
}

void add_pos(buff buffer, void *data, int pos)
{
    // Check for empty buffer, invalid position, or no data
    DIE(buffer == NULL, "ERROR: Empty Buffer\n");
    DIE(pos > len(buffer), "ERROR: Invalid Position in Buffer\n");
    DIE(data == NULL, "ERROR: No DATA Receieved\n");

    // buffer is full => increase its capacity
    if (len(buffer) == buffer->cap)
    {
        buffer->cap += buffer->cap / 2;
        buffer->data = realloc(buffer->data, sizeof(void *) * buffer->cap);
    }

    // Make room for the new element
    memmove(&buffer->data[pos + 1], &buffer->data[pos], sizeof(void *) * (len(buffer) - pos));
    // Allocate new element and copy the data into it
    buffer->data[pos] = malloc(buffer->size);
    memcpy(buffer->data[pos], data, buffer->size);
    // Increase the length of the buffer
    ++(buffer->len);
}

void del_pos(buff buffer, int pos)
{
    // Check for empty buffer / invalid position
    DIE(buffer == NULL, "ERROR: Empty Buffer\n");
    DIE(pos > len(buffer), "ERROR: Invalid Position in Buffer\n");

    // Start loop that iterates over the elements in the buffer 
    // starting from the position to be deleted
    int i = pos + 1;
    while (i++ < len(buffer))
    {
        // Copy each element to the previous position
        // Move each element with one position to the left
        void *prev = buffer->data[i];
        void *curr = buffer->data[i + 1];
        // Overwriting the element at the position to be deleted
        memcpy(prev, curr, buffer->size);
    }
    // Free the deleted element
    free(buffer->data[pos]);
    // Decrement length buffer
    --(buffer->len);
}