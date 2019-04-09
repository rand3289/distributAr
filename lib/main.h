#ifndef INCLUDED_MAIN_H
#define INCLUDED_MAIN_H

#include <chrono>
typedef std::chrono::high_resolution_clock::time_point Time; // units for measuring time anywhere in the framework

typedef unsigned int ClusterID; // Unique ID of each cluster as seen by tracker and other clusters

#define TIME_SERVER_ID (0xFFFFFFFF)
#define TRACKER_CLUSTER_ID 0	// all communications in the framework are performed using a ClusterID
#define MULTICAST_PORT 5432	// all servers are listening on this port

// based on 1500 MTU the payload should not exceed 1456 bytes - header
#define MAX_OUTPUT_NODES ((1456-4*8)/4) // more like max pulses - this needs to fit into a single UDP packet
#define MAX_MSG_Q_SIZE (256) // maximum number of network messages in incoming and outgoing queue for each thread
#define PACKET_LIFE_TIME_MS (200) // packets older than this many ms will be dropped

#endif // INCLUDED_MAIN_H
