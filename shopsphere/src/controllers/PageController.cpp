#include "controllers/PageController.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace ShopSphere {
namespace Controllers {

std::string PageController::loadTemplate(const std::string &relativeTemplatePath) {
    // Look in frontend/templates/ or templates/
    std::vector<std::string> searchPaths = {
        "frontend/templates/" + relativeTemplatePath,
        "templates/" + relativeTemplatePath,
        "../frontend/templates/" + relativeTemplatePath,
        "./" + relativeTemplatePath
    };

    for (const auto &path : searchPaths) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }

    return "<html><body><h1>ShopSphere</h1><p>Page template not found: " + relativeTemplatePath + "</p></body></html>";
}

void PageController::homePage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("index.html"));
    callback(resp);
}

void PageController::productsPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("products.html"));
    callback(resp);
}

void PageController::productDetailPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("product_detail.html"));
    callback(resp);
}

void PageController::cartPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("cart.html"));
    callback(resp);
}

void PageController::wishlistPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("wishlist.html"));
    callback(resp);
}

void PageController::checkoutPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("checkout.html"));
    callback(resp);
}

void PageController::ordersPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("orders.html"));
    callback(resp);
}

void PageController::orderSuccessPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("order_success.html"));
    callback(resp);
}

void PageController::loginPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("login.html"));
    callback(resp);
}

void PageController::registerPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("register.html"));
    callback(resp);
}

void PageController::profilePage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("profile.html"));
    callback(resp);
}

void PageController::adminLoginPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("admin/login.html"));
    callback(resp);
}

void PageController::adminDashboardPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp->setBody(loadTemplate("admin/dashboard.html"));
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
