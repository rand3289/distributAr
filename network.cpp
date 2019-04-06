#include "network.h"
#include "server.h"
#include "misc.h"
#include <chrono>
#include <iostream>
#include <mutex> // lock_quard

//1#include <fstream> // FOR DEBUGGING ONLY !!!
//1#include <sstream>
//1#include <string>
//1#include <thread>

using namespace std;


shared_ptr<Subscription>  Network::subscribe(const ClusterID local, const ClusterID remote) {
    for(auto server: servers){ // check if they are local - no need to join multicast
	if( remote == server->getCluster().getId() ){
	    return make_shared<Subscription>(*this,remote);
	}
    }
    std::lock_guard<SpinLock> lock(spinLock);
    weak_ptr<Subscription> sub = subscriptions[remote];
    shared_ptr<Subscription> subscription = sub.lock();
    if(!subscription){
	subscription.reset(new Subscription(*this,remote));
        IP ip = idToMulticast(remote);
        cout << "Cluster " << local <<  " is connecting to " << remote << " (IP:" << ip << ")" << endl;
	multicast.JoinMulticast(ip);
    }
    return subscription;
}


void Network::unsubscribe(const ClusterID remote){
    std::lock_guard<SpinLock> lock(spinLock);
    auto sub = subscriptions.find(remote);
    if( subscriptions.end() == sub ){ // there is no such subscription.
	return;
    }
    IP ip = idToMulticast(remote);
    multicast.LeaveMulticast(ip);
    subscriptions.erase(sub); // subscriptions.erase(remote);
    cout << "All clusters disconnected from cluster " << remote << " (IP:" << ip << ")" << endl;
}


int Network::verifyPacket(TimeBuffer& bb){
    bb.prepareRead();		// convert from network time
    if( bb.sizeData() <= 0){	// empty // TODO: check max size???
	return 0;
    }

    const static std::chrono::milliseconds dropDelay(DATA_DROP_DELAY_MS);
    const Time time = bb.getTime(); // now that the header is parsed, get the time
    const Time now = std::chrono::high_resolution_clock::now();
    if(time + dropDelay < now){
	// ++dropped; // inc when it's displayed
	// cout << "Dropping old data. now: " << now << " cutoff time: " << (time+DATA_DROP_DELAY_MS) << ' dropped ' << dropped << endl;
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
