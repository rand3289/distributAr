#include "network.h"
#include "server.h"
#include "misc.h"
#include <chrono>
#include <iostream>
#include <mutex> // lock_quard
using namespace std;


void Network::getSubscriptions(unordered_map<ClusterID, vector<Server*> >& subscribers){
    std::lock_guard<SpinLock> lock(spinLock);
    if(!dirty){ return; }
    dirty = false;
    subscribers.clear();

    for(auto i: subscriptions){
        ClusterID key1 = i.first;
        for(auto j: i.second){
	    ClusterID key2 = j->getClusterID();
            for(auto& serv: servers){ // for every server
                const ClusterID id = serv->getCluster().getId();
                if(id == key2){
                    subscribers[key1].push_back(&*serv);
                }
            }
        }
    }
}


void  Network::subscribe(const ClusterID local, const ClusterID remote) {
    std::lock_guard<SpinLock> lock(spinLock);
    dirty = true;

    for(auto i: subscriptions[local]){
        if( remote == i->getClusterID() ){ return; } // already connected
    }

    auto sub = make_shared<Subscription>(*this,remote);
    subscriptions[local].push_back(sub);

    for(auto server: servers){ // if remote is in the same process, no need to join a multicast group
	if( remote == server->getCluster().getId() ){
            return;
	}
    }

    IP ip = idToMulticast(remote);
    cout << "Cluster " << local <<  " is connecting to " << remote << " (IP:" << ip << ")" << endl;
    multicast.JoinMulticast(ip);
}


// when all references to Subscription go out of scope, subscription to the multicast group is deleted
void Network::unsubscribe(const ClusterID local, const ClusterID remote){
    std::lock_guard<SpinLock> lock(spinLock);
    dirty = true;

    auto all = subscriptions[local];
    for(auto i = all.begin(); i!= all.end(); ++i){
        if(remote == (*i)->getClusterID() ){
	    all.erase(i); // erase multicast subscription for cluster "local" only
	    return;
	}
    }
}


void Network::unsubscribe(const ClusterID remote){
    std::lock_guard<SpinLock> lock(spinLock);
    for(auto server: servers){ // if remote is in the same process, no need to leave a multicast group
	if( remote == server->getCluster().getId() ){
            return;
	}
    }
    IP ip = idToMulticast(remote);
    multicast.LeaveMulticast(ip);
    cout << "All clusters disconnected from cluster " << remote << " (IP:" << ip << ")" << endl;
}


int Network::verifyPacket(TimeBuffer& bb){
    bb.prepareRead();		// convert from network time
    if( bb.sizeData() <= 0){	// empty // TODO: check max size???
	return 0;
    }

    const static std::chrono::milliseconds dropDelay(PACKET_LIFE_TIME_MS);
    const Time time = bb.getTime(); // now that the header is parsed, get the time
    const Time now = std::chrono::high_resolution_clock::now();
// TODO: make a field and insert a local timestamp into each packet read from the network ???

    if(time + dropDelay < now){
	// static unsigned int dropped = 0; // count of dropped out of sequence packets
	// ++dropped; // inc when it's displayed
	// cout << "Dropping old data. now: " << now << " cutoff time: " << (time+PACKET_LIFE_TIME_MS) << ' dropped ' << dropped << endl;
	// cout << '-';
	cout << std::chrono::duration_cast<std::chrono::milliseconds>((now - time)).count() << "msOLD   ";
//	cout << "old data t=" << time << " now=" << now << endl;
	return 0;
    }

//a    const static Time uninitTime; // default initialized time that would be inserted into sequence upon first access
    ClusterID id = bb.getSrcClusterId();
    Time& t = sequences[id]; // when was the last packet received from that cluster?
    if( t > time ){ // out of order?
//	static ofstream log("dropPacket"+std::to_string((long long)std::this_thread::get_id())+".txt");
//1	stringstream fname; fname << "dropPacket" << std::this_thread::get_id() << ".txt";
//1	static ofstream log(fname.str());
//1	log << "time=" << t << " - " << time << " = " << (t-time) << " cluster=" << id <<" size=" << bb.sizeData() << endl;
	// ++dropped; // inc when it's displayed
	// cout << dropped << "_UDP_LOST "; // << t << ">=" << header.time << ")";
//a	if(t == uninitTime){ t = time; }
//	cout << '~'; cout.flush();
	cout << "packet's timestamp is out of order" << endl;
	t = time;
	return 0;
    }
    t = time;

    return bb.sizeData();
}
