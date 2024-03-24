#include "../include/client.h"

char *cmd;
int fdTCP;
struct message recvBuffer;
struct message sendBuffer;

int max(int a, int b) { return (a > b ? a : b); }

void subscribe(char *topic, int sf)
{
    // Clear sendBuffer data
    memset(sendBuffer.data, 0, sendBuffer.length);
    // Copy 's' character to the start of the buffer -- "s" = "subscribe" cmd
    memcpy(sendBuffer.data, "s", COMMAND_SEND_LEN);
    // Copy the topic string to the buffer after the 's' char
    memcpy(sendBuffer.data + COMMAND_SEND_LEN, topic, strlen(topic));
    // Copy the subscription store and forward
    memcpy(sendBuffer.data + TOPIC_LEN + COMMAND_SEND_LEN + sizeof(char), &sf, sizeof(sf));
    // Add a null terminator at the end of the buffer after the SF
    memcpy(sendBuffer.data + TOPIC_LEN + COMMAND_SEND_LEN + sizeof(char) + sizeof(sf), "\0", NULL_LEN);
    // Send the buffer to the server
    int rc = send(fdTCP, sendBuffer.data, TOPIC_LEN + COMMAND_SEND_LEN + sizeof(char) + sizeof(sf) + NULL_LEN, 0);
    // Check if the send failed and exit if it did
    DIE(rc == -1, "Fail: Send SUBSCRIBE MESSAGE to Server\n");
    // SUCCESS MESSAGE -- FEEDBACK
    fputs("Subscribed to topic.\n", stdout);
}

void unsubscribe(char *topic)
{
    // Clear the sendBuffer data
    memset(sendBuffer.data, 0, sendBuffer.length);
    // Copy 'u' character to the start of the buffer -- "u" = "unsubscribe" cmd
    memcpy(sendBuffer.data, "u", COMMAND_SEND_LEN);
    // Copy the topic string to the buffer after the 'u' char
    memcpy(sendBuffer.data + COMMAND_SEND_LEN, topic, strlen(topic));
    // Add a null terminator at the end of the buffer
    memcpy(sendBuffer.data + TOPIC_LEN + COMMAND_SEND_LEN + sizeof(char), "\0", NULL_LEN);
    // Send the buffer to the server
    int rc = send(fdTCP, sendBuffer.data, TOPIC_LEN + NULL_LEN, 0);
    // Check if the send failed and exit if it did
    DIE(rc == -1, "Fail: Send UNSUBSCRIBE MESSAGE to Server\n");
    // SUCCESS MESSAGE -- FEEDBACK
    fputs("Unsubscribed to topic.\n", stdout);
}

void processInput(fd_set *console_fd) 
{
    char *client_cmd;
    // Check if stdin is ready to be read
    if (FD_ISSET(STDIN_FILENO, console_fd)) {
        // Read the cmd from the console
        fgets(cmd, CMD_LEN, stdin);
        // Parse the cmd
        client_cmd = strtok(cmd, " \n");
        if (!strncmp(client_cmd, "subscribe", CMD_SUBSCRIBE)) { /* SUBSCRIBE */
            /* subscribe <TOPIC> <SF> */
            char *TOPIC = strtok(NULL, " ");
            client_cmd = strtok(NULL, " ");
            subscribe(TOPIC, atoi(client_cmd)); /* <TOPIC> <SF> */
        } else if (!strncmp(client_cmd, "unsubscribe", CMD_UNSUBSCRIBE)) {  /* UNSUBSCRIBE */
            /* unsubscribe <TOPIC> */
            char *TOPIC = strtok(NULL, " ");
            unsubscribe(TOPIC);
        } else if (!strncmp(client_cmd, "exit", CMD_EXIT)) {    /* EXIT */
            /* exit */
            shutdown(fdTCP, SHUT_RDWR);
            close(fdTCP);
            free(cmd);
            exit(EXIT_SUCCESS);
        }
    }
}

/**
 * @brief Process an INT message received from the SERVER and print it
 * @param IP_SERVER IP address to send the message
 * @param PORT_SERVER port number of the server to send the message
 * @param TOPIC topic of the message
 */
void processIntMessage(char *IP_SERVER, uint16_t PORT_SERVER, char *TOPIC)
{
    uint8_t sign = 0;
    // Extract sign of the int from the message data
    memcpy(&sign, sendBuffer.data + TOPIC_LEN + TYPE_LEN, SIGN_LEN);
    uint32_t number = 0;
    // Extract the integer value from the message data
    memcpy(&number, sendBuffer.data + TOPIC_LEN + TYPE_LEN + SIGN_LEN, sizeof(uint32_t));
    // Convert integer value from message data
    int p = ntohl(number);
    // Negate the integer if the sign is set
    p = (sign == 1) ? p*(-1) : p;
    // Print message format : <IP_CLIENT>:<PORT_CLIENT> - <TOPIC> - <INT> - <VALUE MESSAGE>
    fprintf(stdout, "%s:%u - %s - INT - %d\n", IP_SERVER, ntohs(PORT_SERVER), TOPIC, p);
}

/**
 * @brief Process a SHORT_REAL message received from the SERVER and print it
 * @param IP_SERVER IP address to send the message
 * @param PORT_SERVER port number of the server to send the message
 * @param TOPIC topic of the message
 */
void processShortRealMessage(char *IP_SERVER, uint16_t PORT_SERVER, char *TOPIC)
{
    uint16_t number = 0;
    // Extract the short real value from the message data
    memcpy(&number, sendBuffer.data + TOPIC_LEN + TYPE_LEN, sizeof(uint16_t));
    // Convert the short real from network byte order to host byte order
    unsigned short p = ntohs(number);
    float res = (float)p / 100;
    // Print message format : <IP_CLIENT>:<PORT_CLIENT> - <TOPIC> - <SHORT_REAL> - <VALUE MESSAGE>
    fprintf(stdout, "%s:%u - %s - SHORT_REAL - %.2f\n", IP_SERVER, ntohs(PORT_SERVER), TOPIC, res);
}

/**
 * @brief Conevert uint32_t to float
 * @param number number to convert
 * @param point position of the decimal point in the number
 * @param sign sign of the number
 * @return float value of the number
 */
float convert_number(uint32_t number, uint8_t point, uint8_t sign)
{
    // Convert the number from network byte order to host byte order
    float p = ntohl(number);
    // Adjust the decimal point to the correct position
    for (int k = 0; k < point; k++) p /= 10;
    // Negate the integer if the sign is set
    p = (sign == 1) ? (-1)*p : p;
    return p; /* return float value of the number */
}

/**
 * @brief Process a FLOAT message received from the SERVER and print it
 * @param IP_SERVER IP address to send the message
 * @param PORT_SERVER port number of the server to send the message
 * @param TOPIC topic of the message
 */
void processFloatMessage(char *IP_SERVER, uint16_t PORT_SERVER, char *TOPIC)
{
    uint8_t sign = 0;
    // Extract sign of the float from the message data
    memcpy(&sign, sendBuffer.data + TOPIC_LEN + TYPE_LEN, SIGN_LEN);
    uint32_t number = 0;
    // Extract the float value from the message data
    memcpy(&number, sendBuffer.data + TOPIC_LEN + TYPE_LEN + SIGN_LEN, sizeof(uint32_t));
    uint8_t module = 0;
    // Extract the module of negative power of 10
    memcpy(&module, sendBuffer.data + TOPIC_LEN + TYPE_LEN + SIGN_LEN + sizeof(uint32_t), sizeof(uint8_t));
    float res = convert_number(number, module, sign);
    // Print message format : <IP_CLIENT>:<PORT_CLIENT> - <TOPIC> - <FLOAT> - <VALUE MESSAGE>
    fprintf(stdout, "%s:%u - %s - FLOAT - %f\n", IP_SERVER, ntohs(PORT_SERVER), TOPIC, res);
}

/**
 * @brief Process a STRING message received from the SERVER and print it
 * @param IP_SERVER IP address to send the message
 * @param PORT_SERVER port number of the server to send the message
 * @param TOPIC topic of the message
 */
void processStringMessage(char *IP_SERVER, uint16_t PORT_SERVER, char *TOPIC) {
    // Declare an char arr to store the string message
    char string[CONTENT_LEN + TYPE_LEN];
    // Extract the string message from the received message
    memcpy(string, sendBuffer.data + TOPIC_LEN + TYPE_LEN, sizeof(string) - TYPE_LEN);
    // Last element is set to null terminator
    string[CONTENT_LEN] = '\0';
    // Print message format : <IP_CLIENT>:<PORT_CLIENT> - <TOPIC> - <STRING> - <VALUE MESSAGE>
    fprintf(stdout, "%s:%u - %s - STRING - %s\n", IP_SERVER, ntohs(PORT_SERVER), TOPIC, string);
}

void processMessage(void) 
{
    uint8_t TYPE;
    char *IP_SERVER;
    uint16_t PORT_SERVER;
    char *TOPIC;

    // TOPIC char arr and copy the TOPIC from the message data
    TOPIC = (char*) malloc(TOPIC_LEN * sizeof(char));
    memcpy(TOPIC, sendBuffer.data, TOPIC_LEN);
    TOPIC[TOPIC_LEN] = '\0';

    // TYPE field from the message data copy to the TYPE variable
    TYPE = 0;
    memcpy(&TYPE, sendBuffer.data + TOPIC_LEN, sizeof(uint8_t));

    // IP_SERVER char arr and copy the IP_SERVER from the message data
    IP_SERVER = (char*) malloc(IP_CHAR_LEN * sizeof(char));
    memcpy(IP_SERVER, sendBuffer.data + CONTENT_LEN + TOPIC_LEN + TYPE_LEN, IP_CHAR_LEN);

    // PORT_SERVER field from the message data copy to the PORT_SERVER variable
    PORT_SERVER = 0;
    memcpy(&PORT_SERVER, sendBuffer.data + CONTENT_LEN + TOPIC_LEN + TYPE_LEN + IP_CHAR_LEN, PORT_LEN);

    // Call the appropriate function to process the message based on the message type
    switch(TYPE) {
        case 0:
            processIntMessage(IP_SERVER, PORT_SERVER, TOPIC);
            break;
        case 1:
            processShortRealMessage(IP_SERVER, PORT_SERVER, TOPIC);
            break;
        case 2:
            processFloatMessage(IP_SERVER, PORT_SERVER, TOPIC);
            break;
        case 3:
            processStringMessage(IP_SERVER, PORT_SERVER, TOPIC);
            break;
        default:
            fprintf(stderr, "Invalid message.\n");
    }

    free(TOPIC);
    free(IP_SERVER);
}
