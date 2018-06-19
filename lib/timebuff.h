#ifndef INCLUDE_TIMEBUFF_H
#define INCLUDE_TIMEBUFF_H

#include "main.h"	// ClusterID, Time
#include "timesync.h"	// toNetwork() and toLocal()
#include <limits>
#include <memory>
#include <chrono>
#include <string> // to_string()
//using namespace std::chrono;


class TimeBuffer {
    #pragma pack(push,1) // make sure members are not padded for alignment
	ClusterID srcId;
	Time time; 	// for each firing node +-32 millisecond offset can be stored in buffer's int16_t using microseconds
        uint16_t count;	// MAX_OUTPUT_NODES should be smaller than 0xFFFF
	uint16_t idx[MAX_OUTPUT_NODES]; // set index in this buffer when node fires
        int16_t  dt[MAX_OUTPUT_NODES]; // set time data in this buffer when node fires
    #pragma pack(pop)
public:
	// size of the used portion of the TimeBuffer
	inline size_t size()      const { return sizeof(srcId) + sizeof(time) + sizeof(count) + 2*sizeof(int16_t)*count; }
	inline size_t capacity()  const { return MAX_OUTPUT_NODES; } // how much can I put in total? should be MAX_OUTPUT_NODES
	inline size_t available() const { return MAX_OUTPUT_NODES - count; }
	inline size_t sizeData()  const { return count; } // how many impulses were stored

        inline const Time& getTime() const { return time; } // for checking if the packet is too old // time is set in write()
	inline ClusterID getSrcClusterId () const { return srcId; }
	inline void setSrcClusterId(const ClusterID cid) { srcId = cid; }
	inline void prepareRead(){ time = TimeSync::toLocal(time); } // convert packet's network time to local cluster time
	inline TimeBuffer(): srcId(0), time(std::chrono::high_resolution_clock::time_point::min()), count(0) { }

	inline int read(size_t index, Time& timeOutParam){
		int16_t dtime = dt[index];
		timeOutParam = time + std::chrono::microseconds(dtime);
		return idx[index]; // node index on parent cluster
	}

	// return -1 if delta time exceeds 16 bit or if MAX_OUTPUT_NODES have been written
	inline int write(uint16_t index, const Time& t){
		if(index > 0xFFFF){ throw "Node index " + std::to_string(index) + " exceeds maximum network transmission size of 0xFFFF"; }
		int deltaTus = 0;
		if( time == std::chrono::high_resolution_clock::time_point::min() ){
		    time = t;
		} else {
		    deltaTus = std::chrono::duration_cast<std::chrono::microseconds>(t - time).count();
		    if( deltaTus < std::numeric_limits<int16_t>::min() ){ return 0; } // throw away old impulses
                }
		if(count >= MAX_OUTPUT_NODES || deltaTus > std::numeric_limits<int16_t>::max() ){
			// this could have a nasty side effect if writer calls write() on a full buffer several times
			time = TimeSync::toNetwork(time); // prepare this packet to be sent: convert local time to network time
			return -1; // could not write this impulse to buffer
		}
		idx[count] = index;
		dt [count] = deltaTus;
		return ++count;
	}
};


typedef std::shared_ptr<TimeBuffer> TBPtr; // define a common Pointer type used for handling all TimeBuffers


#endif // INCLUDE_TIMEBUFF_H
