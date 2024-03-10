#include "main.h"
#include "udp.h"
#include <string.h> // strlen()
#include <stdio.h>  // sprintf()
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;


// server responds to "TIME" command with "T timestamp"
class TimeServ {
    Udp udp;
    sockaddr_in trackerAddr;
public:
    TimeServ(IP ip, unsigned short port){
        trackerAddr.sin_family = AF_INET;
        trackerAddr.sin_port = port;
        trackerAddr.sin_addr.s_addr = ip;
    }
    void run();
};


void TimeServ::run(){
    char buff[32];
    sockaddr_in from;
    const string cmdID("ID " + std::to_string(TIME_SERVER_ID) ); // tracker registration command
    const auto updateInterval = std::chrono::seconds(180);
    Time nextTrackerUpdate = std::chrono::high_resolution_clock::now();

    IP tsip;
    cout << "Running time server on port " << udp.GetNetworkAddress(tsip) << endl;

    while(true){
        int ret = udp.ReadSelect( reinterpret_cast<char*>(buff), 1, &from, 30*1000*1000); // read 1 byte of the incoming command (faster)
    Time t = std::chrono::high_resolution_clock::now();
        if(ret>0){ // don't waste time parsing the command however make sure it did not timeout
        unsigned long long int timeUs = duration_cast<std::chrono::microseconds>(t.time_since_epoch()).count();
        sprintf( buff,"T %llu", timeUs); // faster than using std::string
            udp.Write(buff, strlen(buff)+1, from); // +1 for null
        cout << "TimeServ " << buff << endl;
        }
    if(t > nextTrackerUpdate) { // register id with tracker (request id)
        cout << "Sending my ID to tracker" << endl;
            udp.Write(cmdID.c_str(), cmdID.length()+1, trackerAddr);
        nextTrackerUpdate = t+updateInterval;
    }
    }
}


int main(int argc, char* argv[]){
    if(argc<3){
            cout << "distributAr (distributed Architecture framework) time server" << endl;
            cout << "usage: " << argv[0] << " <tracker addr/ip>  <tracker port#>" << endl;
            return 0;
    }

    struct hostent* he = gethostbyname(argv[1]); // tracker's IP
    if(!he){
            cerr << "Can not resolve address for " << argv[1] << endl;
        return 1;
    }
    IP ip = *reinterpret_cast<IP*>(he->h_addr_list[0]);

    unsigned short port = atoi(argv[2]); // tracker's port
    port = htons(port);

    TimeServ ts(ip, port);
    ts.run();
}
