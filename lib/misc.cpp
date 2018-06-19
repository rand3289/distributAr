#include "misc.h"
#include <dlfcn.h>  // dlopen()
#include <string.h> // strerror()
using namespace std;
// TODO: port OS dependent code to other OS


shared_ptr<ICluster> loadDll(std::string& dllName){
	cout << "Loading a cluster from " << dllName << endl;
	void* handle = dlopen(dllName.c_str(), RTLD_NOW);
	if(!handle){
		cerr << "Unable to open DLL " << dllName << endl << dlerror() << endl;
		return shared_ptr<ICluster> ();
	}

	typedef shared_ptr<ICluster> (*DllFunc)();
	DllFunc getClass = reinterpret_cast<DllFunc> ( dlsym(handle,"getCluster") );
	if(!getClass){
		cerr << "Unable to get Cluster from DLL " << dllName << " " << dlerror() << endl;
		return shared_ptr<ICluster> ();
	}
	return (*getClass)(); // do NOT dlclose() - DLL stays attached forever
}


void boostPriority(std::thread& t){
	std::thread::native_handle_type threadId = t.native_handle();
	sched_param sp;
	sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if(pthread_setschedparam(threadId, SCHED_FIFO, &sp)){
	    cout << "Failed to boost thread priority: " << strerror(errno) << " ERRNO:" << errno << endl << "Run the server under a priveleged account." << endl << endl;
	} else {
	    cout << "Running timing thread with priority " << sp.sched_priority << endl;
	}
}


ostream& operator<<(ostream& out, const Time& t) {
     const std::intmax_t perSec = 1000000;
     const std::intmax_t count = std::chrono::duration_cast<std::chrono::microseconds>(t.time_since_epoch()).count();
     const std::intmax_t sec = count / perSec;
     const std::intmax_t fraction = count % perSec;
     out << sec << "." << fraction << "s";
     return out;
}

/*
template <typename Rep>
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, Rep>& d){
	out << d.count();
	switch(d.period){ // till C++17
		case std::nano:        out << "ns"; break;
		case std::micro:       out << "us"; break;
		case std::milli:       out << "ms"; break;
		case std::ratio<1>:    out << "s";  break;
		case std::ratio<60>:   out << "m";  break;
		case std::ratio<3600>: out << "h";  break;
		default:               out << "??"; break;
	}
}
*/

std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::nano>&        d){ out << d.count() << "ns"; return out; }
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::micro>&       d){ out << d.count() << "us"; return out; }
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::milli>&       d){ out << d.count() << "ms"; return out; }
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::ratio<1>>&    d){ out << d.count() << "s";  return out; }
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::ratio<60>>&   d){ out << d.count() << "m";  return out; }
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::ratio<3600>>& d){ out << d.count() << "h";  return out; }
