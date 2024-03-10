#include "server.h"
#include "icluster.h"
#include "udp.h"
#include "network.h" // idToMulticast()
#include "misc.h"
#include <sstream>
using namespace std;


// CONNECT DROP RUN EXIT
// misc and tracker: ACK NACK RET LIST
// cluster commands: ADD INFO LOAD SAVE 
string Server::parseCommand(char* cmd){
    cout << "Incoming command: " << cmd << endl;
    istringstream ss(cmd);
    string command;
    unsigned int param1;
    ss >> command;
    ss >> param1;

    if(command == "CONNECT"){
	IP ip = Network::idToMulticast(param1);
	cout << "Connecting to cluster " << ipStr(param1) << " (IP:" << ip << ")" << endl;
	network.subscribe(cluster->getId(), param1);
	return "ACK";
    } else if(command == "DROP"){
	cout << "Dropping connection to cluster " << param1 << endl;
        network.unsubscribe(cluster->getId(), param1);
	return "ACK";
    } else if(command == "RUN"){
	ClusterID old = cluster->getId();
	if(old != param1 ){
	    cout << "Changing Cluster ID from " << old << " to " << param1 << endl;
            cluster->setId(param1);
            IP mcIP = Network::idToMulticast(param1);
            mCastGrp.sin_addr.s_addr = mcIP; // ClusterID has changed. Update the multicast IP
	}
	return "ACK";
    } else if(command == "EXIT"){
	cout << "Exit command received. Terminating!" << endl;
	exit(0);
    }

    return cluster->command(cmd);
}


void Server::processCommands() {
    sockaddr_in from;
    int ret = udp.ReadSelect( reinterpret_cast<char*>(buff), buffSize, &from); // non-blocking

    if(ret>0){
        buff[ret] = 0; // make sure this is a null terminated string
        string reply = parseCommand(buff);
        if( !reply.empty() ){
            udp.Write(reply.c_str(), reply.length()+1, from); // +1 for null
        }
    }
}


// callback from Cluster::read()
int Server::write(int nodeIndex, const Time& time){
    while( timeBuff->write(nodeIndex, time) < 0 ){ // while calls write() second time if it fails
        timeBuff->setSrcClusterId(cluster->getId());
        outQ.push(timeBuff); // do this first to decrease latency.  Writing to network takes more time
        // writing through non-multicast socket.  Packets will have a real src port number, not MULTICAST_PORT.
        // TODO: writing to network can be done on a different thread since a single network interface
        // can write only a single packet at a time
        udp.Write((char*)&*timeBuff, timeBuff->size(), mCastGrp);
        timeBuff = std::make_shared<TimeBuffer>();
    }
    return 0;
}


void Server::performIO(){
    TBPtr bb;
    // sleep here if queue is empty and isWaitForInput()
    if(cluster->isWaitForInput()){
        inQ.pop(bb); // blocks untill multicast packet is available
        cluster->write(bb);
    } else if( inQ.popNoBlock(bb) ){ // is multicast packet available?
        cluster->write(bb);
    }

    if( cluster->getId() > 0 ){ // can send data only if it has a clusterID. Reading is different: it has to clear it's incoming queue
        cluster->read( *this ); // calls back Server::write()
    }
}


void Server::requestIdFromTracker(){
    string cmd = "ID " + to_string(cluster->getId());
    cout << "Updating the tracker with my cluster " << cmd << endl;
    udp.Write(cmd.c_str(), cmd.length()+1, tracker);
}


void Server::run(IP ip, unsigned short port, ClusterID id){
    if(!cluster){
	cerr << "ERROR!  Thread EXITING!  Could not load the DLL.  Make sure to 'setenv LD_LIBRARY_PATH .'" << endl;
	return;
    }

    tracker.sin_family = AF_INET;
    tracker.sin_port = port;
    tracker.sin_addr.s_addr= ip;

    mCastGrp.sin_family = AF_INET;
    mCastGrp.sin_port = htons(MULTICAST_PORT);
    mCastGrp.sin_addr.s_addr = Network::idToMulticast(id); // in case Cluster ID was specified

    cluster->setId(id);

    // randomize the thread start so they call requestIdFromTracker() at different times
    std::this_thread::sleep_for(std::chrono::milliseconds(rand()%3000)); // a few seconds is enough

    Time prev = std::chrono::high_resolution_clock::now();
    static const double trackerUpdateRate(60000); // microseconds
    static const int UPDATE_PERIOD = trackerUpdateRate/500; // about twice per second
    int commandPeriod = 100;

    try{
        while(true){
	    requestIdFromTracker();
    	    for(int i = UPDATE_PERIOD; i; --i){
	        processCommands();
	        for(int j = commandPeriod; j; --j){
		    performIO();
	        }
    	    }
	    // adjust the number of times we performIO() before processingCommand() by  matching trackerUpdateRate
	    Time now = std::chrono::high_resolution_clock::now();
	    double dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev).count();
	    double ratio = trackerUpdateRate/dt;
	    commandPeriod = (commandPeriod + ratio*(double)commandPeriod)/2;
	    commandPeriod = commandPeriod > 0 ? commandPeriod : 1;
	    cout << "dt=" << dt << "ms ratio=" << ratio << " commandPeriod=" << commandPeriod << endl;
	    prev = now;
        }
    } catch (const char* ex){
        cerr << "Exception caught: " << ex << ". Exiting" << endl;
    } catch (const string& ex){
        cerr << "Exception caught: " << ex << ". Exiting" << endl;
    } catch (const std::bad_alloc& ex) {
        cerr << "Out of memory! Exiting" << endl;
    } catch (const std::exception& ex){
        cerr << "Exception caught: " << ex.what() << endl; 
    } catch (...){
	cerr << "Unknown exception caught" << endl;
    }

} // run()
