#include "algorithms/NLPSearch.h"
#include "utils/StringUtils.h"
#include <sstream>
#include <regex>
#include <cmath>

namespace ShopSphere {
namespace Algorithms {

NLPSearch::NLPSearch() {
    stopWords_ = {
        "a", "an", "the", "and", "or", "in", "on", "at", "for", "with", "by",
        "to", "of", "from", "is", "are", "was", "were", "it", "this", "that",
        "i", "want", "need", "find", "looking", "show", "me", "buy", "get", "please"
    };
}

ParsedSearchQuery NLPSearch::parseQuery(const std::string &query, 
                                       const std::vector<std::string> &knownCategories,
                                       const std::vector<std::string> &knownBrands) const {
    ParsedSearchQuery result;
    result.rawQuery = query;
    std::string lower = Utils::toLower(Utils::trim(query));
    result.normalizedQuery = lower;

    // 1. Detect price constraints using patterns like "under 5000", "below 10000", "between 1000 and 5000"
    std::regex underRegex(R"((?:under|below|less\s+than|<=?)\s*(?:rs\.?|inr|₹)?\s*([0-9]+(?:\.[0-9]+)?))");
    std::smatch match;
    if (std::regex_search(lower, match, underRegex)) {
        result.maxPrice = std::stod(match[1].str());
        lower = std::regex_replace(lower, underRegex, " ");
    }

    std::regex aboveRegex(R"((?:above|over|more\s+than|>=?)\s*(?:rs\.?|inr|₹)?\s*([0-9]+(?:\.[0-9]+)?))");
    if (std::regex_search(lower, match, aboveRegex)) {
        result.minPrice = std::stod(match[1].str());
        lower = std::regex_replace(lower, aboveRegex, " ");
    }

    // 2. Detect deal / offer intents
    if (lower.find("deal") != std::string::npos || 
        lower.find("offer") != std::string::npos || 
        lower.find("discount") != std::string::npos) {
        result.isUrgentOrDeal = true;
    }

    // 3. Match known categories
    for (const auto &cat : knownCategories) {
        std::string catLower = Utils::toLower(cat);
        if (lower.find(catLower) != std::string::npos) {
            result.detectedCategory = cat;
            break;
        }
    }

    // 4. Match known brands
    for (const auto &brand : knownBrands) {
        std::string brandLower = Utils::toLower(brand);
        if (!brandLower.empty() && lower.find(brandLower) != std::string::npos) {
            result.detectedBrand = brand;
            break;
        }
    }

    // 5. Tokenize remaining words, filter stopwords
    std::istringstream iss(lower);
    std::string token;
    while (iss >> token) {
        // Strip non-alphanumeric chars
        std::string cleanToken;
        for (char c : token) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                cleanToken.push_back(c);
            }
        }
        if (!cleanToken.empty() && !isStopWord(cleanToken)) {
            result.keywords.push_back(cleanToken);
        }
    }

    return result;
}

std::string NLPSearch::correctSpelling(const std::string &token, 
                                      const std::vector<std::string> &vocabulary, 
                                      int maxDistance) const {
    if (token.empty() || vocabulary.empty()) return token;

    std::string lowerToken = Utils::toLower(token);
    std::string bestMatch = token;
    int minDistance = maxDistance + 1;

    for (const auto &word : vocabulary) {
        std::string wordLower = Utils::toLower(word);
        if (wordLower == lowerToken) return word;

        int dist = Utils::levenshteinDistance(lowerToken, wordLower);
        if (dist < minDistance) {
            minDistance = dist;
            bestMatch = word;
        }
    }

    return (minDistance <= maxDistance) ? bestMatch : token;
}

bool NLPSearch::isStopWord(const std::string &word) const {
    return stopWords_.find(Utils::toLower(word)) != stopWords_.end();
}

} // namespace Algorithms
} // namespace ShopSphere
