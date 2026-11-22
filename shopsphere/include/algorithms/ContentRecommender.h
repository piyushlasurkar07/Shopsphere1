#ifndef SHOPSPHERE_CONTENT_RECOMMENDER_H
#define SHOPSPHERE_CONTENT_RECOMMENDER_H

#include <string>
#include <vector>
#include <unordered_map>

namespace ShopSphere {
namespace Algorithms {

struct ProductDocument {
    int id{0};
    std::string name;
    std::string brand;
    std::string category;
    std::string description;
};

class ContentRecommender {
public:
    ContentRecommender() = default;

    // Build TF-IDF index from current database products
    void fit(const std::vector<ProductDocument> &docs);

    // Get top-K recommended product IDs similar to target productId
    std::vector<std::pair<int, double>> recommend(int productId, size_t limit = 6) const;

    // Clear index
    void clear();

private:
    std::vector<ProductDocument> docs_;
    std::unordered_map<int, size_t> idToIndex_;
    std::unordered_map<std::string, double> idf_;
    std::vector<std::unordered_map<std::string, double>> tfidfVectors_;

    std::vector<std::string> tokenize(const std::string &text) const;
    double cosineSimilarity(const std::unordered_map<std::string, double> &v1,
                            const std::unordered_map<std::string, double> &v2) const;
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_CONTENT_RECOMMENDER_H
