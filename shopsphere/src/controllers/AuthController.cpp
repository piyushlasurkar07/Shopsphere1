#include "controllers/AuthController.h"
#include "services/AppServices.h"
#include "utils/StringUtils.h"
#include <json/json.h>

namespace ShopSphere {
namespace Controllers {

void AuthController::registerUser(const drogon::HttpRequestPtr &req, 
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid request payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string username = (*json).get("username", "").asString();
    std::string email = (*json).get("email", "").asString();
    std::string password = (*json).get("password", "").asString();
    std::string fullName = (*json).get("full_name", "").asString();
    std::string phone = (*json).get("phone", "").asString();
    std::string address = (*json).get("address", "").asString();

    std::string errorMsg, token;
    bool success = Services::AppServices::instance().registerUser(username, email, password, fullName, phone, address, errorMsg, token);

    Json::Value root;
    root["success"] = success;
    if (success) {
        root["message"] = "Registration successful.";
        root["token"] = token;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        drogon::Cookie cookie("ss_session", token);
        cookie.setPath("/");
        cookie.setHttpOnly(true);
        resp->addCookie(cookie);
        callback(resp);
    } else {
        root["message"] = errorMsg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    }
}

void AuthController::loginUser(const drogon::HttpRequestPtr &req, 
                               std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid request payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string email = (*json).get("email", "").asString();
    std::string password = (*json).get("password", "").asString();

    std::string errorMsg, token;
    Models::User user;
    bool success = Services::AppServices::instance().loginUser(email, password, errorMsg, token, user);

    Json::Value root;
    root["success"] = success;
    if (success) {
        root["message"] = "Login successful.";
        root["token"] = token;
        Json::Value u;
        u["id"] = user.id;
        u["username"] = user.username;
        u["email"] = user.email;
        u["full_name"] = user.fullName;
        root["user"] = u;

        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        drogon::Cookie cookie("ss_session", token);
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

void AuthController::logoutUser(const drogon::HttpRequestPtr &req, 
                                std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto token = req->getCookie("ss_session");
    if (!token.empty()) {
        Services::SessionService::instance().destroySession(token);
    }

    Json::Value root;
    root["success"] = true;
    root["message"] = "Logged out successfully.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    drogon::Cookie cookie("ss_session", "");
    cookie.setPath("/");
    cookie.setMaxAge(0);
    resp->addCookie(cookie);
    callback(resp);
}

void AuthController::getCurrentUser(const drogon::HttpRequestPtr &req, 
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto token = req->getCookie("ss_session");
    Services::SessionInfo session;
    if (token.empty() || !Services::SessionService::instance().getSession(token, session)) {
        Json::Value root;
        root["authenticated"] = false;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        callback(resp);
        return;
    }

    Models::User user;
    Database::DatabaseManager::instance().getUserById(session.userId, user);

    Json::Value root;
    root["authenticated"] = true;
    root["id"] = user.id;
    root["username"] = user.username;
    root["email"] = user.email;
    root["full_name"] = user.fullName;
    root["phone"] = user.phone;
    root["address"] = user.address;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void AuthController::updateProfile(const drogon::HttpRequestPtr &req, 
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto token = req->getCookie("ss_session");
    Services::SessionInfo session;
    if (token.empty() || !Services::SessionService::instance().getSession(token, session) || session.userId <= 0) {
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
        err["message"] = "Invalid request payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string fullName = (*json).get("full_name", "").asString();
    std::string phone = (*json).get("phone", "").asString();
    std::string address = (*json).get("address", "").asString();

    bool updated = Database::DatabaseManager::instance().updateUserProfile(session.userId, fullName, phone, address);
    Json::Value root;
    root["success"] = updated;
    root["message"] = updated ? "Profile updated successfully." : "Failed to update profile.";
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
