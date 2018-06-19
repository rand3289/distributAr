#ifndef INCLUDED_ATOMQ_H
#define INCLUDED_ATOMQ_H

#include "guard.h"
#include <vector>
#include <iostream>


template <typename T> class PrivateQ;
// this class provides a thread safe stack (LIFO) that can be used as a queue.
// it is implemented this way because operations on a vector are faster than on a list
template <typename T> // T = shared_ptr<???>
class Q {
protected:
    std::vector<T>* data;
    friend PrivateQ<T>;
    size_t maxSize;
public:
    inline Q(int maxDepth): data( new std::vector<T>() ), maxSize(maxDepth) {}
//    size_t capacity(){ return data ? maxSize-data->size() : 0; }

    virtual ~Q(){
        Guard<std::vector<T> > owner(data);
	if( owner() ){
	    owner()->clear();
	    owner.free();
	}
    }

    inline bool push(T& obj) {
        Guard<std::vector<T> > owner(data);
	if( !owner() ) {
	    return false;
	}
	if( owner()->size() > maxSize ){
	    std::cout << ']';
	    return false;
	}
	owner()->push_back(obj);
	return true;
    }

    inline bool pop(T& obj){
        Guard<std::vector<T> > owner(data);
        if( !owner() || owner()->empty() ){
	    return false;
        }
        obj = std::move(owner()->back());
        owner()->pop_back();
        return true;
    }
}; // Q


// Instances of this type can not be shared between threads
template <typename T>
class PrivateQ: public Q<T> {
public:
    PrivateQ(): Q<T>(0xFFFFFFFF) {}

    // these methods will perform MT safe operations on thread safe class Q
    bool swap(Q<T>& with) {
	Guard<std::vector<T> > owner(with.data);
	if(!owner()){
	    return false;
	}
	owner()->swap(*(this->data));
	return true;
    }

    bool copyTo(Q<T>& dest) {
	Guard<std::vector<T> > owner(dest.data);
	if(!owner()){
	    return false;
	}
	size_t capacity = dest.maxSize - owner()->size();
	if( capacity >= this->data->size() ){ // respect destination capacity - does it fit?
	    owner()->insert(owner()->begin(), this->data->begin(), this->data->end());
	} else {
//	    std::cout << '}';
	    while(capacity){
		T& obj = (*this->data)[--capacity];
		owner()->push_back(obj);
	    }
	}
	return true;
    }

    bool copyFrom(Q<T>& from) {
	Guard<std::vector<T> > owner(from.data);
	if(!owner()){
	    return false;
	}
	this->data->insert(this->data->end(), owner()->begin(), owner()->end() );
	return true;
    }

    // These methods are NOT MT safe since the instance of this class can NOT be shared among threads
    inline void clear(){ this->data->clear(); }
    inline void add(T& obj) { this->data->push_back(obj); }
    inline void pop()       { this->data->pop_back();     }
//    inline const T& top() const { return this->data->back();  }
    inline size_t size() const { return this->data->size();  }
    inline bool empty()  const { return this->data->empty(); }
};


#endif // INCLUDED_ATOMQ_H
