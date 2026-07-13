#include <sys/socket.h> // for socket()
#include <netinet/in.h> // for struct sockaddr_in
#include <netdb.h> // for getprotobyname
#include <unistd.h> // Required for gethostname
#include <cstring>
#include <vector>
#include <map>
#include <poll.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include "client.hpp"
#include "channel.hpp"

class server
{
    private:
        int socket_fd;
        int reuse_flag;
        const std::string& servport;
        const std::string& servpass;
        int client_fd;
        std::vector<struct pollfd> pollfds;
        std::vector<client> clients;
		    std::map<std::string, Channel> Channels;
    public:
        server(const std::string& portnum, const std::string& authpass);
        ~server();
        int creat_sokect();
        client *getClient(int fd);
        int listen_and_monitorfdstatus();
        int procces_connections();
        int acceptNewClient();
        int handelNewData(int cliFd);
      	void parse_and_exe(client *curClient, std::vector<std::string> splited_cmd);
        std::vector<std::string> split_recved_buffer(std::string buff);
        std::vector<std::string> splited_cmd(std::string& cmd);
        void closeAllFds();
        void removeFd(int fd);
        void removeClient(int fd);
        void handleAuthentication(client* curr_client, std::vector<std::string>& cmd);
        void handlePrivmsg(client* curr_client, std::vector<std::string>& cmd);
        void handleInvite(client* curr_client, std::vector<std::string>& cmd);
        bool isValidNickName(std::string& nickName);
	  	void handleJoin(client * curr_client, std::vector<std::string> & command);
	  	void handleSingleJoin(client * curr_client, std::string & channelName, std::string &channelKey);
		void handleTopic(client * curr_client, std::vector<std::string> & command);
		void manageTopic(client * curr_client, std::vector<std::string> & command);
		void broadcastNamesList(client * currentClient,std::string &command, std::map<std::string, Channel>::iterator &it);
		bool checkChannelModes(client *currentClient, std::string &channelName, std::string &channelKey, std::map<std::string, Channel>::iterator &it);
		void handleMode(client * currentClient, std::vector<std::string> &command);
		bool handleSingleMode(client *currentClient,char mode, short addOrRemove, std::map<std::string, Channel>::iterator it, std::string parameter, std::string channelName);

};
void sendErrorMessage(client * currentClient, std::string command, std::string message,std::string errCode);
