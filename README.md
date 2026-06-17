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

