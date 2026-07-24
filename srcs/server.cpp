
#include "../includes/server.hpp"

bool g_running = true;

void signalHandler(int signum)
{
    (void)signum;
    g_running = false;
}

std::string normalize(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		str[i] = std::tolower(str[i]);
		if (str[i] == '[')
			str[i] = '{';
		if (str[i] == ']')
			str[i] = '}';
		if (str[i] == '\\')
			str[i] = '|';
	}
	return str;
}

server::server(const std::string& portnum, const std::string& authpass):socket_fd(-1), reuse_flag(1),
                                                                        servport(portnum),servpass(authpass), client_fd(-1)

{
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
		{
            clients.erase(clients.begin() + i);
			break;
		}
    }
	for(std::map<std::string, Channel>::iterator it = this->Channels.begin(); it != this->Channels.end();)
	{
		if(it->second.isMember(fd))
			it->second.removeMember(fd);
		if(it->second.isOperator(fd))
			it->second.removeOperator(fd);
		if(it->second.isInvited(fd))
			it->second.removeInvite(fd);
		if(it->second.getMembers().empty())
			this->Channels.erase(it++);
		else
			it++;
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
    // second arg in listen It's the size of a small holding queue for connections that have finished
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
    // may be i need to handel signals here

    struct pollfd Newpollfd;

    Newpollfd.fd = this->socket_fd;
    Newpollfd.events = POLLIN;
    Newpollfd.revents = 0; //i can use it or not ??

    this->pollfds.push_back(Newpollfd);
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGPIPE, SIG_IGN);
    std::cout << "server: waiting for connections...\n";
    while (g_running)
    {
        if (poll(&pollfds[0], pollfds.size(), -1) == -1)
        {
            if (errno == EINTR)
                break;
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
    newClient.setIpAdd(inet_ntoa(ipv4->sin_addr));

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
    while (std::getline(text, token))
    {
        size_t cpos = token.find_first_of("\r\n");
        if (cpos != std::string::npos)
            token = token.substr(0, cpos);
		std::cout << token ;
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
        if (word[0] == ':')
        {
            std::string lastword;
            word = word.substr(1);
            std::getline(msg, lastword);
            if (lastword.empty())
                vec.push_back(word);
            else
                vec.push_back(word + lastword);
            break;
        }
        vec.push_back(word);
    }
    return vec;
}

void server::parse_and_exe(client *curClient, std::vector<std::string> splited_cmd)
{
	std::string Command;
	if(splited_cmd.empty())
		return;
	splited_cmd[0] = normalize(splited_cmd[0]);
	Command = splited_cmd[0];
    if((!Command.compare("user") || !Command.compare("nick") || !Command.compare("pass")))
        handleAuthentication(curClient, splited_cmd);
	else if(!Command.compare("join"))
		handleJoin(curClient, splited_cmd);
	else if(!Command.compare("topic"))
		handleTopic(curClient, splited_cmd);
	else if(!Command.compare("mode"))
		handleMode(curClient, splited_cmd);
  else if(!Command.compare("privmsg"))
		handlePrivmsg(curClient, splited_cmd);
  else if(!Command.compare("invite"))
		handleInvite(curClient, splited_cmd);
	else if(!Command.compare("kick"))
		handleKick(curClient, splited_cmd);
	else
		sendErrorMessage(curClient, Command, " :Unknown command", "421");

};

bool server::isValidNickName(std::string& nickName)
{
    size_t nicksize = nickName.size();
    if (isdigit(nickName[0]) || nickName[0] == '&' || nickName[0] == '#' || nickName[0] == ':' || nicksize > 9)
                    return false;
    for (size_t i = 0; i < nicksize; i++)
    {
        if (!isdigit(nickName[i]) && !isalpha(nickName[i]) && nickName[i] != '[' && nickName[i] != ']' && nickName[i] != '{'
                    && nickName[i] != '}' && nickName[i] != '\\' && nickName[i] != '`' && nickName[i] != '^' && nickName[i] != '-' && nickName[i] != '|')
                    return false;
    }
    return true;
}

void server::handleAuthentication(client* curr_client, std::vector<std::string>& cmd)
{
	std::string Command;

	Command = cmd[0];
    int cmdsize = cmd.size();
    if (!Command.compare("pass"))
    {
        if (curr_client->isAuthenticat())
            return (sendErrorMessage(curr_client, "PASS", " :You may not reregister", "462"), void());
        if (cmdsize < 2)
            return (sendErrorMessage(curr_client, "PASS", " :Not enough parameters", "461"), void()); // what if the pasword contiene spaces
        if (cmd[1].compare(this->servpass))
            return (sendErrorMessage(curr_client, "PASS", " :Password incorrect", "464"), curr_client->setPassStatusFalse(), void());
        curr_client->setAuthenRequirment(1);
    }
    else if (!Command.compare("nick") && curr_client->checkAuthenRequirment(1) == true)
    {
        if (cmdsize < 2 || cmd[1].empty())
            return (sendErrorMessage(curr_client, "NICK", " :No nickname given", "431"), void());
        if (!isValidNickName(cmd[1]))
            return(sendErrorMessage(curr_client, "NICK", " :Erroneus nickname", "432"), void());
        for (size_t i = 0 ; i < this->clients.size(); i++)
        {
            if (normalize(clients[i].getNickName()) == normalize(cmd[1]) && clients[i].getFD() != curr_client->getFD())
                return (sendErrorMessage(curr_client, "NICK", " :Nickname is already in use", "433"), void());
        }
        curr_client->setNickName(cmd[1]);
        curr_client->setAuthenRequirment(2);
    }
    else if (curr_client->checkAuthenRequirment(2) == true)
    {
        if (curr_client->isAuthenticat() )
            return (sendErrorMessage(curr_client, "USER", " :You may not reregister", "462"), void());
        if (cmdsize < 5 || cmd[1].empty())
            return (sendErrorMessage(curr_client, "USER", " :Not enough parameters", "461"), void());
        curr_client->setUserName(cmd[1]);
        curr_client->setRealName(cmd[4]);
        curr_client->setAuthenRequirment(3);
    }

    std::string RPL_WELCOME = ":ircserv 001 "+curr_client->getNickName() \
    +" :Welcome to the Internet Relay Network " + curr_client->getClientName() + "\r\n";
    if (curr_client->checkAuthenRequirment(3) == true && curr_client->isAuthenticat() == false)
        return (curr_client->setAsAuthenticated(), 	send(curr_client->getFD(), RPL_WELCOME.c_str(), RPL_WELCOME.length(), 0), void());
    return;
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
		currClient->readBuffer += std::string(buffer, bytes);
		size_t pos;

        while ((pos = currClient->readBuffer.find('\n')) != std::string::npos)
        {
            std::string line = currClient->readBuffer.substr(0, pos);
            currClient->readBuffer.erase(0, pos + 1);

            if (!line.empty() && line[line.size() - 1] == '\r')
                line.erase(line.size() - 1);

            if (!line.empty())
                parse_and_exe(currClient, splited_cmd(line));
        }
    }
    return EXIT_SUCCESS;
}

server::~server(){};
