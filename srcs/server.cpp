
#include "../includes/server.hpp"

server::server(const std::string& portnum, const std::string& authpass):socket_fd(-1), reuse_flag(1),
                                                                        servport(portnum), client_fd(-1)
                                                                        // servport(portnum),
{
    (void)(authpass);
}


int server::creat_sokect()
{
    struct addrinfo hints, *serverinfo;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    if (getaddrinfo(NULL, this->servport.c_str(), &hints, &serverinfo) != 0)
    {
        std::cout << "ERROR: getaddrinfo failed.\n";
        return EXIT_FAILURE;
    }
    this->socket_fd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
    if (this->socket_fd < 0)
    {
        std::cout << "ERROR: creation of socket failed.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &this->reuse_flag, sizeof(this->reuse_flag)) == -1)
    {
        std::cout << "ERROR: failed to reuse socket.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    // " You probably noticed that when you run listener, above, it just sits there until a packet arrives.
    // What happened is that it called recvfrom(), there was no data, and so recvfrom() is said to “block”
    // (that is, sleep there) until some data arrives.

    // Lots of functions block. accept() blocks. All the recv() functions block.
    // The reason they can do this is because they’re allowed to.
    // When you first create the socket descriptor with socket(),
    // the kernel sets it to blocking. If you don’t want a socket to be blocking,
    // you have to make a call to fcntl() "

    if (fcntl(this->socket_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "ERROR: failed to set NONBLOCK for socket fd.\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }

    // "Normally, recv() on a blocking socket blocks — your program just sits there frozen until data arrives.
    // Once you set O_NONBLOCK, recv() (or read()) refuses to wait.
    // If there's no data, it returns immediately with -1,
    // and sets errno to EAGAIN or EWOULDBLOCK (which one depends on the OS/libc)"

    if (bind(this->socket_fd, serverinfo->ai_addr, serverinfo->ai_addrlen) ==  -1)
    {
        std::cout << "ERROR: bind the socket failed. " << "\n";
        freeaddrinfo(serverinfo);
        return EXIT_FAILURE;
    }
    freeaddrinfo(serverinfo);
    return EXIT_SUCCESS;
}

void server::closeAllFds()
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        std::cout << "Client (" << clients[i].getFD() << ") Disconnected\n";
        close(clients[i].getFD());
    }
    if (this->socket_fd != -1)
    {
        std::cout << "Server (" << this->socket_fd << ") Disconnected\n";
        close(this->socket_fd);
    }
}


void server::removeClient(int fd)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i].getFD() == fd)
            clients.erase(clients.begin() + i);
    }
}


void server::removeFd(int fd)
{
    for (size_t i = 0;i < pollfds.size(); i++)
    {
        if (pollfds[i].fd == fd)
            pollfds.erase(pollfds.begin() + i);
    }
}



int server::listen_and_monitorfdstatus()
{
    // second arg io listen It's the size of a small holding queue for connections that have finished
    // the TCP handshake but that your program hasn't called accept() on yet using sockaddr_storage.
    // The moment you accept() one,
    // it leaves that queue — it doesn't count against the limit anymore.
    /*
        * Maximum queue length specifiable by listen.
        #define SOMAXCONN       128
    */
    if (listen(this->socket_fd, SOMAXCONN) == -1)
    {
        std::cout << "ERROR: Failed to start listening for incoming connections.\n";
        return EXIT_FAILURE;
    }
    // may be i need handel signals here

    struct pollfd Newpollfd;

    Newpollfd.fd = this->socket_fd;
    Newpollfd.events = POLLIN;
    Newpollfd.revents = 0; //i can use it or not ??

    this->pollfds.push_back(Newpollfd);

    std::cout << "server: waiting for connections...\n";
    while (1)
    {
        if (poll(&pollfds[0], pollfds.size(), -1) == -1)
        {
            std::cout << "pool failed";
            return EXIT_FAILURE;
        }
        this->procces_connections();
    }
    closeAllFds();
    return EXIT_SUCCESS;
}



int server::procces_connections()
{
    for (size_t i = 0; i < pollfds.size(); i++)
    {
        if (pollfds[i].revents & POLLIN)
        {
            // std::cout << "under poll revent\n";
            if (pollfds[i].fd == this->socket_fd)
                this->acceptNewClient();
            else
                this->handelNewData(pollfds[i].fd);
        }
    }
    return EXIT_SUCCESS;
}

int server::acceptNewClient()
{
    struct sockaddr_storage newCli_inf;
    client newClient;
    socklen_t NewCli_sockaddr_size;

    NewCli_sockaddr_size = sizeof(newCli_inf);
    this->client_fd = accept(this->socket_fd, (struct sockaddr*)&newCli_inf, &NewCli_sockaddr_size);

    if (this->client_fd == -1)
    {
        std::cout << "ERROR: accept clinet socket failed.\n";
        return EXIT_FAILURE;
    }

    if (fcntl(this->client_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "ERROR: failed to set NONBLOCK for socket fd.\n";
        close(this->client_fd);
        return EXIT_FAILURE;
    }

    struct pollfd temp_pfd;
    temp_pfd.fd = this->client_fd;
    temp_pfd.events = POLLIN;
    temp_pfd.revents = 0;

    struct sockaddr_in *ipv4 = (struct sockaddr_in*)&newCli_inf;
    newClient.setFD(this->client_fd);
    // std::cout << newClien << "\n";
    newClient.setIpAdd(inet_ntoa(ipv4->sin_addr));
    std::cout << newClient.getIpAdd() << "\n";

    this->clients.push_back(newClient);
    this->pollfds.push_back(temp_pfd);
	std::cout << "Client <" << this->client_fd << "> Connected\n";

    return EXIT_SUCCESS;
}

client *server::getClient(int fd)
{
    for (size_t i = 0; i < this->clients.size(); i++)
    {
        if (this->clients[i].getFD() == fd)
            return &clients[i];
    }
    return NULL;
}

std::vector<std::string> server::split_recved_buffer(std::string buff)
{
    std::istringstream text(buff);
    std::string token;
    std::vector<std::string> cmds;
    while (std::getline(text, token)) // \n\r
    {
        size_t cpos = token.find_first_of("\n\r");
        if (cpos != std::string::npos)
            token = token.substr(0, cpos);
        cmds.push_back(token);
    }
    return cmds;
}

std::vector<std::string> server::splited_cmd(std::string& cmd)
{
    std::istringstream msg(cmd);
    std::vector<std::string> vec;
    std::string word;
    while (msg >> word)
    {
        vec.push_back(word);
    }
    return vec;
}

void server::parse_and_exe(client *curClient, std::vector<std::string> splited_cmd)
{
	std::string Command;

	Command = splited_cmd[0];
	if(!Command.compare("JOIN"))
		server::handleJoin(curClient, splited_cmd);
	if(!Command.compare("TOPIC"))
		server::handleTopic(curClient, splited_cmd);
};

void server::handleJoin(client * curr_client, std::vector<std::string> & command)
{
	std::string response;
	if(command.size() < 2)
	{
		response = ":ircserv " + curr_client->getUserName() + " " + command[0] + " :Not enough parameters 461";
		send(curr_client->getFD(), response.c_str(), response.length(), 0);
		return;
	}
	Channel targetChannel(command[1]);
	if(!validateChannelName(command[1]))
	{
		std::cout << command[1] << " is not a valid Channel Name" << std::endl;
		response = ":ircserv " + curr_client->getUserName() + " " + command[0] + " :Bad Channel Mask 476";
		send(curr_client->getFD(), response.c_str(), response.length(), 0);
		return;
	}
	std::map<std::string, Channel>::iterator it;
	it = this->Channels.find(command[1]);
	if(it == this->Channels.end())
	{
		std::cout << "creating channel working" << std::endl;
		this->Channels.insert(std::make_pair(command[1], targetChannel));
		// this->Channels[command[1]].addMember(curr_client);
		for (std::map<std::string, Channel>::iterator it = this->Channels.begin(); it != this->Channels.end(); it++)
		{
			if (it->first == command[1])
			{
				it->second.addMember(curr_client);
				std::cout << std::boolalpha << it->second.isOperator(*curr_client) << std::endl;
			}
		}
	}
	else{
		std::cout << "flavor text" <<  std::flush;
	}
	// std::vector<client *> members =this->Channels[command[1]].getMembers();
	// unsigned long i =0;
	// while (i < members.size())
	// {
	// 	std::cout << members[i]->getUserName() << std::endl;
	// 	i++;
	// }

}

void server::handleTopic(client * curr_client, std::vector<std::string> & command)
{
	std::string response;
	if(command.size() < 2)
	{
		response = ":ircserv " + curr_client->getUserName() + " " + command[0] + " :Not enough parameters 461";
		send(curr_client->getFD(), response.c_str(), response.length(), 0);
		return;
	}
	std::map<std::string, Channel>::iterator it = this->Channels.find(command[1]);
	if(it == this->Channels.end())
	{
		response = curr_client->getUserName() + " " + command[1] + " :No such channel 403";
		send(curr_client->getFD(), response.c_str(), response.length(), 0);
		return;
	}
	if(!it->second.isMember(*curr_client))
	{
		response = curr_client->getUserName() + " " + command[1] + " :You're not on that channel 442";
		send(curr_client->getFD(), response.c_str(), response.length(), 0);
		return;
	}
	if(it->second.getTopic().empty())
	{
		response = curr_client->getUserName() + " " + command[1] + " :No topic is set 331";
		send(curr_client->getFD(), response.c_str(), response.length(), 0);
		return;
	}
	response = curr_client->getUserName() + " " + command[1] + " :" + it->second.getTopic();
	send(curr_client->getFD(), response.c_str(), response.length(), 0);

}


int server::handelNewData(int cliFd)
{
    char buffer[1024];
    std::memset(buffer, 0, 1024);
    int bytes = recv(cliFd, buffer, sizeof(buffer) -1, 0);
    std::vector<std::string> msg;
    client *currClient = getClient(cliFd);
    if (bytes <= 0)
    {
        if (bytes == -1)
        {
            std::cout << "no messages are available at the socket (maybe ctrlc or ctr..)\n";
            return EXIT_SUCCESS;
        }
        std::cout << "he peer has performed an orderly shutdown.\n";
        removeClient(cliFd);
        removeFd(cliFd);
        close(cliFd);
    }
    else
    {
        std::cout << "msg recved\n";
        currClient->clientSetBuff(split_recved_buffer(buffer));
        msg = currClient->clientGetBuff();
        for (size_t i =0; i < msg.size(); i++)
        {
            if (!msg[i].empty())
                parse_and_exe(currClient, splited_cmd(msg[i]));
        }
    }
    return EXIT_SUCCESS;
}

server::~server(){};
