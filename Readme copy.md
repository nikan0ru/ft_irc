IRC (Internet Relay chat) 

    " IRC is a protocol defined in RFC 1459."
    is a earliest form of online communication 
    enabling real-time text-based interactions 
    between users in chat rooms or private messages.
    We built IRC on top of TCP/IP, but we didn't design it in a way that forces it to stay there — it could potentially work on other network types too(Private LAN, Tor, Radio/Mesh).

RFC — Request for Comments
    RFCs are the official technical documents that define the standards,
    protocols, and guidelines that power the internet and networked systems.

RFCs cover things like:
    How data is transmitted (TCP, UDP, IP)
    How emails work (SMTP)
    How the web works (HTTP)
    Security protocols (TLS, SSH)
    And thousands of other technical specs

IRC protocol have  a core conceponents:
    # server : 
        The backbone of IRC, handling client connections, managing channels, and         relaying  messages, IRC servers connect to each other in a tree shape, where each server acts as a hub for those below it, forming one big organized network:
            - Lets clients (users) connect and talk to each other.
            - Lets other servers connect to it, building a larger network.
        so irc server do hings like :
            handling client connections, managing channels, and relaying messages....

        1. Message Routing

        When you send a message, your client doesn't send it directly to other users
        The server receives it and forwards it to the right place
        If the recipient is on another server, your server passes it along the tree


        2. User Authentication & Registration

        The server checks and registers your nickname when you connect
        It makes sure no two users have the same nickname on the network
        It can require passwords to connect


        3. Channel Management

        Servers create and manage chat rooms (channels)
        They track who is inside each channel
        They enforce channel rules like bans, invite-only, password-protected rooms


        4. Permission & Mode Control

        Servers enforce user modes (are you an operator? are you invisible?)
        They enforce channel modes (is the channel moderated? limited users?)
        They give or remove operator privileges (@ status)


        5. Keeping Track of the Network

        Each server maintains a map of the whole IRC network
        It knows which servers are connected and which users are where
        If a server disconnects, it updates the rest of the network


        6. Broadcasting Announcements

        Server sends system messages to users (welcome messages, errors, notices)
        It broadcasts events like "User X has joined" or "Server Y has disconnected"


        7. Enforcing Rules / Killing Connections

        Servers can kill (forcefully disconnect) misbehaving users
        They can ban IPs or hostnames from connecting
        Operators on the server level can manage the entire network


        8. Ping / Keepalive

        Servers regularly ping clients to check if they are still connected
        If a client doesn't respond → server drops the connection
        This keeps the network clean from ghost/dead connections
    
    # clients : A client is anything connecting to a server that is not another
                server, basically, it's the user's side of IRC (your chat app/program).
                How is Each Client Identified?
                    Every client has a unique nickname with a max of 9 characters 
                    — no two users can have the same one.
                What Info Does the Server Store About Each Client?
                    The server must know 3 things about every connected client:
                        🖥️ Host NameThe real name of the machine the client is running on
                        👤 UsernameThe user's name on that machine
                        🔗 Connected ServerWhich server they are connected to
                so :
                    A client is any non-server connection, identified by a unique 9-character nickname,
                    and the server tracks their hostname, username, and which server they're on.

    # Channels: Public or private chat rooms where users can communicate.
    # Commands: Text-based instructions like /join, /msg, and /nick.

    Building the Server
        To create a server we need to use socket programming.
        What is Socket Programming:
        A socket is an endpoint for communication between two machines over a network.
        In network programming, a socket allows you to establish a connection between a client and a server,
        enabling them to send and receive data. Sockets provide the interface for both network
        communication protocols (such as TCP and UDP) and local interprocess communication (IPC).

        --- what is an endpoint an endpoint is a specific point where communication are starts
        or ends in a network connection. ex: You (endpoint 1) <----call----> Friend (endpoint 2)

        in networking terms
        an endpoint indentified by two things:
            IP Address + Port --> endpoint
        ex: 192.168.11 : 8080

        IP Address → which machine on the network
        Port → which specific program/service on that machine






# 🏗️ Architecture & Linux Kernel TCP Internals

This section details how the server interacts with the Linux Kernel networking stack, mapping user-space C++ system calls (`socket`, `bind`, `listen`, `poll`, `accept`, `recv`, `send`) directly to internal kernel data structures (`struct tcp_sock`), dual hash table lookups (`lhash2` & `ehash`), and system memory queues.

---

## 1. Memory Architecture & File Descriptor Mapping

This diagram illustrates how an integer File Descriptor (`fd`) in the C++ server process maps through the Virtual File System (VFS) down to the underlying transport control blocks and global kernel lookup tables.

```text
+-----------------------------------------------------------------------------------------------------------------+
|                                                   USER SPACE                                                    |
|                                                                                                                 |
|   C++ Server Application (Process Table)                                                                        |
|   +---------------------------------------------------------------------------------------------------------+   |
|   | File Descriptor (FD) Array (task_struct -> files -> fd_array[])                                         |   |
|   |                                                                                                         |   |
|   |   fd[3]  <---  Listening Socket Handle (listen_fd)                                                       |   |
|   |   fd[4]  <---  Connected Client Socket Handle (client_fd)                                                  |   |
|   +---------------------------------------------------------------------------------------------------------+   |
+-----------------------------------------------------------------------------------------------------------------+
                                       │                                   │
                                       ▼                                   ▼
===================================================================================================================
                                         KERNEL SPACE (VFS & Socket Layer)
===================================================================================================================
|                                                                                                                 |
|   Open File Table (struct file)                                                                                 |
|   +---------------------------------------+               +---------------------------------------+             |
|   | struct file for [fd 3]                |               | struct file for [fd 4]                |             |
|   |  - f_flags: O_NONBLOCK                |               |  - f_flags: O_NONBLOCK                |             |
|   |  - f_op   : socket_file_ops           |               |  - f_op   : socket_file_ops           |             |
|   |  - private_data ──────────────────┐   |               |  - private_data ──────────────────┐   |             |
|   +-----------------------------------|---+               +-----------------------------------|---+             |
|                                       │                                                       │                 |
|   BSD Socket Layer (struct socket)    │                                                       │                 |
|   +-----------------------------------▼---+               +-----------------------------------▼---+             |
|   | struct socket (Listening)             |               | struct socket (Client Connected)      |             |
|   |  - state: SS_UNCONNECTED              |               |  - state: SS_CONNECTED                |             |
|   |  - sk   ──────────────────────────┐   |               |  - sk   ──────────────────────────┐   |             |
|   +-------------------------------|-------+               +-------------------------------|-------+             |
|                                   │                                                       │                     |
====================================|=======================================================|======================
                                    │ KERNEL SPACE (TCP/IP Protocol Stack)                  │
====================================|=======================================================|======================
|                                   ▼                                                       ▼                     |
|   Full Socket Object (struct tcp_sock)                    Full Socket Object (struct tcp_sock)                  |
|   +---------------------------------------------------+   +---------------------------------------------------+ |
|   | Listening Server Socket                           |   | Established Client Socket                         | |
|   |  - sk_state : TCP_LISTEN                          |   |  - sk_state : TCP_ESTABLISHED                     | |
|   |  - skc_num  : 6667 (Bound Port)                 |   |  - 4-Tuple   : LocalIP:6667 <-> ClientIP:54321  | |
|   |  - skc_rcv_saddr : 0.0.0.0                        |   |                                                   | |
|   |                                                   |   |  - sk_receive_queue (RX Queue):                   | |
|   |  - sk_ack_backlog (Accept Queue):                 |   |    [sk_buff] -> [sk_buff] (Incoming Data)         | |
|   |    [child_sock_1] -> [child_sock_2]               |   |                                                   | |
|   |    (Fully established connections awaiting accept)|   |  - sk_write_queue (TX Queue):                     | |
|   |                                                   |   |    [sk_buff] -> [sk_buff] (Outgoing Data)         | |
|   +---------------------------------------------------+   +---------------------------------------------------+ |
|                                   │                                                       │                     |
|                                   ▼                                                       ▼                     |
|   ===========================================================================================================   |
|   │                                   GLOBAL KERNEL TCP HASH TABLES                         │   |
|   │                                                                                                         │   |
|   │   1. lhash2 (Listening Hash Table)                        2. ehash (Established Hash Table)             │   |
|   │   +-----------------------------------------------+       +-----------------------------------------+   │   |
|   │   | Key: 2-Tuple [DstIP, DstPort]                 |       | Key: 4-Tuple [SrcIP,SrcPort,DstIP,DstP] |   │   |
|   │   | Value: Pointer to Listening struct tcp_sock   |       | Value: Pointer to Active struct tcp_sock|   │   |
|   │   +-----------------------------------------------+       |   (Holds: TCP_ESTABLISHED & request_sock)|   │   |
|   │                                                           +-----------------------------------------+   │   |
|   ===========================================================================================================   |
+-----------------------------------------------------------------------------------------------------------------+
```

---

## 2. Socket Lifecycle & Execution Flow

This sequential flowchart tracks connection initialization, automated TCP handshakes, data exchange, and disconnect handling.

```text
===================================================================================================================
PHASE 1: SERVER INITIALIZATION (socket -> bind -> fcntl -> listen)
===================================================================================================================

 [C++ USER SPACE]                                     [LINUX KERNEL / TCP STACK]
 ────────────────                                     ──────────────────────────
 
 1. int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    │
    └──────────────────────────────────────────────► Allocates struct socket & struct tcp_sock
                                                     - State: TCP_CLOSE
                                                     - FD Index: fd[3]
                                                     - Hash Tables: NOT in lhash2, NOT in ehash

 2. bind(listen_fd, &addr, sizeof(addr));
    │
    └──────────────────────────────────────────────► Saves IP (skc_rcv_saddr) & Port (skc_num)
                                                     - Registers port in bhash / bhash2 (bind tables)

 3. fcntl(listen_fd, F_SETFL, O_NONBLOCK);
    │
    └──────────────────────────────────────────────► Sets O_NONBLOCK flag on struct file for fd[3]

 4. listen(listen_fd, SOMAXCONN);
    │
    └──────────────────────────────────────────────► Changes state: TCP_CLOSE ──► TCP_LISTEN
                                                     - Hashes 2-Tuple [IP:Port]
                                                     - Inserts socket into global lhash2 table
                                                     - Initializes Accept Queue (sk_ack_backlog)


===================================================================================================================
PHASE 2: AUTOMATED TCP 3-WAY HANDSHAKE (Network Inbound)
===================================================================================================================

 [CLIENT / NETWORK]                                   [LINUX KERNEL / TCP STACK]
 ──────────────────                                   ──────────────────────────

 1. Client sends [TCP SYN] ────────────────────────► Packet Arrives at Network Card (NIC)
                                                     │
                                                     ▼
                                                     Socket Lookup on Rx:
                                                     1. Search ehash via 4-Tuple [SrcIP,SrcPort,DstIP,DstPort]
                                                        └──► Result: NO MATCH (Not an existing connection)
                                                     2. Fallback to lhash2 via 2-Tuple [DstIP, DstPort]
                                                        └──► Result: MATCH FOUND! (Server listening socket fd[3])
                                                     │
                                                     ▼
                                                     Kernel Handshake Processing:
                                                     - Allocates lightweight request_sock
                                                     - State: TCP_NEW_SYN_RECV
                                                     - Stores 4-Tuple in request_sock
                                                     - Inserts request_sock into ehash table
                                                     - Puts request_sock in SYN Queue
                                                     │
                                   ◄─────────────────┘ Kernel sends [TCP SYN+ACK] reply

 2. Client sends [TCP ACK] ────────────────────────► Packet Arrives at NIC
                                                     │
                                                     ▼
                                                     Socket Lookup on Rx:
                                                     1. Search ehash via 4-Tuple
                                                        └──► Result: MATCH FOUND! (Finds request_sock)
                                                     │
                                                     ▼
                                                     Kernel Completion Processing:
                                                     - Clones full struct tcp_sock from listening socket
                                                     - Sets state to TCP_ESTABLISHED
                                                     - Copies 4-Tuple into child socket
                                                     - Replaces request_sock with child socket in ehash
                                                     - Moves child socket into Accept Queue (sk_ack_backlog)
                                                     - Marks poll() on listen_fd with POLLIN


===================================================================================================================
PHASE 3: CONNECTION ACCEPTANCE (poll -> accept)
===================================================================================================================

 [C++ USER SPACE]                                     [LINUX KERNEL / TCP STACK]
 ────────────────                                     ──────────────────────────

 1. poll(&pollfds, size, -1);
    │
    ├─► Wakes up because listen_fd[3] has POLLIN flag
    │
 2. int client_fd = accept(listen_fd, ...);
    │
    └──────────────────────────────────────────────► Inspects listen_fd's Accept Queue (sk_ack_backlog)
                                                     - Pops the top TCP_ESTABLISHED child socket
                                                     - Assigns next available index: fd[4] (client_fd)
                                                     - Sets O_NONBLOCK on client_fd
                                                     - Returns integer fd[4] to C++ program


===================================================================================================================
PHASE 4: ACTIVE DATA COMMUNICATION (PRIVMSG, JOIN, KICK, etc.)
===================================================================================================================

--- [RECEIVING DATA FROM CLIENT] ---

 [CLIENT / NETWORK]                                   [LINUX KERNEL / TCP STACK]
 ──────────────────                                   ──────────────────────────

 1. Client sends [TCP PSH+ACK] (Data) ─────────────► Packet Arrives at NIC
                                                     │
                                                     ▼
                                                     Socket Lookup on Rx:
                                                     1. Search ehash via 4-Tuple [SrcIP,SrcPort,DstIP,DstPort]
                                                        └──► Result: MATCH FOUND! (Child socket for client_fd)
                                                     │
                                                     ▼
                                                     - Kernel strips TCP headers
                                                     - Places raw payload into client_fd's RX Queue (sk_receive_queue)
                                                     - Sets POLLIN flag on client_fd in pollfds array

 [C++ USER SPACE]
 ────────────────
 2. poll() returns -> detects POLLIN on client_fd[4]
 3. bytes = recv(client_fd, buffer, 1024, 0);
    │
    └──────────────────────────────────────────────► Copies payload from kernel sk_receive_queue into buffer
                                                     - Frees packet sk_buff from kernel memory


--- [SENDING DATA TO CLIENT] ---

 [C++ USER SPACE]
 ────────────────
 1. send(client_fd, "PRIVMSG #1337 :Hi\r\n", len, 0);
    │
    └──────────────────────────────────────────────► [LINUX KERNEL]
                                                     - Copies string into client_fd's TX Queue (sk_write_queue)
                                                     - Wraps data in TCP packets & transmits via NIC
                                                     - Keeps copy in TX Queue until Client returns TCP ACK
                                                     - Once ACK arrives, kernel frees buffer from TX Queue


===================================================================================================================
PHASE 5: CONNECTION TEARDOWN & DISCONNECT HANDLING
===================================================================================================================

 [CLIENT / NETWORK]                                   [LINUX KERNEL / TCP STACK]
 ──────────────────                                   ──────────────────────────

 1. Client closes app / sends [TCP FIN] ───────────► Packet Arrives at NIC
                                                     │
                                                     ▼
                                                     Socket Lookup on Rx:
                                                     1. Search ehash via 4-Tuple
                                                        └──► Result: MATCH FOUND! (Child socket for client_fd)
                                                     │
                                                     ▼
                                                     - Changes state: TCP_ESTABLISHED ──► TCP_CLOSE_WAIT
                                                     - Marks client_fd with POLLIN in pollfds array

 [C++ USER SPACE]
 ────────────────
 2. poll() returns -> detects POLLIN on client_fd[4]
 3. int bytes = recv(client_fd, buffer, 1024, 0);
    │
    └──────────────────────────────────────────────► Returns 0 bytes (EOF / Disconnect Signal)

 4. close(client_fd);
    │
    └──────────────────────────────────────────────► [LINUX KERNEL]
                                                     - Sends TCP FIN+ACK to client
                                                     - Removes socket from ehash table
                                                     - Frees fd[4] index in process FD table
                                                     - Destroys associated struct file & tcp_sock memory
```

---

## 3. System Call & State Reference Matrix

| Step / Syscall | C++ Function | Kernel Socket State | Active Hash Table | Active Queue | Key Event / Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1. Create** | `socket()` | `TCP_CLOSE` | *None* | *None* | Allocates `struct tcp_sock` & FD handle (`listen_fd`). |
| **2. Bind** | `bind()` | `TCP_CLOSE` | `bhash` / `bhash2` | *None* | Binds IP & Port to socket structure. |
| **3. Listen** | `listen()` | `TCP_LISTEN` | **`lhash2`** (2-Tuple) | `sk_ack_backlog` | Registers listening socket for incoming 2-Tuple lookups. |
| **4. Handshake** | *Kernel Internal* | `TCP_NEW_SYN_RECV` | **`ehash`** (4-Tuple) | SYN Queue | Creates temporary `request_sock` upon receiving `SYN`. |
| **5. Complete** | *Kernel Internal* | `TCP_ESTABLISHED` | **`ehash`** (4-Tuple) | `sk_ack_backlog` | Clones full `tcp_sock` child socket; moves it to Accept Queue. |
| **6. Accept** | `accept()` | `TCP_ESTABLISHED` | **`ehash`** (4-Tuple) | `sk_ack_backlog` | Pops child socket from Accept Queue; returns `client_fd`. |
| **7. Read Data** | `recv()` | `TCP_ESTABLISHED` | **`ehash`** (4-Tuple) | `sk_receive_queue` | Copies data from kernel RX Queue into user-space buffer. |
| **8. Write Data**| `send()` | `TCP_ESTABLISHED` | **`ehash`** (4-Tuple) | `sk_write_queue` | Copies data to kernel TX Queue for network transmission. |
| **9. Disconnect**| `recv() == 0` | `TCP_CLOSE_WAIT` | **`ehash`** (4-Tuple) | *None* | Kernel receives `FIN`; `recv()` returns `0` bytes. |
| **10. Close** | `close()` | `TCP_CLOSED` | *Removed* | *Flushed* | Removes socket from `ehash` and frees kernel memory. |