#ifndef INCLUDED_UDP_H
#define INCLUDED_UDP_H


#ifdef WIN32
    #include <winsock2.h>
    #define INIT_NETWORK() WSADATA wsaData; WSAStartup(0x0202, &wsaData);
#else // linux?
    #include <netinet/in.h>
    #include <netdb.h> // gethostbyname()
    #define INIT_NETWORK()
    typedef int SOCKET;
#endif

#define DO_NOT_BLOCK 0
#define ANY_PORT 0
typedef unsigned long IP; // sockaddr_in::sin_addr.s_addr defines it as "unsigned long"


class Udp {
    SOCKET udpSocket;
public:
    Udp(unsigned short port = ANY_PORT, bool multicastRX = false);
    ~Udp();
    unsigned short GetNetworkAddress(IP& ip) const;
    int ReadSelect(char* buffer, int size, sockaddr_in* from, long timeoutMicroseconds = DO_NOT_BLOCK) const;
    inline int ReadSelect(char* buffer, int size, long timeoutMicroseconds = DO_NOT_BLOCK) const { return ReadSelect(buffer, size, 0, timeoutMicroseconds); }
    int Read(char* buffer, int size, sockaddr_in* from = 0) const;
    int Write(const char* buffer, int size, const sockaddr_in& addr) const;
    int JoinMulticast(IP mcastIp);
    int LeaveMulticast(IP mcastIp);
};


#endif // INCLUDED_UDP_H
