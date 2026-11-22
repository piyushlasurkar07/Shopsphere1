#ifndef SHOPSPHERE_PRODUCT_GRAPH_H
#define SHOPSPHERE_PRODUCT_GRAPH_H

/**
 * ============================================================================
 * ShopSphere - Graph-Based Product Recommendation Engine
 * ============================================================================
 * 
 * ALGORITHMIC ANALYSIS:
 * 
 * 1. Graph Representation:
 *    - Weighted Adjacency List: std::unordered_map<int, std::vector<GraphEdge>>
 *    - Each product is a Node (V) identified by its product ID.
 *    - Edges (E) represent relational affinity between product pairs:
 *      * Category affinity (+3.0 base weight)
 *      * Brand affinity (+2.0 base weight)
 *      * Price tier proximity (up to +1.5 based on price difference)
 *      * Co-purchase count in actual customer orders (+5.0 per co-purchase)
 *      * Co-view count in browsing history (+1.5 per shared user session)
 * 
 * 2. Traversal Algorithm:
 *    - Best-First Search with Priority Queue (Dijkstra-style affinity expansion).
 *    - Given a target product node, the traversal finds the top-K highest affinity
 *      connected nodes while avoiding unavailable or deleted items.
 * 
 * 3. Time Complexity:
 *    - Adding / Updating an edge: O(1) average.
 *    - Recommendation Query: O((V_sub + E_sub) * log(V_sub)) where V_sub and E_sub
 *      are the reachable neighborhood vertices and edges of the product, bounded by top-K.
 * 
 * 4. Space Complexity:
 *    - O(V + E) where V is the number of active products and E is the total number of
 *      relationship edges stored in the adjacency list.
 * ============================================================================
 */

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace ShopSphere {
namespace Algorithms {

struct GraphEdge {
    int targetId;
    double weight{0.0};
    int coPurchaseCount{0};
    int coViewCount{0};
};

struct RecommendedProduct {
    int productId;
    double score{0.0};
    std::string reason;
};

class ProductGraph {
public:
    ProductGraph() = default;
    ~ProductGraph() = default;

    // Add or update a product node in the graph
    void addProductNode(int productId, int categoryId, const std::string &brand, double price, int stock);

    // Remove a product node (e.g. when deleted)
    void removeProductNode(int productId);

    // Update stock availability (out-of-stock items are not recommended)
    void setStock(int productId, int stock);

    // Add or increment an edge with custom weight
    void addEdge(int fromId, int toId, double weight, const std::string &type = "attribute");

    // Record an actual co-purchase interaction between two products from a customer order
    void recordCoPurchase(int productA, int productB);

    // Record an actual co-view interaction from a user browsing session
    void recordCoView(int productA, int productB);

    // Find top-K recommended products for a given source product
    std::vector<RecommendedProduct> getRecommendations(int productId, 
                                                       size_t limit = 6, 
                                                       const std::unordered_set<int> &excludeIds = {}) const;

    // Clear the entire graph
    void clear();

    // Diagnostics: total nodes and edges
    size_t nodeCount() const;
    size_t edgeCount() const;

private:
    struct ProductMeta {
        int id;
        int categoryId;
        std::string brand;
        double price;
        int stock;
        bool active{true};
    };

    std::unordered_map<int, ProductMeta> products_;
    std::unordered_map<int, std::vector<GraphEdge>> adjList_;
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_PRODUCT_GRAPH_H
