#include "controllers/AdminController.h"
#include "services/AppServices.h"
#include "utils/Crypto.h"
#include "utils/StringUtils.h"
#include <json/json.h>

namespace ShopSphere {
namespace Controllers {

bool AdminController::verifyAdminAuth(const drogon::HttpRequestPtr &req, int &outAdminId) {
    auto token = req->getCookie("ss_admin_session");
    Services::SessionInfo session;
    if (!token.empty() && Services::SessionService::instance().getSession(token, session)) {
        if (session.adminId > 0 && session.role == "admin") {
            outAdminId = session.adminId;
            return true;
        }
    }
    return false;
}

void AdminController::checkSetupStatus(const drogon::HttpRequestPtr &req, 
                                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int count = Database::DatabaseManager::instance().getAdminCount();
    Json::Value root;
    root["setup_required"] = (count == 0);
    root["admin_count"] = count;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::setupFirstAdmin(const drogon::HttpRequestPtr &req, 
                                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string username = (*json).get("username", "").asString();
    std::string email = (*json).get("email", "").asString();
    std::string password = (*json).get("password", "").asString();
    std::string fullName = (*json).get("full_name", "").asString();

    std::string errorMsg;
    bool success = Services::AppServices::instance().setupFirstAdmin(username, email, password, fullName, errorMsg);

    Json::Value root;
    root["success"] = success;
    root["message"] = success ? "Initial administrator created successfully. You can now log in." : errorMsg;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    if (!success) resp->setStatusCode(drogon::k400BadRequest);
    callback(resp);
}

void AdminController::loginAdmin(const drogon::HttpRequestPtr &req, 
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string email = (*json).get("email", "").asString();
    std::string password = (*json).get("password", "").asString();

    std::string errorMsg, token;
    Models::Admin admin;
    bool success = Services::AppServices::instance().loginAdmin(email, password, errorMsg, token, admin);

    Json::Value root;
    root["success"] = success;
    if (success) {
        root["message"] = "Admin authenticated successfully.";
        root["token"] = token;
        Json::Value a;
        a["id"] = admin.id;
        a["username"] = admin.username;
        a["email"] = admin.email;
        a["full_name"] = admin.fullName;
        root["admin"] = a;

        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        drogon::Cookie cookie("ss_admin_session", token);
        cookie.setPath("/");
        cookie.setHttpOnly(true);
        resp->addCookie(cookie);
        callback(resp);
    } else {
        root["message"] = errorMsg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
    }
}

void AdminController::logoutAdmin(const drogon::HttpRequestPtr &req, 
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto token = req->getCookie("ss_admin_session");
    if (!token.empty()) {
        Services::SessionService::instance().destroySession(token);
    }

    Json::Value root;
    root["success"] = true;
    root["message"] = "Logged out from admin panel.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    drogon::Cookie cookie("ss_admin_session", "");
    cookie.setPath("/");
    cookie.setMaxAge(0);
    resp->addCookie(cookie);
    callback(resp);
}

void AdminController::getStats(const drogon::HttpRequestPtr &req, 
                               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    // Dynamic database statistics
    auto stats = Database::DatabaseManager::instance().getAdminStats();
    // LRU Cache telemetry
    auto cacheStats = Services::AppServices::instance().getProductCache().getStats();
    // Graph telemetry
    size_t graphNodes = Services::AppServices::instance().getGraph().nodeCount();
    size_t graphEdges = Services::AppServices::instance().getGraph().edgeCount();
    // Trie telemetry
    size_t trieTerms = Services::AppServices::instance().getTrie().size();
    // Priority queue size
    size_t pqOrders = Services::AppServices::instance().getPriorityQueue().size();

    Json::Value root;
    root["success"] = true;
    root["total_products"] = stats.totalProducts;
    root["total_customers"] = stats.totalCustomers;
    root["total_orders"] = stats.totalOrders;
    root["total_revenue"] = stats.totalRevenue;
    root["pending_orders"] = stats.pendingOrders;
    root["completed_orders"] = stats.completedOrders;
    root["total_reviews"] = stats.totalReviews;
    root["average_rating"] = stats.averageRating;

    // Advanced DSA Telemetry
    Json::Value dsa;
    dsa["cache_hits"] = static_cast<Json::UInt64>(cacheStats.hits);
    dsa["cache_misses"] = static_cast<Json::UInt64>(cacheStats.misses);
    dsa["cache_evictions"] = static_cast<Json::UInt64>(cacheStats.evictions);
    dsa["cache_size"] = static_cast<Json::UInt64>(cacheStats.size);
    dsa["cache_capacity"] = static_cast<Json::UInt64>(cacheStats.capacity);
    dsa["cache_hit_ratio"] = cacheStats.hitRatio;
    dsa["graph_nodes"] = static_cast<Json::UInt64>(graphNodes);
    dsa["graph_edges"] = static_cast<Json::UInt64>(graphEdges);
    dsa["trie_terms_indexed"] = static_cast<Json::UInt64>(trieTerms);
    dsa["priority_queue_size"] = static_cast<Json::UInt64>(pqOrders);
    root["dsa_telemetry"] = dsa;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::getProducts(const drogon::HttpRequestPtr &req, 
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto products = Database::DatabaseManager::instance().getAllProducts();
    Json::Value root;
    root["success"] = true;
    Json::Value arr(Json::arrayValue);

    for (const auto &p : products) {
        Json::Value it;
        it["id"] = p.id;
        it["name"] = p.name;
        it["brand"] = p.brand;
        it["category_id"] = p.categoryId;
        it["category_name"] = p.categoryName;
        it["price"] = p.price;
        it["original_price"] = p.originalPrice;
        it["discount_percent"] = p.discountPercent;
        it["stock_quantity"] = p.stockQuantity;
        it["rating"] = p.rating;
        it["review_count"] = p.reviewCount;
        it["image_url"] = p.imageUrl;
        it["is_featured"] = p.isFeatured;
        arr.append(it);
    }
    root["products"] = arr;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::createProduct(const drogon::HttpRequestPtr &req, 
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    Models::Product p;
    p.name = (*json).get("name", "").asString();
    p.brand = (*json).get("brand", "").asString();
    p.categoryId = (*json).get("category_id", 1).asInt();
    p.description = (*json).get("description", "").asString();
    p.specifications = (*json).get("specifications", "").asString();
    p.price = (*json).get("price", 0.0).asDouble();
    p.originalPrice = (*json).get("original_price", p.price).asDouble();
    p.discountPercent = (*json).get("discount_percent", 0).asInt();
    p.stockQuantity = (*json).get("stock_quantity", 0).asInt();
    p.imageUrl = (*json).get("image_url", "https://images.unsplash.com/photo-1505740420928-5e560c06d30e?w=500").asString();
    p.isFeatured = (*json).get("is_featured", false).asBool();
    p.isTrending = (*json).get("is_trending", false).asBool();
    p.isBestseller = (*json).get("is_bestseller", false).asBool();

    int newId = 0;
    bool success = Services::AppServices::instance().createProduct(p, newId);

    Json::Value root;
    root["success"] = success;
    if (success) {
        root["message"] = "Product created successfully.";
        root["product_id"] = newId;
    } else {
        root["message"] = "Failed to create product.";
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::updateProduct(const drogon::HttpRequestPtr &req, 
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                   int id) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    Models::Product p;
    p.id = id;
    p.name = (*json).get("name", "").asString();
    p.brand = (*json).get("brand", "").asString();
    p.categoryId = (*json).get("category_id", 1).asInt();
    p.description = (*json).get("description", "").asString();
    p.specifications = (*json).get("specifications", "").asString();
    p.price = (*json).get("price", 0.0).asDouble();
    p.originalPrice = (*json).get("original_price", p.price).asDouble();
    p.discountPercent = (*json).get("discount_percent", 0).asInt();
    p.stockQuantity = (*json).get("stock_quantity", 0).asInt();
    p.imageUrl = (*json).get("image_url", "").asString();
    p.isFeatured = (*json).get("is_featured", false).asBool();
    p.isTrending = (*json).get("is_trending", false).asBool();
    p.isBestseller = (*json).get("is_bestseller", false).asBool();

    bool success = Services::AppServices::instance().updateProduct(p);
    Json::Value root;
    root["success"] = success;
    root["message"] = success ? "Product updated successfully." : "Failed to update product.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::deleteProduct(const drogon::HttpRequestPtr &req, 
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                   int id) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    bool success = Services::AppServices::instance().deleteProduct(id);
    Json::Value root;
    root["success"] = success;
    root["message"] = success ? "Product deleted successfully." : "Failed to delete product.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::getOrders(const drogon::HttpRequestPtr &req, 
                               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto orders = Database::DatabaseManager::instance().getAllOrders();
    Json::Value root;
    root["success"] = true;
    Json::Value arr(Json::arrayValue);

    for (const auto &o : orders) {
        Json::Value it;
        it["id"] = o.id;
        it["order_number"] = o.orderNumber;
        it["customer_name"] = o.customerName;
        it["customer_email"] = o.customerEmail;
        it["customer_phone"] = o.customerPhone;
        it["shipping_address"] = o.shippingAddress;
        it["total_amount"] = o.totalAmount;
        it["order_status"] = o.orderStatus;
        it["payment_method"] = o.paymentMethod;
        it["payment_status"] = o.paymentStatus;
        it["tracking_number"] = o.trackingNumber;
        it["is_urgent"] = o.isUrgent;
        it["priority_score"] = o.priorityScore;
        it["created_at"] = o.createdAt;
        it["items_count"] = static_cast<int>(o.items.size());
        arr.append(it);
    }
    root["orders"] = arr;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::updateOrderStatus(const drogon::HttpRequestPtr &req, 
                                       std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                       int id) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string status = (*json).get("status", "").asString();
    bool updated = Database::DatabaseManager::instance().updateOrderStatus(id, status);

    Json::Value root;
    root["success"] = updated;
    root["message"] = updated ? "Order status updated." : "Failed to update order status.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::getPriorityQueueSnapshot(const drogon::HttpRequestPtr &req, 
                                               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto snapshot = Services::AppServices::instance().getPriorityQueue().getQueueSnapshot();
    Json::Value root;
    root["success"] = true;
    root["queue_count"] = static_cast<int>(snapshot.size());

    Json::Value arr(Json::arrayValue);
    for (const auto &item : snapshot) {
        Json::Value it;
        it["order_id"] = item.orderId;
        it["order_number"] = item.orderNumber;
        it["customer_name"] = item.customerName;
        it["total_amount"] = item.totalAmount;
        it["is_urgent"] = item.isUrgent;
        it["priority_score"] = item.calculatedPriority;
        arr.append(it);
    }
    root["queued_orders"] = arr;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::processNextPriorityOrder(const drogon::HttpRequestPtr &req, 
                                              std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    Algorithms::OrderPriorityItem processedOrder;
    bool success = Services::AppServices::instance().processNextPriorityOrder(processedOrder);

    Json::Value root;
    root["success"] = success;
    if (success) {
        root["message"] = "Highest priority order extracted and marked 'Processing'.";
        root["order_id"] = processedOrder.orderId;
        root["order_number"] = processedOrder.orderNumber;
        root["priority_score"] = processedOrder.calculatedPriority;
    } else {
        root["message"] = "Priority queue is currently empty. No pending orders to process.";
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::getCustomers(const drogon::HttpRequestPtr &req, 
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto users = Database::DatabaseManager::instance().getAllUsers();
    Json::Value root;
    root["success"] = true;
    Json::Value arr(Json::arrayValue);

    for (const auto &u : users) {
        Json::Value it;
        it["id"] = u.id;
        it["username"] = u.username;
        it["email"] = u.email;
        it["full_name"] = u.fullName;
        it["phone"] = u.phone;
        it["address"] = u.address;
        it["created_at"] = u.createdAt;
        arr.append(it);
    }
    root["customers"] = arr;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::getReviews(const drogon::HttpRequestPtr &req, 
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto reviews = Database::DatabaseManager::instance().getAllReviews();
    Json::Value root;
    root["success"] = true;
    Json::Value arr(Json::arrayValue);

    for (const auto &r : reviews) {
        Json::Value it;
        it["id"] = r.id;
        it["product_id"] = r.productId;
        it["product_name"] = r.productName;
        it["user_name"] = r.userName;
        it["rating"] = r.rating;
        it["title"] = r.reviewTitle;
        it["text"] = r.reviewText;
        it["sentiment"] = r.sentiment;
        it["sentiment_score"] = r.sentimentScore;
        it["created_at"] = r.createdAt;
        arr.append(it);
    }
    root["reviews"] = arr;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::deleteReview(const drogon::HttpRequestPtr &req, 
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                  int id) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    bool deleted = Database::DatabaseManager::instance().deleteReview(id);
    Json::Value root;
    root["success"] = deleted;
    root["message"] = deleted ? "Review deleted." : "Failed to delete review.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AdminController::updateCredentials(const drogon::HttpRequestPtr &req, 
                                       std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int adminId = 0;
    if (!verifyAdminAuth(req, adminId)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Admin authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string newEmail = (*json).get("email", "").asString();
    std::string newPassword = (*json).get("password", "").asString();

    if (!Utils::isValidEmail(newEmail) || newPassword.length() < 8) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Valid email and minimum 8-character password required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string salt = Utils::Crypto::generateSalt(16);
    std::string hash = Utils::Crypto::hashPassword(newPassword, salt);
    bool updated = Database::DatabaseManager::instance().updateAdminCredentials(adminId, newEmail, hash, salt);

    Json::Value root;
    root["success"] = updated;
    root["message"] = updated ? "Admin credentials updated successfully." : "Failed to update credentials.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
