#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

#include <chrono>
typedef std::chrono::high_resolution_clock::time_point Time; // units for measuring time anywhere in the framework

typedef unsigned int NodeID;    // Each node/neuron in a cluster has a unique id of this type
typedef unsigned int ClusterID; // Unique ID of each cluster as seen by tracker and other clusters

#define TIME_SERVER_ID (0xFFFFFFFF)
#define TRACKER_CLUSTER_ID 0	// all communications in the framework are performed using a ClusterID
#define MULTICAST_PORT 5432	// all servers are listening on this port

// based on 1500 MTU the payload should not exceed 1456 bytes - header
#define MAX_OUTPUT_NODES ((1456-4*8)/4) // more like max pulses - this needs to fit into a single UDP packet
#define MAX_MSGQ_SIZE 1024 // maximum number of network messages in queue 

#define FRAMES_PER_SECOND 60 // rate at which the device will be read and clusters should send data to other clusters
#define PROPAGATION_DELAY (std::chrono::milliseconds(1000/FRAMES_PER_SECOND)) // how long it takes to fire one node ~16ms
#define CLUSTER_PROPAGATION_DELAY (8*PROPAGATION_DELAY) // impulses older than this will not be sent
#define DATA_DROP_DELAY_MS (10*PROPAGATION_DELAY) // packets older than this many ms will be dropped

#endif // INCLUDED_MAIN_H
