
#include <string>


class client
{
    private:
        int fd;
        std::string nickname;
        std::string IpAdd;
        std::string buffer;
        std::string password;
        // bool authentication;
    public:
        client();

        void setFD(int FD);
        void setIpAdd(std::string CIpAdd);
    
        int getFD();
    
        ~client();
};