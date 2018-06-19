#ifndef INCLUDED_GUARD_H
#define INCLUDED_GUARD_H


template <typename T>
class Guard {
    T*& owner;
    T* locked;
    const T* LOCKEDINDICATOR;
public:
    explicit inline Guard(T*& guarded): owner(guarded), locked(reinterpret_cast<T*>(1)), LOCKEDINDICATOR(locked) {
	do { // __atomic are gcc specific extensions
	    locked = __atomic_exchange_n(&owner, locked, __ATOMIC_SEQ_CST); // get ownership
	} while( locked == LOCKEDINDICATOR );
    }

    inline ~Guard() {
	locked = __atomic_exchange_n(&owner, locked, __ATOMIC_SEQ_CST); // return ownership
    }

    inline T* operator()() { return locked; }
    inline void free() { delete locked; locked = 0; } // in destructor null will be stored in original pointer
};


#endif // INCLUDED_GUARD_H
