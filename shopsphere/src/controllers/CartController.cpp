#include "controllers/CartController.h"
#include "services/AppServices.h"
#include "utils/Crypto.h"
#include <json/json.h>

namespace ShopSphere {
namespace Controllers {

std::pair<int, std::string> CartController::getAuthContext(const drogon::HttpRequestPtr &req) {
    int userId = 0;
    auto token = req->getCookie("ss_session");
    Services::SessionInfo session;
    if (!token.empty() && Services::SessionService::instance().getSession(token, session)) {
        userId = session.userId;
    }

    std::string sessionId = req->getCookie("ss_cart_sid");
    if (sessionId.empty()) {
        sessionId = Utils::Crypto::generateToken(16);
    }
    return {userId, sessionId};
}

void CartController::getCart(const drogon::HttpRequestPtr &req, 
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto auth = getAuthContext(req);
    auto cart = Database::DatabaseManager::instance().getCart(auth.first, auth.second);

    Json::Value root;
    root["success"] = true;
    root["subtotal"] = cart.subtotal;
    root["discount"] = cart.discount;
    root["shipping_fee"] = cart.shippingFee;
    root["total"] = cart.total;
    root["total_items"] = cart.totalItems;

    Json::Value items(Json::arrayValue);
    for (const auto &item : cart.items) {
        Json::Value it;
        it["id"] = item.id;
        it["product_id"] = item.productId;
        it["product_name"] = item.productName;
        it["product_brand"] = item.productBrand;
        it["price"] = item.productPrice;
        it["image_url"] = item.productImage;
        it["quantity"] = item.quantity;
        it["stock_quantity"] = item.stockQuantity;
        it["item_total"] = item.itemTotal;
        items.append(it);
    }
    root["items"] = items;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    if (req->getCookie("ss_cart_sid").empty()) {
        drogon::Cookie cookie("ss_cart_sid", auth.second);
        cookie.setPath("/");
        resp->addCookie(cookie);
    }
    callback(resp);
}

void CartController::addToCart(const drogon::HttpRequestPtr &req, 
                               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto auth = getAuthContext(req);
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
    int quantity = (*json).get("quantity", 1).asInt();

    Models::Product p;
    if (!Database::DatabaseManager::instance().getProductById(productId, p)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Product not found.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    if (p.stockQuantity < quantity) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Insufficient stock.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    bool added = Database::DatabaseManager::instance().addToCart(auth.first, auth.second, productId, quantity);
    Database::DatabaseManager::instance().logUserActivity(auth.first, auth.second, "cart_add", productId);

    Json::Value root;
    root["success"] = added;
    root["message"] = added ? "Added to cart." : "Failed to add to cart.";

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    if (req->getCookie("ss_cart_sid").empty()) {
        drogon::Cookie cookie("ss_cart_sid", auth.second);
        cookie.setPath("/");
        resp->addCookie(cookie);
    }
    callback(resp);
}

void CartController::updateQuantity(const drogon::HttpRequestPtr &req, 
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto auth = getAuthContext(req);
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
    int quantity = (*json).get("quantity", 1).asInt();

    bool updated = Database::DatabaseManager::instance().updateCartQuantity(auth.first, auth.second, productId, quantity);

    Json::Value root;
    root["success"] = updated;
    root["message"] = updated ? "Cart updated." : "Failed to update quantity.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void CartController::removeItem(const drogon::HttpRequestPtr &req, 
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto auth = getAuthContext(req);
    auto json = req->getJsonObject();
    int productId = 0;
    if (json) {
        productId = (*json).get("product_id", 0).asInt();
    } else {
        auto pParam = req->getParameter("product_id");
        if (!pParam.empty()) {
            try { productId = std::stoi(pParam); } catch (...) {}
        }
    }

    bool removed = Database::DatabaseManager::instance().removeFromCart(auth.first, auth.second, productId);

    Json::Value root;
    root["success"] = removed;
    root["message"] = removed ? "Item removed from cart." : "Failed to remove item.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void CartController::clearCart(const drogon::HttpRequestPtr &req, 
                               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto auth = getAuthContext(req);
    bool cleared = Database::DatabaseManager::instance().clearCart(auth.first, auth.second);

    Json::Value root;
    root["success"] = cleared;
    root["message"] = "Cart cleared.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
