#ifndef INCLUDED_SERVER_H
#define INCLUDED_SERVER_H

#include "main.h"
#include "udp.h"
#include "queue.h"
#include "timebuff.h"
#include "icluster.h"
#include "misc.h"

#include <unordered_map>
#include <string>
#include <memory>

class Network;
class Subscription;

typedef FreeQueue<std::shared_ptr<TimeBuffer>, 256> BBQ; // size is time sensitive

class Server: public IWriter {
protected:
    BBQ inQ;   // incoming buffer queue
    BBQ outQ;  // outgoing buffer queue
    std::shared_ptr<TimeBuffer> timeBuff;
    Network& network;

    Udp udp;
    std::shared_ptr<ICluster> cluster;
    sockaddr_in tracker;
    sockaddr_in mCastGrp; // Multicast address - IP depends on ClusterID
    std::unordered_map<ClusterID, std::shared_ptr<Subscription> > subscriptions;

    char buff[0xFFFF]; // command buffer
    const unsigned int buffSize = sizeof(buff)-1; // leave space for NULL termination

    int write(int nodeIndex, const Time& time); // WritePulse() implementation
    std::string parseCommand(char* cmd);
    void processCommands();
    void requestIdFromTracker();
    void performIO();
public:
    Server(Network& net, std::string& dllName): network(net), cluster( loadDll(dllName) ) {
	timeBuff = std::make_shared<TimeBuffer>();
	if(cluster){
	    timeBuff->setSrcClusterId(cluster->getId()); // if 0, first packet will be lost
        }
    }
    virtual ~Server() {}
    BBQ& getIncomingQ() { return inQ; }
    BBQ& getOutgoingQ() { return outQ; }
    ICluster& getCluster(){ return *cluster; }
    void run(IP ip, unsigned short port, ClusterID id);
};


#endif //INCLUDED_SERVER_H
