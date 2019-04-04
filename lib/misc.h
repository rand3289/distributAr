#ifndef INCLUDED_MISC_H
#define INCLUDED_MISC_H

#include "icluster.h"
#include "main.h" // Time and IP
#include <memory> // shared_ptr
#include <ostream>
#include <iostream>
#include <cstring> // strlen()
#include <thread>
#include <sstream>


std::shared_ptr<ICluster> loadDll(std::string& dllName);
void boostPriority(std::thread& t);
std::ostream& operator<<(std::ostream& out, const Time& t);
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::nano>& );
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::micro>& );
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::milli>& );
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::ratio<1>>& );
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::ratio<60>>& );
std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, std::ratio<3600>>& );
// template <typename Rep>std::ostream& operator<<(std::ostream& out, const std::chrono::duration<long int, Rep>& t);

inline std::string ipStr(IP ip){
    std::stringstream out;
    // TODO: make it portable
    // out << (ip>>24) << "." << ((ip>>16)&0xFF) << "." << ((ip>>8)&0xFF) << "." << (ip&0xFF); // bit endian
    out << (ip&0xFF) << "." << ((ip>>8)&0xFF) << "." << ((ip>>16)&0xFF) << "." << (ip>>24); // little endian
    return out.str();
}


//inline std::ostream& operator<<(std::ostream& out, IP ip){
//    out << (int)(ip>>24) << '.' << ((int)(ip>>16)&0xFF) << '.' << ((int)(ip>>8)&0xFF) << '.' << (int)(ip&0xFF);
//    return out;
//}


template<typename T, typename M>	// mod can be an enum, int, short
inline bool modLess(T a, T b, M mod){	// compare two numbers a and b given b=(a+x)%mod, for example a = 9, b=0, mod=10 then b>a
	T half = mod/2;
	if(a < half){
		return b<half ? a<b : a+half<b;  // last case:  0----a-----HALF-----b----mod
	} else {
		return b>=half ? a<b : b+half<a; // last case:  0----b-----HALF-----a----mod
	}
}


inline int getMs(Time t){
	unsigned long long ms = std::chrono::duration_cast<std::chrono::milliseconds>( t.time_since_epoch() ).count();
	return ms % std::numeric_limits<int>::max(); // change epoch instead???
}


inline IP localIP(){
	struct hostent* he = gethostbyname("127.0.0.1");
	if(!he){
		std::cerr << "ERROR: Can not resolve local address!!!" << std::endl;
        	return 0;
    	}
	IP ip = *reinterpret_cast<IP*>(he->h_addr_list[0]);
	return ip;
}

// function might still come back with 127.0.0.1 if replacement is 127.0.0.1
inline IP replaceLo(IP ip, IP replacement){
	const static IP lip = localIP();
	return ip == lip ? replacement : ip;
}


inline void spinner(){
	const static char* syms = "/~\\|";
	const static size_t mod = strlen(syms); // sizeof(syms)
	static size_t idx = 0;
	std::cout << "\b\b\b " << syms[idx++%mod] << ' ';
}

// inline int bitsToBytes(int bits){ return bits/8 + (bits%8 ? 1 : 0); }

#endif // INCLUDED_MISC_H
