#include "algorithms/ProductGraph.h"
#include <iostream>
#include <cassert>

using namespace ShopSphere::Algorithms;

int main() {
    std::cout << "[RUNNING] ProductGraphTest..." << std::endl;

    ProductGraph graph;

    // Test 1: Empty graph
    assert(graph.nodeCount() == 0);
    assert(graph.edgeCount() == 0);
    auto emptyRecs = graph.getRecommendations(1);
    assert(emptyRecs.empty());
    std::cout << "  ✓ Test 1: Empty graph behavior verified." << std::endl;

    // Test 2: Add product nodes (categoryId: 1 = Laptops, 2 = Audio)
    graph.addProductNode(1, 1, "Dell", 65000.0, 10);
    graph.addProductNode(2, 1, "Dell", 72000.0, 5);
    graph.addProductNode(3, 1, "HP", 68000.0, 8);
    graph.addProductNode(4, 2, "Sony", 15000.0, 12);
    graph.addProductNode(5, 2, "Sony", 8000.0, 0); // Out of stock

    assert(graph.nodeCount() == 5);
    assert(graph.edgeCount() > 0);
    std::cout << "  ✓ Test 2: Nodes added and initial attribute edges generated." << std::endl;

    // Test 3: Recommendation by category and brand
    auto recs = graph.getRecommendations(1, 3);
    assert(!recs.empty());
    // Product 2 shares both Category (Laptops) and Brand (Dell), so it should rank first
    assert(recs[0].productId == 2);
    std::cout << "  ✓ Test 3: Highest affinity product matches same category and brand." << std::endl;

    // Test 4: Out-of-stock product filtering
    auto audioRecs = graph.getRecommendations(4, 3);
    for (const auto &r : audioRecs) {
        assert(r.productId != 5); // Product 5 has stock = 0 and must not be recommended
    }
    std::cout << "  ✓ Test 4: Out-of-stock products excluded from recommendations." << std::endl;

    // Test 5: Co-purchase weighting
    graph.recordCoPurchase(1, 4); // Customer bought Laptop (1) and Headphones (4) together
    auto updatedRecs = graph.getRecommendations(1, 3);
    bool foundCoPurchased = false;
    for (const auto &r : updatedRecs) {
        if (r.productId == 4) {
            foundCoPurchased = true;
            assert(r.reason.find("Frequently Bought Together") != std::string::npos);
        }
    }
    assert(foundCoPurchased);
    std::cout << "  ✓ Test 5: Co-purchase weighting and reason correctly reflected." << std::endl;

    // Test 6: Node removal
    graph.removeProductNode(2);
    assert(graph.nodeCount() == 4);
    std::cout << "  ✓ Test 6: Node removal verified." << std::endl;

    std::cout << "[PASSED] ProductGraphTest completed successfully!" << std::endl;
    return 0;
}
