#include "controllers/OrderController.h"
#include "services/AppServices.h"
#include "utils/Crypto.h"
#include <json/json.h>

namespace ShopSphere {
namespace Controllers {

std::pair<int, std::string> OrderController::getAuthContext(const drogon::HttpRequestPtr &req) {
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

void OrderController::checkout(const drogon::HttpRequestPtr &req, 
                               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto auth = getAuthContext(req);
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid checkout data.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string name = (*json).get("name", "").asString();
    std::string email = (*json).get("email", "").asString();
    std::string phone = (*json).get("phone", "").asString();
    std::string address = (*json).get("address", "").asString();
    std::string paymentMethod = (*json).get("payment_method", "Cash on Delivery").asString();
    bool isUrgent = (*json).get("is_urgent", false).asBool();

    if (name.empty() || email.empty() || phone.empty() || address.empty()) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Please fill in all shipping details.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string orderNumber, errorMsg;
    bool success = Services::AppServices::instance().checkout(auth.first, auth.second, name, email, phone, address, paymentMethod, isUrgent, orderNumber, errorMsg);

    Json::Value root;
    root["success"] = success;
    if (success) {
        root["message"] = "Order placed successfully!";
        root["order_number"] = orderNumber;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        callback(resp);
    } else {
        root["message"] = errorMsg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    }
}

void OrderController::getUserOrders(const drogon::HttpRequestPtr &req, 
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto auth = getAuthContext(req);
    if (auth.first <= 0) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Sign in to view your orders.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    auto orders = Database::DatabaseManager::instance().getOrdersByUserId(auth.first);

    Json::Value root;
    root["success"] = true;
    Json::Value arr(Json::arrayValue);

    for (const auto &o : orders) {
        Json::Value item;
        item["id"] = o.id;
        item["order_number"] = o.orderNumber;
        item["customer_name"] = o.customerName;
        item["total_amount"] = o.totalAmount;
        item["order_status"] = o.orderStatus;
        item["payment_method"] = o.paymentMethod;
        item["payment_status"] = o.paymentStatus;
        item["tracking_number"] = o.trackingNumber;
        item["is_urgent"] = o.isUrgent;
        item["created_at"] = o.createdAt;
        item["items_count"] = static_cast<int>(o.items.size());
        arr.append(item);
    }
    root["orders"] = arr;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void OrderController::getOrderDetails(const drogon::HttpRequestPtr &req, 
                                      std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                      int id) {
    auto auth = getAuthContext(req);
    Models::Order o;
    if (!Database::DatabaseManager::instance().getOrderById(id, o)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Order not found.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    if (auth.first > 0 && o.userId != auth.first) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Access denied.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k403Forbidden);
        callback(resp);
        return;
    }

    Json::Value root;
    root["success"] = true;
    root["id"] = o.id;
    root["order_number"] = o.orderNumber;
    root["customer_name"] = o.customerName;
    root["customer_email"] = o.customerEmail;
    root["customer_phone"] = o.customerPhone;
    root["shipping_address"] = o.shippingAddress;
    root["payment_method"] = o.paymentMethod;
    root["payment_status"] = o.paymentStatus;
    root["subtotal"] = o.subtotal;
    root["discount"] = o.discount;
    root["shipping_fee"] = o.shippingFee;
    root["total_amount"] = o.totalAmount;
    root["order_status"] = o.orderStatus;
    root["tracking_number"] = o.trackingNumber;
    root["is_urgent"] = o.isUrgent;
    root["created_at"] = o.createdAt;

    Json::Value items(Json::arrayValue);
    for (const auto &it : o.items) {
        Json::Value item;
        item["product_id"] = it.productId;
        item["product_name"] = it.productName;
        item["price"] = it.price;
        item["quantity"] = it.quantity;
        item["total"] = it.total;
        items.append(item);
    }
    root["items"] = items;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void OrderController::cancelOrder(const drogon::HttpRequestPtr &req, 
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                  int id) {
    auto auth = getAuthContext(req);
    bool cancelled = Database::DatabaseManager::instance().cancelOrder(id, auth.first);
    if (cancelled) {
        // Also remove from DSA Priority Queue
        Services::AppServices::instance().getPriorityQueue().cancelOrder(id);
    }

    Json::Value root;
    root["success"] = cancelled;
    root["message"] = cancelled ? "Order cancelled successfully." : "Unable to cancel order. Only Pending or Processing orders can be cancelled.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void OrderController::trackOrder(const drogon::HttpRequestPtr &req, 
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    std::string trk = req->getParameter("number");
    if (trk.empty()) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Tracking or order number is required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    // Strip "TRK-" if present
    if (trk.rfind("TRK-", 0) == 0) {
        trk = trk.substr(4);
    }

    Models::Order o;
    if (!Database::DatabaseManager::instance().getOrderByNumber(trk, o)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "No order found matching tracking number.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    Json::Value root;
    root["success"] = true;
    root["order_number"] = o.orderNumber;
    root["tracking_number"] = o.trackingNumber;
    root["status"] = o.orderStatus;
    root["created_at"] = o.createdAt;
    root["customer_name"] = o.customerName;
    root["total_amount"] = o.totalAmount;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
