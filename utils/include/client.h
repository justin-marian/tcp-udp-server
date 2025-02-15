#ifndef _SUBSCRIBER_H_
#define _SUBSCRIBER_H_

#include "queue.h"
#include "helper.h"
#include "message.h"

extern char *cmd;
extern int fdTCP;
extern struct message sendBuffer;
extern struct message recvBuffer;

/* Returns the maximum of two ints */
int max(int a, int b);

/**
 * @brief Processes the message to send to server
 * and prints it to the console, depending on type:
 * INT, SHORT_REAL, FLOAT and STRING
*/
void processMessage(void);

/**
 * @brief Processes the user input from the console and 
 * sends it to the server if it is a valid SUBSCRIBE/UNSUBSCRIBE cmd
 * @param __console_fd pointer to the set of file descriptors to monitor for input
*/
void processInput(fd_set *__console_fd);

/**
 * @brief Sends a subscription message to the server with specified : topic & SF
 * @param __topic topic to subscribe to
 * @param __sf SF to use for the subscription
*/
void subscribe(char *__topic, int __sf);

/**
 * @brief Sends an unsubscribe message to the server with specified : topic
 * @param __topic topic to unsubscribe from
*/
void unsubscribe(char *__topic);

#endif /* _SUBSCRIBER_H_ */