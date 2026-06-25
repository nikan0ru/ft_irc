
#include <string>
#include <vector>


class client
{
    private:
        int fd;
        std::string nickname;
        std::string IpAdd;
        std::vector<std::string> buffer;
        std::string password;
        // bool authentication;
    public:
        client();
        void clientSetBuff(std::vector<std::string> buff);
        void setFD(int FD);
        void setIpAdd(std::string CIpAdd);
        int getFD();
    
        ~client();
};