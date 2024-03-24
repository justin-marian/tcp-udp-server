#include "../include/action.h"

int fdClients, fdTCP, fdUDP, WISH_PORT;
struct message sendBuffer;
buff clients;

int max(int a, int b) { return (a > b ? a : b); }

void accept_new_connection(int fdTCP, fd_set *type_fd, int *fdClients, bool *receiveID, int val,
                           struct sockaddr_in client_addr, socklen_t client_len)
{
    // Indicate that server received a new ID from the client
    *receiveID = true;
    // Accept the new connection & store its socket descriptor in fdTCP
    fdTCP = accept(fdTCP, (struct sockaddr *)&client_addr, &client_len);
    DIE(fdTCP == -1, "Fail: Connection Refused\n");

    // Disable the Nagle algorithm to reduce latency
    int rc = setsockopt(fdTCP, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(int));
    DIE(rc == -1, "Fail: Disable Nagle SERVER\n");

    // New socket descriptor in set of active descriptors
    FD_SET(fdTCP, type_fd);
    // Update the highest file descriptor if needed
    if (fdTCP > *fdClients)
        *fdClients = fdTCP;
}

void format_buffer(char *format, char *buffer, char *ipaddr, struct sockaddr *addr)
{
    // Socket address, human-readable IP address & copy it to the IP address buffer
    strcpy(ipaddr, inet_ntoa(((struct sockaddr_in *)addr)->sin_addr));
    // Clear the format buffer & copy the message buffer to it
    memset(format, 0, BUFFER_SERVER_LEN + IP_CHAR_LEN + 2 * NULL_LEN);
    // Copy the IP address & port to the format buffer
    memcpy(format, buffer, BUFFER_SERVER_LEN);
    memcpy(format + BUFFER_SERVER_LEN, ipaddr, strlen(ipaddr));
    memcpy(format + BUFFER_SERVER_LEN + IP_CHAR_LEN, &((struct sockaddr_in *)addr)->sin_port, sizeof(uint16_t));
    // Add a carriage return character at the end of the format buffer
    memcpy(format + BUFFER_SERVER_LEN + IP_CHAR_LEN + 2 * NULL_LEN, "\r", 1);
}

void receive_udp_message(int fd_client, char *buffer, buff clients)
{
    socklen_t addrLen = sizeof(struct sockaddr);
    struct sockaddr *addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
    // Clear the buffer to receive the message
    memset(buffer, 0, BUFFER_SERVER_LEN);
    // Receive the message from the client
    int rc = recvfrom(fd_client, buffer, BUFFER_SERVER_LEN, 0, addr, &addrLen);
    DIE(rc == -1, "Fail: Receive Message Clients\n");

    // Extract the topic from the received message buffer
    char topic[TOPIC_LEN + NULL_LEN];
    memcpy(topic, buffer, TOPIC_LEN);
    topic[TOPIC_LEN] = '\0';

    // Create a string format for the received message that includes the IP address and port number of the sender
    char format[BUFFER_SERVER_LEN + IP_CHAR_LEN + 2 * NULL_LEN + TYPE_LEN];

    // Allocate memory for the IP address string
    char ipaddr[IP_CHAR_LEN];
    // Null character at the end of the IP address string
    ipaddr[IP_CHAR_LEN - 1] = '\0';

    int i = 0;
    // Iterate through all clients
    while (i < clients->len)
    {
        void *p = get_pos(clients, i);
        client *c = (client *)p;

        int j = 0;
        // Iterate through all topics
        while (j < c->intrests.topics->len)
        {
            format_buffer(format, buffer, ipaddr, addr);
            // If the current client has subscribed to the received message topic, send the message to the client
            if (!strcmp(*(char **)get_pos(c->intrests.topics, j), topic))   /* TOPICS */
            {
                if (c->sock != -1)
                    send(c->sock, format, BUFFER_SERVER_LEN + IP_CHAR_LEN + 2 * NULL_LEN + TYPE_LEN, 0);
                else if (*(int *)get_pos(c->intrests.sfs, j) == 1)      /* SFS */
                {
                    // If the client is not currently connected, add the message to the client's inWaiting queue
                    char *temp = (char *)malloc(BUFFER_SERVER_LEN + IP_CHAR_LEN + 2 * NULL_LEN + TYPE_LEN);
                    memcpy(temp, format, BUFFER_SERVER_LEN + IP_CHAR_LEN + 2 * NULL_LEN + TYPE_LEN);
                    queue_enq(c->inWaiting, &temp);
                }
            }
            j++;
        }
        i++;
    }
    free(addr);
}

void subscribe(char *topic, int sf, int client_sock, buff clients)
{
    int i = 0;
    // Iterate through all clients
    while (i < clients->len)
    {
        // Get the i-th client from the clients
        void *p = get_pos(clients, i);
        client *c = (client *)p;

        // If the client's socket matches the provided socket
        if (c->sock == client_sock)
        {
            // Allocate memory for the new topic and copy it to the allocated memory
            char *new_topic = (char *)malloc(TOPIC_LEN + NULL_LEN);
            memcpy(new_topic, topic, TOPIC_LEN + NULL_LEN);
            // Add new topic & SF flag to the client's interests buffer
            add_pos(c->intrests.topics, &new_topic, 0);
            add_pos(c->intrests.sfs, &sf, 0);
        }
        i++;
    }
}

void unsubscribe(char *topic, int client_sock, buff clients)
{
    int i = 0;
    // Iterate through all clients
    while (i < clients->len)
    {
        // Get the i-th client from the clients
        void *p = get_pos(clients, i);
        client *c = (client *)p;
        // If the client's socket matches the provided socket
        if (c->sock == client_sock)
        {
            // Iterate through all topics in the client's interests
            for (int j = 0; j < c->intrests.topics->len; j++)
            {
                // Get the j-th topic from the client's interests list
                void *q = get_pos(c->intrests.topics, j);
                char *t = *(char **)q;

                // If the topic matches the provided topic
                if (!strcmp(t, topic))
                {
                    // Delete the topic and its associated
                    // Subscription flag from the client's buffer
                    del_pos(c->intrests.topics, j);
                    del_pos(c->intrests.sfs, j);
                }
            }
        }
        i++;
    }
}

void handle_id_client(int fd_client, fd_set *type_fd, struct sockaddr_in client_addr)
{
    int client_exists = 0;
    // Initialize loop counter and flag variable to 0
    for (int i = 0; i < clients->len; i++)
    {
        // Get the current client at index i
        void *p = get_pos(clients, i);
        client *c = (client *)p;

        // If the client ID matches the ID in the sendBuffer
        if (!strcmp(c->ID, sendBuffer.data))
        {
            client_exists = 1;
            // If the client is already connected
            if (c->sock != -1)
            {
                // Close the new client socket & remove it
                close(fd_client);
                FD_CLR(fd_client, type_fd);
                printf("Client %s already connected.\n", c->ID);
            }
            else
            {
                // If the client is not yet connected
                c->sock = fd_client;
                // Set the client's socket to the new client socket
                printf("New client %s connected from %s:%d\n", sendBuffer.data, inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
                while (!queue_empty(c->inWaiting))
                {
                    // Send all the messages that the client missed while it was disconnected
                    char **message = queue_deq(c->inWaiting);
                    send(fd_client, *message, BUFFER_SERVER_LEN + IP_CHAR_LEN + 2 * NULL_LEN + TYPE_LEN, 0);
                }
            }
            break;
        }
    }
    // If the client ID was not found in the list of clients
    if (!client_exists)
    {
        // Create a new client
        client new_client;
        new_client.sock = fd_client;
        // Initialize the client's interests topic buffer
        new_client.intrests.topics = init(sizeof(char *), sizeof(char *));

        // Set the client's ID, initialize the waiting queue and interests sf buffer
        memcpy(new_client.ID, sendBuffer.data, ID_LEN);
        new_client.inWaiting = queue_create();
        new_client.intrests.sfs = init(sizeof(int), sizeof(int));

        // Add the new client to the list of clients
        add_pos(clients, &new_client, len(clients));
        // NEW CONNECTION CREATED
        printf("New client %s connected from %s:%d\n", sendBuffer.data, inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
    }
}

void disconnect_client(int fd_client, fd_set *type_fd)
{
    int clients_len = clients->len;
    // Iterate through all clients in the clients
    for (int i = 0; i < clients_len; i++)
    {
        // Get the i-th client from the clients list
        void *nr_disconn = get_pos(clients, i);
        int socket_disconn = ((client *)nr_disconn)->sock;

        // If the client's socket does not match the provided socket, continue to the next client
        if (socket_disconn != fd_client)
            continue;
        else
        {
            // Close the client's socket and remove
            close(fd_client);
            FD_CLR(fd_client, type_fd);

            // Set the client's socket to -1 to indicate that it has been disconnected
            ((client *)get_pos(clients, i))->sock = -1;
            nr_disconn = get_pos(clients, i);

            // Get the client's ID
            char *ID_Client_disconnect = ((client *)nr_disconn)->ID;
            // DISCONNECT WITH SUCCESS
            printf("Client %s disconnected.\n", ID_Client_disconnect);
        }
    }
}

void handle_request(int fd_client, struct sockaddr_in client_addr)
{
    int sf; /* Store-Forward */
    char topic[TOPIC_LEN + COMMAND_SEND_LEN]; /* topic name */

    char action; /* store action type 's' - subscribe & 'u' - unsubscriber */
    // Extract the action type from the data in the sendBuffer
    memcpy(&action, sendBuffer.data, 1);

    // check the action type
    if (action == 's') /* Subscribe action */
    {
        // Extract topic name & SF flag from the sendBuffer
        memcpy(topic, sendBuffer.data + 1, TOPIC_LEN + COMMAND_SEND_LEN);
        memcpy(&sf, sendBuffer.data + TOPIC_LEN + COMMAND_SEND_LEN + sizeof(char), sizeof(sf));
        // Perform the subscription
        subscribe(topic, sf, fd_client, clients);
    }
    else if (action == 'u') /* Unsubscribe action */
        unsubscribe(topic, fd_client, clients);
    else    /* Invalid action type */
        fprintf(stderr, "Invalid message.\n");
}

void command_exit(int fdCLIENT, fd_set *type_fd)
{
    // Create a buffer to store the user's command received
    char cmd[CMD_EXIT + NULL_LEN];
    // Read the user's command from the console
    fgets(cmd, CMD_EXIT + NULL_LEN, stdin);

    // Check if the user's command is "exit", if not, print an error message and return
    DIE(strncmp(cmd, "exit", CMD_EXIT + NULL_LEN) == 1, "Fail: Invalid CMD\n");

    // If the command is "exit", close all client connections
    // And shutdown the TCP and UDP sockets
    if (!strncmp(cmd, "exit", CMD_EXIT + NULL_LEN))
    {
        // Loop through all file descriptors and close the client connections
        for (int fd = 0; fd <= fdClients; fd++)
        {
            // Remove the client connection from the file descriptor table
            close(fdCLIENT);
            fd_set *conn_fd = type_fd;
            FD_CLR(fdCLIENT, conn_fd);
        }
        // Shutdown the TCP/UDP sockets & free memory buffer
        shutdown(fdTCP, SHUT_RDWR);
        shutdown(fdUDP, SHUT_RDWR);
        close(fdTCP);
        close(fdUDP);
        free(sendBuffer.data);
        exit(EXIT_SUCCESS);
    }
    // If the user's command is not "exit", free the send buffer
    // And exit the program with failure status
    free(sendBuffer.data);
    exit(EXIT_FAILURE);
}