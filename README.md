# 📌 Subscription Client Management Platform  

This project is a **client-server application** that facilitates **message management** using **sockets**. It follows the **client-server model**, where the **server acts as a broker**, enabling **clients to publish and subscribe to messages**.  

The implementation includes:  

- 🔗 **TCP and UDP communication** for data transmission.  
- ⚡ **Multiplexing TCP and UDP connections** to handle multiple clients simultaneously.  
- 📡 **A predefined protocol over UDP** to ensure structured message exchange.  
- 🖧 **A client-server architecture** using sockets for network communication.  

---

## ⚙️ 1. System Architecture  

The project consists of three main components: **a server, TCP clients, and a UDP client**. These components work together to implement a **publish-subscribe messaging system**.

### 🖥️ Server  

- Opens **two sockets**: **one TCP and one UDP**.  
- Listens for incoming connections and messages on all available **IP addresses**.  
- Manages **client subscriptions**, **message forwarding**, and **store-and-forward (SF) functionality**.  

### 🖧 TCP Clients  

- Connect to the server via **TCP sockets**.  
- Can **subscribe** or **unsubscribe** from topics.  
- Receive and display **only messages related to their subscribed topics**.  

### 📡 UDP Client  

- Publishes messages to the server using a **predefined protocol**.  
- Messages are **distributed only** to TCP clients that are subscribed to the **relevant topics**.  
- The UDP client does **not require persistent connections**.  

### 📥 Store-and-Forward (SF) Functionality  

- When a **TCP client disconnects**, the server **stores messages** for its subscribed topics.  
- Upon **reconnection**, the stored messages are **delivered to the client** to ensure **no data loss**.  
- Ensures **message reliability** even in case of temporary disconnections.  

---

## 🗂️ 2. Buffer Structure  

The buffer data structure provides **dynamic storage** for elements with a fixed initial capacity, which can **grow as needed**. It is used for **queueing messages** and **managing client subscriptions** in a **publish-subscribe system**.  

### 🛑 Store/Forward Subscription Structure  

In addition to standard buffer functionality, this structure is used for **store/forward client subscriptions**, ensuring that disconnected clients receive messages upon reconnection.  

- Maintains **two buffers**:
  - **📂 Subscribed topics buffer** – Tracks topics each client is subscribed to.  
  - **🗄️ Store/forward message buffer** – Stores messages for clients with active store-forward subscriptions.  

### 🏗️ Buffer Functions  

- **`buff init(int cap, size_t size)`** – Initializes a buffer with a specified **capacity** and **element size**.  
- **`int len(buff buffer)`** – Returns the **current number of elements** stored in the buffer.  
- **`void *get_pos(buff buffer, int pos)`** – Retrieves the **element at a given position** in the buffer.  
- **`void add_pos(buff buffer, void *data, int pos)`** – Inserts a **new element** at the specified position.  
- **`void del_pos(buff buffer, int pos)`** – Removes the **element at a specific position** and shifts remaining elements.  

This buffer implementation is **optimized for real-time applications**, ensuring data storage and retrieval for message queues and client subscriptions.

---

## 📑 3. Header Files  

### 📨 `message.h`  

- Defines **maximum lengths** for message content, topic names, client IDs, and command strings.  
- Contains **command length constants** for `"subscribe"`, `"unsubscribe"`, and `"exit"` commands.  
- Includes standard **C libraries** and **networking headers** for handling sockets, message transmission, and address conversions.  

### ❗ `helper.h`  

- Provides **error handling utilities**.  
- Defines the **`DIE()`** macro, which acts as a **generic error handler**.  
- If an error condition occurs, `DIE()` prints an error message and terminates execution.  

### 🛠️ `list.h` / `queue.h`  

#### 📜 `list.h`  

- Implements a **linked list** for dynamically storing elements.  
- Provides **list manipulation functions**:
  - `cons()` – Creates a new list node with a given element.  
  - `cdr_and_free()` – Removes the **head of the list** and deallocates memory.  

#### 🔄 `queue.h`  

- Implements a **queue** based on the linked list structure.  
- Provides **queue management functions**:
  - `queue_create()` – Initializes a new queue.  
  - `queue_enq()` – Adds an element to the queue.  
  - `queue_deq()` – Removes and returns the front element.  
  - `queue_destroy()` – Frees memory for the entire queue.  

These data structures are used in **message buffering, client subscription tracking, and store-and-forward functionalities**.

---

## 🧑‍💻 4. Subscriber  

The client connects to the server over **TCP sockets**, subscribes to topics, receives messages, and processes commands from the user.  

**🛠️ Key Functionalities:**  

- **🔗 Establishing a Connection:**  
  - Opens a TCP socket to connect to the server.  
  - Sends a unique **client ID** to the server for identification.  
  - Disables the **Nagle algorithm** to reduce latency.  

- **📥 Handling User Input and Server Messages:**  
  - Uses `select()` to wait for activity on **stdin** or the **TCP socket**.  
  - Processes **user commands** and **incoming messages** from the server.  

### 📝 Commands  

- **`subscribe <topic> <SF>`** – Client subscribes to a topic. If **SF (store-and-forward)** is enabled, the server saves messages while the client is offline.  
- **`unsubscribe <topic>`** – Client unsubscribes from a topic.  
- **`exit`** – The client closes the TCP connection and releases allocated resources.  

### 📡 Message Handling  

- Processes **different message types**, including:  
  - **`processIntMessage()`** – Handles integer-based messages.  
  - **`processShortRealMessage()`** – Handles floating-point messages.  
  - Extracts metadata such as **server IP, port, and topic**.  

- **🛑 Error Handling:**  
  - Errors are handled using the **`DIE()` macro**.  
  - The client terminates the connection and releases memory when shutting down.  

---

## 🖥️ 5. Server  

The server **manages client connections, subscriptions, and messages**. It listens on **both TCP and UDP sockets**, forwarding messages to subscribed clients.  

### 🔑 Key Functions  

- **🔌 `accept_new_connection()`** – Accepts new TCP clients, disables **Nagle's algorithm**, and registers them.  
- **📥 `receive_udp_message()`** – Extracts topic from **UDP messages** and forwards them to **subscribed clients**.  
- **📡 `subscribe()`** – Adds a client to a topic’s subscription list.  
- **🚫 `unsubscribe()`** – Removes a client from a topic’s subscription list.  
- **📨 Message Processing:**  
  - **Forwards messages** to subscribed clients if they are **online**.  
  - If a client is **offline with SF enabled**, messages are **stored** for later delivery.  

The server ensures **real-time message delivery**, while **disconnected clients receive stored messages upon reconnection**.

---
