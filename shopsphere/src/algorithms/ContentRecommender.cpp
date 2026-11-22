#include "algorithms/ContentRecommender.h"
#include "utils/StringUtils.h"
#include <cmath>
#include <sstream>
#include <algorithm>

namespace ShopSphere {
namespace Algorithms {

std::vector<std::string> ContentRecommender::tokenize(const std::string &text) const {
    std::string lower = Utils::toLower(text);
    std::vector<std::string> tokens;
    std::string current;
    for (char c : lower) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current.push_back(c);
        } else if (!current.empty()) {
            if (current.size() > 2) {
                tokens.push_back(current);
            }
            current.clear();
        }
    }
    if (current.size() > 2) {
        tokens.push_back(current);
    }
    return tokens;
}

void ContentRecommender::fit(const std::vector<ProductDocument> &docs) {
    clear();
    docs_ = docs;
    size_t N = docs.size();
    if (N == 0) return;

    for (size_t i = 0; i < N; ++i) {
        idToIndex_[docs[i].id] = i;
    }

    // 1. Calculate Document Frequency (DF)
    std::unordered_map<std::string, size_t> df;
    std::vector<std::unordered_map<std::string, double>> tfVectors(N);

    for (size_t i = 0; i < N; ++i) {
        std::string fullText = docs[i].name + " " + docs[i].brand + " " + 
                               docs[i].category + " " + docs[i].description;
        auto tokens = tokenize(fullText);
        std::unordered_map<std::string, size_t> counts;
        for (const auto &tok : tokens) {
            counts[tok]++;
        }

        size_t totalTokens = tokens.empty() ? 1 : tokens.size();
        for (const auto &pair : counts) {
            tfVectors[i][pair.first] = static_cast<double>(pair.second) / totalTokens;
            df[pair.first]++;
        }
    }

    // 2. Compute Inverse Document Frequency (IDF)
    for (const auto &pair : df) {
        idf_[pair.first] = std::log((1.0 + N) / (1.0 + pair.second)) + 1.0;
    }

    // 3. Compute TF-IDF Vectors
    tfidfVectors_.resize(N);
    for (size_t i = 0; i < N; ++i) {
        for (const auto &pair : tfVectors[i]) {
            double tfidf = pair.second * idf_[pair.first];
            tfidfVectors_[i][pair.first] = tfidf;
        }
    }
}

double ContentRecommender::cosineSimilarity(const std::unordered_map<std::string, double> &v1,
                                            const std::unordered_map<std::string, double> &v2) const {
    double dot = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;

    for (const auto &p : v1) {
        norm1 += p.second * p.second;
        auto it = v2.find(p.first);
        if (it != v2.end()) {
            dot += p.second * it->second;
        }
    }
    for (const auto &p : v2) {
        norm2 += p.second * p.second;
    }

    if (norm1 <= 0.0 || norm2 <= 0.0) return 0.0;
    return dot / (std::sqrt(norm1) * std::sqrt(norm2));
}

std::vector<std::pair<int, double>> ContentRecommender::recommend(int productId, size_t limit) const {
    std::vector<std::pair<int, double>> results;
    auto it = idToIndex_.find(productId);
    if (it == idToIndex_.end() || tfidfVectors_.empty()) {
        return results;
    }

    size_t targetIdx = it->second;
    const auto &targetVec = tfidfVectors_[targetIdx];

    for (size_t i = 0; i < tfidfVectors_.size(); ++i) {
        if (i == targetIdx) continue;
        double sim = cosineSimilarity(targetVec, tfidfVectors_[i]);
        if (sim > 0.01) {
            results.push_back({docs_[i].id, sim});
        }
    }

    std::sort(results.begin(), results.end(), [](const std::pair<int, double> &a, const std::pair<int, double> &b) {
        return a.second > b.second;
    });

    if (results.size() > limit) {
        results.resize(limit);
    }
    return results;
}

void ContentRecommender::clear() {
    docs_.clear();
    idToIndex_.clear();
    idf_.clear();
    tfidfVectors_.clear();
}

} // namespace Algorithms
} // namespace ShopSphere
