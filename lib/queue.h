#ifndef INCLUDED_FREE_QUEUE_H
#define INCLUDED_FREE_QUEUE_H

#include <atomic>
#include <cstdint> // uint16_t

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

#endif  // INCLUDED_
