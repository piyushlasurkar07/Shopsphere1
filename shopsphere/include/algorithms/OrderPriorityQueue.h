#ifndef SHOPSPHERE_ORDER_PRIORITY_QUEUE_H
#define SHOPSPHERE_ORDER_PRIORITY_QUEUE_H

#include "utils/SpinLock.h"
#include <vector>
#include <string>
#include <chrono>

namespace ShopSphere {
namespace Algorithms {

struct OrderPriorityItem {
    int orderId{0};
    std::string orderNumber;
    int userId{0};
    std::string customerName;
    double totalAmount{0.0};
    bool isUrgent{false};
    std::chrono::system_clock::time_point createdAt;
    double calculatedPriority{0.0};

    void computePriorityScore();

    bool operator<(const OrderPriorityItem &other) const {
        return calculatedPriority < other.calculatedPriority;
    }

    bool operator>(const OrderPriorityItem &other) const {
        return calculatedPriority > other.calculatedPriority;
    }
};

class OrderPriorityQueue {
public:
    OrderPriorityQueue() = default;
    ~OrderPriorityQueue() = default;

    void push(const OrderPriorityItem &item);
    bool pop(OrderPriorityItem &outItem);
    bool peek(OrderPriorityItem &outItem) const;
    bool cancelOrder(int orderId);
    bool contains(int orderId) const;
    std::vector<OrderPriorityItem> getQueueSnapshot() const;
    size_t size() const;
    bool empty() const;
    void clear();

private:
    std::vector<OrderPriorityItem> heap_;
    mutable Utils::SpinLock lock_;

    void heapifyUp(size_t index);
    void heapifyDown(size_t index);
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_ORDER_PRIORITY_QUEUE_H
