# Subscription Client Management Platform

This project is a **client-server application** that facilitates **message management** using **sockets**. It follows the **client-server model**, where the **server acts as a broker**, enabling **clients to publish and subscribe to messages**.  

The implementation includes:  

- **TCP and UDP communication** for data transmission.  
- **Multiplexing TCP and UDP connections** to handle multiple clients simultaneously.  
- **A predefined protocol over UDP** to ensure structured message exchange.  
- **A client-server architecture** using sockets for network communication.

## 1. System Architecture  

The project consists of three main components: **a server, TCP clients, and a UDP client**. These components work together to implement a **publish-subscribe messaging system**.

### Server

- Opens two sockets: **one TCP and one UDP**.  
- Listens for incoming connections and messages on all available **IP addresses**.  
- Manages **client subscriptions**, **message forwarding**, and **store-and-forward (SF) functionality**.  

### TCP Clients

- Connect to the server via **TCP sockets**.  
- Can **subscribe** or **unsubscribe** from topics.  
- Receive and display only messages related to their **subscribed topics**.  

### UDP Client

- Publishes messages to the server using a **predefined protocol**.  
- Messages are **distributed only** to TCP clients that are subscribed to the **relevant topics**.  
- The UDP client does not require persistent connections.  

### Store-and-Forward (SF) Functionality

- When a **TCP client disconnects**, the server **stores messages** for its subscribed topics.  
- Upon **reconnection**, the stored messages are **delivered to the client** to ensure no data loss.  
- Ensures **message reliability** even in case of temporary disconnections.  

## 2. Buffer Structure

The buffer data structure provides **dynamic storage** for elements with a fixed initial capacity, which can **grow as needed**. It is used for **queueing messages** and **managing client subscriptions** in a publish-subscribe system.  

### Store/Forward Subscription Structure  

In addition to standard buffer functionality, this structure is used for **store/forward client subscriptions**, ensuring that disconnected clients receive messages upon reconnection.  

- Maintains two buffers:
  - **Subscribed topics buffer** – Tracks topics each client is subscribed to.  
  - **Store/forward message buffer** – Stores messages for clients with active store-forward subscriptions.  

### Buffer Functions  

#### `buff init(int cap, size_t size)`  

- Initializes a buffer with a specified **capacity** and **element size**.  
- Dynamically allocates memory for the buffer.  

#### `int len(buff buffer)`  

- Returns the **current number of elements** stored in the buffer.  

#### `void *get_pos(buff buffer, int pos)`  

- Retrieves the **element at a given position** in the buffer.  
- Returns a pointer to the element.  

#### `void add_pos(buff buffer, void *data, int pos)`  

- Inserts a **new element** at the specified position.  
- If necessary, the buffer is **expanded dynamically**.  

#### `void del_pos(buff buffer, int pos)`  

- Removes the **element at a specific position** and shifts remaining elements.  
- If memory optimization is enabled, the buffer may **shrink** when underutilized.  

This buffer implementation is **optimized for real-time applications**, ensuring data storage and retrieval for message queues and client subscriptions.

## 3. Header Files

### `message.h` 

This header file defines constants and data structures used for **message management** in the client-server application.  

- Defines **maximum lengths** for message content, topic names, client IDs, and command strings.  
- Contains **command length constants** for `"subscribe"`, `"unsubscribe"`, and `"exit"` commands.  
- Defines a **message structure** with a length field and a dynamically allocated buffer for message content.  
- Includes standard **C libraries** and **networking headers** for handling sockets, message transmission, and address conversions.  

### `helper.h`

This header file provides **error handling utilities**.  

- Defines the **`DIE()`** macro, which acts as a **generic error handler**.  
- If an error condition occurs, `DIE()` prints an error message and terminates execution with a failure status.  
- Used throughout the project for **socket operations, memory allocation failures, and other critical errors**.  

### `list.h` / `queue.h`

These headers provide **data structures** for managing client subscriptions and message queues.  

### `list.h`

Implements a **linked list** for storing elements dynamically.  

- Defines a `cell` structure containing:
  - A **void pointer** to store any data type.  
  - A pointer to the **next cell** in the list.  
- Provides **list manipulation functions**:
  - `cons()` – Creates a new list node with a given element.  
  - `cdr_and_free()` – Removes the **head of the list** and deallocates memory.  

### `queue.h`

Implements a **queue** based on the linked list structure.  

- Defines a `queue` structure with:
  - **Pointers to the head and tail** of the linked list.  
- Provides **queue management functions**:
  - `queue_create()` – Initializes a new queue.  
  - `queue_enq()` – Adds an element to the end of the queue.  
  - `queue_deq()` – Removes and returns the front element.  
  - `queue_empty()` – Checks if the queue is empty.  
  - `queue_peek()` – Returns the front element without removing it.  
  - `queue_destroy()` – Frees memory for the entire queue and its elements.  

These data structures are used in **message buffering, client subscription tracking, and store-and-forward functionalities**.

## 4. Subscriber

The client is responsible for connecting to the server over **TCP sockets**, subscribing to topics, receiving messages, and processing commands from the user.
The implementation is handled in `subscriber.c` and `client.c`, while `client.h` provides necessary utilities for console message handling.  

**Key Functionalities:**

- **Establishing a Connection:**  
  - Opens a TCP socket to connect to the server.  
  - Sends a unique **client ID** to the server for identification.  
  - Disables the **Nagle algorithm** to reduce latency.  

- **Handling User Input and Server Messages:**  
  - Uses `select()` to wait for activity on either **standard input (stdin)** or the **TCP socket**.  
  - If input is detected from **stdin**, processes user commands.  
  - If data is received from the **server**, it is stored and processed accordingly.  

**Commands:**

- **`subscribe <topic> <SF>`**  
  - The client sends a **subscription request** to the server.  
  - The server will forward messages from this topic, even if the client is offline when **SF (store-and-forward)** is enabled.  

- **`unsubscribe <topic>`**  
  - Sends an **unsubscribe request**, removing the client from the topic's subscriber list.  

- **`exit`**  
  - Closes the TCP connection and releases allocated resources.  

**Message Handling:**  

- **Receiving Data from the Server:**  
  - The client stores incoming messages in a **receive buffer (`recvBuffer`)**.  
  - Messages are processed when a **complete message is received**.  

- **Processing Different Message Types:**  
  - **`processIntMessage()`** – Handles integer-based messages.  
  - **`processShortRealMessage()`** – Handles floating-point messages.  
  - Extracts metadata such as **server IP, port, and topic**.  

**Error Handling:**

- Errors are detected and handled using the **`DIE()` macro**.  
- The client terminates the connection and releases memory when shutting down.  

**Program Lifecycle:**  

1. Initializes **buffers and network settings**.  
2. Connects to the **server** and **sends the client ID**.  
3. Enters a loop where it waits for **user commands** or **server messages**.  
4. Processes **incoming messages** or **user requests** accordingly.  
5. When the client exits, the connection is **closed**, and allocated resources are **freed**.  

## 5. Server

The server is responsible for managing client connections, handling messages, and forwarding them to subscribed clients in a **publish-subscribe messaging system**. It listens on **both TCP and UDP sockets** and maintains a buffer of client subscriptions.  

**Key Functions:**

- **`accept_new_connection()`** – Accepts new TCP clients, disables the **Nagle algorithm** for reduced latency, and registers them in the active connections set.  
- **`receive_udp_message()`** – Extracts topic from incoming UDP messages and forwards them to subscribed clients. If a client is offline with **store-and-forward (SF)** enabled, the message is stored for later delivery.  
- **`subscribe()`** – Adds a client to a topic’s subscription list and enables SF if requested.  
- **`unsubscribe()`** – Removes a client from a topic’s subscription list.  
- **Message Processing:**  
  - Iterates through all **subscribed clients** and **forwards messages** if they are connected.  
  - If a client is **offline with SF enabled**, messages are added to its pending queue.  

The server handles **real-time message delivery** while ensuring **disconnected clients receive stored messages upon reconnection**.
