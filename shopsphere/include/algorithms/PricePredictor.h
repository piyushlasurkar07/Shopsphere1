#ifndef SHOPSPHERE_PRICE_PREDICTOR_H
#define SHOPSPHERE_PRICE_PREDICTOR_H

#include <string>
#include <vector>
#include <memory>

namespace ShopSphere {
namespace Algorithms {

struct PriceSample {
    int categoryId{0};
    int brandId{0};
    double originalPrice{0.0};
    int discountPercent{0};
    double rating{0.0};
    size_t descriptionLength{0};
    double actualPrice{0.0};
};

struct PricePredictionResult {
    bool success{false};
    double predictedPrice{0.0};
    double confidenceScore{0.0};
    std::string statusMessage;
};

// Single Decision Tree Node for Random Forest Regressor
struct DecisionTreeNode {
    bool isLeaf{false};
    double leafValue{0.0};
    int featureIndex{-1};
    double threshold{0.0};
    std::unique_ptr<DecisionTreeNode> left;
    std::unique_ptr<DecisionTreeNode> right;
};

class PricePredictor {
public:
    PricePredictor();
    ~PricePredictor() = default;

    // Train the Random Forest Regressor ensemble
    bool train(const std::vector<PriceSample> &samples, int numTrees = 10, int maxDepth = 4);

    // Predict price using ensemble averaging
    PricePredictionResult predict(const PriceSample &sample) const;

    bool isTrained() const;
    size_t sampleCount() const;

private:
    bool isTrained_{false};
    size_t sampleCount_{0};
    std::vector<std::unique_ptr<DecisionTreeNode>> trees_;

    std::unique_ptr<DecisionTreeNode> buildTree(const std::vector<PriceSample> &samples, 
                                               int depth, 
                                               int maxDepth);
    double evaluateTree(const DecisionTreeNode *node, const PriceSample &sample) const;
    static double getFeature(const PriceSample &s, int featureIdx);
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_PRICE_PREDICTOR_H
