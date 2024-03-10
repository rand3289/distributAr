#include "udp.h"
#include <string>
#include <iostream>
using namespace std;


#ifdef WIN32
    #pragma comment(lib, "Ws2_32.lib") // link to winsock2 library
    #define close(s) closesocket(s)
    #define UNREACHABLE WSAECONNRESET
    #undef errno // do not use stdlib version here
    #define errno WSAGetLastError()
    typedef int socklen_t;
//    #include <winsock2.h>
//    #define INIT_NETWORK() WSADATA wsaData; WSAStartup(0x0202, &wsaData);
#else // linux?
    #include <unistd.h> // close()
    #include <sys/types.h>
    #include <sys/socket.h>
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define UNREACHABLE (ECONNREFUSED)
//    #define INIT_NETWORK()
//    #include <netinet/in.h>
//    #include <netdb.h> // gethostbyname()
//    typedef int SOCKET;
#endif


#include <fcntl.h> // TODO: make this portable

Udp::Udp(unsigned short port, bool multicastRX): udpSocket(INVALID_SOCKET) {
    INIT_NETWORK();
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if( INVALID_SOCKET == udpSocket ){
            throw( string("socket error in socket(). errno: ") + to_string(errno) );
    }

    int flags = fcntl(udpSocket, F_GETFL, 0);
    if(SOCKET_ERROR == fcntl(udpSocket, F_SETFL, flags | O_NONBLOCK) ){
        throw( string("socket error in fcntl(). errno: ") + to_string(errno) );
    }

    if(multicastRX){
        int reuse = 1;
        if(setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse, sizeof(reuse)) <0){
        close(udpSocket);
            udpSocket = INVALID_SOCKET;
        throw( string("socket error in setsockopt().  can not set SO_REUSEADDR") );
        }
    }

    sockaddr_in addr;
        addr.sin_family = AF_INET;
    addr.sin_addr.s_addr= INADDR_ANY;
        addr.sin_port = htons(port);

    if(SOCKET_ERROR == ::bind(udpSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr) ) ) {
        close(udpSocket);
            udpSocket = INVALID_SOCKET;
        throw( string("socket error in bind().  port: ") + to_string(port) +  ". errno: " + to_string(errno) );
    }
}


Udp::~Udp(){
    close(udpSocket);
}


unsigned short Udp::GetNetworkAddress(IP& ip) const {
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (SOCKET_ERROR == getsockname(udpSocket, (sockaddr*) &sin, &len) ){
        throw( string("socket error in getsockname(). errno: ") + to_string(errno) );
    }
    ip = sin.sin_addr.s_addr;

    if(INADDR_ANY == ip) {  // if socket is bound to ANY address
        char hostname[128]; // find out local host name and it's IP address
        if(SOCKET_ERROR != gethostname(hostname, sizeof(hostname))){
            struct hostent* he = gethostbyname(hostname);
            if(he){
                ip = *reinterpret_cast<IP*>(he->h_addr_list[0]);
            } else {
                throw string("socket error in gethostname(): returned 0");
            }
        }
    }

    return sin.sin_port;
}


int Udp::Write(const char* buffer, int size, const sockaddr_in& addr) const {
    // TODO: assume O_NONBLOCK is set and we need to check for errors & resend
    int ret = 0;
    do {
        ret = sendto(udpSocket, buffer, size, 0, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr) );
    } while(SOCKET_ERROR == ret && (EAGAIN == errno || EWOULDBLOCK == errno) ); // wait for write to be come available

    if(SOCKET_ERROR == ret) {
        cerr << "socket error in sendto(). errono: " << errno << endl;
    return 0;
    }
    return ret;
}



int Udp::Read(char* buffer, int size, sockaddr_in* from) const{ // without select
    unsigned int structSize = from ? sizeof(sockaddr_in) : 0;
    int ret = recvfrom(udpSocket, buffer, size, 0, reinterpret_cast<sockaddr*>(from), &structSize); // MSG_DONTWAIT ???
    if(SOCKET_ERROR != ret){ return ret; }

    if(EWOULDBLOCK == errno || EAGAIN == errno){ // no data available
        return 0;
    } else if(UNREACHABLE == errno){ // read receives write errors
    return -1;
    }

    cerr << "socket error in recvfrom(). errno: " << to_string(errno) << endl;
    return -2;
}


int Udp::ReadSelect(char* buffer, int size, sockaddr_in* from, long timeoutMicroseconds) const {
    struct timeval tv;
    tv.tv_sec  = timeoutMicroseconds / 1000000;
    tv.tv_usec = timeoutMicroseconds % 1000000;

    fd_set fdr,fde;    
    FD_ZERO(&fdr);
    FD_SET(udpSocket, &fdr);
    FD_ZERO(&fde);
    FD_SET(udpSocket, &fde);

    int ret = select(udpSocket+1, &fdr, NULL, &fde, &tv);
    if( 0 == ret ){ // timeout
            return 0;
    } else if (SOCKET_ERROR == ret){
        cerr << "socket error in select(). errno: " << to_string(errno) << endl;
        return -2;
    }

    unsigned int structSize = from ? sizeof(sockaddr_in) : 0;
    ret = recvfrom(udpSocket, buffer, size, 0, reinterpret_cast<sockaddr*>(from), &structSize);

        if(SOCKET_ERROR == ret){
        if(UNREACHABLE == errno){ // read receives write errors
        return -1;
        }
            cerr << "socket error in recvfrom(). errno: " << to_string(errno) << endl;
        return -3;
        }
    return ret;
}


int Udp::JoinMulticast(IP mcastIp){
    // set IP_MULTICAST_IF ???
    ip_mreq group;
    group.imr_multiaddr.s_addr = mcastIp;
    group.imr_interface.s_addr = INADDR_ANY;
    if( setsockopt(udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)& group, sizeof(group) ) < 0 ){
    cerr << "socket error in setsockopt(). errno: " << to_string(errno) << "\n did you run 'route add -net 224.0.0.0 netmask 224.0.0.0 <interface>' ?" << endl;
    return -1;
    }
    return 0;
}


int Udp::LeaveMulticast(IP mcastIp){
    // set IP_MULTICAST_IF ???
    ip_mreq group;
    group.imr_multiaddr.s_addr = mcastIp;
    group.imr_interface.s_addr = INADDR_ANY;
    if( setsockopt(udpSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)& group, sizeof(group) ) < 0 ){
    cerr << "socket error in setsockopt(). errno: " << to_string(errno) << endl;
    return -1;
    }
    return 0;

}
