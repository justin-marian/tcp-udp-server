#ifndef _HELPER_H
#define _HELPER_H 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * Example to use macro DIE:
 * FILE *fp = fopen("myfile.txt", "r");
 * DIE(fp == NULL, "Error opening file!");
 */

/**
 * Error handler, takes a condition and a variable
 * number of arguments, function prints an error 
 * message to the standard error stream.
 */
#define DIE(condition, ...)                                  \
    do                                                       \
    {                                                        \
        if ((condition))                                     \
        {                                                    \
            fprintf(stderr, "[(%s:%d)]: ",                   \
                    __FILE__, __LINE__);                     \
            fprintf(stderr, __VA_ARGS__);                    \
            fprintf(stderr, "\n");                           \
            exit(EXIT_FAILURE);                              \
        }                                                    \
    } while (0)

#endif
