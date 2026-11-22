#include "algorithms/TrieSearch.h"
#include "utils/StringUtils.h"
#include <algorithm>
#include <sstream>

namespace ShopSphere {
namespace Algorithms {

TrieSearch::TrieSearch() : root_(std::make_unique<TrieNode>()), totalTerms_(0) {}

void TrieSearch::insertSingleTerm(const std::string &clean, int productId) {
    if (clean.empty()) return;

    TrieNode *current = root_.get();
    for (char c : clean) {
        if (current->children.find(c) == current->children.end()) {
            current->children[c] = std::make_unique<TrieNode>();
        }
        current = current->children[c].get();
    }

    if (!current->isEndOfWord) {
        current->isEndOfWord = true;
        totalTerms_++;
    }
    current->productIds.insert(productId);
    current->frequency++;
}

void TrieSearch::insert(const std::string &term, int productId) {
    std::string clean = Utils::toLower(Utils::trim(term));
    if (clean.empty()) return;

    insertSingleTerm(clean, productId);

    std::istringstream iss(clean);
    std::string word;
    while (iss >> word) {
        std::string cleanWord;
        for (char c : word) if (std::isalnum(static_cast<unsigned char>(c))) cleanWord.push_back(c);
        if (cleanWord.size() > 2 && cleanWord != clean) {
            insertSingleTerm(cleanWord, productId);
        }
    }
}

void TrieSearch::indexProduct(int productId, const std::string &name, const std::string &brand, const std::string &category) {
    insert(name, productId);
    if (!brand.empty()) insert(brand, productId);
    if (!category.empty()) insert(category, productId);
}

void TrieSearch::remove(const std::string &term, int productId) {
    std::string clean = Utils::toLower(Utils::trim(term));
    if (clean.empty()) return;

    auto removeOne = [this](const std::string &t, int pid) {
        TrieNode *current = root_.get();
        for (char c : t) {
            auto it = current->children.find(c);
            if (it == current->children.end()) return;
            current = it->second.get();
        }
        current->productIds.erase(pid);
        if (current->productIds.empty()) {
            current->isEndOfWord = false;
            if (totalTerms_ > 0) totalTerms_--;
        }
    };

    removeOne(clean, productId);
    std::istringstream iss(clean);
    std::string word;
    while (iss >> word) {
        std::string cleanWord;
        for (char c : word) if (std::isalnum(static_cast<unsigned char>(c))) cleanWord.push_back(c);
        if (cleanWord.size() > 2 && cleanWord != clean) {
            removeOne(cleanWord, productId);
        }
    }
}

std::vector<SearchSuggestion> TrieSearch::searchPrefix(const std::string &prefix, size_t limit) const {
    std::vector<SearchSuggestion> results;
    std::string clean = Utils::toLower(Utils::trim(prefix));
    if (clean.empty()) return results;

    const TrieNode *current = root_.get();
    for (char c : clean) {
        auto it = current->children.find(c);
        if (it == current->children.end()) {
            return results;
        }
        current = it->second.get();
    }

    std::string currentPrefix = clean;
    collectSuggestions(current, currentPrefix, results, limit);

    std::sort(results.begin(), results.end(), [](const SearchSuggestion &a, const SearchSuggestion &b) {
        if (a.frequency != b.frequency) return a.frequency > b.frequency;
        return a.term.length() < b.term.length();
    });

    if (results.size() > limit) {
        results.resize(limit);
    }
    return results;
}

std::vector<int> TrieSearch::getProductIds(const std::string &prefix, size_t limit) const {
    auto suggestions = searchPrefix(prefix, limit);
    std::unordered_set<int> uniqueIds;
    std::vector<int> orderedIds;

    for (const auto &sug : suggestions) {
        for (int id : sug.productIds) {
            if (uniqueIds.insert(id).second) {
                orderedIds.push_back(id);
                if (orderedIds.size() >= limit) return orderedIds;
            }
        }
    }
    return orderedIds;
}

bool TrieSearch::contains(const std::string &term) const {
    std::string clean = Utils::toLower(Utils::trim(term));
    if (clean.empty()) return false;

    const TrieNode *current = root_.get();
    for (char c : clean) {
        auto it = current->children.find(c);
        if (it == current->children.end()) return false;
        current = it->second.get();
    }
    return current->isEndOfWord;
}

void TrieSearch::incrementFrequency(const std::string &term) {
    std::string clean = Utils::toLower(Utils::trim(term));
    if (clean.empty()) return;

    TrieNode *current = root_.get();
    for (char c : clean) {
        auto it = current->children.find(c);
        if (it == current->children.end()) return;
        current = it->second.get();
    }
    if (current->isEndOfWord) {
        current->frequency++;
    }
}

void TrieSearch::clear() {
    root_ = std::make_unique<TrieNode>();
    totalTerms_ = 0;
}

size_t TrieSearch::size() const {
    return totalTerms_;
}

void TrieSearch::collectSuggestions(const TrieNode *node, 
                                   std::string &currentPrefix, 
                                   std::vector<SearchSuggestion> &results, 
                                   size_t limit) const {
    if (!node) return;

    if (node->isEndOfWord) {
        SearchSuggestion suggestion;
        suggestion.term = currentPrefix;
        suggestion.frequency = node->frequency;
        suggestion.productIds.assign(node->productIds.begin(), node->productIds.end());
        results.push_back(std::move(suggestion));
    }

    for (const auto &pair : node->children) {
        currentPrefix.push_back(pair.first);
        collectSuggestions(pair.second.get(), currentPrefix, results, limit);
        currentPrefix.pop_back();
    }
}

} // namespace Algorithms
} // namespace ShopSphere
