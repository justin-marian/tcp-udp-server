// TCP SUBSCRIBERS -- fdTCP used by TCP clients to communicate with server
#include "utils/include/client.h"

int main(int argc, char **argv)
{
    // Declare file descriptor sets & server address struct
    fd_set fdIN;
    struct sockaddr_in server_addr;

    // Disable buffering for stdout & stderr
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    setvbuf(stderr, NULL, _IONBF, BUFSIZ);

    // Allocate memory for : ID client, sendBuffer, & recvBuffer
    char *ID = (char *)malloc(ID_LEN * sizeof(char));
    recvBuffer.length = BUFFER_CLIENT_LEN;
    sendBuffer.length = BUFFER_CLIENT_LEN;
    recvBuffer.data = (char *)malloc(BUFFER_CLIENT_LEN * sizeof(char));
    sendBuffer.data = (char *)malloc(BUFFER_CLIENT_LEN * sizeof(char));

    // Clear IN socket descriptor set
    FD_ZERO(&fdIN);

    // Copy ID from parser cmd line to ID buffer
    strncpy(ID, argv[1], ID_LEN);

    // Create TCP socket
    fdTCP = socket(AF_INET, SOCK_STREAM, 0);
    DIE(fdTCP == -1, "Fail: Creating Client Socket\n");

    int val = 1;
    // Disable Nagle algorithm to reduce latency
    int rc = setsockopt(fdTCP, IPPROTO_TCP, TCP_NODELAY, (char *)&val, sizeof(int));
    DIE(rc == -1, "Fail: Disable Nagle CLIENT\n");

    // Set up server address
    server_addr.sin_family = AF_INET; /* IPV4 */
    server_addr.sin_port = htons(atoi(argv[3])); /* PORT_SERVER */
    // Translate address dotted in 32-bit number
    rc = inet_aton(argv[2], &server_addr.sin_addr); /* IP_SERVER */
    DIE(rc == 0, "Invalid SERVER DOT IP Address\n");

    // Connect to server
    rc = connect(fdTCP, (struct sockaddr *)&server_addr, sizeof(server_addr));
    DIE(rc == -1, "Fail: Connect Client to Server\n");

    // Copy ID to sendBuffer
    memset(sendBuffer.data, 0, sendBuffer.length);
    memcpy(sendBuffer.data, ID, ID_LEN);
    sendBuffer.data[ID_LEN] = '\0';
    // Send it to the server
    rc = send(fdTCP, sendBuffer.data, ID_LEN + 1, 0);
    DIE(rc == -1, "Fail: Sending ID_CLIENT to Server\n");

    // stdin & TCP socket file descriptors used to
    // read file descriptor set
    FD_SET(STDIN_FILENO, &fdIN);
    FD_SET(fdTCP, &fdIN);

    while (true)
    {
        // Make sure to not modify the fdIN descriptor
        fd_set curr_fd = fdIN;

        // Wait for activity on input file descriptors, receive messages
        rc = select(fdTCP + 1, &curr_fd, NULL, NULL, NULL);
        DIE(rc == -1, "Fail: Select Socket Server\n");
        
        // Command introduced by the client
        cmd = (char *)malloc(CMD_LEN * sizeof(char));

        // Process user input ONLY from STDIN
        processInput(&curr_fd);

        // Check if the user input is a valid command
        DIE(strncmp(*argv, "subscribe", CMD_SUBSCRIBE) == 1 ||
            strncmp(*argv, "unsubscribe", CMD_UNSUBSCRIBE) == 1 ||
            strncmp(*argv, "exit", CMD_EXIT) == 1, "Fail: Wrong COMMAND\n");

        // Verify : data available to read from the TCP socket
        if (FD_ISSET(fdTCP, &curr_fd))
        {
            // Clear recvBuffer be sure that we don't corrupt the current message
            // With the previous one to SERVER
            memset(recvBuffer.data, 0, recvBuffer.length);
            int buff_size = 0;

            while (true)
            {
                // Read data from TCP socket into recvBuffer
                buff_size = recv(fdTCP, recvBuffer.data, BUFFER_CLIENT_LEN, 0);
                DIE(buff_size == -1, "Fail: Receive Message Server\n");

                // Verify : there is no data more to read from socket, after a time
                // There will be no more activity received for this TCP socket
                if (!buff_size)
                {
                    close(fdTCP);
                    return EXIT_FAILURE;
                }

                bool line_separator = false;
                /* \r - carriage return, line separator Unix */
                for (int i = 0; i < buff_size; i++)
                {
                    // Check if the received data has a separator
                    if (recvBuffer.data[i] == '\r')
                    {
                        line_separator = true;
                        break;
                    }
                }

                // If '\r' was found it means that the message was full received, break the loop
                if (line_separator)
                {
                    // Copy the data from recvBuffer to sendBuffer
                    memcpy(sendBuffer.data, recvBuffer.data, BUFFER_CLIENT_LEN);
                    memcpy(recvBuffer.data, recvBuffer.data + BUFFER_CLIENT_LEN, BUFFER_CLIENT_LEN);
                    break;
                }
            }
            // Process the received message
            processMessage();
        }
    }

    free(cmd);
    free(ID);

    free(recvBuffer.data);
    free(sendBuffer.data);
    // Shutdown and close the TCP socket
    shutdown(fdTCP, SHUT_RDWR);
    close(fdTCP);
    // !!! WHOLE PROCESS FINISHED WITH SUCCESS !!!
    return EXIT_SUCCESS;
}
