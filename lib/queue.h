#ifndef INCLUDED_RINGBUFF_H
#define INCLUDED_RINGBUFF_H

#include <atomic>
#include <cstdint> // uint16_t
//#include <cstddef> // size_t
//#include <mutex>   // lock_guard
//#include "spinlock.h"

// one producer, one consumer fixed size lock free queue.
template < typename T, uint16_t SIZE >
class FreeQueue {
	std::atomic<std::uint16_t> ain{0};
	T buff[SIZE];
	std::atomic<std::uint16_t> aout{0};
public:
	bool push(const T& val) {
	    auto in = ain.load();
	    auto nextIn = (in+1) % SIZE;
	    auto out = aout.load();
	    if(nextIn == out)  { return false; } // full
	    buff[in] = val;
	    ain.store(nextIn);
	    return true;
	}

	bool pop(T& val) {
	    auto out = aout.load();
	    auto in  = ain.load();
	    if( in == out ) { return false; } // empty
            val = buff[out];
	    out = (out+1) % SIZE;
	    aout.store(out);
	    return true;
	}
};

/*
// many producer, many consumer fixed size spin lock queue.
template < typename T, size_t SIZE >
class SpinQueue {
	unsigned int in  = 0;
	unsigned int out = 0;
	SpinLock lock;
	T buff[SIZE];
public:
	bool push(const T& val) {
	    std::lock_guard<SpinLock> guard(lock);
	    const unsigned int nextIn = (in+1) % SIZE;
	    if(nextIn == out)  { return false; } // full
	    buff[in] = val;
	    in = nextIn;
	    return true;
	}

	bool pop(T& val) {
	    std::lock_guard<SpinLock> guard(lock);
	    if( in == out ) { return false; } // empty
            val = buff[out];
	    out = (out+1) % SIZE;
	    return true;
	}
};
*/

#endif  // INCLUDED_RINGBUFF_H
