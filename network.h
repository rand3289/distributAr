#ifndef INCLUDED_NETWORK_H
#define INCLUDED_NETWORK_H

#include "main.h"
#include "udp.h"
#include "timebuff.h"
#include "spinlock.h"
#include <unordered_map>
#include <vector>
#include <memory>

class Server;

struct TimeDefault: public Time {
    TimeDefault(): Time(std::chrono::high_resolution_clock::time_point::min()) {}
};


class Multicast {
protected:
    const static IP baseMulticastIP = 0xE0000100; // 224.0.0.0 - 239.255.255.255 // must be converted to network byte order
    Udp multicast;
    int read(TimeBuffer& tb) {
        return multicast.ReadSelect( (char*)&tb, sizeof(TimeBuffer), 500); // sleep a few microseconds. TODO: ajust this value
    }
public:
    Multicast(): multicast(MULTICAST_PORT, true) {}
    static IP idToMulticast(ClusterID id){ return htonl(baseMulticastIP+id); }
    static ClusterID multicastToId(IP multiGroup){ return ntohl(multiGroup)- baseMulticastIP; }
};


class Network: public Multicast {
    SpinLock spinLock; // would mutex used with std::shared_lock provide better performance?
    bool dirty = false; // flag: subscriptions were updates since getSubscriptions() was called
    std::unordered_map<ClusterID, std::vector<ClusterID> > subscriptions;
    std::unordered_map<ClusterID, TimeDefault> sequences;
    std::vector<std::shared_ptr<Server> >& servers; // list of local clusters to figure out subscriptions
public:
    Network(std::vector<std::shared_ptr<Server> >& serverS): servers(serverS) { }
    int readVerify(TimeBuffer& tb);

    void subscribe(ClusterID local, ClusterID remote); // subscribe to a multicast group
    void unsubscribe(ClusterID local, ClusterID remote); // remove or mark multicast subscription for delete
    void getSubscriptions(std::unordered_map<ClusterID, std::vector<Server*> >& subscribers);
};


#endif // INCLUDED_NETWORK_H
