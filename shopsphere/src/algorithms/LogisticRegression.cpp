#include "algorithms/LogisticRegression.h"
#include "utils/StringUtils.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <unordered_set>

namespace ShopSphere {
namespace Algorithms {

LogisticRegressionClassifier::LogisticRegressionClassifier() : isTrained_(false), sampleCount_(0) {}

double LogisticRegressionClassifier::sigmoid(double z) {
    if (z > 35.0) return 1.0;
    if (z < -35.0) return 0.0;
    return 1.0 / (1.0 + std::exp(-z));
}

std::vector<double> LogisticRegressionClassifier::extractFeatures(const std::string &text) const {
    std::vector<double> features(vocab_.size(), 0.0);
    std::string lower = Utils::toLower(text);
    std::istringstream iss(lower);
    std::string token;
    size_t count = 0;

    while (iss >> token) {
        std::string clean;
        for (char c : token) {
            if (std::isalnum(static_cast<unsigned char>(c))) clean.push_back(c);
        }
        if (clean.size() > 2) {
            auto it = vocab_.find(clean);
            if (it != vocab_.end()) {
                features[it->second] += 1.0;
                count++;
            }
        }
    }

    // L2-normalize feature vector
    if (count > 0) {
        double norm = 0.0;
        for (double v : features) norm += v * v;
        norm = std::sqrt(norm);
        if (norm > 0) {
            for (double &v : features) v /= norm;
        }
    }
    return features;
}

bool LogisticRegressionClassifier::train(const std::vector<CategoryTrainingSample> &samples, 
                                        double learningRate, 
                                        int epochs) {
    sampleCount_ = samples.size();
    vocab_.clear();
    categories_.clear();
    weights_.clear();
    biases_.clear();
    isTrained_ = false;

    // Minimum 5 samples with at least 2 categories required
    std::unordered_set<std::string> uniqueCats;
    for (const auto &s : samples) {
        if (!s.category.empty()) uniqueCats.insert(s.category);
    }

    if (samples.size() < 5 || uniqueCats.size() < 2) {
        return false;
    }

    categories_.assign(uniqueCats.begin(), uniqueCats.end());
    std::sort(categories_.begin(), categories_.end());

    // 1. Build Vocabulary
    size_t vocabIdx = 0;
    for (const auto &s : samples) {
        std::string lower = Utils::toLower(s.text);
        std::istringstream iss(lower);
        std::string token;
        while (iss >> token) {
            std::string clean;
            for (char c : token) if (std::isalnum(static_cast<unsigned char>(c))) clean.push_back(c);
            if (clean.size() > 2 && vocab_.find(clean) == vocab_.end()) {
                vocab_[clean] = vocabIdx++;
            }
        }
    }

    size_t V = vocab_.size();
    size_t K = categories_.size();
    if (V == 0) return false;

    // 2. Feature matrix
    std::vector<std::vector<double>> X(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        X[i] = extractFeatures(samples[i].text);
    }

    // 3. Initialize weights and biases
    weights_.assign(K, std::vector<double>(V, 0.0));
    biases_.assign(K, 0.0);

    // 4. Batch Gradient Descent with L2 Regularization (One-vs-Rest)
    double lambda = 0.01;
    for (int epoch = 0; epoch < epochs; ++epoch) {
        for (size_t k = 0; k < K; ++k) {
            std::vector<double> dW(V, 0.0);
            double dB = 0.0;

            for (size_t i = 0; i < samples.size(); ++i) {
                double y = (samples[i].category == categories_[k]) ? 1.0 : 0.0;
                
                // Linear hypothesis: z = w.x + b
                double z = biases_[k];
                for (size_t j = 0; j < V; ++j) {
                    z += weights_[k][j] * X[i][j];
                }

                double pred = sigmoid(z);
                double error = pred - y;

                for (size_t j = 0; j < V; ++j) {
                    dW[j] += error * X[i][j];
                }
                dB += error;
            }

            // Update weights with regularization
            for (size_t j = 0; j < V; ++j) {
                weights_[k][j] -= learningRate * ((dW[j] / samples.size()) + lambda * weights_[k][j]);
            }
            biases_[k] -= learningRate * (dB / samples.size());
        }
    }

    isTrained_ = true;
    return true;
}

ClassificationResult LogisticRegressionClassifier::predict(const std::string &productText) const {
    ClassificationResult result;
    if (!isTrained_ || categories_.empty()) {
        result.success = false;
        result.statusMessage = "Insufficient training data: Model requires at least 5 products across multiple categories.";
        return result;
    }

    auto features = extractFeatures(productText);
    size_t V = vocab_.size();
    size_t K = categories_.size();

    std::vector<double> rawScores(K);
    double sumExp = 0.0;

    for (size_t k = 0; k < K; ++k) {
        double z = biases_[k];
        for (size_t j = 0; j < V; ++j) {
            z += weights_[k][j] * features[j];
        }
        rawScores[k] = z;
        sumExp += std::exp(std::max(-20.0, std::min(20.0, z)));
    }

    // Softmax probabilities
    size_t bestK = 0;
    double maxProb = -1.0;

    for (size_t k = 0; k < K; ++k) {
        double prob = std::exp(std::max(-20.0, std::min(20.0, rawScores[k]))) / (sumExp > 0 ? sumExp : 1.0);
        result.classProbabilities.push_back({categories_[k], prob});
        if (prob > maxProb) {
            maxProb = prob;
            bestK = k;
        }
    }

    result.success = true;
    result.predictedCategory = categories_[bestK];
    result.confidence = std::round(maxProb * 100.0) / 100.0;
    result.statusMessage = "Prediction computed successfully using C++ Logistic Regression.";
    return result;
}

bool LogisticRegressionClassifier::isTrained() const {
    return isTrained_;
}

size_t LogisticRegressionClassifier::sampleCount() const {
    return sampleCount_;
}

} // namespace Algorithms
} // namespace ShopSphere
