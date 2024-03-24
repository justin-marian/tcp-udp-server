#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

// Length NULL char -- 1byte
#define NULL_LEN 1
// Length field TYPE message -- 1byte
#define TIP_DATE_LEN 1
// MAX Length TOPIC client
#define TOPIC_LEN 50
// MAX Length CONTENT message
#define CONTENT_LEN 1500
// MAX Length ID client
#define ID_LEN 10
// MAX Legth buffer used by clients
#define BUFFER_CLIENT_LEN 1570
// MAX Legth buffer used by server
#define BUFFER_SERVER_LEN 1551
// MAX Length CMD entered by users
#define CMD_LEN 100
// Length SUBSCRIBE CMD to a topic
#define CMD_SUBSCRIBE 9
// Length UNSUBSCRIBE CMD to a topic
#define CMD_UNSUBSCRIBE 11
// Length EXIT CMD server/topic
#define CMD_EXIT 4
// Length representation IP addr XXX.XXX.XXX.XXX\0
#define IP_CHAR_LEN 16
// Length field in a client message
#define TYPE_LEN 1
// Length representation PORT nr -- short 2bytes
#define PORT_LEN 2
// Length SIGN nr for negative/positive nr -- 1byte
#define SIGN_LEN 1
// One of the commands sent by client: subscribe (s) / unsubscribe (u)
#define COMMAND_SEND_LEN 1
// MAX NR CONNECTIONS that can be listen in queue
#define NO_CONNS_LISTEN 5000

// Store message with a specific length and content
struct message
{
    int length; /* length of data message */
    char *data; /* pointer to a character array -- holds message */
};

#endif /* _MESSAGE_H_ */