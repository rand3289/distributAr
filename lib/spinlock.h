#ifndef INCLUDED_SPINLOCK_H
#define INCLUDED_SPINLOCK_H

#include <atomic>

// from https://en.cppreference.com/w/cpp/atomic/atomic_flag

class SpinLock { // for use with std::lock_guard
    std::atomic_flag lockf = ATOMIC_FLAG_INIT;
public:
    SpinLock(){}
    SpinLock(const SpinLock& ) = delete;
    SpinLock(const SpinLock&&) = delete;

    void lock() {
        while (lockf.test_and_set(std::memory_order_acquire)) /*nop*/; // acquire lock or spin
    }

    bool try_lock() {
        return !lockf.test_and_set(std::memory_order_acquire); // acquire lock once
    }

    void unlock() {
        lockf.clear(std::memory_order_release);                // release lock
    }
};

#endif // INCLUDED_SPINLOCK_H
