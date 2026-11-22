#ifndef SHOPSPHERE_DATABASE_MANAGER_H
#define SHOPSPHERE_DATABASE_MANAGER_H

#include "models/Models.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>

struct sqlite3;
struct sqlite3_stmt;

namespace ShopSphere {
namespace Database {

class DatabaseManager {
public:
    static DatabaseManager &instance();

    bool initialize(const std::string &dbPath, const std::string &schemaSqlPath = "");
    void close();

    // User Operations
    bool createUser(const Models::User &user, int &outId);
    bool getUserByEmail(const std::string &email, Models::User &outUser);
    bool getUserById(int userId, Models::User &outUser);
    bool updateUserProfile(int userId, const std::string &fullName, const std::string &phone, const std::string &address);
    bool updateUserPassword(int userId, const std::string &passwordHash, const std::string &salt);
    std::vector<Models::User> getAllUsers();

    // Admin Operations
    int getAdminCount();
    bool createAdmin(const Models::Admin &admin, int &outId);
    bool getAdminByEmail(const std::string &email, Models::Admin &outAdmin);
    bool getAdminById(int adminId, Models::Admin &outAdmin);
    bool updateAdminCredentials(int adminId, const std::string &email, const std::string &passwordHash, const std::string &salt);

    // Category Operations
    std::vector<Models::Category> getAllCategories();
    bool getCategoryById(int categoryId, Models::Category &outCat);
    bool createCategory(const Models::Category &cat, int &outId);

    // Product Operations
    std::vector<Models::Product> getAllProducts(int categoryId = 0, 
                                                const std::string &search = "", 
                                                double minPrice = 0.0, 
                                                double maxPrice = 0.0, 
                                                const std::string &sortBy = "");
    bool getProductById(int productId, Models::Product &outProduct);
    bool createProduct(const Models::Product &prod, int &outId);
    bool updateProduct(const Models::Product &prod);
    bool deleteProduct(int productId);
    bool updateProductStock(int productId, int quantityChange);
    std::vector<Models::Product> getFeaturedProducts(size_t limit = 8);

    // Cart Operations
    Models::Cart getCart(int userId, const std::string &sessionId);
    bool addToCart(int userId, const std::string &sessionId, int productId, int quantity);
    bool updateCartQuantity(int userId, const std::string &sessionId, int productId, int quantity);
    bool removeFromCart(int userId, const std::string &sessionId, int productId);
    bool clearCart(int userId, const std::string &sessionId);

    // Wishlist Operations
    std::vector<Models::WishlistItem> getWishlist(int userId);
    bool addToWishlist(int userId, int productId);
    bool removeFromWishlist(int userId, int productId);
    bool isInWishlist(int userId, int productId);

    // Order Operations
    bool createOrder(const Models::Order &order, int &outOrderId);
    bool getOrderById(int orderId, Models::Order &outOrder);
    bool getOrderByNumber(const std::string &orderNumber, Models::Order &outOrder);
    std::vector<Models::Order> getOrdersByUserId(int userId);
    std::vector<Models::Order> getAllOrders();
    bool updateOrderStatus(int orderId, const std::string &status);
    bool cancelOrder(int orderId, int userId = 0);

    // Review Operations
    std::vector<Models::Review> getReviewsByProduct(int productId);
    std::vector<Models::Review> getAllReviews();
    bool addReview(const Models::Review &review, int &outId);
    bool deleteReview(int reviewId);
    bool hasUserReviewed(int productId, int userId);

    // Activity Log
    void logUserActivity(int userId, const std::string &sessionId, const std::string &activityType, int productId, const std::string &details = "");
    std::vector<std::pair<int, int>> getCoPurchasedPairs();
    std::vector<std::pair<int, int>> getCoViewedPairs();

    // Admin Analytics
    Models::AdminStats getAdminStats();

    // Raw query execution helpers
    bool executeSql(const std::string &sql);

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    sqlite3 *db_{nullptr};
    std::string dbPath_;
    std::mutex dbMutex_;

    bool runSchema(const std::string &schemaPath);
    void recalculateProductRating(int productId);
};

} // namespace Database
} // namespace ShopSphere

#endif // SHOPSPHERE_DATABASE_MANAGER_H
