#ifndef INCLUDED_GRAPHIO_H
#define INCLUDED_GRAPHIO_H

#include <fstream>
#include <vector>
#include <string>
using namespace std;

struct GraphNode {
   unsigned int id;
   int label; 
   GraphNode(unsigned int idNode, int labelNode): id(idNode), label(labelNode) {}
};

struct GraphEdge{
   unsigned int from;
   unsigned int to; 
   int label;
   GraphEdge(unsigned int fromNode, unsigned int toNode, int labelNode): from(fromNode), to(toNode), label(labelNode) {}
};


class GraphIO{
	std::fstream file;
	void ParseLine(string& line, vector<GraphNode>& nodesOut, vector<GraphEdge>& edgesOut);
	int ParseLabel(vector<string>& tokens, unsigned int start);
public:
	GraphIO();
	~GraphIO();
	string InitGraph(); // returns graph name
	void WriteNode(unsigned int id, int label);
	void WriteEdge(unsigned int from, unsigned int to, int label);
	void CloseGraph();
	bool Parse(const string& file, vector<GraphNode>& nodesOut, vector<GraphEdge>& edgesOut);
};


#endif // INCLUDED_GRAPHIO_H
