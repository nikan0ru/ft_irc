# Tasks

RFC 2812	Command syntax, registration, numerics, JOIN, PRIVMSG, TOPIC, INVITE, KICK, message framing.

RFC 2811	Channel-management semantics for i, t, k, l, and operator behavior.

RFC 1459	Original IRC baseline and compatibility context.

RFC 2810	IRC architecture context.

RFC 2813	Server-to-server protocol; read mainly to confirm it is out of scope.


# JOIN
# MODE
# INVITE
# KICK


# Clients

A client is anything connecting to a server that is not another server. Each client is distinguished from other clients by a unique nickname. In addition to the nickname, all servers must have the following information about all clients: the real name/address of the host that the client is connecting from, the username of the client on that host, and the server to which the client is connected.

Nicknames are non-empty strings with the following restrictions:

    They MUST NOT contain any of the following characters: space (' ', 0x20), comma (',', 0x2C), asterisk ('*', 0x2A), question mark ('?', 0x3F), exclamation mark ('!', 0x21), at sign ('@', 0x40).
    They MUST NOT start with any of the following characters: dollar ('$', 0x24), colon (':', 0x3A).
    They MUST NOT start with a character listed as a channel type, channel membership prefix, or prefix listed in the IRCv3 multi-prefix Extension.
    They SHOULD NOT contain any dot character ('.', 0x2E).

Servers MAY have additional implementation-specific nickname restrictions and SHOULD avoid the use of nicknames which are ambiguous with commands or command parameters where this could lead to confusion or error.

# Channels

A channel is a named group of one or more clients.
All clients in the channel will receive all messages addressed to that channel.
The channel is created implicitly when the first client joins it, and the channel ceases to exist when the last client leaves it.
While the channel exists, any client can reference the channel using the name of the channel.
Networks that support the concept of ‘channel ownership’ may persist specific channels in some way while no clients are connected to them.

Channel names are strings (beginning with specified prefix characters).
Apart from the requirement of the first character being a valid channel type prefix character; the only restriction on a channel name is that it may not contain any spaces (' ', 0x20), a control G / BELL ('^G', 0x07), or a comma (',', 0x2C) (which is used as a list item separator by the protocol).

There are several types of channels used in the IRC protocol. The first standard type of channel is a regular channel, which is known to all servers that are connected to the network. The prefix character for this type of channel is ('#', 0x23). The second type are server-specific or local channels, where the clients connected can only see and talk to other clients on the same server. The prefix character for this type of channel is ('&', 0x26). Other types of channels are described in the Channel Types section.

Along with various channel types, there are also channel modes that can alter the characteristics and behaviour of individual channels. See the Channel Modes section for more information on these.

To create a new channel or become part of an existing channel, a user is required to join the channel using the JOIN command. If the channel doesn’t exist prior to joining, the channel is created and the creating user becomes a channel operator. If the channel already exists, whether or not the client successfully joins that channel depends on the modes currently set on the channel. For example, if the channel is set to invite-only mode (+i), the client only joins the channel if they have been invited by another user or they have been exempted from requiring an invite by the channel operators.

Channels also contain a topic. The topic is a line shown to all users when they join the channel, and all users in the channel are notified when the topic of a channel is changed. Channel topics commonly state channel rules, links, quotes from channel members, a general description of the channel, or whatever the channel operators want to share with the clients in their channel.

A user may be joined to several channels at once, but a limit may be imposed by the server as to how many channels a client can be in at one time. This limit is specified by the CHANLIMIT RPL_ISUPPORT parameter. See the Feature Advertisement section for more details on RPL_ISUPPORT.

# Channel Operators

Channel operators (or “chanops”) on a given channel are considered to ‘run’ or ‘own’ that channel. In recognition of this status,
 channel operators are endowed with certain powers which let them moderate and keep control of their channel.
The commands which may only be used by channel moderators include:

    KICK: Eject a client from the channel
    MODE: Change the channel’s modes
    INVITE: Invite a client to an invite-only channel (mode +i)
    TOPIC: Change the channel topic in a mode +t channel

Channel moderators are identified by the channel member prefix ('@' for standard channel operators, '%' for halfops) next to their nickname whenever it is associated with a channel (e.g. replies to the NAMES, WHO, and WHOIS commands).

Specific prefixes and moderation levels are covered in the Channel Membership Prefixes section.

# Message Format

An IRC message is a single line, delimited by a pair of CR ('\r', 0x0D) and LF ('\n', 0x0A) characters.

    When reading messages from a stream, read the incoming data into a buffer. Only parse and process a message once you encounter the \r\n at the end of it. If you encounter an empty message, silently ignore it.
    When sending messages, ensure that a pair of \r\n characters follows every single message your software sends out.
