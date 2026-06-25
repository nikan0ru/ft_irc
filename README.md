# ft_irc
Internet Relay Chat or IRC is a text-based (the protocol relies purely on plain-text strings rather than graphical data or rich media) communication protocol on the Internet.
It offers real-time messaging that can be either public or private. Users can exchange
direct messages and join group channels.

IRC clients connect to IRC servers in order to join channels. IRC servers are connected
together to form a network.

--------------------
1. Plain-Text Strings
This is the native language of IRC. The core IRC protocol (defined in RFC 1459 and RFC 2812) is an entirely text-based, line-driven protocol.

What it is: Raw alphanumeric characters encoded typically in UTF-8 or ASCII (e.g., PRIVMSG #channel :Hello world!).

How the server handles it: Effortlessly. The server reads the bytes from a socket, parses the commands (like NICK, JOIN, PRIVMSG), and broadcasts the text to other connected sockets.

Resource Impact: Extremely low. It uses minimal bandwidth and memory.
---------------------

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

in cpp we have socket func thats create an endpoint for communication


cpp```
    #include <sys/socket.h>

    int socket(int domain, int type, int protocol);
```
the <sys/socket.h> is the main sockets header

cpp```
    if ((server::socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "ERROR: creation of socket fialed";
        return EXIT_FAILURE;
    }
```



