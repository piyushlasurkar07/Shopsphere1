#include "algorithms/LRUCache.h"
#include <iostream>
#include <string>
#include <cassert>

using namespace ShopSphere::Algorithms;

int main() {
    std::cout << "[RUNNING] LRUCacheTest..." << std::endl;

    // Test with capacity 3
    LRUCache<int, std::string> cache(3);

    // Test 1: Empty cache
    assert(cache.size() == 0);
    assert(cache.capacity() == 3);
    std::string val;
    assert(!cache.get(1, val));
    std::cout << "  ✓ Test 1: Empty cache lookup miss verified." << std::endl;

    // Test 2: Put and Get
    cache.put(1, "Product 1 Details");
    cache.put(2, "Product 2 Details");
    cache.put(3, "Product 3 Details");

    assert(cache.size() == 3);
    assert(cache.get(1, val) && val == "Product 1 Details");
    assert(cache.get(2, val) && val == "Product 2 Details");
    std::cout << "  ✓ Test 2: Insertion and retrieval verified." << std::endl;

    // Test 3: Eviction when capacity exceeded
    // At this point, recency order (MRU to LRU): 2, 1, 3.
    // Putting product 4 should evict the least recently used, which is 3!
    cache.put(4, "Product 4 Details");
    assert(cache.size() == 3);
    assert(!cache.get(3, val)); // Product 3 was evicted!
    assert(cache.get(4, val) && val == "Product 4 Details");
    assert(cache.get(1, val) && val == "Product 1 Details");
    std::cout << "  ✓ Test 3: O(1) LRU eviction correctly evicted key 3." << std::endl;

    // Test 4: Invalidation upon product update
    cache.remove(1);
    assert(cache.size() == 2);
    assert(!cache.get(1, val));
    std::cout << "  ✓ Test 4: Cache invalidation verified." << std::endl;

    // Test 5: Cache Telemetry
    auto stats = cache.getStats();
    assert(stats.hits > 0);
    assert(stats.misses > 0);
    assert(stats.evictions == 1);
    assert(stats.hitRatio > 0.0);
    std::cout << "  ✓ Test 5: Cache telemetry verified: hits=" << stats.hits 
              << ", misses=" << stats.misses << ", ratio=" << stats.hitRatio << std::endl;

    std::cout << "[PASSED] LRUCacheTest completed successfully!" << std::endl;
    return 0;
}
