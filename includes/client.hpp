#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <string>
#include <vector>
#include <set>

class client
{
    private:
        int fd;
        std::string nickname;
		std::string userName;
        std::string IpAdd;
        std::vector<std::string> buffer;
		std::set<std::string> channelsJoined;
        std::string password;
        // bool authentication;
    public:
        client();
        void clientSetBuff(std::vector<std::string> buff);
        void setFD(int FD);
        void setIpAdd(std::string CIpAdd);
        int getFD();

        ~client();
		const std::string &getUserName()
		{
			return this->userName;
		}
		std::vector<std::string> &getBuffer()
		{
			return this->buffer;
		}
};
#endif
