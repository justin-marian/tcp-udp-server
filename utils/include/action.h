#ifndef _ACTION_H_
#define _ACTION_H_

#include "helper.h"
#include "queue.h"
#include "message.h"
#include "buffer.h"

typedef struct Client
{
    int sock;
    char ID[ID_LEN];
    topic_sfs intrests;
    queue inWaiting;
} client;

extern int fdClients, fdTCP, fdUDP, WISH_PORT;
extern struct message sendBuffer;
extern buff clients;

/* Returns the maximum of two ints */
extern int max(int a, int b);

/**
 * @brief Accepts a new connection TCP socket and performs necessary setup. Receives a flag
 * __receiveID to put the server in the mode to receive a client ID, then accepts the TCP connection
 * @param __sockfd file descriptor of the socket on which the connection is established
 * @param __type_fd file descriptor set used for select()
 * @param __fdClients pointer to the maximum file descriptor used by a client
 * @param __receiveID flag indicating if the ID of the client is to be received or not
 * @param __val flags for the socket
 * @param __client_addr client's address
 * @param __client_len length of the client's address
*/
extern void accept_new_connection(int __sockfd, fd_set *__type_fd, int *__fdClients, bool *__receiveID, int __val,
                                  struct sockaddr_in __client_addr, socklen_t __client_len);

/**
 * @brief Receives a message from a client connected over UDP
 * @param __fd_client The file descriptor of the UDP socket
 * @param __buffer The buffer to store the received message
 * @param __clients The buffer containing all the clients
*/
extern void receive_udp_message(int __fd_client, char *__buffer, buff __clients);

/**
 * @brief Subscribes a client to a topic, the client is identified through socket
 * @param __topic name of the topic
 * @param __sf SF value for a specific topic
 * @param __client_sock client's socket file descriptor
 * @param __clients buffer containing all the clients
*/
extern void subscribe(char *__topic, int __sf, int __client_sock, buff __clients);

/**
 * @brief Unsubscribes a client from a topic, the client is identified through socket
 * @param __topic name of the topic
 * @param __client_sock client's socket file descriptor
 * @param __clients buffer containing all the clients
*/
extern void unsubscribe(char *__topic, int __client_sock, buff __clients);

/**
 * @brief Handles the reception of the ID of a client
 * @param __fd_client file descriptor of the client's socket
 * @param __type_fd file descriptor set used for select()
 * @param __client_addr client's address
*/
void handle_id_client(int __fd_client, fd_set *__type_fd, struct sockaddr_in __client_addr);

/**
 * @brief Disconnects a client from the server, prints client ID disconnected
 * @param __fd_client file descriptor of the client's socket
 * @param __type_fd file descriptor set used for select()
*/
void disconnect_client(int __fd_client, fd_set *__type_fd);

/**
 * @brief Handles a request from a client : subscribe/unsubscribe
 * @param __fd_client file descriptor of the client's socket
 * @param __client_addr client's address
*/
void handle_request(int __fd_client, struct sockaddr_in __client_addr);

/**
 * @brief Shutdown & Close the whole TCP/UDP connections
 * @param __fd_client file descriptor of the client's socket
 * @param __type_fd file descriptor set used for select()
*/
void command_exit(int __fd_client, fd_set *__type_fd);

#endif /* _ACTION_H_ */