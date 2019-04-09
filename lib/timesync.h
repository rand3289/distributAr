#ifndef INCLUDED_TIMESYNC_H
#define INCLUDED_TIMESYNC_H

#include "main.h" // Time
#include "udp.h"


// synchronize time with a custom UPD server
// server has to respond to "TIME" command with "T timestamp"
// IP:port of the server is queried from a tracker whose address is given in constructor
class TimeSync {
    static long long delta; // time difference in microseconds - use ___atomic_zzz to load/store
    Time lastLocal;
    Time lastReply;
    sockaddr_in trackerAddr;
    sockaddr_in serverAddr;
    Udp udp;
    void calculate(const Time& t0, long long remoteUs);
public:
    TimeSync(IP trackerIP, unsigned short trackerPort);
    void sync();

    static inline Time toNetwork(const Time& localTime  ){ // TODO: use std::atomic<> instead???
	long long dt = __atomic_load_n(&delta, __ATOMIC_SEQ_CST); // TODO: change memory order to more relaxed
	return localTime - std::chrono::microseconds(dt);
    }
    
    static inline Time toLocal(const Time& networkTime){
	long long dt = __atomic_load_n(&delta, __ATOMIC_SEQ_CST); // TODO: change memory order to more relaxed
	return networkTime + std::chrono::microseconds(dt);
    }
};


#endif // INCLUDED_TIMESYNC_H
