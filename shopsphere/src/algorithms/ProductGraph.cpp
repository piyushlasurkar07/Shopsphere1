#include "algorithms/ProductGraph.h"
#include "utils/StringUtils.h"
#include <algorithm>
#include <queue>
#include <cmath>

namespace ShopSphere {
namespace Algorithms {

void ProductGraph::addProductNode(int productId, int categoryId, const std::string &brand, double price, int stock) {
    ProductMeta meta;
    meta.id = productId;
    meta.categoryId = categoryId;
    meta.brand = Utils::toLower(Utils::trim(brand));
    meta.price = price;
    meta.stock = stock;
    meta.active = true;
    products_[productId] = meta;

    // Dynamically calculate attribute edges with existing nodes in the same category or brand
    for (const auto &pair : products_) {
        int otherId = pair.first;
        if (otherId == productId || !pair.second.active) continue;

        double weight = 0.0;
        if (pair.second.categoryId == categoryId) {
            weight += 3.0; // Same category affinity
        }
        if (!brand.empty() && pair.second.brand == meta.brand) {
            weight += 2.0; // Same brand affinity
        }
        if (meta.price > 0 && pair.second.price > 0) {
            double priceDiff = std::abs(meta.price - pair.second.price);
            double maxP = std::max(meta.price, pair.second.price);
            if (maxP > 0) {
                weight += std::max(0.0, 1.5 * (1.0 - (priceDiff / maxP)));
            }
        }

        if (weight > 0.0) {
            addEdge(productId, otherId, weight, "attribute");
            addEdge(otherId, productId, weight, "attribute");
        }
    }
}

void ProductGraph::removeProductNode(int productId) {
    products_.erase(productId);
    adjList_.erase(productId);

    // Remove edges pointing to this product from other nodes
    for (auto &pair : adjList_) {
        auto &edges = pair.second;
        edges.erase(std::remove_if(edges.begin(), edges.end(), [productId](const GraphEdge &e) {
            return e.targetId == productId;
        }), edges.end());
    }
}

void ProductGraph::setStock(int productId, int stock) {
    auto it = products_.find(productId);
    if (it != products_.end()) {
        it->second.stock = stock;
    }
}

void ProductGraph::addEdge(int fromId, int toId, double weight, const std::string &type) {
    if (fromId == toId) return;

    auto &edges = adjList_[fromId];
    for (auto &edge : edges) {
        if (edge.targetId == toId) {
            if (type == "copurchase") {
                edge.coPurchaseCount++;
                edge.weight += weight;
            } else if (type == "coview") {
                edge.coViewCount++;
                edge.weight += weight;
            } else {
                edge.weight = std::max(edge.weight, weight);
            }
            return;
        }
    }

    GraphEdge newEdge;
    newEdge.targetId = toId;
    newEdge.weight = weight;
    if (type == "copurchase") newEdge.coPurchaseCount = 1;
    if (type == "coview") newEdge.coViewCount = 1;
    edges.push_back(newEdge);
}

void ProductGraph::recordCoPurchase(int productA, int productB) {
    if (productA == productB) return;
    // High affinity multiplier (+5.0) for actual items purchased together
    addEdge(productA, productB, 5.0, "copurchase");
    addEdge(productB, productA, 5.0, "copurchase");
}

void ProductGraph::recordCoView(int productA, int productB) {
    if (productA == productB) return;
    // Moderate affinity multiplier (+1.5) for items browsed in the same session
    addEdge(productA, productB, 1.5, "coview");
    addEdge(productB, productA, 1.5, "coview");
}

std::vector<RecommendedProduct> ProductGraph::getRecommendations(int productId, 
                                                                 size_t limit, 
                                                                 const std::unordered_set<int> &excludeIds) const {
    std::vector<RecommendedProduct> recommendations;
    auto it = products_.find(productId);
    if (it == products_.end() || !it->second.active) {
        return recommendations;
    }

    auto adjIt = adjList_.find(productId);
    if (adjIt == adjList_.end() || adjIt->second.empty()) {
        return recommendations;
    }

    // Max-Heap priority queue to rank by affinity score
    struct QueueEntry {
        int id;
        double score;
        std::string reason;

        bool operator<(const QueueEntry &other) const {
            return score < other.score; // Max-heap: highest score first
        }
    };

    std::priority_queue<QueueEntry> pq;
    std::unordered_set<int> visited;
    visited.insert(productId);

    for (const auto &edge : adjIt->second) {
        int targetId = edge.targetId;
        if (excludeIds.count(targetId) || visited.count(targetId)) continue;

        auto targetMeta = products_.find(targetId);
        if (targetMeta == products_.end() || !targetMeta->second.active) continue;
        if (targetMeta->second.stock <= 0) continue; // Skip out of stock products

        std::string reason = "Similar Product";
        if (edge.coPurchaseCount > 0) {
            reason = "Frequently Bought Together (" + std::to_string(edge.coPurchaseCount) + "x)";
        } else if (edge.coViewCount > 0) {
            reason = "Customers Also Viewed";
        } else if (targetMeta->second.categoryId == it->second.categoryId) {
            reason = "Same Category";
        }

        pq.push({targetId, edge.weight, reason});
        visited.insert(targetId);
    }

    while (!pq.empty() && recommendations.size() < limit) {
        auto top = pq.top();
        pq.pop();
        recommendations.push_back({top.id, top.score, top.reason});
    }

    return recommendations;
}

void ProductGraph::clear() {
    products_.clear();
    adjList_.clear();
}

size_t ProductGraph::nodeCount() const {
    return products_.size();
}

size_t ProductGraph::edgeCount() const {
    size_t count = 0;
    for (const auto &pair : adjList_) {
        count += pair.second.size();
    }
    return count;
}

} // namespace Algorithms
} // namespace ShopSphere
