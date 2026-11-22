#ifndef SHOPSPHERE_SENTIMENT_ANALYZER_H
#define SHOPSPHERE_SENTIMENT_ANALYZER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace ShopSphere {
namespace Algorithms {

struct SentimentResult {
    std::string sentimentLabel; // "Positive", "Neutral", "Negative"
    double compoundScore{0.0};  // Scaled from -1.0 (most negative) to +1.0 (most positive)
    double positiveScore{0.0};
    double neutralScore{0.0};
    double negativeScore{0.0};
    
    // Aspect-based sentiment scores
    double qualityAspect{0.0};
    double deliveryAspect{0.0};
    double priceAspect{0.0};
};

class SentimentAnalyzer {
public:
    SentimentAnalyzer();

    // Analyze sentiment of customer review text
    SentimentResult analyze(const std::string &text) const;

private:
    std::unordered_map<std::string, double> lexicon_;
    std::unordered_map<std::string, double> boosters_;
    std::vector<std::string> negations_;

    void initLexicon();
    double analyzeSegment(const std::vector<std::string> &tokens) const;
};

} // namespace Algorithms
} // namespace ShopSphere

#endif // SHOPSPHERE_SENTIMENT_ANALYZER_H
