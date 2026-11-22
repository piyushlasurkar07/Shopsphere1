#ifndef SHOPSPHERE_LRU_CACHE_H
#define SHOPSPHERE_LRU_CACHE_H

/**
 * ============================================================================
 * ShopSphere - LRU Cache using Hash Map and Doubly Linked List
 * ============================================================================
 * 
 * ALGORITHMIC ANALYSIS:
 * 
 * 1. Data Structure Architecture:
 *    - Hash Map (std::unordered_map<Key, CacheNode*>):
 *      Provides average O(1) key-to-node pointer lookup.
 *    - Doubly Linked List (Custom Doubly Linked List with Head and Tail sentinels):
 *      Allows O(1) node detachment, O(1) insertion at head (most recently used),
 *      and O(1) eviction of the tail node (least recently used).
 * 
 * 2. Time Complexity:
 *    - get(key): Average O(1).
 *      Map lookup O(1) + node detachment O(1) + move to head O(1).
 *    - put(key, value): Average O(1).
 *      If key exists: update value and move to head in O(1).
 *      If key is new: create node, insert at head in O(1). If capacity exceeded,
 *      remove tail node (LRU item) and erase from map in O(1).
 *    - remove(key) / invalidate(key): Average O(1).
 *    - clear(): O(N) to deallocate all nodes.
 * 
 * 3. Space Complexity:
 *    - O(Capacity): Exact upper bound on memory usage. Cache never exceeds capacity.
 * ============================================================================
 */

#include "utils/SpinLock.h"
#include <unordered_map>
#include <cstddef>

namespace ShopSphere {
namespace Algorithms {

struct CacheStats {
    size_t hits{0};
    size_t misses{0};
    size_t evictions{0};
    size_t size{0};
    size_t capacity{0};
    double hitRatio{0.0};
};

template <typename Key, typename Value>
class LRUCache {
private:
    struct CacheNode {
        Key key;
        Value value;
        CacheNode *prev{nullptr};
        CacheNode *next{nullptr};

        CacheNode() = default;
        CacheNode(const Key &k, const Value &v) : key(k), value(v) {}
    };

    size_t capacity_;
    size_t size_{0};
    CacheNode *head_{nullptr}; // Sentinel head: most recently used is head_->next
    CacheNode *tail_{nullptr}; // Sentinel tail: least recently used is tail_->prev

    std::unordered_map<Key, CacheNode*> map_;
    mutable Utils::SpinLock lock_;

    // Telemetry statistics
    mutable size_t hits_{0};
    mutable size_t misses_{0};
    size_t evictions_{0};

    // Detach an existing node from its current position
    void detachNode(CacheNode *node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    // Attach node immediately after sentinel head (marking it MRU)
    void attachToHead(CacheNode *node) {
        node->next = head_->next;
        node->prev = head_;
        head_->next->prev = node;
        head_->next = node;
    }

    // Evict least recently used node (immediately before sentinel tail)
    void evictLRU() {
        if (size_ == 0) return;
        CacheNode *lru = tail_->prev;
        detachNode(lru);
        map_.erase(lru->key);
        delete lru;
        size_--;
        evictions_++;
    }

public:
    explicit LRUCache(size_t capacity = 100) : capacity_(capacity), size_(0) {
        head_ = new CacheNode();
        tail_ = new CacheNode();
        head_->next = tail_;
        tail_->prev = head_;
    }

    ~LRUCache() {
        clear();
        delete head_;
        delete tail_;
    }

    // Disable copy
    LRUCache(const LRUCache &) = delete;
    LRUCache &operator=(const LRUCache &) = delete;

    // Retrieve an item from the cache. Updates recency on hit.
    bool get(const Key &key, Value &value) {
        Utils::SpinLockGuard guard(lock_);
        auto it = map_.find(key);
        if (it == map_.end()) {
            misses_++;
            return false;
        }

        CacheNode *node = it->second;
        detachNode(node);
        attachToHead(node); // Move to head as MRU
        value = node->value;
        hits_++;
        return true;
    }

    // Insert or update an item in the cache
    void put(const Key &key, const Value &value) {
        Utils::SpinLockGuard guard(lock_);
        if (capacity_ == 0) return;

        auto it = map_.find(key);
        if (it != map_.end()) {
            // Key already exists: update value and promote to head
            CacheNode *node = it->second;
            node->value = value;
            detachNode(node);
            attachToHead(node);
            return;
        }

        // Evict LRU if at capacity
        if (size_ >= capacity_) {
            evictLRU();
        }

        // Create new node and attach to head
        CacheNode *newNode = new CacheNode(key, value);
        attachToHead(newNode);
        map_[key] = newNode;
        size_++;
    }

    // Invalidate/remove an item when modified or deleted
    bool remove(const Key &key) {
        Utils::SpinLockGuard guard(lock_);
        auto it = map_.find(key);
        if (it == map_.end()) return false;

        CacheNode *node = it->second;
        detachNode(node);
        map_.erase(it);
        delete node;
        size_--;
        return true;
    }

    // Check if key exists without altering recency
    bool contains(const Key &key) const {
        Utils::SpinLockGuard guard(lock_);
        return map_.find(key) != map_.end();
    }

    // Clear all cached entries
    void clear() {
        Utils::SpinLockGuard guard(lock_);
        CacheNode *curr = head_->next;
        while (curr != tail_) {
            CacheNode *next = curr->next;
            delete curr;
            curr = next;
        }
        head_->next = tail_;
        tail_->prev = head_;
        map_.clear();
        size_ = 0;
    }

    // Get cache telemetry statistics
    CacheStats getStats() const {
        Utils::SpinLockGuard guard(lock_);
        CacheStats stats;
        stats.hits = hits_;
        stats.misses = misses_;
        stats.evictions = evictions_;
        stats.size = size_;
        stats.capacity = capacity_;
        size_t total = hits_ + misses_;
        stats.hitRatio = (total > 0) ? (static_cast<double>(hits_) / total) : 0.0;
        return stats;
    }

    size_t size() const {
        Utils::SpinLockGuard guard(lock_);
        return size_;
    }

    size_t capacity() const {
        return capacity_;
    }
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_LRU_CACHE_H
