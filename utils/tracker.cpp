#include "udp.h"
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <chrono>
using namespace std;


#define ENV_UPDATE_TIME (std::chrono::seconds(600))
#define RESERVED_CLUSTER_ID 1024
typedef unsigned int ClusterID; // also defined in main.h
typedef std::chrono::high_resolution_clock Clock;


class Tracker{
    Udp udp;
    unordered_map<ClusterID, sockaddr_in> clusters;
    unordered_map<ClusterID, Clock::time_point> updateTime;
    // cluster id update timestamp.  Purge anything older than ENV_UPDATE_TIME ... isOld()
    ClusterID next = RESERVED_CLUSTER_ID+1; // first ClusterID to return in ID request command
    ClusterID getNextClusterId() { return next++; }

    bool isOld(ClusterID cluster){ return updateTime[cluster] < (Clock::now() - ENV_UPDATE_TIME); }
    ClusterID findByValue(const sockaddr_in& value) const;
    bool sendCommand(const sockaddr_in& addr, const string& cmd);
public:
    Tracker(unsigned short port): udp(port) {}
    void run();
};


bool operator==(const sockaddr_in& lhs, const sockaddr_in& rhs){
    return (lhs.sin_addr.s_addr == rhs.sin_addr.s_addr) && (lhs.sin_port == rhs.sin_port);
}


ClusterID Tracker::findByValue(const sockaddr_in& value) const {
    for(auto it: clusters){
        if( it.second == value){
	    return it.first;
        }
    }
    return 0;
}


bool Tracker::sendCommand(const sockaddr_in& addr, const string& cmd) {
    if(cmd.length() < 1){ return false; }
    udp.Write(cmd.c_str(), cmd.length()+1, addr);
    cout << "Reply: " << cmd << endl;
    return true;
}


void Tracker::run(){
    char buff[1024];
    
    while(true){
        sockaddr_in from;
        int size = sizeof(buff)-1;
        if( !udp.ReadSelect(reinterpret_cast<char*>(&buff), size, &from, 1000000) || 0==size ){
	    cout <<'.';
	    cout.flush();
            continue; // read timed out or failed
        }

	buff[size]=0; // make sure it a "C" string
	cout << endl << "Incoming: " << buff << endl; 
	istringstream is(buff);
	string cmd;
	is >> cmd;

	ostringstream reply;
      	const ClusterID cluster = findByValue(from);

	if(cmd == "ID"){ // cluster is requesting a new ID
	    ClusterID newId;
	    is >> newId; // server supplied a non-zero cluster ID?

	    if(0!=newId){ // update tracker's clusterID for that cluster
	      if(newId!=cluster){
		clusters.erase(cluster);
	      }
	      clusters[newId] = from;
	      updateTime[newId] = Clock::now();
	      string io = (newId > RESERVED_CLUSTER_ID) ? "" : "RESERVED ";
	      cout << io << "Cluster " << cluster << " updated its id to " << newId << endl;
	      continue;
	    }

	    if(0==cluster){ // cluster not found
	        newId = getNextClusterId();
	        clusters[newId] = from;
	    } else { // cluster exists - give it the old ID
		newId = cluster;
	    }

	    updateTime[newId] = Clock::now();
	    cout << "Cluster " << cluster << " reserved ID " << newId << endl;
	    reply << "RUN " << newId;
	    sendCommand(from, reply.str());

	} else if (cmd == "GET"){  // get cluster IP and port
	    ClusterID what;
	    is >> what;

	    auto it = clusters.find(what);
	    if(clusters.end() == it){ // not found
		reply << "DROP " << what;
	    } else if( isOld(what) ){ // cluster has not updated it's IP:PORT in a long time - probably died or got renamed
		if(what>RESERVED_CLUSTER_ID){ // IO clusters are special - they can not move and we should not delete them
		    cout << "cluster " << what << " did not update it's registration in a while.  DELETING." << endl; 
		    clusters.erase(it);
		    reply << "DROP " << what;
		}
	    } else {
	        sockaddr_in& addr = it->second;
	        reply << "POINT " <<  what << " " << addr.sin_addr.s_addr << " " << addr.sin_port;
	    }
	    sendCommand(from, reply.str());

	} else if (cmd == "LIST"){ // client wants a list of all clusters
	    for(auto it: clusters){
		if( !isOld(it.first) ){ // don't delete old clusters.  just skip
	            reply << it.first << " ";
		}
	    }
	    sendCommand(from, reply.str() );

	} else if (cmd == "ACK"){
	    cout << "Server ACKnowledged" << endl;
	} else {
	    cout << "UNKNOWN COMMAND " << endl;
	}
    } // while(true)
}


int main(int argc, char* argv[]){
    if(argc != 2){
        cout << "usage: tracker port#" << endl;
        return 0;
    }
    unsigned short port = atoi(argv[1]);
    cout << "Running on port " << port << endl;

    Tracker tracker(port);
    try {
        tracker.run();
    } catch (string& ex){
        cerr << "Exception caught: " << ex << ". Exiting" << endl;
    } catch (std::bad_alloc&) {
        cerr << "Out of memory! Exiting" << endl;
    } catch (std::exception& ex){
      cerr << "Exception caught: " << ex.what() << endl; 
    }
}
