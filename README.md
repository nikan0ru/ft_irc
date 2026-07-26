*This project has been created as part of the 42 curriculum by lhchiban, yrhandou.*

# ft_irc

## Description

`ft_irc` is an Internet Relay Chat (IRC) server written in C++98.

The goal of this project is to build a server that allows multiple IRC clients to connect over TCP/IP, authenticate with a password, register a nickname and username, join channels, exchange private messages, and communicate inside channels.

The server accepts multiple clients simultaneously over TCP/IP and allows them to authenticate, register, join channels, send messages, and use channel operator commands.

## Technical Choices

The server is written in C++98 and uses non-blocking I/O.

The server handles all clients through a single polling mechanism. Each connected client has its own state, including authentication status, nickname, username, joined channels, and input buffer.

Because TCP is stream-based, received data may arrive partially or with multiple commands in the same packet. The server stores incoming data in a client buffer and only processes complete IRC commands once a line ending is received.

Main internal components:

- `Server` — manages the listening socket, polling loop, client connections, receiving data, sending data, and command dispatching
- `Client` — stores client-related information such as file descriptor, nickname, username, authentication state, and buffers
- `Channel` — stores channel members, operators, topic, modes, key, invite list, and user limit
- Command handlers — implement IRC commands and numeric replies


## Features List

Mandatory features implemented by the server:

- Client connection through TCP/IP
- Password-based authentication
- Nickname and username registration
- Channel creation and joining
- Private messages between users
- Channel messages broadcast to joined users
- Channel operators and regular users
- Operator commands:
  - `KICK` — eject a client from a channel
  - `INVITE` — invite a client to a channel
  - `TOPIC` — view or change the channel topic
  - `MODE` — change channel modes
- Supported channel modes:
  - `i` — invite-only channel
  - `t` — restrict topic changes to channel operators
  - `k` — set or remove channel key/password
  - `o` — give or remove channel operator privilege
  - `l` — set or remove user limit


## Instructions

### Compilation

Compile the project using:

```bash
make
```

The Makefile provides the required rules:

```bash
make
make all
make clean
make fclean
make re
```

### Execution

Run the server with:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

Arguments:

- `<port>`: the port number on which the server listens for incoming IRC connections.
- `<password>`: the connection password required by IRC clients.

## Usage Example

### Registering a client

```text
PASS secretpass
NICK alice
USER alice 0 * :Alice
```

### Joining a channel

```text
JOIN #chat
```

### Sending a message to a channel

```text
PRIVMSG #chat :Hello channel
```

### Sending a private message to a user

```text
PRIVMSG bob :Hello Bob
```

### Changing a topic

```text
TOPIC #chat :New channel topic
```

### Setting channel modes

```text
MODE #chat +i
MODE #chat +t
MODE #chat +k password
MODE #chat +l 10
MODE #chat +o bob
```

### Removing channel modes

```text
MODE #chat -i
MODE #chat -t
MODE #chat -k
MODE #chat -l
MODE #chat -o bob
```

### Resources

Beej's Guide to Networking Concepts : https://www.beej.us/guide/bgnet/html/split/

Network Management : https://linux-kernel-labs.github.io/refs/heads/master/lectures/networking.html

TCP Listen Queues : https://arthurchiao.art/blog/tcp-listen-a-tale-of-two-queues/#11-why-listen-queues

RFC 1459 : https://datatracker.ietf.org/doc/html/rfc1459

The Socket Interface : https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/Sockets.html

## AI Usage

AI tools were used as a learning and support aid during the project.

AI was used for:

- Explaining IRC concepts and command behavior
- Understanding socket programming and non-blocking I/O
- Reviewing possible project architecture
- Helping write and improve documentation

