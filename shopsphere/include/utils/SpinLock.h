#ifndef SHOPSPHERE_SPIN_LOCK_H
#define SHOPSPHERE_SPIN_LOCK_H

#include <atomic>

namespace ShopSphere {
namespace Utils {

class SpinLock {
public:
    SpinLock() : flag_(ATOMIC_FLAG_INIT) {}

    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // Spin-wait
        }
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_;
};

class SpinLockGuard {
public:
    explicit SpinLockGuard(SpinLock &lock) : lock_(lock) {
        lock_.lock();
    }
    ~SpinLockGuard() {
        lock_.unlock();
    }

private:
    SpinLock &lock_;
};

} // namespace Utils
} // namespace ShopSphere

#endif // SHOPSPHERE_SPIN_LOCK_H
