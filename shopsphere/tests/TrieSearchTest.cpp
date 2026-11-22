#include "algorithms/TrieSearch.h"
#include <iostream>
#include <cassert>

using namespace ShopSphere::Algorithms;

int main() {
    std::cout << "[RUNNING] TrieSearchTest..." << std::endl;

    TrieSearch trie;

    // Test 1: Empty Trie
    assert(trie.size() == 0);
    assert(!trie.contains("laptop"));
    auto emptySuggestions = trie.searchPrefix("lap");
    assert(emptySuggestions.empty());
    std::cout << "  ✓ Test 1: Empty state behavior verified." << std::endl;

    // Test 2: Insertion and exact lookup
    trie.insert("Gaming Laptop", 101);
    trie.insert("Ultrabook Laptop", 102);
    trie.insert("Laptop Bag", 103);
    trie.insert("Mechanical Keyboard", 104);
    trie.insert("Wireless Mouse", 105);

    assert(trie.contains("gaming laptop"));
    assert(trie.contains("mechanical keyboard"));
    assert(!trie.contains("smartphone"));
    std::cout << "  ✓ Test 2: Insertion and contains verified." << std::endl;

    // Test 3: Prefix search & autocomplete
    auto suggestions = trie.searchPrefix("lap");
    assert(!suggestions.empty());
    assert(suggestions.size() >= 2);
    std::cout << "  ✓ Test 3: Prefix search found " << suggestions.size() << " terms for prefix 'lap'." << std::endl;

    // Test 4: Product ID retrieval
    auto productIds = trie.getProductIds("gaming");
    assert(!productIds.empty());
    assert(productIds[0] == 101);
    std::cout << "  ✓ Test 4: Product ID retrieval verified." << std::endl;

    // Test 5: Dynamic removal on product deletion/update
    trie.remove("Gaming Laptop", 101);
    assert(!trie.contains("gaming laptop"));
    auto remaining = trie.getProductIds("gaming");
    assert(remaining.empty());
    std::cout << "  ✓ Test 5: Dynamic removal and index synchronization verified." << std::endl;

    std::cout << "[PASSED] TrieSearchTest completed successfully!" << std::endl;
    return 0;
}
