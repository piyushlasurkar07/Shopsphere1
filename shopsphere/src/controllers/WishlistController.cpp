#include "controllers/WishlistController.h"
#include "services/AppServices.h"
#include <json/json.h>

namespace ShopSphere {
namespace Controllers {

int WishlistController::getAuthUserId(const drogon::HttpRequestPtr &req) {
    auto token = req->getCookie("ss_session");
    Services::SessionInfo session;
    if (!token.empty() && Services::SessionService::instance().getSession(token, session)) {
        return session.userId;
    }
    return 0;
}

void WishlistController::getWishlist(const drogon::HttpRequestPtr &req, 
                                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int userId = getAuthUserId(req);
    if (userId <= 0) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Sign in to view your wishlist.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto items = Database::DatabaseManager::instance().getWishlist(userId);
    Json::Value root;
    root["success"] = true;
    Json::Value arr(Json::arrayValue);

    for (const auto &item : items) {
        Json::Value it;
        it["id"] = item.id;
        it["product_id"] = item.productId;
        it["product_name"] = item.productName;
        it["product_brand"] = item.productBrand;
        it["price"] = item.productPrice;
        it["original_price"] = item.originalPrice;
        it["discount_percent"] = item.discountPercent;
        it["image_url"] = item.productImage;
        it["stock_quantity"] = item.stockQuantity;
        it["added_at"] = item.addedAt;
        arr.append(it);
    }
    root["items"] = arr;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void WishlistController::toggleWishlist(const drogon::HttpRequestPtr &req, 
                                       std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int userId = getAuthUserId(req);
    if (userId <= 0) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Sign in to save items to your wishlist.";
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

    int productId = (*json).get("product_id", 0).asInt();
    bool inList = Database::DatabaseManager::instance().isInWishlist(userId, productId);

    Json::Value root;
    if (inList) {
        Database::DatabaseManager::instance().removeFromWishlist(userId, productId);
        root["success"] = true;
        root["in_wishlist"] = false;
        root["message"] = "Removed from wishlist.";
    } else {
        Database::DatabaseManager::instance().addToWishlist(userId, productId);
        Database::DatabaseManager::instance().logUserActivity(userId, "", "wishlist_add", productId);
        root["success"] = true;
        root["in_wishlist"] = true;
        root["message"] = "Added to wishlist.";
    }

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void WishlistController::removeFromWishlist(const drogon::HttpRequestPtr &req, 
                                           std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int userId = getAuthUserId(req);
    if (userId <= 0) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Authentication required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    int productId = 0;
    auto json = req->getJsonObject();
    if (json) {
        productId = (*json).get("product_id", 0).asInt();
    } else {
        auto pParam = req->getParameter("product_id");
        if (!pParam.empty()) {
            try { productId = std::stoi(pParam); } catch (...) {}
        }
    }

    bool removed = Database::DatabaseManager::instance().removeFromWishlist(userId, productId);
    Json::Value root;
    root["success"] = removed;
    root["message"] = removed ? "Removed from wishlist." : "Failed to remove.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void WishlistController::moveToCart(const drogon::HttpRequestPtr &req, 
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int userId = getAuthUserId(req);
    if (userId <= 0) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Authentication required.";
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

    int productId = (*json).get("product_id", 0).asInt();
    // Add to cart
    Database::DatabaseManager::instance().addToCart(userId, "", productId, 1);
    // Remove from wishlist
    Database::DatabaseManager::instance().removeFromWishlist(userId, productId);

    Json::Value root;
    root["success"] = true;
    root["message"] = "Moved item to cart.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
