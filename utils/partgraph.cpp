// This is a simple brute force graph partitioning program.
// Use it to split connected computing nodes (vertices) into groups.
// Each group will run on a single machine and minimize the number of connections(edges) among the groups.
// This way communications among most computing nodes will be done in memory on a local host.

#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include "graphio.h"
using namespace std;
using namespace std::chrono;


struct Vertex {
    int id;
    vector<int> edges;
    Vertex(int identifier): id(identifier) {}
    void swap(Vertex& rhs){ std::swap(id,rhs.id); edges.swap(rhs.edges); }
};


int countHyperEdges(vector<Vertex>& partition){
    int count = 0;
    for(auto v: partition){          // for every vertex in this partition
        for(auto edge: v.edges){     // and for each of it's edges
            bool external = true;    // assume it is an external edge (hyperedge)
	    for(auto v2: partition){ // if it's edge connects to a vertex in this partition, it is internal
                if(edge == v2.id){ external=false; break; }
	    }
            if(external){ ++count; }
        }
    }
    return count;
}


void optimizePartitions(vector<vector<Vertex>>& nodes){
    int part1 = rand() % nodes.size(); // pick two distinct partitions
    int part2 = rand() % nodes.size();
    while (part1 == part2){
        part2 = rand() % nodes.size();
    }
    int vert1 = rand() % nodes[part1].size(); // pick 2 vertices in both partitions
    int vert2 = rand() % nodes[part2].size();

    int edgeCount = countHyperEdges(nodes[part1]) + countHyperEdges(nodes[part2]);
    nodes[part1][vert1].swap( nodes[part2][vert2] );
    int newEdgeCount = countHyperEdges(nodes[part1]) + countHyperEdges(nodes[part2]);

    if(edgeCount < newEdgeCount){
        nodes[part1][vert1].swap( nodes[part2][vert2] );
    }
}


void loadGraph(const string& filename, vector<vector<Vertex>>& verts, vector<GraphEdge>& edges){
    vector<GraphNode> nodes;
    GraphIO::Parse(filename, nodes, edges);

    for(unsigned int i = 0; i < nodes.size(); ++i){
        vector<Vertex>& v = verts[i%verts.size()];
        unsigned int id = nodes[i].id;
        v.push_back( Vertex(id) );
        for(GraphEdge e: edges){
            if(e.from == id){
	        v.back().edges.push_back(e.to);
            }
        }
    }
}


// save new optimized graph.  Write partitions to labels for each node.
string saveGraph(vector<vector<Vertex>>& verts, vector<GraphEdge>& edges){
    GraphIO graph;
    string name = graph.InitGraph();
    for(size_t i = 0; i < verts.size(); ++i){
        auto vgroup = verts[i];
        for(auto v: vgroup){
            graph.WriteNode(v.id, i); // i(group) becomes label
	}
    }
    for(auto e: edges){
        graph.WriteEdge(e.from, e.to, e.label);
    }
    graph.CloseGraph();
    return name;
}


void printPartitions(vector<vector<Vertex>>& nodes){
    cout << "Partitioned graph.  Each line is one partition.  First (number) is the partition external edge count." << endl;

    int total = 0;
    for(auto part: nodes){
        int edges = countHyperEdges(part);
        total += edges;
        cout << "(" << edges << ")";
        for(Vertex& v: part){
            cout << " " << v.id;
        }
        cout << endl;
    }
    cout << "Total edges: " << total << endl << endl;
}


int kbhit(){ // from https://www.flipcode.com/archives/_kbhit_for_Linux.shtml
    const int STDIN = 0;

    fd_set rdset;
    FD_ZERO(&rdset);
    FD_SET(STDIN, &rdset);

    timeval timeout;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    return select(STDIN + 1, &rdset, NULL, NULL, &timeout);
}


int main(int argc, char* argv[]){
    if(argc < 3){
        cout << "distributAr (distributed Architecture framework) graph optimizer" << endl;
        cout << "usage: " << argv[0] << " filename.dot #partitions" << endl;
	return 1;
    }
    string dotFileName = argv[1];

    int groupCount = atoi(argv[2]);
    if(groupCount < 2){
        cout << "Invalid group size!" << endl;
	return 2;
    }

    cout << "Loading graph from " << dotFileName << endl;
    vector<vector<Vertex>> nodes(groupCount);
    vector<GraphEdge> edges;
    loadGraph(dotFileName, nodes, edges);

    for(unsigned int i=0; i < nodes.size(); ++i){
        if(nodes[i].size() < 1){
            cout << "Error: group " << i  << " is empty!  " << endl;
            cout << "Not enough vertices in a graph to partition it into " << groupCount << " groups." << endl;
            return 3;
        }
    }

    cout << "Optiminzing partitions. Press any key to stop." << endl;
    auto start = system_clock::now();
    long long optCount = 0;
    while( !kbhit() ){
        optimizePartitions(nodes);
        ++optCount;
    }

    int runtime = duration_cast<seconds>(system_clock::now() - start).count();
    cout << "Performed "<< optCount << " optimizations in " << runtime << " seconds." << endl;
    printPartitions(nodes);
    cout << "Saving optimized graph." << endl;
    string graphName = saveGraph(nodes,edges);
    cout << "Graph saved to " << graphName << endl;
}
