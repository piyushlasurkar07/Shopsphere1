#ifndef SHOPSPHERE_NLP_SEARCH_H
#define SHOPSPHERE_NLP_SEARCH_H

#include <string>
#include <vector>
#include <unordered_set>

namespace ShopSphere {
namespace Algorithms {

struct ParsedSearchQuery {
    std::string rawQuery;
    std::string normalizedQuery;
    std::vector<std::string> keywords;
    std::string detectedCategory;
    std::string detectedBrand;
    double maxPrice{-1.0};
    double minPrice{-1.0};
    bool isUrgentOrDeal{false};
};

class NLPSearch {
public:
    NLPSearch();

    // Parse user input into intelligent search parameters
    ParsedSearchQuery parseQuery(const std::string &query, 
                                 const std::vector<std::string> &knownCategories = {},
                                 const std::vector<std::string> &knownBrands = {}) const;

    // Correct typos using Levenshtein distance against known vocabulary
    std::string correctSpelling(const std::string &token, 
                                const std::vector<std::string> &vocabulary, 
                                int maxDistance = 2) const;

    // Check if word is a common stopword
    bool isStopWord(const std::string &word) const;

private:
    std::unordered_set<std::string> stopWords_;
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_NLP_SEARCH_H
