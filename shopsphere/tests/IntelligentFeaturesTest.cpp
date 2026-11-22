#include "algorithms/NLPSearch.h"
#include "algorithms/ContentRecommender.h"
#include "algorithms/SentimentAnalyzer.h"
#include "algorithms/LogisticRegression.h"
#include "algorithms/PricePredictor.h"
#include <iostream>
#include <cassert>

using namespace ShopSphere::Algorithms;

int main() {
    std::cout << "[RUNNING] IntelligentFeaturesTest..." << std::endl;

    // 1. Test NLP Search
    {
        NLPSearch nlp;
        std::vector<std::string> categories = {"Laptops", "Smartphones", "Audio"};
        std::vector<std::string> brands = {"Apple", "Samsung", "Sony"};

        auto parsed = nlp.parseQuery("buy gaming laptops under 50000 with discount", categories, brands);
        assert(parsed.detectedCategory == "Laptops");
        assert(parsed.maxPrice == 50000.0);
        assert(parsed.isUrgentOrDeal == true);

        // Typo correction test: "lapotp" -> "laptop"
        std::vector<std::string> vocab = {"laptop", "smartphone", "headphone"};
        std::string corrected = nlp.correctSpelling("lapotp", vocab);
        assert(corrected == "laptop");
        std::cout << "  ✓ NLP Search & Typo Correction verified." << std::endl;
    }

    // 2. Test Content-Based Recommender
    {
        ContentRecommender rec;
        std::vector<ProductDocument> docs = {
            {1, "MacBook Pro", "Apple", "Laptops", "Powerful M2 chip with retina display"},
            {2, "MacBook Air", "Apple", "Laptops", "Lightweight laptop with M2 chip and all day battery"},
            {3, "Sony WH-1000XM5", "Sony", "Audio", "Wireless noise cancelling premium headphones"}
        };
        rec.fit(docs);
        auto results = rec.recommend(1, 2);
        assert(!results.empty());
        assert(results[0].first == 2); // MacBook Air is most similar to MacBook Pro
        std::cout << "  ✓ Content Recommender TF-IDF Cosine Similarity verified." << std::endl;
    }

    // 3. Test VADER Sentiment Analysis
    {
        SentimentAnalyzer sa;
        auto pos = sa.analyze("This laptop has exceptional build quality and super fast delivery! Highly recommended.");
        assert(pos.sentimentLabel == "Positive");
        assert(pos.compoundScore > 0.4);
        assert(pos.qualityAspect > 0.0);
        assert(pos.deliveryAspect > 0.0);

        auto neg = sa.analyze("Terrible product, completely broken and horrible customer support.");
        assert(neg.sentimentLabel == "Negative");
        assert(neg.compoundScore < -0.4);

        auto empty = sa.analyze("");
        assert(empty.sentimentLabel == "Neutral");
        std::cout << "  ✓ VADER-style Sentiment & Aspect Analysis verified." << std::endl;
    }

    // 4. Test Logistic Regression Classifier
    {
        LogisticRegressionClassifier lr;
        assert(!lr.isTrained());
        auto emptyPred = lr.predict("Dell gaming laptop 16GB RAM");
        assert(!emptyPred.success); // Gracefully handles empty / insufficient data

        std::vector<CategoryTrainingSample> samples = {
            {"Dell XPS high performance laptop computer", "Laptops"},
            {"Lenovo ThinkPad business laptop with Intel core", "Laptops"},
            {"Apple MacBook Pro retina screen laptop", "Laptops"},
            {"Sony noise cancelling wireless over ear headphones", "Audio"},
            {"Bose QuietComfort bluetooth sound headphones", "Audio"},
            {"JBL bass portable wireless speaker", "Audio"}
        };
        bool trained = lr.train(samples);
        assert(trained);
        assert(lr.isTrained());

        auto pred = lr.predict("HP Pavilion Intel core laptop computer");
        assert(pred.success);
        assert(pred.predictedCategory == "Laptops");
        std::cout << "  ✓ Logistic Regression Category Classification verified." << std::endl;
    }

    // 5. Test Random Forest Price Predictor
    {
        PricePredictor rf;
        assert(!rf.isTrained());
        PriceSample testSample{1, 1, 60000.0, 10, 4.5, 120, 0.0};
        auto emptyResult = rf.predict(testSample);
        assert(!emptyResult.success); // Gracefully handles insufficient data

        std::vector<PriceSample> samples = {
            {1, 1, 50000.0, 10, 4.2, 100, 45000.0},
            {1, 1, 60000.0, 15, 4.5, 120, 51000.0},
            {1, 2, 70000.0, 10, 4.6, 140, 63000.0},
            {1, 2, 80000.0, 20, 4.8, 160, 64000.0},
            {2, 3, 10000.0, 5, 4.0, 80, 9500.0},
            {2, 3, 15000.0, 10, 4.4, 90, 13500.0}
        };
        bool trained = rf.train(samples);
        assert(trained);
        assert(rf.isTrained());

        auto result = rf.predict(testSample);
        assert(result.success);
        assert(result.predictedPrice > 30000.0 && result.predictedPrice < 70000.0);
        std::cout << "  ✓ Random Forest Price Prediction verified." << std::endl;
    }

    std::cout << "[PASSED] IntelligentFeaturesTest completed successfully!" << std::endl;
    return 0;
}
