#ifndef SHOPSPHERE_TRIE_SEARCH_H
#define SHOPSPHERE_TRIE_SEARCH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utility>

namespace ShopSphere {
namespace Algorithms {

struct TrieNode {
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    bool isEndOfWord{false};
    std::unordered_set<int> productIds; // IDs of matching products for this term
    int frequency{0};                   // Usage/query count for ranking
};

struct SearchSuggestion {
    std::string term;
    int frequency{0};
    std::vector<int> productIds;
};

class TrieSearch {
public:
    TrieSearch();
    ~TrieSearch() = default;

    // Insert a searchable term associated with a product (also indexes sub-tokens)
    void insert(const std::string &term, int productId);

    // Index a complete product with its name, brand, and category
    void indexProduct(int productId, const std::string &name, const std::string &brand = "", const std::string &category = "");

    // Remove a product association from a term
    void remove(const std::string &term, int productId);

    // Prefix search returning autocomplete suggestions up to limit
    std::vector<SearchSuggestion> searchPrefix(const std::string &prefix, size_t limit = 10) const;

    // Get matching product IDs directly for a given query or prefix
    std::vector<int> getProductIds(const std::string &prefix, size_t limit = 20) const;

    // Check if an exact term exists in the Trie
    bool contains(const std::string &term) const;

    // Record a search term occurrence to increase ranking frequency
    void incrementFrequency(const std::string &term);

    // Clear all nodes
    void clear();

    // Total terms indexed
    size_t size() const;

private:
    std::unique_ptr<TrieNode> root_;
    size_t totalTerms_{0};

    void insertSingleTerm(const std::string &cleanTerm, int productId);
    void collectSuggestions(const TrieNode *node, 
                            std::string &currentPrefix, 
                            std::vector<SearchSuggestion> &results, 
                            size_t limit) const;
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_TRIE_SEARCH_H
