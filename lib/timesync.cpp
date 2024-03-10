#include "timesync.h"
#include "misc.h"
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
using namespace std;
using namespace std::chrono;


#define SYNC_INTERVAL 60   // seconds
#define SYNC_TIMEOUT (3*SYNC_INTERVAL) // seconds - how long before we refresh time server's address if there is no reply

long long TimeSync::delta; // static


TimeSync::TimeSync(IP ip, unsigned short port):
    lastLocal(std::chrono::high_resolution_clock::now() - chrono::seconds(SYNC_TIMEOUT) ), lastReply(lastLocal) {
    trackerAddr.sin_family = AF_INET;
    trackerAddr.sin_port = port;
    trackerAddr.sin_addr.s_addr = ip;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 0; // htons(TIMESERV_PORT);
    serverAddr.sin_addr.s_addr = 0;
}


void TimeSync::calculate(const Time& t0, long long remoteUs){
//    long long rcved = duration_cast<std::chrono::microseconds>(t0.time_since_epoch()).count();
    long long sent = duration_cast<std::chrono::microseconds>(lastLocal.time_since_epoch()).count();
    long long trip = duration_cast<std::chrono::microseconds>(t0-lastLocal).count(); //rcved - sent;
    long long diff = (remoteUs - sent) - trip/2;
    
    long long deltat = __atomic_load_n(&delta, __ATOMIC_SEQ_CST);
    cout << "  TimeSync trip=" << trip << " diff=" << diff << " delta0=" << deltat;
    deltat += (diff-deltat)/3; // slowly approach
    __atomic_store_n(&delta, deltat, __ATOMIC_SEQ_CST);
    cout << " delta=" << deltat << endl;
}


void TimeSync::sync(){
    const string getTimeServCmd("GET "+ std::to_string(TIME_SERVER_ID)); // GET time server address
    char buff[0xFF];

    while(true){
        int size = udp.ReadSelect(buff, sizeof(buff)-1, 1000*1000);
        Time now = std::chrono::high_resolution_clock::now();

        if(size > 0){
            buff[size]=0; // in case remote did not send a null
//	    cout << "  TimeSync incoming command: " << buff << endl;

	    // TODO: try sscanf() because it's faster.  scan command and first number right away (time or ip)
	    istringstream rs(buff);
            string cmd;
            rs >> cmd;

            if("T" == cmd){ // process first because timing is important
	        long long remote;
	        rs >> remote;
	        calculate(now, remote);
	        lastReply = now;
            } else if("POINT"==cmd){
                ClusterID retCid;
                rs >> retCid;
                if(TIME_SERVER_ID == retCid){
		    IP ip;
                    rs >> ip; // if tracker sent us 127.0.0.1 use tracker's address (they are running on one server)
		    serverAddr.sin_addr.s_addr = replaceLo(ip, trackerAddr.sin_addr.s_addr);
		    rs >> serverAddr.sin_port;
                }
            } else if("DROP"==cmd){
		serverAddr.sin_addr.s_addr = 0; // reset address
		serverAddr.sin_port = 0; // reset address
	    }
        } else if( now > lastReply + std::chrono::seconds(SYNC_TIMEOUT) ){
            // have not received a server address from the tracker yet? or did the sync timeout?
	    cout << "  TimeSync requesting time server's address from tracker." << endl;
            udp.Write(getTimeServCmd.c_str(), getTimeServCmd.length()+1, trackerAddr);
	    lastReply = now;
        } else if(serverAddr.sin_port && now > lastLocal + std::chrono::seconds(SYNC_INTERVAL) ){ // is address valid?
	    cout << "  TimeSync requesting time from time server." << endl; 
	    string cmd ("TIME");
	    lastLocal = std::chrono::high_resolution_clock::now();
            udp.Write(cmd.c_str(), cmd.length()+1, serverAddr);
        }
    } // while(true)
}
