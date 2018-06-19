#ifndef INCLUDED_INDEXER_H
#define INCLUDED_INDEXER_H

#include "serial.h"
#include <unordered_map>


// turn a sparce T into a dense id  For example 1,99,5 -> 1,2,3 or a,z,b -> 1,2,3
template <typename T>
class Indexer {
    std::unordered_map<T, int> index;
public:
    int get(T val){
        int& idx = index[val];
        if(0==idx){
	    idx=index.size(); // reference
        }
	return idx;
    }

    int serialize(std::ostream& os) const {
        return ::serialize(index, os);
    }

    int deserialize(std::istream& is){
        return ::deserialize(index, is);
    }
};


#endif // INCLUDED_INDEXER_H
