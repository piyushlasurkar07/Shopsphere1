#include "algorithms/SentimentAnalyzer.h"
#include "utils/StringUtils.h"
#include <cmath>
#include <sstream>
#include <algorithm>

namespace ShopSphere {
namespace Algorithms {

SentimentAnalyzer::SentimentAnalyzer() {
    initLexicon();
}

void SentimentAnalyzer::initLexicon() {
    lexicon_ = {
        {"superb", 3.5}, {"excellent", 3.4}, {"exceptional", 3.4}, {"fantastic", 3.4}, {"amazing", 3.3},
        {"great", 2.8}, {"awesome", 3.2}, {"good", 2.0}, {"love", 3.0},
        {"best", 3.2}, {"perfect", 3.4}, {"wonderful", 3.1}, {"happy", 2.4},
        {"satisfying", 2.3}, {"satisfied", 2.2}, {"impressed", 2.5}, {"worth", 2.0},
        {"valuable", 2.2}, {"recommend", 2.5}, {"recommended", 2.5}, {"fast", 1.8}, {"durable", 2.4},
        {"sturdy", 2.2}, {"premium", 2.6}, {"solid", 2.0}, {"reliable", 2.4},
        {"bad", -2.4}, {"terrible", -3.4}, {"horrible", -3.5}, {"poor", -2.3},
        {"worst", -3.7}, {"disappointed", -2.6}, {"disappointing", -2.5},
        {"waste", -3.0}, {"broken", -2.8}, {"damaged", -2.7}, {"slow", -1.8},
        {"cheap", -1.5}, {"fragile", -2.0}, {"faulty", -2.8}, {"useless", -3.2},
        {"defective", -3.0}, {"delay", -1.8}, {"delayed", -2.0}, {"hated", -3.1},
        {"ugly", -2.2}, {"unhappy", -2.4}, {"fake", -3.3}, {"scam", -3.8}
    };

    boosters_ = {
        {"very", 0.3}, {"extremely", 0.4}, {"super", 0.35}, {"really", 0.25},
        {"absolutely", 0.4}, {"totally", 0.3}, {"highly", 0.35},
        {"barely", -0.2}, {"hardly", -0.25}, {"slightly", -0.15}
    };

    negations_ = {"not", "never", "no", "isnt", "wasnt", "didnt", "cant", "dont", "wont", "without"};
}

SentimentResult SentimentAnalyzer::analyze(const std::string &text) const {
    SentimentResult result;
    if (text.empty()) {
        result.sentimentLabel = "Neutral";
        return result;
    }

    std::vector<std::string> rawTokens;
    std::string token;
    std::istringstream iss(text);
    while (iss >> token) {
        rawTokens.push_back(token);
    }

    if (rawTokens.empty()) {
        result.sentimentLabel = "Neutral";
        return result;
    }

    double totalValence = 0.0;
    int scoredWords = 0;

    std::vector<std::string> qualityTokens;
    std::vector<std::string> deliveryTokens;
    std::vector<std::string> priceTokens;

    for (size_t i = 0; i < rawTokens.size(); ++i) {
        std::string raw = rawTokens[i];
        
        std::string clean;
        for (char c : raw) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                clean.push_back(c);
            }
        }
        if (clean.empty()) continue;

        std::string lower = Utils::toLower(clean);

        if (lower == "quality" || lower == "build" || lower == "material" || lower == "product" || lower == "laptop") {
            size_t startIdx = (i >= 2) ? i - 2 : 0;
            size_t endIdx = std::min(i + 2, rawTokens.size() - 1);
            for (size_t j = startIdx; j <= endIdx; ++j) {
                qualityTokens.push_back(rawTokens[j]);
            }
        }
        if (lower == "delivery" || lower == "shipping" || lower == "courier" || lower == "packaging") {
            size_t startIdx = (i >= 2) ? i - 2 : 0;
            size_t endIdx = std::min(i + 2, rawTokens.size() - 1);
            for (size_t j = startIdx; j <= endIdx; ++j) {
                deliveryTokens.push_back(rawTokens[j]);
            }
        }
        if (lower == "price" || lower == "cost" || lower == "money" || lower == "value") {
            size_t startIdx = (i >= 2) ? i - 2 : 0;
            size_t endIdx = std::min(i + 2, rawTokens.size() - 1);
            for (size_t j = startIdx; j <= endIdx; ++j) {
                priceTokens.push_back(rawTokens[j]);
            }
        }

        auto it = lexicon_.find(lower);
        if (it != lexicon_.end()) {
            double valence = it->second;

            bool isAllCaps = (clean.size() > 1);
            for (char c : clean) {
                if (!std::isupper(static_cast<unsigned char>(c))) {
                    isAllCaps = false;
                    break;
                }
            }
            if (isAllCaps) {
                valence += (valence > 0 ? 0.5 : -0.5);
            }

            if (i > 0) {
                std::string prevRaw = rawTokens[i - 1];
                std::string prevClean;
                for (char c : prevRaw) if (std::isalpha(static_cast<unsigned char>(c))) prevClean.push_back(c);
                std::string prevLower = Utils::toLower(prevClean);

                auto bIt = boosters_.find(prevLower);
                if (bIt != boosters_.end()) {
                    valence += (valence > 0 ? bIt->second : -bIt->second);
                }

                if (std::find(negations_.begin(), negations_.end(), prevLower) != negations_.end()) {
                    valence *= -0.75;
                }
            }

            totalValence += valence;
            scoredWords++;
        }
    }

    size_t exclamations = std::count(text.begin(), text.end(), '!');
    if (exclamations > 0 && totalValence != 0.0) {
        totalValence += (totalValence > 0 ? 0.3 * std::min(exclamations, (size_t)3) 
                                          : -0.3 * std::min(exclamations, (size_t)3));
    }

    double alpha = 15.0;
    double compound = totalValence / std::sqrt((totalValence * totalValence) + alpha);
    compound = std::max(-1.0, std::min(1.0, compound));
    result.compoundScore = std::round(compound * 100.0) / 100.0;

    if (result.compoundScore >= 0.05) {
        result.sentimentLabel = "Positive";
    } else if (result.compoundScore <= -0.05) {
        result.sentimentLabel = "Negative";
    } else {
        result.sentimentLabel = "Neutral";
    }

    result.qualityAspect = analyzeSegment(qualityTokens);
    result.deliveryAspect = analyzeSegment(deliveryTokens);
    result.priceAspect = analyzeSegment(priceTokens);

    return result;
}

double SentimentAnalyzer::analyzeSegment(const std::vector<std::string> &tokens) const {
    if (tokens.empty()) return 0.0;
    double sum = 0.0;
    for (const auto &tok : tokens) {
        std::string clean;
        for (char c : tok) {
            if (std::isalpha(static_cast<unsigned char>(c))) {
                clean.push_back(c);
            }
        }
        std::string lower = Utils::toLower(clean);
        auto it = lexicon_.find(lower);
        if (it != lexicon_.end()) {
            sum += it->second;
        }
    }
    double compound = sum / std::sqrt((sum * sum) + 5.0);
    return std::round(std::max(-1.0, std::min(1.0, compound)) * 100.0) / 100.0;
}

} // namespace Algorithms
} // namespace ShopSphere
