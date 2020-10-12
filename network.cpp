#include "network.h"
#include "server.h"
#include "misc.h"
#include <chrono>
#include <iostream>
#include <mutex> // lock_quard
#include <algorithm>
using namespace std;


void Network::getSubscriptions(unordered_map<ClusterID, vector<Server*> >& subscribers){
    std::lock_guard<SpinLock> lock(spinLock);
    if(!dirty){ return; }
    dirty = false;
    subscribers.clear();

    for(auto i: subscriptions){          // for every local
        ClusterID local = i.first;
        for(ClusterID remote: i.second){ // for every remote 
            for(auto& serv: servers){    // for every server
                if(local == serv->getCluster().getId() ){
                    subscribers[remote].push_back(&*serv);
                }
            }
        }
    }
}


void  Network::subscribe(const ClusterID local, const ClusterID remote) {
    std::lock_guard<SpinLock> lock(spinLock);
    dirty = true;

    for(ClusterID i: subscriptions[local]){
        if( remote == i ){ return; } // already connected
    }

    subscriptions[local].push_back(remote);

    for(auto server: servers){ // if remote is in the same process, no need to join a multicast group
	if( remote == server->getCluster().getId() ){
            return;
	}
    }

    IP ip = idToMulticast(remote);
    cout << "Cluster " << local <<  " is connecting to " << remote << " (IP:" << ip << ")" << endl;
    multicast.JoinMulticast(ip);
}


void Network::unsubscribe(const ClusterID local, const ClusterID remote){
    std::lock_guard<SpinLock> lock(spinLock);
    dirty = true;

    vector<ClusterID>& all = subscriptions[local];
    all.erase( std::find(all.begin(), all.end(), remote) );  // erase multicast subscription for cluster "local" only

    for(auto i: subscriptions){ // any other local clusters subscribe to remote???
        auto conns = i.second;
        if(conns.end() != std::find(conns.begin(), conns.end(), remote) ){
            return;
        }
    }

    for(auto server: servers){ // if remote is in the same process, no need to leave a multicast group
	if( remote == server->getCluster().getId() ){
            return;
	}
    }

    IP ip = idToMulticast(remote);
    multicast.LeaveMulticast(ip);
    cout << "All local clusters disconnected from cluster " << remote << " (IP:" << ip << ")" << endl;
}


int Network::readVerify(TimeBuffer& bb){
    int ret = read(bb);
    if(!ret) { return 0; }

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
	cout << std::chrono::duration_cast<std::chrono::milliseconds>((now - time)).count() << "msOLD   ";
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
//???	t = time;
	return 0;
    }
    t = time;

    return bb.sizeData();
}
