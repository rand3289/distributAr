#ifndef INCLUDED_BLOCKQ_H
#define INCLUDED_BLOCKQ_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <assert.h>


// Blocking queue based on C++11 condition variable
template <typename T>
class BlockQ {
	std::mutex m;
	std::condition_variable cv;
	std::vector<T> data;
public:

	// it is the responsibility of the owner to make sure other threads are done using the BlockQ
	virtual ~BlockQ(){
		std::lock_guard<std::mutex> lock(m);
		data.clear();
	}

	void push(const T& obj){
		{
			std::lock_guard<std::mutex> lock(m);
			data.push_back(obj);
		}
		cv.notify_one();
	}

	bool pop(T& obj){
		const static std::chrono::seconds timeout(10);
		std::unique_lock<std::mutex> ulock(m);
		try {
			while( 0 == cv.wait_for(ulock, timeout, std::bind(&std::vector<T>::size,&data)) ); // wait_for() returns size
		} catch(...){  // if you woke up here with SIGSEGV this object was deleted :)
			std::cerr << "CAUGHT EXEPTION" << std::endl;
			return false;
		}
		std::cout << "POP" << std::endl; std::cout.flush();
		obj = data.back();
		data.pop_back();
		return true;
	}

	bool popNoWait(T& obj){
		std::lock_guard<std::mutex> lock(m);
		if ( 0 == data.size() ){
			return false;
		}
		obj = data.back();
		data.pop_back();
		return true;
	}

	size_t size(){
		std::lock_guard<std::mutex> lock(m);
		return data.size();
	}
};

#endif
