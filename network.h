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
class Subscription;

struct TimeDefault: public Time {
    TimeDefault(): Time(std::chrono::high_resolution_clock::time_point::min()) {}
};


class Network {
    const static IP baseMulticastIP = 0xE0000100; // 224.0.0.0 - 239.255.255.255 // must be converted to network byte order
    Udp multicast;
    SpinLock spinLock; // would mutex used with std::shared_lock provide better performance?
    bool dirty = false; // subscriptions were updates since getSubscriptions() was called
    std::unordered_map<ClusterID, std::vector<std::shared_ptr<Subscription> > > subscriptions;
    std::unordered_map<ClusterID, TimeDefault> sequences;
    std::vector<std::shared_ptr<Server> >& servers; // list of local clusters to figure out subscriptions
    int verifyPacket(TimeBuffer& bb);
public:
    Network(std::vector<std::shared_ptr<Server> >& serverS): multicast(MULTICAST_PORT, true), servers(serverS) { }

    inline static IP idToMulticast(ClusterID id){ return htonl(baseMulticastIP+id); }
    inline static ClusterID multicastToId(IP multiGroup){ return ntohl(multiGroup)- baseMulticastIP; }

    void subscribe(ClusterID local, ClusterID remote); // subscribe to a multicast group
    void unsubscribe(ClusterID remote); // removes multicast subscription
    void unsubscribe(ClusterID local, const ClusterID remote); // mark multicast subscription for delete
    void getSubscriptions(std::unordered_map<ClusterID, std::vector<Server*> >& subscribers);

    inline bool read(TimeBuffer& tb){
        int ret = multicast.ReadSelect( (char*)&tb, sizeof(TimeBuffer), 500); // sleep a few microseconds. TODO: ajust this value
        return ret && verifyPacket(tb);
    }
};


class Subscription {
    Network& network;
    ClusterID cluster;
public:
    Subscription(Network& net, ClusterID id): network(net), cluster(id) {}
    ~Subscription(){ network.unsubscribe(cluster); }
    ClusterID getClusterID(){ return cluster; }
};


#endif // INCLUDED_NETWORK_H
