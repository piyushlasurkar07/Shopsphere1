#ifndef SHOPSPHERE_APP_SERVICES_H
#define SHOPSPHERE_APP_SERVICES_H

#include "models/Models.h"
#include "database/DatabaseManager.h"
#include "algorithms/TrieSearch.h"
#include "algorithms/ProductGraph.h"
#include "algorithms/LRUCache.h"
#include "algorithms/OrderPriorityQueue.h"
#include "algorithms/NLPSearch.h"
#include "algorithms/ContentRecommender.h"
#include "algorithms/SentimentAnalyzer.h"
#include "algorithms/LogisticRegression.h"
#include "algorithms/PricePredictor.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace ShopSphere {
namespace Services {

// Global Session Store for Users and Admins
struct SessionInfo {
    std::string token;
    int userId{0};
    int adminId{0};
    std::string role;
    std::string email;
    std::string fullName;
    std::chrono::system_clock::time_point expiresAt;
};

class SessionService {
public:
    static SessionService &instance();

    std::string createSession(int userId, int adminId, const std::string &role, const std::string &email, const std::string &fullName, int hoursValid = 48);
    bool getSession(const std::string &token, SessionInfo &outInfo);
    void destroySession(const std::string &token);

private:
    SessionService() = default;
    std::unordered_map<std::string, SessionInfo> sessions_;
    mutable std::mutex mutex_;
};

class AppServices {
public:
    static AppServices &instance();

    bool initialize(const std::string &dbPath, const std::string &schemaSqlPath);

    // Core Algorithms Accessors
    Algorithms::TrieSearch &getTrie() { return trie_; }
    Algorithms::ProductGraph &getGraph() { return graph_; }
    Algorithms::LRUCache<int, std::string> &getProductCache() { return productCache_; }
    Algorithms::OrderPriorityQueue &getPriorityQueue() { return orderQueue_; }
    Algorithms::NLPSearch &getNlp() { return nlp_; }
    Algorithms::SentimentAnalyzer &getSentimentAnalyzer() { return sentimentAnalyzer_; }
    Algorithms::LogisticRegressionClassifier &getClassifier() { return classifier_; }
    Algorithms::PricePredictor &getPricePredictor() { return pricePredictor_; }

    // Service Workflows
    // 1. Products
    std::vector<Models::Product> getProducts(int categoryId, const std::string &q, double minPrice, double maxPrice, const std::string &sortBy);
    bool getProductDetailsJson(int productId, std::string &outJson);
    bool createProduct(const Models::Product &prod, int &outId);
    bool updateProduct(const Models::Product &prod);
    bool deleteProduct(int productId);
    std::vector<Algorithms::SearchSuggestion> getSearchSuggestions(const std::string &prefix, size_t limit = 10);
    std::vector<Algorithms::RecommendedProduct> getRecommendations(int productId, size_t limit = 6);

    // 2. Auth & Customer
    bool registerUser(const std::string &username, const std::string &email, const std::string &password, const std::string &fullName, const std::string &phone, const std::string &address, std::string &outError, std::string &outToken);
    bool loginUser(const std::string &email, const std::string &password, std::string &outError, std::string &outToken, Models::User &outUser);

    // 3. Admin Auth & Management
    bool setupFirstAdmin(const std::string &username, const std::string &email, const std::string &password, const std::string &fullName, std::string &outError);
    bool loginAdmin(const std::string &email, const std::string &password, std::string &outError, std::string &outToken, Models::Admin &outAdmin);
    bool processNextPriorityOrder(Algorithms::OrderPriorityItem &outOrder);

    // 4. Cart & Checkout
    bool checkout(int userId, const std::string &sessionId, const std::string &name, const std::string &email, const std::string &phone, const std::string &address, const std::string &paymentMethod, bool isUrgent, std::string &outOrderNumber, std::string &outError);

    // 5. Reviews
    bool submitReview(int productId, int userId, const std::string &userName, int rating, const std::string &title, const std::string &text, std::string &outError);

    // In-memory indexing synchronization
    void syncDSAData();

private:
    AppServices();
    Algorithms::TrieSearch trie_;
    Algorithms::ProductGraph graph_;
    Algorithms::LRUCache<int, std::string> productCache_{100};
    Algorithms::OrderPriorityQueue orderQueue_;
    Algorithms::NLPSearch nlp_;
    Algorithms::ContentRecommender contentRecommender_;
    Algorithms::SentimentAnalyzer sentimentAnalyzer_;
    Algorithms::LogisticRegressionClassifier classifier_;
    Algorithms::PricePredictor pricePredictor_;
};

} // namespace Services
} // namespace ShopSphere

#endif // SHOPSPHERE_APP_SERVICES_H
