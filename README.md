# distributAr

**distribut**ed**Ar**chitecture is a Tiny distributed computation framework for ANNs and more!
It provide mechanisms for sharing buffers among running threads. Threads running in a single process communicate through shared memory. Threads running in differrent processes or on different machines communicate via network multicast. Control plane is implemented with a simple text protocol.  You create a plugin dll to do whatever you want implementing ICluster interface and distributAr runs it in a single thread.  You can have as many threads as you want doing processing and as many instances of the server as you want. For now there is one tracker and one time server (should become redundant).  Connectivity among clusters is user defined. You can create a graph file and upload it into distributAr or connect clusters individually.

distributAr's primary purpose is to distribute Artificial Neural Networks among multiple hosts or CPUs
To avoid Combinatorial explosion distributAr creates subnets(clusters) with a small number of nodes.
Connect some nodes in this subnet to N other subnets (not nodes in other subnets directly).
Let each of those subnets(clusters) figure out the connectivity of input nodes to it's internal nodes.

### The project has a few unusual goals
1) keep the code base small!!! Currently its a bit over 2000 lines of code.
2) reduce dependency on external libraries

If you want to bloat the project with your bright ideas, please fork.  
If you want to run on windows/mac, put/move all OS dependent code into lib/  
This project requires C++11 compliant compiler.  

### Building distributAr

doing git clone, changing to main dir and typing make compiles 5 executable and one library:

**libdummy.so.1**  - a dummy ANN (cluster of neurons) that sends and receives some random data  
**nets** - main server.  Loads libdummy.so.1  then connects to tracker and waits for client commands  
**nett** - tracker.  Each server thread registers with the tracker and gets a "cluster id". Client gets info from tracker.  
**netw** - time server facilitates synchronization of time among server instances  
**netc** - client to send commands to tracker and server instances  
**netg** - Not required to run distributAr. Creates random network configurations for testing by creating a random graph and saving it to a DOT file.  
http://en.wikipedia.org/wiki/Random_graph  
http://en.wikipedia.org/wiki/DOT_language  

### Project parts
Client: builds network according to the graph or allows issuing commands to tracker/server/IoServer  
Tracker: keeps track of cluster ID to IP:port mappings.  As clusters come up, they get IDs from the tracker by registering.  
Server: houses clusters of neurons each running on its own thread.  server can load any DLL implementing ICluster interface.  
Time Server: synchronizes time among running processes. lib/timesync.* is the client  
Network class - provides multicast read connectivity to subscribe/read from clusters running on other computers  
TimeBuff class - data structure for placing impulses into network packets  
udp.*    - wrapper around UDP protocol  
Graph Generator: genrates a random graph representing a network structure  
serial.h - provides serialization primitives  

