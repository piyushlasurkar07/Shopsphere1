#include "services/AppServices.h"
#include "utils/Crypto.h"
#include "utils/StringUtils.h"
#include <sstream>
#include <iostream>
#include <iomanip>

namespace ShopSphere {
namespace Services {

// ----------------------------------------------------------------------------
// SessionService Implementation
// ----------------------------------------------------------------------------
SessionService &SessionService::instance() {
    static SessionService instance;
    return instance;
}

std::string SessionService::createSession(int userId, int adminId, const std::string &role, 
                                          const std::string &email, const std::string &fullName, 
                                          int hoursValid) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string token = Utils::Crypto::generateToken(32);
    SessionInfo info;
    info.token = token;
    info.userId = userId;
    info.adminId = adminId;
    info.role = role;
    info.email = email;
    info.fullName = fullName;
    info.expiresAt = std::chrono::system_clock::now() + std::chrono::hours(hoursValid);
    sessions_[token] = info;
    return token;
}

bool SessionService::getSession(const std::string &token, SessionInfo &outInfo) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (token.empty()) return false;
    auto it = sessions_.find(token);
    if (it == sessions_.end()) return false;

    if (std::chrono::system_clock::now() > it->second.expiresAt) {
        sessions_.erase(it);
        return false;
    }
    outInfo = it->second;
    return true;
}

void SessionService::destroySession(const std::string &token) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(token);
}

// ----------------------------------------------------------------------------
// AppServices Implementation
// ----------------------------------------------------------------------------
AppServices &AppServices::instance() {
    static AppServices instance;
    return instance;
}

AppServices::AppServices() {}

bool AppServices::initialize(const std::string &dbPath, const std::string &schemaSqlPath) {
    if (!Database::DatabaseManager::instance().initialize(dbPath, schemaSqlPath)) {
        return false;
    }
    syncDSAData();
    return true;
}

void AppServices::syncDSAData() {
    trie_.clear();
    graph_.clear();
    productCache_.clear();
    orderQueue_.clear();

    auto products = Database::DatabaseManager::instance().getAllProducts();
    std::vector<Algorithms::ProductDocument> docs;

    for (const auto &p : products) {
        trie_.indexProduct(p.id, p.name, p.brand, p.categoryName);
        graph_.addProductNode(p.id, p.categoryId, p.brand, p.price, p.stockQuantity);
        docs.push_back({p.id, p.name, p.brand, p.categoryName, p.description});
    }

    if (!docs.empty()) {
        contentRecommender_.fit(docs);
    }

    // Load co-purchase and co-view history into Graph
    auto coPurchases = Database::DatabaseManager::instance().getCoPurchasedPairs();
    for (const auto &pair : coPurchases) {
        graph_.recordCoPurchase(pair.first, pair.second);
    }

    auto coViews = Database::DatabaseManager::instance().getCoViewedPairs();
    for (const auto &pair : coViews) {
        graph_.recordCoView(pair.first, pair.second);
    }

    // Load pending customer orders into OrderPriorityQueue
    auto allOrders = Database::DatabaseManager::instance().getAllOrders();
    for (const auto &o : allOrders) {
        if (o.orderStatus == "Pending" || o.orderStatus == "Processing") {
            Algorithms::OrderPriorityItem item;
            item.orderId = o.id;
            item.orderNumber = o.orderNumber;
            item.userId = o.userId;
            item.customerName = o.customerName;
            item.totalAmount = o.totalAmount;
            item.isUrgent = o.isUrgent;
            item.createdAt = std::chrono::system_clock::now(); // approximation for startup
            orderQueue_.push(item);
        }
    }

    std::cout << "[DSA Engine] Synchronized Trie terms: " << trie_.size()
              << ", Graph nodes: " << graph_.nodeCount()
              << ", Priority queue pending orders: " << orderQueue_.size() << std::endl;
}

// ----------------------------------------------------------------------------
// Product Workflows
// ----------------------------------------------------------------------------
std::vector<Models::Product> AppServices::getProducts(int categoryId, const std::string &q, 
                                                      double minPrice, double maxPrice, 
                                                      const std::string &sortBy) {
    if (q.empty()) {
        return Database::DatabaseManager::instance().getAllProducts(categoryId, "", minPrice, maxPrice, sortBy);
    }

    // Try Trie-based matching first
    auto matchedIds = trie_.getProductIds(q, 50);
    if (!matchedIds.empty() && categoryId == 0 && minPrice == 0.0 && maxPrice == 0.0 && sortBy.empty()) {
        std::vector<Models::Product> results;
        for (int id : matchedIds) {
            Models::Product p;
            if (Database::DatabaseManager::instance().getProductById(id, p)) {
                results.push_back(p);
            }
        }
        return results;
    }

    // Fall back to NLP and SQL filter
    auto categories = Database::DatabaseManager::instance().getAllCategories();
    std::vector<std::string> catNames;
    for (const auto &c : categories) catNames.push_back(c.name);

    auto parsed = nlp_.parseQuery(q, catNames);
    if (parsed.maxPrice > 0.0 && maxPrice == 0.0) maxPrice = parsed.maxPrice;
    if (parsed.minPrice > 0.0 && minPrice == 0.0) minPrice = parsed.minPrice;

    std::string effectiveQuery = q;
    if (!parsed.keywords.empty()) {
        std::ostringstream oss;
        for (const auto &k : parsed.keywords) oss << k << " ";
        effectiveQuery = Utils::trim(oss.str());
    }

    return Database::DatabaseManager::instance().getAllProducts(categoryId, effectiveQuery, minPrice, maxPrice, sortBy);
}

bool AppServices::getProductDetailsJson(int productId, std::string &outJson) {
    // 1. Check LRU Cache (Average O(1) retrieval)
    if (productCache_.get(productId, outJson)) {
        return true;
    }

    // 2. Fetch from Database if not cached
    Models::Product prod;
    if (!Database::DatabaseManager::instance().getProductById(productId, prod)) {
        return false;
    }

    // Serialize to JSON string
    std::ostringstream oss;
    oss << "{"
        << "\"id\":" << prod.id << ","
        << "\"name\":\"" << Utils::sanitizeHtml(prod.name) << "\","
        << "\"brand\":\"" << Utils::sanitizeHtml(prod.brand) << "\","
        << "\"category_id\":" << prod.categoryId << ","
        << "\"category_name\":\"" << Utils::sanitizeHtml(prod.categoryName) << "\","
        << "\"description\":\"" << Utils::sanitizeHtml(prod.description) << "\","
        << "\"specifications\":\"" << Utils::sanitizeHtml(prod.specifications) << "\","
        << "\"price\":" << prod.price << ","
        << "\"original_price\":" << prod.originalPrice << ","
        << "\"discount_percent\":" << prod.discountPercent << ","
        << "\"rating\":" << prod.rating << ","
        << "\"review_count\":" << prod.reviewCount << ","
        << "\"stock_quantity\":" << prod.stockQuantity << ","
        << "\"image_url\":\"" << prod.imageUrl << "\","
        << "\"is_featured\":" << (prod.isFeatured ? "true" : "false") << ","
        << "\"is_trending\":" << (prod.isTrending ? "true" : "false") << ","
        << "\"is_bestseller\":" << (prod.isBestseller ? "true" : "false")
        << "}";

    outJson = oss.str();

    // Store in LRU Cache for subsequent requests
    productCache_.put(productId, outJson);
    return true;
}

bool AppServices::createProduct(const Models::Product &prod, int &outId) {
    if (!Database::DatabaseManager::instance().createProduct(prod, outId)) {
        return false;
    }

    // Dynamically update Trie and Graph
    trie_.indexProduct(outId, prod.name, prod.brand, prod.categoryName);
    graph_.addProductNode(outId, prod.categoryId, prod.brand, prod.price, prod.stockQuantity);
    return true;
}

bool AppServices::updateProduct(const Models::Product &prod) {
    if (!Database::DatabaseManager::instance().updateProduct(prod)) {
        return false;
    }

    // Invalidate LRU Cache entry
    productCache_.remove(prod.id);

    // Update Trie and Graph
    trie_.indexProduct(prod.id, prod.name, prod.brand, prod.categoryName);
    graph_.setStock(prod.id, prod.stockQuantity);
    return true;
}

bool AppServices::deleteProduct(int productId) {
    if (!Database::DatabaseManager::instance().deleteProduct(productId)) {
        return false;
    }

    // Invalidate LRU Cache entry
    productCache_.remove(productId);

    // Remove from Graph
    graph_.removeProductNode(productId);
    return true;
}

std::vector<Algorithms::SearchSuggestion> AppServices::getSearchSuggestions(const std::string &prefix, size_t limit) {
    return trie_.searchPrefix(prefix, limit);
}

std::vector<Algorithms::RecommendedProduct> AppServices::getRecommendations(int productId, size_t limit) {
    return graph_.getRecommendations(productId, limit);
}

// ----------------------------------------------------------------------------
// Auth & Customer Workflows
// ----------------------------------------------------------------------------
bool AppServices::registerUser(const std::string &username, const std::string &email, 
                               const std::string &password, const std::string &fullName, 
                               const std::string &phone, const std::string &address, 
                               std::string &outError, std::string &outToken) {
    if (!Utils::isValidEmail(email)) {
        outError = "Please enter a valid email address.";
        return false;
    }
    if (password.length() < 6) {
        outError = "Password must be at least 6 characters long.";
        return false;
    }
    if (username.empty() || fullName.empty()) {
        outError = "Please fill in all required fields.";
        return false;
    }

    Models::User existing;
    if (Database::DatabaseManager::instance().getUserByEmail(email, existing)) {
        outError = "An account with this email address already exists.";
        return false;
    }

    std::string salt = Utils::Crypto::generateSalt(16);
    std::string hash = Utils::Crypto::hashPassword(password, salt);

    Models::User newUser;
    newUser.username = username;
    newUser.email = email;
    newUser.passwordHash = hash;
    newUser.salt = salt;
    newUser.fullName = fullName;
    newUser.phone = phone;
    newUser.address = address;

    int newId = 0;
    if (!Database::DatabaseManager::instance().createUser(newUser, newId)) {
        outError = "Failed to create user account. Username or email may already be in use.";
        return false;
    }

    outToken = SessionService::instance().createSession(newId, 0, "customer", email, fullName);
    return true;
}

bool AppServices::loginUser(const std::string &email, const std::string &password, 
                            std::string &outError, std::string &outToken, Models::User &outUser) {
    if (email.empty() || password.empty()) {
        outError = "Please enter your email and password.";
        return false;
    }

    if (!Database::DatabaseManager::instance().getUserByEmail(email, outUser)) {
        outError = "Invalid email or password.";
        return false;
    }

    if (!Utils::Crypto::verifyPassword(password, outUser.salt, outUser.passwordHash)) {
        outError = "Invalid email or password.";
        return false;
    }

    outToken = SessionService::instance().createSession(outUser.id, 0, "customer", outUser.email, outUser.fullName);
    return true;
}

// ----------------------------------------------------------------------------
// Admin Workflows
// ----------------------------------------------------------------------------
bool AppServices::setupFirstAdmin(const std::string &username, const std::string &email, 
                                  const std::string &password, const std::string &fullName, 
                                  std::string &outError) {
    int adminCount = Database::DatabaseManager::instance().getAdminCount();
    if (adminCount > 0) {
        outError = "Admin setup is already completed. Initial administrator exists.";
        return false;
    }

    if (!Utils::isValidEmail(email)) {
        outError = "Please enter a valid administrator email address.";
        return false;
    }
    if (password.length() < 8) {
        outError = "Admin password must be at least 8 characters long.";
        return false;
    }

    std::string salt = Utils::Crypto::generateSalt(16);
    std::string hash = Utils::Crypto::hashPassword(password, salt);

    Models::Admin newAdmin;
    newAdmin.username = username;
    newAdmin.email = email;
    newAdmin.passwordHash = hash;
    newAdmin.salt = salt;
    newAdmin.fullName = fullName;
    newAdmin.role = "super_admin";

    int newId = 0;
    if (!Database::DatabaseManager::instance().createAdmin(newAdmin, newId)) {
        outError = "Failed to initialize administrator.";
        return false;
    }

    std::cout << "[Admin Security] Initial administrator created successfully with ID: " << newId << std::endl;
    return true;
}

bool AppServices::loginAdmin(const std::string &email, const std::string &password, 
                             std::string &outError, std::string &outToken, Models::Admin &outAdmin) {
    if (email.empty() || password.empty()) {
        outError = "Please enter your administrator email and password.";
        return false;
    }

    if (!Database::DatabaseManager::instance().getAdminByEmail(email, outAdmin)) {
        outError = "Invalid administrator credentials.";
        return false;
    }

    if (!Utils::Crypto::verifyPassword(password, outAdmin.salt, outAdmin.passwordHash)) {
        outError = "Invalid administrator credentials.";
        return false;
    }

    outToken = SessionService::instance().createSession(0, outAdmin.id, "admin", outAdmin.email, outAdmin.fullName);
    return true;
}

bool AppServices::processNextPriorityOrder(Algorithms::OrderPriorityItem &outOrder) {
    if (!orderQueue_.pop(outOrder)) {
        return false; // Queue empty
    }

    // Update status in SQLite Database to 'Processing'
    Database::DatabaseManager::instance().updateOrderStatus(outOrder.orderId, "Processing");
    return true;
}

// ----------------------------------------------------------------------------
// Checkout & Cart Workflows
// ----------------------------------------------------------------------------
bool AppServices::checkout(int userId, const std::string &sessionId, 
                           const std::string &name, const std::string &email, 
                           const std::string &phone, const std::string &address, 
                           const std::string &paymentMethod, bool isUrgent, 
                           std::string &outOrderNumber, std::string &outError) {
    auto cart = Database::DatabaseManager::instance().getCart(userId, sessionId);
    if (cart.items.empty()) {
        outError = "Your cart is empty. Please add products before checking out.";
        return false;
    }

    // Verify stock availability
    for (const auto &item : cart.items) {
        Models::Product p;
        if (!Database::DatabaseManager::instance().getProductById(item.productId, p)) {
            outError = "Product '" + item.productName + "' is no longer available.";
            return false;
        }
        if (p.stockQuantity < item.quantity) {
            outError = "Insufficient stock for '" + p.name + "'. Available: " + std::to_string(p.stockQuantity);
            return false;
        }
    }

    Models::Order newOrder;
    newOrder.orderNumber = Utils::Crypto::generateOrderNumber();
    newOrder.userId = userId;
    newOrder.customerName = name;
    newOrder.customerEmail = email;
    newOrder.customerPhone = phone;
    newOrder.shippingAddress = address;
    newOrder.paymentMethod = paymentMethod.empty() ? "Cash on Delivery" : paymentMethod;
    newOrder.paymentStatus = (paymentMethod == "Cash on Delivery") ? "Pending" : "Completed";
    newOrder.subtotal = cart.subtotal;
    newOrder.discount = cart.discount;
    newOrder.shippingFee = cart.shippingFee;
    newOrder.totalAmount = cart.total;
    newOrder.orderStatus = "Pending";
    newOrder.isUrgent = isUrgent;
    newOrder.trackingNumber = "TRK-" + newOrder.orderNumber;

    for (const auto &cItem : cart.items) {
        Models::OrderItem oItem;
        oItem.productId = cItem.productId;
        oItem.productName = cItem.productName;
        oItem.price = cItem.productPrice;
        oItem.quantity = cItem.quantity;
        oItem.total = cItem.itemTotal;
        newOrder.items.push_back(oItem);
    }

    int orderId = 0;
    if (!Database::DatabaseManager::instance().createOrder(newOrder, orderId)) {
        outError = "Failed to create order. Please try again.";
        return false;
    }

    // Enqueue into DSA Order Priority Queue
    Algorithms::OrderPriorityItem queueItem;
    queueItem.orderId = orderId;
    queueItem.orderNumber = newOrder.orderNumber;
    queueItem.userId = userId;
    queueItem.customerName = name;
    queueItem.totalAmount = newOrder.totalAmount;
    queueItem.isUrgent = isUrgent;
    queueItem.createdAt = std::chrono::system_clock::now();
    orderQueue_.push(queueItem);

    // Update Product Graph co-purchase relationships
    for (size_t i = 0; i < newOrder.items.size(); ++i) {
        for (size_t j = i + 1; j < newOrder.items.size(); ++j) {
            graph_.recordCoPurchase(newOrder.items[i].productId, newOrder.items[j].productId);
        }
    }

    // Clear cart
    Database::DatabaseManager::instance().clearCart(userId, sessionId);

    outOrderNumber = newOrder.orderNumber;
    return true;
}

// ----------------------------------------------------------------------------
// Review Workflows
// ----------------------------------------------------------------------------
bool AppServices::submitReview(int productId, int userId, const std::string &userName, 
                               int rating, const std::string &title, const std::string &text, 
                               std::string &outError) {
    if (productId <= 0 || rating < 1 || rating > 5 || text.empty()) {
        outError = "Please provide a valid rating (1-5 stars) and review text.";
        return false;
    }

    if (userId > 0 && Database::DatabaseManager::instance().hasUserReviewed(productId, userId)) {
        outError = "You have already submitted a review for this product.";
        return false;
    }

    // Run VADER-style sentiment analysis in C++
    auto sentimentResult = sentimentAnalyzer_.analyze(text);

    Models::Review rev;
    rev.productId = productId;
    rev.userId = userId;
    rev.userName = userName.empty() ? "Customer" : userName;
    rev.rating = rating;
    rev.reviewTitle = title;
    rev.reviewText = text;
    rev.sentiment = sentimentResult.sentimentLabel;
    rev.sentimentScore = sentimentResult.compoundScore;
    rev.aspectQuality = sentimentResult.qualityAspect;
    rev.aspectDelivery = sentimentResult.deliveryAspect;
    rev.aspectPrice = sentimentResult.priceAspect;
    rev.verifiedPurchase = (userId > 0);

    int reviewId = 0;
    if (!Database::DatabaseManager::instance().addReview(rev, reviewId)) {
        outError = "Failed to submit review. Duplicate review detected.";
        return false;
    }

    // Invalidate product details in LRU cache so new rating is immediately reflected
    productCache_.remove(productId);
    return true;
}

} // namespace Services
} // namespace ShopSphere
