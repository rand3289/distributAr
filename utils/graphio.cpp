#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace std::chrono;
#include <string.h> // strdup()
#include "graphio.h"
using namespace std;


GraphIO::GraphIO(){}
GraphIO::~GraphIO(){ CloseGraph(); }


string GraphIO::InitGraph(){ // returns graph file name
    stringstream gname; // create a graph name from current time
    const long long yr20 = (2020-1970)*365*  24*60*60  *1000L; // approximate milliseconds 1970-2020
    long long now = duration_cast<milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
    long long dt = (now - yr20) / 100; // tenth of a second
    gname << dt/100000 << "_" << std::setfill('0') << std::setw(5) << dt%100000;
    const string fname = gname.str() + ".dot";

    file.open(fname.c_str(), ios_base::out);
    file << "digraph " << gname.str() << " {" << endl;
    file.flush();
    return fname;
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
  while( getline(input,str) ){
    try{
      ParseLine(str,nodesOut, edgesOut);
    } catch (string& err){
        cerr << "Exception: " << err << endl;
    return false;
    }
  }
  return true;
}
