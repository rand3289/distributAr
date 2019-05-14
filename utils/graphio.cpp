#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
using namespace std::chrono;
#include <string.h> // strdup()
#include "graphio.h"
using namespace std;


GraphIO::GraphIO(){}
GraphIO::~GraphIO(){ CloseGraph(); }


string GraphIO::InitGraph(){
	stringstream date;
	date << std::chrono::system_clock::now().time_since_epoch().count();
	file.open((date.str()+".dot").c_str(), ios_base::out);

	file << "digraph ";
	file << date.str();
	file << " {" << endl;

	return date.str();
}


void GraphIO::WriteNode(unsigned int id, int label){
  file << id << " [label=" << label << "]" << endl;
}


void GraphIO::WriteEdge(unsigned int from, unsigned int to, int label){
  file << from << " -> " << to << " [label=" << label << "]" << endl;
}


void GraphIO::CloseGraph(){
	file << "}" << endl;
	file.close();
}


//################################# PARSING ####################################

// this is a hack... use boost???
void tokenize(string& str, vector<string>& tokens){
  char* vstr = strdup(str.c_str());
  char* found = strtok(vstr," =");
  while(found){
    string s(found);
    tokens.push_back(s);
    found = strtok(0," =");
  }
  free(vstr);
}

// 1 token: [label=0123456789] 
// 5 tokens: [ label = 0123456789 ]
int GraphIO::ParseLabel(vector<string>& tokens, unsigned int start){
  for(; start < tokens.size(); ++start){
    int label = atoi(tokens[start].c_str());
    if(label){
      return label;
    }
  } // for
  return -1; // value not found
} // ParseLabel()


void GraphIO::ParseLine(string& line, vector<GraphNode>& nodesOut, vector<GraphEdge>& edgesOut){
  vector<string> tokens;
  tokenize(line, tokens);
  if(tokens.size() < 2){
	return;
  }

  if( string::npos != tokens[1].find("-") ){ // edge contains "-"
    int label = ParseLabel(tokens,3); // skip "1 -> 2"
    GraphEdge edge(atoi(tokens[0].c_str()), atoi(tokens[2].c_str()), label);
    edgesOut.push_back(edge);
    return;
  }

  int node = atoi(tokens[0].c_str()); // if line starts with a number
  if(node){
    int label = ParseLabel(tokens,2);
    GraphNode gnode(node, label);
    nodesOut.push_back(gnode);
  }
}


// file is a full path to a *.dot file
// nodesOut will contain a list of nodes and their metadata in the graph
// edgesOut will contain a list of all edges in the graph
bool GraphIO::Parse(const string& file, vector<GraphNode>& nodesOut, vector<GraphEdge>& edgesOut){
  ifstream input(file);
  string str;
  while( !getline(input,str).eof() ){
    if( input.bad() ){
        cerr << "Error parsing file. Can not read from file." << endl;
        return false;
    }
    try{
      ParseLine(str,nodesOut, edgesOut);
    } catch (string& err){
        cerr << "Exception: " << err << endl;
	return false;
    }
  }
  return true;
}
