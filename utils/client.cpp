#include "main.h"
#include "graphio.h"
#include "udp.h"
#include "misc.h"  // ipStr()
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <thread>
using namespace std;
#include <stdlib.h> // atoi()


#define COMMAND_RETRY_COUNT 3

class Commander{
    Udp udp;
    unordered_map<ClusterID, sockaddr_in> clusters;
public:
    Commander(const sockaddr_in& trackerAddr) { clusters[TRACKER_CLUSTER_ID] = trackerAddr; }
    int command(ClusterID cluster, const string& cmd);
    int commandWithRetry(ClusterID cluster, const string& cmd);
    int trackerCommand(const string& cmd){ return command(TRACKER_CLUSTER_ID, cmd); }
    string waitReply(unsigned int timeoutMs);
    int createNetwork(const string& dotFileName);

    int listClusters(vector<ClusterID>& idsOut);
    int getCluster(ClusterID id);
    int getClusters();
    int showReply();
    void printCluster(ClusterID id){
    IP ip = clusters[id].sin_addr.s_addr;
        cout << "\t" << id << " -> " << ipStr(ip) << " (" << ip << "):" << clusters[id].sin_port << endl;
    }
};


int Commander::commandWithRetry(ClusterID cluster, const string& cmd){
    string reply;
    unsigned int retry = COMMAND_RETRY_COUNT;
    do{
    command(cluster, cmd);
    reply = waitReply(5*1000*1000);
    } while( retry-- && (reply != "ACK") ); // wait for ACKs from the cluster.  Reissue if not received

    if(reply!="ACK") {
        cerr << "Command " << cmd << " failed: " << reply << endl;
    return 0;
    }
    return 1;
}


int Commander::createNetwork(const string& dotFileName){
    cout << "Creating network according to '" << dotFileName <<  "' DOT graph file." << endl;
    vector<GraphNode> nodes;
    vector<GraphEdge> edges;
    bool ok = GraphIO::Parse(dotFileName,nodes, edges);
    cout << "Parsed " << dotFileName << " result=" << (ok?"ok ": "failed ") << " nodes=" << nodes.size() << " edges=" << edges.size() << endl;
    if(!ok){
        return -1;
    }

    vector<ClusterID> ids;
    do {
        ids.clear();
        cout << "Waiting for " << nodes.size() << " clusters to join the network." << endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    listClusters(ids);
    // Time server can not be used as a cluster
    ids.erase( std::find(ids.begin(), ids.end(), TIME_SERVER_ID) );
    cout << "Found " << ids.size() << " clusters." << endl;
//    if(ids.end() == find(ids.begin(), ids.end(), IO_CLUSTER_ID) ){
//        cout << "IO cluster is NOT on line. Waiting..." << endl;
//        continue;
//    }
    } while( ids.size() < nodes.size() );

    cout << "Getting IP:port for all clusters" << endl;
    for(ClusterID id: ids) {
        getCluster(id);
    }

    cout << "Issuing new ClusterIDs to clusters" << endl;
    for(unsigned int i=0; i<ids.size(); ++i){
//    if(IO_CLUSTER_ID == ids[i]){      swap(ids[i], ids.back());     } // move IO cluster id to the back
//    if(IO_CLUSTER_ID == nodes[i].id){ swap(nodes[i], nodes.back()); } // move IO cluster graph node to the back

        ClusterID oldId = ids[i];
        ClusterID newId = nodes[i].id;
//    if(IO_CLUSTER_ID != oldId && IO_CLUSTER_ID != newId) {
        if(!commandWithRetry(oldId, "RUN "+ to_string(newId) ) ) { return 0; }
//    }
    }

    cout << "Refreshing IP:port for all clusters" << endl;
    sockaddr_in tracker = clusters[TRACKER_CLUSTER_ID]; // clear all clusters exept tracker
    clusters.clear();
    clusters[TRACKER_CLUSTER_ID] = tracker;

    std::this_thread::sleep_for(std::chrono::seconds(5)); // let clusters tell tracker about their new ClusterID
    for(ClusterID id: ids) {
        getCluster(id);
    }

    cout << "Populating clusters." << endl;
    for(GraphNode node: nodes) { // add nodes to each cluster
        if(node.label > 0){
//        if(!commandWithRetry(node.id, "ADD " + to_string(node.label)) ) { return 0; } // ADD count
        commandWithRetry(node.id, "ADD " + to_string(node.label)); // ADD node count to cluster. Do not fail if cluster NACKs
        }
    }

    cout << "Connecting clusters." << endl;
    for(GraphEdge edge: edges) {
        if(edge.label > 0){
            if(!commandWithRetry(edge.from, "CONNECT "+ to_string(edge.to) +" "+ to_string(edge.label) )) { return 0; } ; // CONNECT to count
    }
    }

    cout << "Network is running!" << endl;
    return 0;
}


string Commander::waitReply(unsigned int timeoutUs){
    char buff[0xFFFF];
    int size = udp.ReadSelect(buff, sizeof(buff), timeoutUs);
    if(size <= 0){
        return ""; // read failed
    }
    buff[size]=0;
    return buff;
}


int Commander::getCluster(ClusterID id){
    trackerCommand("GET " + std::to_string(id));
    string reply = waitReply(1000*1000);

    istringstream rs(reply);
    string cmd;
    ClusterID retCid;
    IP ip;
    unsigned short port;
    rs >> cmd;
    rs >> retCid;
    rs >> ip;
    rs >> port;

    // if tracker is giving us 127.0.0.1, it does not know cluster's real IP
    ip = replaceLo(ip, clusters[TRACKER_CLUSTER_ID].sin_addr.s_addr);
    
    if( cmd!="POINT" || retCid!=id ){
        cerr << "Unexpected reply: '" << reply << "' Expected: 'POINT " << id << " <ip> <port>" << endl;
        return 0;
    }
      
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr= ip;
    
    clusters[id] = addr;
    return id;
}


int Commander::listClusters(vector<ClusterID>& idsOut){
    trackerCommand("LIST");
    string reply = waitReply(1000*1000);

    istringstream ss(reply);
    while( !ss.eof() && !ss.fail() ){
        unsigned int id;
        ss >> id;
        if( 0!=id && !ss.fail() ){
            idsOut.push_back(id);
        }
    }
    return idsOut.size();
}


int Commander::getClusters(){
    vector<ClusterID> ids;
    int size = listClusters(ids);
    cout << size << " clusters found" << endl;

    for(ClusterID id: ids){
    if( getCluster(id) ){
        printCluster(id);
    }
    }
    return size;
}


int Commander::command(ClusterID cluster, const string& cmd){
    sockaddr_in& addr = clusters[cluster];
    for(int i = 0; 0==addr.sin_port && i<3 ; ++i){
        cout << "Requesting cluster information from tracker" << endl;
        getCluster(cluster);
        addr = clusters[cluster];
    }
    if(0==addr.sin_port){
      cerr << "Unable to get cluster info" << endl;
      return 1;
    }

    cout << "Command to cluster " << cluster << ": " << cmd << endl;
    udp.Write(cmd.c_str(), cmd.length()+1, addr);
    return 0;
}


int Commander::showReply(){
    string rep = waitReply(5*1000*1000);
    if( !rep.empty() ){
        cout << "reply: " << rep << endl;
    }
    return rep == "ACK";
}


void validateArgc(int argc, int minArgc){
  if(argc < minArgc){
    cerr << "Missing arguments..." << endl;
    exit(1);
  }
}


int main(int argc, char* argv[]){
    if( argc<4 ){
        cout << "distributAr (distributed Architecture framework) client" << endl;
        cout << "usage: " << argv[0] << " trackerIP  trackerPort command <parameters>" << endl;
        cout << "supported commands are:" << endl;
        cout << "\tlist - list all clusters reporting to tracker" << endl;
        cout << "\tcreate graphFileName - build the network according to *.dot file" << endl;
    cout << "\tget cluster - get info for cluster from tracker" << endl;

    cout << "\tinfo cluster - ask cluster to print info to screen" << endl;
    cout << "\tadd cluster N - add N nodes to cluster" << endl;
    cout << "\trun cluster1 cluster2 - start cluster1 running under id cluster2" << endl;
    cout << "\tsave cluster saveNum - save cluster state into restore point saveNum" << endl;
    cout << "\tload cluster saveNum - restore cluster from file (set ID first)" << endl;
    cout << "\tconnect fromID toID  - connect two clusters (make toID subscribe to fromID's multicast group)" << endl;
    cout << "\tdrop fromID toID - delete cluster connection" << endl;
    cout << "\tClusterID key - get cluster or device variable" << endl;
    cout << "\tClusterID key=value - set cluster or device variable" << endl;
        return 0;
    }

    struct hostent* he = gethostbyname(argv[1]); // tracker's IP
    if(!he){
        cerr << "Can not resolve address for " << argv[1] << endl;
    return 1;
    }
    IP ip = *reinterpret_cast<IP*>(he->h_addr_list[0]);

    unsigned short port = atoi(argv[2]); // tracker's port
    if(!port){
      cerr << "Error scanning port from " << argv[2] << endl;
      return 1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr= ip;
    Commander commander(addr);

    string cmd(argv[3]);
    if(cmd == "list"){ // command to tracker
      return commander.getClusters();
    }

    if(cmd == "get"){ // command to tracker
        validateArgc(argc, 5);
        ClusterID id = atoi(argv[4]);
        if( commander.getCluster(id) ){
        commander.printCluster(id);
    }
    return 0;
    }

    if(cmd == "create"){
        validateArgc(argc, 5);
        string file(argv[4]);
        commander.createNetwork(file);
    return commander.showReply();
    }

    if(cmd == "info"){
        validateArgc(argc, 5);
    ClusterID cluster = atoi(argv[4]);
    commander.command(cluster, "INFO");
    return commander.showReply();
    }

    if(cmd=="add"){
        validateArgc(argc, 6);
        ClusterID cluster = atoi(argv[4]);
        string count(argv[5]);
    commander.command(cluster, "ADD "+ count);
    return commander.showReply();
    }

    if(cmd=="run"){
        validateArgc(argc,6);
    ClusterID cluster = atoi(argv[4]);
        string newCid(argv[5]);
    commander.command(cluster, "RUN "+newCid);
    return commander.showReply();
    }

    if(cmd=="save"){
        validateArgc(argc,6);
    ClusterID cluster = atoi(argv[4]);
    string saveNum(argv[5]);
    commander.command(cluster, "SAVE "+saveNum);
    return commander.showReply();
    }

    if(cmd=="load"){
        validateArgc(argc,6);
    ClusterID cluster = atoi(argv[4]);
    string saveNum(argv[5]);
    commander.command(cluster, "LOAD "+saveNum);
    return commander.showReply();
    }

    if(cmd=="drop"){
    validateArgc(argc,6);
    string deleteCluster(argv[4]);
    ClusterID cluster = atoi(argv[5]);
    commander.command(cluster, "DROP "+ deleteCluster);
    return commander.showReply();
    }

    if(cmd == "connect"){
        validateArgc(argc, 6);
        string from = argv[4];
        ClusterID to = atoi(argv[5]);
        commander.command(to, "CONNECT "+from);
    return commander.showReply();
    }

    // commands starting with ClusterID and following with a key or a key=value pair allow setting and displaying cluster and device variables
    ClusterID cluster = atoi(cmd.c_str());
    if(cluster){
    validateArgc(argc,5);
    commander.command(cluster, argv[4]);
    return commander.showReply();
    }
    
    cerr << "Unknown command " << cmd << endl;
    return 1;
} // main()
