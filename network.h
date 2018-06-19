#ifndef INCLUDED_NETWORK_H
#define INCLUDED_NETWORK_H

#include "main.h"
#include "udp.h"
#include "timebuff.h"
#include "atomq.h"
#include <unordered_map>
#include <vector>
#include <memory>

class Server;
class Subscription;

struct TimeDefault: public Time {
    TimeDefault(): Time(std::chrono::high_resolution_clock::time_point::min()) {}
};

class Network {
    const static IP baseMulticastIP = 0xE0000100; // 224.0.0.0 - 239.255.255.255 // must be converted to network byte order
    Udp multicast;
    int* criticalSection = 0;
    std::unordered_map<ClusterID, std::weak_ptr<Subscription> > subscriptions;
//    std::unordered_map<ClusterID, Time> sequences;
    std::unordered_map<ClusterID, TimeDefault> sequences;
    std::vector<std::shared_ptr<Server> >& servers; // list of clusters to figure out subscriptions
//    unsigned int dropped = 0; // count of dropped out of sequence packets
    int verifyPacket(TimeBuffer& bb);
public:
    Network(std::vector<std::shared_ptr<Server> >& serverS): multicast(MULTICAST_PORT, true), servers(serverS) { }

    inline static IP idToMulticast(ClusterID id){ return htonl(baseMulticastIP+id); }
    inline static ClusterID multicastToId(IP multiGroup){ return ntohl(multiGroup)- baseMulticastIP; }

    // when all references to Subscription go out of scope, subscription to the multicast group is deleted
    bool isSubscribed(ClusterID local, ClusterID remote){ return true; } // TODO!!!
    std::shared_ptr<Subscription> subscribe(ClusterID local, ClusterID remote);
    void unsubscribe(ClusterID remote);

    inline bool read(TimeBuffer& tb){
        int ret = multicast.ReadSelect( (char*)&tb, sizeof(TimeBuffer), 500); // sleep a few microseconds. TODO: ajust this value
        return ret && verifyPacket(tb);
    }
};


class Subscription{
    Network& network;
    ClusterID cluster;
public:
    Subscription(Network& net, ClusterID id): network(net), cluster(id) {}
    ~Subscription(){ network.unsubscribe(cluster); }
    inline ClusterID whichCluster(){ return cluster; }
};


#endif // INCLUDED_NETWORK_H
