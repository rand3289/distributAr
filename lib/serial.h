#ifndef INCLUDED_SERIAL_H
#define INCLUDED_SERIAL_H
#include <ostream>
#include <istream>
#include <unordered_map>
#include <set>
#include <vector>


template <typename T>
int serialize(const T& t, std::ostream& os){ return t.serialize(os); }

inline int serialize(const int t, std::ostream& os){ os << t << " "; return os.good(); }
inline int serialize(const unsigned int t, std::ostream& os){ os << t << " ";  return os.good(); }


template <typename T>
int serialize(const std::set<T>& s, std::ostream& os){
    os << s.size() << " ";
    for(auto it = s.begin(); it!= s.end(); ++it){
        serialize(*it, os);
    }
    return os.good();
}


template <typename T>
int serialize(const std::vector<T>& v, std::ostream& os){
    os << v.size() << " ";
    for(auto it = v.begin(); it!= v.end(); ++it){
        serialize(*it, os);
    }
    return os.good();
}


template <typename T1, typename T2>
int serialize(const std::unordered_map<T1,T2>& m, std::ostream& os){
    os << m.size() << " ";
    for(auto it = m.begin(); it!= m.end(); ++it){
        serialize(it->first, os);
        serialize(it->second, os);
    }
    return os.good();
}


/******************************* DE ****************************************/
template <typename T>
int deserialize(T& t, std::istream& is){ return t.deserialize(is); }

inline int deserialize(int& outInt, std::istream& is){ is >> outInt; return is.good(); }
inline int deserialize(unsigned int& outInt, std::istream& is){ is >> outInt; return is.good(); }


// generic for containers
template <typename T>
int deserialize(std::insert_iterator<T> it, std::istream& is){
    size_t size;
    is >> size;
    typename T::value_type t;
    for(size_t i = 0; i<size; ++i){
        deserialize(t, is);
        *it=t;
    }
    return is.good();
}


template <typename T>
int deserialize(std::set<T>& s, std::istream& is){
  return deserialize(std::inserter(s,s.end()), is);
}


template <typename T>
int deserialize(std::vector<T>& v, std::istream& is){
  return deserialize(std::inserter(v,v.end()), is);
}


template <typename K, typename V>
int deserialize(std::unordered_map<K,V>& m, std::istream& is){
    K k;
    V v;
    size_t size;
    is >> size;
    for(size_t i = 0; i<size; ++i){
	deserialize(k, is);
        deserialize(v, is);
        m[k] = v;
    }
    return is.good();
}


#endif
