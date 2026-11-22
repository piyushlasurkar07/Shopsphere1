#ifndef SHOPSPHERE_LOGISTIC_REGRESSION_H
#define SHOPSPHERE_LOGISTIC_REGRESSION_H

#include <string>
#include <vector>
#include <unordered_map>

namespace ShopSphere {
namespace Algorithms {

struct CategoryTrainingSample {
    std::string text; // Product name + brand + description
    std::string category;
};

struct ClassificationResult {
    bool success{false};
    std::string predictedCategory;
    double confidence{0.0};
    std::string statusMessage;
    std::vector<std::pair<std::string, double>> classProbabilities;
};

class LogisticRegressionClassifier {
public:
    LogisticRegressionClassifier();

    // Train the multi-class (One-vs-Rest) logistic regression model
    bool train(const std::vector<CategoryTrainingSample> &samples, 
               double learningRate = 0.1, 
               int epochs = 100);

    // Predict category for given product text
    ClassificationResult predict(const std::string &productText) const;

    // Check if model has sufficient training data
    bool isTrained() const;
    size_t sampleCount() const;

private:
    bool isTrained_{false};
    size_t sampleCount_{0};
    std::vector<std::string> categories_;
    std::unordered_map<std::string, size_t> vocab_;
    
    // Weights: one weight vector per category [numCategories][vocabSize]
    std::vector<std::vector<double>> weights_;
    std::vector<double> biases_;

    std::vector<double> extractFeatures(const std::string &text) const;
    static double sigmoid(double z);
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_LOGISTIC_REGRESSION_H
