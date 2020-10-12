#include "queue.h"
#include "timebuff.h"
#include "timesync.h"
#include "network.h"
#include "server.h"
#include "main.h"

#include <unistd.h> // getcwd()
#include <vector>
#include <memory>
#include <thread>
#include <ostream>
#include <sstream>
using namespace std;


int main(int argc, char* argv[]){
    if(argc<4){
        cout << "distributAr (distributed Architecture framework) server" << endl;
        cout << "usage: " << argv[0] << " <tracker addr/ip>  <tracker port#>  <library:N:C>" << endl;
        cout << "library:N:C - load N instances of 'library' starting with ClusterID C" << endl;
        return 0;
    }

    {
        char cwd[1024];
	if(NULL != getcwd(cwd, sizeof(cwd)) ){
	    cout << "Running " << argv[0] << " in " << cwd << endl;
	}
    }

    struct hostent* he = gethostbyname(argv[1]); // tracker's IP
    if(!he){
        cerr << "Can not resolve address for " << argv[1] << endl;
	return 1;
    }
    IP ip = *reinterpret_cast<IP*>(he->h_addr_list[0]);

    unsigned short port = atoi(argv[2]); // tracker's port
    port = htons(port);

    vector<shared_ptr<Server> > servers;
    Network network(servers);

    try {
	TimeSync timeSync(ip,port);
	std::thread tt( [&](){ timeSync.sync(); } ); // timesync runs in its own thread
	boostPriority(tt);
	tt.detach();

	cout << "Starting with " << argc-3 << " cluster type(s).  hardware_concurrency is " << std::thread::hardware_concurrency() << endl;
	for(int i=3; i< argc; ++i){
		int clusterCount = 1;   // if count is not specified, load 1 cluster from the DLL
		int startID = 0;        // no ID specified - request id from tracker
		string dllName;
		string token;
		stringstream ss(argv[i]);

		if( std::getline(ss,token,':') ){
			dllName = token; 
		}
		if( std::getline(ss,token,':')){
			clusterCount = atoi(token.c_str());
		}
		if( std::getline(ss,token,':') ){
			startID = atoi(token.c_str());
		}

		cout<< "Starting " << clusterCount << " clusters from DLL '" << dllName << "' at address " << startID <<endl;
		for(int i = 0; i < clusterCount; ++i){
			shared_ptr<Server> serv = make_shared<Server>(network, dllName);
			servers.push_back(serv);
			const int id = startID > 0 ? startID+i : 0; 
			std::thread( [=](){ serv->run(ip,port,id); } ).detach(); // run servers in own threads
		}
	}

	// TODO: make sure network writes by local servers are not read back or outgoingQs are useless!
	TBPtr timeBuff = make_shared<TimeBuffer>();
	TBPtr data;
	unordered_map<ClusterID, vector<Server*> > subscribers;
	for(int i=0; true; ++i){
            if(0 == i%1000){ // once in a while ( as servers join and leave multicast groups ) rebuild subscribers
                network.getSubscriptions(subscribers); // this locks!!!
            }

	    if ( network.readVerify(*timeBuff) ) {
	        for(Server* s: subscribers[timeBuff->getSrcClusterId()] ){
		    s->getIncomingQ().push(timeBuff); // lock-less
		}
	        timeBuff = make_shared<TimeBuffer>();
	    }

	    for(auto& src: servers){ // this will clear all "out queues" for all servers
		if( src->getOutgoingQ().pop(data) ){
	            for(Server* s: subscribers[data->getSrcClusterId()] ){
		        s->getIncomingQ().push(data); // lock-less
		    }
                }
            }
	} // for(true)

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

    return 1;
}
