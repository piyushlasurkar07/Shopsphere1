#include "algorithms/OrderPriorityQueue.h"
#include <iostream>
#include <cassert>

using namespace ShopSphere::Algorithms;

int main() {
    std::cout << "[RUNNING] OrderPriorityQueueTest..." << std::endl;

    OrderPriorityQueue queue;

    // Test 1: Empty queue behavior
    assert(queue.empty());
    assert(queue.size() == 0);
    OrderPriorityItem dummy;
    assert(!queue.pop(dummy));
    assert(!queue.peek(dummy));
    std::cout << "  ✓ Test 1: Empty queue returns false safely." << std::endl;

    // Test 2: Enqueue actual customer orders with different attributes
    auto now = std::chrono::system_clock::now();

    OrderPriorityItem o1;
    o1.orderId = 1001;
    o1.orderNumber = "SS-1001";
    o1.customerName = "Customer Normal";
    o1.totalAmount = 1500.0;
    o1.isUrgent = false;
    o1.createdAt = now;

    OrderPriorityItem o2;
    o2.orderId = 1002;
    o2.orderNumber = "SS-1002";
    o2.customerName = "Customer Urgent";
    o2.totalAmount = 2000.0;
    o2.isUrgent = true; // Urgent shipping
    o2.createdAt = now;

    OrderPriorityItem o3;
    o3.orderId = 1003;
    o3.orderNumber = "SS-1003";
    o3.customerName = "Customer High Value";
    o3.totalAmount = 25000.0; // High value order
    o3.isUrgent = false;
    o3.createdAt = now;

    queue.push(o1);
    queue.push(o2);
    queue.push(o3);

    assert(queue.size() == 3);
    assert(!queue.empty());
    std::cout << "  ✓ Test 2: Orders enqueued into binary max-heap." << std::endl;

    // Test 3: Priority extraction (highest priority order first)
    // Urgent order (o2) gets +1000 pts bonus, so it must pop first
    OrderPriorityItem firstProcessed;
    assert(queue.pop(firstProcessed));
    assert(firstProcessed.orderId == 1002);
    assert(firstProcessed.isUrgent);
    std::cout << "  ✓ Test 3: Urgent order processed first with ID: " << firstProcessed.orderId << std::endl;

    // Next highest is High Value order (o3) with 25000 amount
    OrderPriorityItem secondProcessed;
    assert(queue.pop(secondProcessed));
    assert(secondProcessed.orderId == 1003);
    std::cout << "  ✓ Test 4: High value order processed second with ID: " << secondProcessed.orderId << std::endl;

    // Test 5: Order cancellation
    queue.push(o1);
    assert(queue.contains(1001));
    bool cancelled = queue.cancelOrder(1001);
    assert(cancelled);
    assert(!queue.contains(1001));
    std::cout << "  ✓ Test 5: Cancelled order removed cleanly from heap." << std::endl;

    std::cout << "[PASSED] OrderPriorityQueueTest completed successfully!" << std::endl;
    return 0;
}
