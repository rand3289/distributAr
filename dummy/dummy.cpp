// This is a dummy cluster DLL for testing
#include "main.h"
#include "icluster.h"
#include "timebuff.h"
#include <thread>
#include <iostream>


class DummyCluster: public ICluster {
//    std::chrono::high_resolution_clock::time_point lastTime;
public:
    virtual std::string command(const char* cmd){ std::cout << "DUMMY got command: " << cmd << std::endl;  std::cout.flush(); return "NACK"; }
    virtual int write(const TBPtr& bb){ return 0; } // can log input for debugging
    virtual int read( IWriter& writer );
};


// Generate random noise :)
int DummyCluster::read( IWriter& writer ){
    enum { MAX_TIME_STEP = 100, MAX_OUT_NODES = 2000 };
    Time lastTime = std::chrono::high_resolution_clock::now();
    const int maxNodesFired = 50 + rand() % 400;

//    std::cout << "D" << maxNodesFired << " ";
    for(int i = 0; i < maxNodesFired; ++i){
    writer.write( rand() % MAX_OUT_NODES, lastTime);
    lastTime = lastTime + std::chrono::microseconds( rand() % MAX_TIME_STEP );
    }
    std::this_thread::sleep_for( std::chrono::milliseconds(10) );
    return maxNodesFired;
}


extern "C" { // each DLL provides a way to load itself:
    std::shared_ptr<ICluster> getCluster() { return std::shared_ptr<ICluster>(new DummyCluster()); }
}

/*

 // use for debugging suspected data transmission problems (dump upon send and after receive)
void dumpDataForDebugging(string filename, char* buffer, int size,  const std::chrono::high_resolution_clock::time_point& now){
    ofstream dump;
    dump.open(filename, ios::app);
    dump << endl << now << endl;
    for(int i = 0; i< size; ++i){
    dump << (int) buffer[i];
    }
    dump.close();
}

*/
