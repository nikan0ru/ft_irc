#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>
#include <vector>
#include <set>

class client
{
    private:
        int fd;
        std::string nickName;
		std::string userName;
        std::string IpAdd;
        std::vector<std::string> buffer;
		std::set<std::string> channelsJoined;
        std::string password;
        bool authentication;
        bool passOk;
        bool hasNick;
        bool hasUser;
    public:
        client();
        void clientSetBuff(std::vector<std::string> buff);
    	const std::string &getUserName() const;
    	const std::string &getNickName() const;
        std::vector<std::string> clientGetBuff();
        void setFD(int FD);
        void setIpAdd(std::string CIpAdd);
        void setAsAuthenticated();
        void setPassStatusFalse();
        void setNickName(std::string nickName);
        void setUserName(std::string userName);
        std::string getIpAdd();
        int getFD();
        bool isAuthenticat();
        void setAuthenRequirment(int id);
        bool checkAuthenRequirment();

        ~client();
};
#endif
