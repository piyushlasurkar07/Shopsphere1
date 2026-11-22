#include "algorithms/OrderPriorityQueue.h"
#include <algorithm>

namespace ShopSphere {
namespace Algorithms {

void OrderPriorityItem::computePriorityScore() {
    double score = 0.0;
    if (isUrgent) {
        score += 1000.0; // Express / Urgent shipping bonus
    }

    // High value order tier
    score += std::min(totalAmount * 0.1, 500.0);

    // Aging factor: minutes since order was created
    auto now = std::chrono::system_clock::now();
    auto elapsedMinutes = std::chrono::duration_cast<std::chrono::minutes>(now - createdAt).count();
    score += static_cast<double>(elapsedMinutes) * 0.5;

    calculatedPriority = score;
}

void OrderPriorityQueue::push(const OrderPriorityItem &item) {
    Utils::SpinLockGuard guard(lock_);
    OrderPriorityItem copy = item;
    copy.computePriorityScore();

    heap_.push_back(copy);
    heapifyUp(heap_.size() - 1);
}

bool OrderPriorityQueue::pop(OrderPriorityItem &outItem) {
    Utils::SpinLockGuard guard(lock_);
    if (heap_.empty()) return false;

    outItem = heap_[0];
    heap_[0] = heap_.back();
    heap_.pop_back();

    if (!heap_.empty()) {
        heapifyDown(0);
    }
    return true;
}

bool OrderPriorityQueue::peek(OrderPriorityItem &outItem) const {
    Utils::SpinLockGuard guard(lock_);
    if (heap_.empty()) return false;
    outItem = heap_[0];
    return true;
}

bool OrderPriorityQueue::cancelOrder(int orderId) {
    Utils::SpinLockGuard guard(lock_);
    bool removedAny = false;
    for (size_t i = 0; i < heap_.size(); ) {
        if (heap_[i].orderId == orderId) {
            heap_[i] = heap_.back();
            heap_.pop_back();
            if (i < heap_.size()) {
                heapifyDown(i);
                heapifyUp(i);
            }
            removedAny = true;
        } else {
            ++i;
        }
    }
    return removedAny;
}

bool OrderPriorityQueue::contains(int orderId) const {
    Utils::SpinLockGuard guard(lock_);
    for (const auto &item : heap_) {
        if (item.orderId == orderId) return true;
    }
    return false;
}

std::vector<OrderPriorityItem> OrderPriorityQueue::getQueueSnapshot() const {
    Utils::SpinLockGuard guard(lock_);
    std::vector<OrderPriorityItem> snapshot = heap_;
    // Sort descending by calculated priority
    std::sort(snapshot.begin(), snapshot.end(), [](const OrderPriorityItem &a, const OrderPriorityItem &b) {
        return a.calculatedPriority > b.calculatedPriority;
    });
    return snapshot;
}

size_t OrderPriorityQueue::size() const {
    Utils::SpinLockGuard guard(lock_);
    return heap_.size();
}

bool OrderPriorityQueue::empty() const {
    Utils::SpinLockGuard guard(lock_);
    return heap_.empty();
}

void OrderPriorityQueue::clear() {
    Utils::SpinLockGuard guard(lock_);
    heap_.clear();
}

void OrderPriorityQueue::heapifyUp(size_t index) {
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (heap_[index] > heap_[parent]) {
            std::swap(heap_[index], heap_[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

void OrderPriorityQueue::heapifyDown(size_t index) {
    size_t size = heap_.size();
    while (true) {
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t largest = index;

        if (left < size && heap_[left] > heap_[largest]) {
            largest = left;
        }
        if (right < size && heap_[right] > heap_[largest]) {
            largest = right;
        }

        if (largest != index) {
            std::swap(heap_[index], heap_[largest]);
            index = largest;
        } else {
            break;
        }
    }
}

} // namespace Algorithms
} // namespace ShopSphere
