// TCP-UDP SERVER -- fdTCP/fdUDP used by server to communicate with clients
#include "utils/include/action.h"

int main(int argc, char **argv)
{
    // Declare file descriptor sets & server address struct
    fd_set fdIN;
    struct sockaddr_in server_addr;
    // Set server address struct & its length
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(struct sockaddr);

    // Allocate memory for sendBuffer
    sendBuffer.length = BUFFER_SERVER_LEN;
    sendBuffer.data = (char *) malloc(sendBuffer.length * sizeof(char));

    // Disable buffering for stdout & stderr
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    setvbuf(stderr, NULL, _IONBF, BUFSIZ);

    // Initialize clients using the custom ADT buff - retains all clients ON/OFF 
    clients = init(sizeof(client), sizeof(client));
    
    // Clear the IN socket descriptor
    FD_ZERO(&fdIN);

    // Create socket for TCP server to responde to TCP clients
    fdTCP = socket(AF_INET, SOCK_STREAM, 0);
    DIE(fdTCP == -1, "Fail: Creating Server Socket\n");

    int val = 1;
    // Disable Nagle algorithm to reduce latency
    int rc = setsockopt(fdTCP, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(int));
    DIE(rc == -1, "Fail: Disable Nagle SERVER\n");

    // Get the port number from the cmd line arguments
    WISH_PORT = atoi(argv[1]);

    // Init server address structure
    size_t server_len = sizeof(server_addr);
    struct sockaddr_in *server_addr_n = &server_addr;
    memset(server_addr_n, 0, server_len);
    // Set up the server address 
    server_addr.sin_addr.s_addr = INADDR_ANY; /* ACCEPT ALL MESSAGES RECEIVED */
    server_addr.sin_port = htons(WISH_PORT); /* PORT_SERVER which we want to make */
    server_addr.sin_family = AF_INET; /* IPV4 */

    // Bind the TCP socket to the server address
    rc = bind(fdTCP, (struct sockaddr *)server_addr_n, server_len);
    DIE(rc == -1, "Fail: Bind Socket SERVER\n");

    // Start listening connection to a specific number of clients
    rc = listen(fdTCP, NO_CONNS_LISTEN);
    DIE(rc == -1, "Fail: Listen Clients\n");

    // Create socket for UDP server to responde to UDP clients
    fdUDP = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(fdUDP == -1, "Fail: Creating UDP Server Socket\n");

    // Bind the UDP socket to the server address
    rc = bind(fdUDP, (struct sockaddr *)server_addr_n, server_len);
    DIE(rc == -1, "Fail: Bind Server-Client Connection\n");

    // stdin, UDP & TCP socket file descriptors used to
    // read file descriptor set   
    FD_SET(STDIN_FILENO, &fdIN);
    FD_SET(fdUDP, &fdIN);
    FD_SET(fdTCP, &fdIN);

    // Keep track of the maximum file descriptor value
    // Use it in the select function
    int fdClients = max(fdTCP, fdUDP);
    // This FLAG tells the server if it (not) waits an ID from client
    bool receiveID = false;

    while (true)
    {
        // Make sure to not modify the fdIN descriptor
        fd_set curr_fd = fdIN;
        // Clear sendBuffer be sure that we don't corrupt the current message
        // With the previous one to CLIENTS
        memset(sendBuffer.data, 0, sendBuffer.length);

        // USED for handling incoming messages from clients
        struct sockaddr addr;
        socklen_t addrLen = sizeof(struct sockaddr);

        // Wait for incoming messages using the select function use fdClients + 1
        // with fdClients I am sure I am handling both types of clients TCP/UDP
        rc = select(fdClients + 1, &curr_fd, NULL, NULL, NULL);
        DIE(rc == -1, "Fail: Select Socket Client\n");

        // Take all the clients connected, is SLOWER, not all the sockets
        // are used in the range of [0, fdClients], it was the EASIEST WAY for me
        // to not missing a socket from any client
        for (int fd = 0; fd <= fdClients; fd++)
        {
            // Verify : data available to read from the TCP/UDP socket
            if (FD_ISSET(fd, &curr_fd))
            {
                if (fd == fdTCP)    // TCP socket has A NEW CONNECTION
                    accept_new_connection(fdTCP, &fdIN, &fdClients, &receiveID, val, client_addr, client_len);
                else if (fd == fdUDP)   // UDP message was received by corresponding client
                    receive_udp_message(fd, sendBuffer.data, clients);
                else if (fd == STDIN_FILENO) // Command introduced by the server cmd line
                    command_exit(fd, &fdIN);
                else    // HANDLE THE CLIENT REQUESTS
                {
                    // NR. OF BYTES + MESSAGE RECEIVED
                    int bytes_recv = recvfrom(fd, sendBuffer.data, BUFFER_SERVER_LEN, 0, &addr, &addrLen);
                    DIE(bytes_recv == -1, "Fail: Bytes Received\n");
                    // IF THE CLIENT HAS DISCONNECTED WE DON'T RECEIVE ANYMORE BYTES
                    if (bytes_recv == 0)
                        disconnect_client(fd, &fdIN);
                    else if (receiveID == true) // SERVER WAITS A MESSAGE FROM AN ID CLIENT
                    {
                        handle_id_client(fd, &fdIN, client_addr);
                        receiveID = false;
                    }
                    else    // HANDLE GENERAL REQUESTS SEND BY CLIENTS
                        handle_request(fd, client_addr);
                }
            }
        }
    }
}
