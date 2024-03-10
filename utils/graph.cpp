#include "main.h" // ClusterID
#include "graphio.h"
#include <string>
#include <iostream>
using namespace std;
#include <math.h>
// save http://en.wikipedia.org/wiki/Random_graph to http://en.wikipedia.org/wiki/DOT_language file


// what are the parameters for connecting?
// list of cluster IDs
// connectivity of the graph? (average # of connections from a single cluster to others)
// connectivity of the IO cluster?
// weight of each connection

// Algorithm for building the network (connecting clusters to other clusters)
class Connector{
    GraphIO graph;
    void Connect(ClusterID cluster, unsigned int clusterCount, int avgConnectivity, int avgWeight);
public:
    // generate the parameters to build the network
    void GenerateParameters(unsigned int networkSize, int& connectivity, int& weight, int& IOconnectivity, int& IOweight);
        void BuildNetwork(unsigned int networkSize, int avgConnectivity, int avgWeight, int avgIOconnectivity, int avgIOweight);
};


void Connector::Connect(ClusterID cluster, unsigned int clusterCount, int avgConnectivity, int avgWeight){
    // TODO: use std::normal_distribution ???
    int connectivity = rand() % avgConnectivity + rand() % avgConnectivity;
    int weight = rand() % avgWeight + rand() % avgWeight;
    for(int i = 0; i < connectivity; ++i){
        unsigned int destination = 1 + (rand() % clusterCount); // TODO: use Discrete Uniform Distribution
        // TODO: randomize weight???
        graph.WriteEdge(cluster,destination, weight);
    }
}


void Connector::BuildNetwork(unsigned int networkSize, int avgConnectivity, int avgWeight, int avgIOconnectivity, int avgIOweight){
    graph.InitGraph();

    for(unsigned int i=1; i <= networkSize; ++i){
        unsigned int nodePerCluster = 1000; // TODO: nodes per cluster should depend on cluster connectivity (from and to)
        graph.WriteNode(i, nodePerCluster); 
    }

//    Connect(IO_CLUSTER_ID, networkSize, avgIOconnectivity, avgIOweight);
    for(unsigned int cluster = 1; cluster <= networkSize; ++cluster){
        Connect(cluster, networkSize, avgConnectivity, avgWeight);
    }

    graph.CloseGraph();
}


void Connector::GenerateParameters(unsigned int networkSize, int& connectivity, int& weight, int& IOconnectivity, int& IOweight){
    int logNetSize = (int) ceil(log(networkSize));
    connectivity = 10 + rand() % logNetSize; // log to balance out the growing network size
    weight = 5 + rand() % MAX_OUTPUT_NODES;
    IOconnectivity = connectivity*10;
    IOweight = weight;
}


int main(int argc, char* argv[]){
    if(argc != 2){
                cout << "distributAr (distributed Architecture framework) random graph generator" << endl;
        cout << "usage: " << argv[0] << " <# of nodes>" << endl;
        return 0;
    }

    try{
        int networkSize = atoi(argv[1]);

        Connector connector;
        int connectivity, weight, IOconnectivity, IOweight;
        connector.GenerateParameters(networkSize, connectivity, weight, IOconnectivity, IOweight);

        cout << "Generating a random graph with " << networkSize << " nodes." << endl;
        connector.BuildNetwork(networkSize,connectivity,weight,IOconnectivity, IOweight);
        cout << "Done." << endl;
    } catch (string& ex){
      cout << "EXCEPTION: " << ex << " EXITING!" << endl;
    }
} // main()
