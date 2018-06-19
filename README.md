# distributAr
distributar is a Tiny distributed computation framework for ANNs and more!
It provide mechanisms for sharing buffers among running threads. Threads running in a single process communicate through shared memory. Threads running in differrent processes or on different machines communicate via network multicast. Control plane is implemented with a simple text protocol.  You create a plugin dll to do whatever you want implementing ICluster interface and distributAr runs it in a single thread.  You can have as many threads as you want doing processing and as many instances of the server as you want. For now there is one tracker and one time server (should become redundant).  Connectivity among clusters is user defined. You can create a graph file and upload it into distributAr or connect clusters individually.

The project has a few unusual goals:
1) keep the code base small!!! Currently its a bit over 2000 lines of code.
2) reduce dependency on external libraries

If you want to bloat the project with your bright ideas, please fork.
If you want to run on windows/mac, put/move all OS dependent code into lib/
This project requires C++11 compliant compiler.

distributAr's primary purpose is to distribute Artificial Neural Networks among multiple hosts or CPUs
To avoid Combinatorial explosion distributAr creates subnets(clusters) with a relatively small number of nodes (10,000 ?)
Connect some nodes in this subnet to N other subnets (not nodes in other subnets directly).
Let each of those subnets(clusters) figure out the connectivity of input nodes to it's internal nodes.
NodeID has to be unique only among nodes on a cluster in order to run, save and restore properly.

Building distributAr:
doing git clone, changing to main dir and typing make compiles 5 executable and one library:
libdummy.so.1  - a dummy ANN (cluster of neurons) that sends and receives some random data
nets - main server.  Loads libdummy.so.1  ,connects to tracker and waits for client commands
nett - tracker.  Each server thread connects to the tracker and gets a "cluster id". Client gets info from tracker
netw - time server allows synchronization of time among main server instances
netc - client to send commands to tracker and main server instances
netg - creates a random graph and saves it to a DOT file. Not required to run distributAr
       http://en.wikipedia.org/wiki/Random_graph
       http://en.wikipedia.org/wiki/DOT_language file

Project parts:
Graph Generator: genrates a random graph representing a network structure
Client: builds network according to the graph or allows issuing commands to tracker/server/IoServer
Tracker: keeps track of cluster ID to IP:port mappings.  As clusters come up, they get IDs from the tracker.
Server: houses clusters of neurons each running on its own thread or performs IO on the physical devices.
server can load any DLL implementing ICluster interface.
Time Server: synchronizes time among running processes. lib/timesync.* is the client
Network class - provides multicast read connectivity to read from clusters running on other computers
TimeBuff class - data structure for placing impulses into network packets
serial.h - provides serialization primitives
udp.*    - wrapper around UDP protocol
