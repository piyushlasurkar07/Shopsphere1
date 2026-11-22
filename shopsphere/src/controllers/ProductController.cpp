#include "controllers/ProductController.h"
#include "services/AppServices.h"
#include <json/json.h>

namespace ShopSphere {
namespace Controllers {

void ProductController::getProducts(const drogon::HttpRequestPtr &req, 
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int categoryId = 0;
    auto catParam = req->getParameter("category_id");
    if (!catParam.empty()) {
        try { categoryId = std::stoi(catParam); } catch (...) {}
    }

    std::string q = req->getParameter("q");
    double minPrice = 0.0, maxPrice = 0.0;
    auto minP = req->getParameter("min_price");
    auto maxP = req->getParameter("max_price");
    if (!minP.empty()) { try { minPrice = std::stod(minP); } catch (...) {} }
    if (!maxP.empty()) { try { maxPrice = std::stod(maxP); } catch (...) {} }
    std::string sortBy = req->getParameter("sort");

    auto products = Services::AppServices::instance().getProducts(categoryId, q, minPrice, maxPrice, sortBy);

    Json::Value root;
    root["success"] = true;
    root["count"] = static_cast<int>(products.size());
    Json::Value items(Json::arrayValue);

    for (const auto &p : products) {
        Json::Value item;
        item["id"] = p.id;
        item["name"] = p.name;
        item["brand"] = p.brand;
        item["category_id"] = p.categoryId;
        item["category_name"] = p.categoryName;
        item["description"] = p.description;
        item["price"] = p.price;
        item["original_price"] = p.originalPrice;
        item["discount_percent"] = p.discountPercent;
        item["rating"] = p.rating;
        item["review_count"] = p.reviewCount;
        item["stock_quantity"] = p.stockQuantity;
        item["image_url"] = p.imageUrl;
        item["is_featured"] = p.isFeatured;
        item["is_trending"] = p.isTrending;
        item["is_bestseller"] = p.isBestseller;
        items.append(item);
    }
    root["products"] = items;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void ProductController::getProductDetails(const drogon::HttpRequestPtr &req, 
                                         std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                         int id) {
    std::string jsonStr;
    if (!Services::AppServices::instance().getProductDetailsJson(id, jsonStr)) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Product not found.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    resp->setBody(jsonStr);
    callback(resp);
}

void ProductController::getSearchSuggestions(const drogon::HttpRequestPtr &req, 
                                            std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    std::string q = req->getParameter("q");
    auto suggestions = Services::AppServices::instance().getSearchSuggestions(q, 8);

    Json::Value root;
    root["success"] = true;
    Json::Value items(Json::arrayValue);
    for (const auto &s : suggestions) {
        Json::Value item;
        item["term"] = s.term;
        item["frequency"] = s.frequency;
        items.append(item);
    }
    root["suggestions"] = items;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void ProductController::getRecommendations(const drogon::HttpRequestPtr &req, 
                                          std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                          int id) {
    auto recs = Services::AppServices::instance().getRecommendations(id, 6);

    Json::Value root;
    root["success"] = true;
    Json::Value items(Json::arrayValue);

    for (const auto &r : recs) {
        Models::Product p;
        if (Database::DatabaseManager::instance().getProductById(r.productId, p)) {
            Json::Value item;
            item["id"] = p.id;
            item["name"] = p.name;
            item["brand"] = p.brand;
            item["price"] = p.price;
            item["original_price"] = p.originalPrice;
            item["discount_percent"] = p.discountPercent;
            item["rating"] = p.rating;
            item["image_url"] = p.imageUrl;
            item["recommendation_score"] = r.score;
            item["recommendation_reason"] = r.reason;
            items.append(item);
        }
    }
    root["recommendations"] = items;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void ProductController::getCategories(const drogon::HttpRequestPtr &req, 
                                      std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto categories = Database::DatabaseManager::instance().getAllCategories();

    Json::Value root;
    root["success"] = true;
    Json::Value items(Json::arrayValue);
    for (const auto &c : categories) {
        Json::Value item;
        item["id"] = c.id;
        item["name"] = c.name;
        item["slug"] = c.slug;
        item["description"] = c.description;
        item["image_url"] = c.imageUrl;
        item["product_count"] = c.productCount;
        items.append(item);
    }
    root["categories"] = items;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void ProductController::predictPrice(const drogon::HttpRequestPtr &req, 
                                     std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid JSON payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    Algorithms::PriceSample sample;
    sample.categoryId = (*json).get("category_id", 1).asInt();
    sample.brandId = (*json).get("brand_id", 1).asInt();
    sample.originalPrice = (*json).get("original_price", 1000.0).asDouble();
    sample.discountPercent = (*json).get("discount_percent", 10).asInt();
    sample.rating = (*json).get("rating", 4.0).asDouble();
    sample.descriptionLength = (*json).get("description_length", 100).asUInt();

    auto result = Services::AppServices::instance().getPricePredictor().predict(sample);

    Json::Value root;
    root["success"] = result.success;
    root["predicted_price"] = result.predictedPrice;
    root["confidence"] = result.confidenceScore;
    root["message"] = result.statusMessage;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void ProductController::classifyCategory(const drogon::HttpRequestPtr &req, 
                                        std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Invalid JSON payload.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string text = (*json).get("text", "").asString();
    auto result = Services::AppServices::instance().getClassifier().predict(text);

    Json::Value root;
    root["success"] = result.success;
    root["predicted_category"] = result.predictedCategory;
    root["confidence"] = result.confidence;
    root["message"] = result.statusMessage;

    Json::Value probs(Json::arrayValue);
    for (const auto &p : result.classProbabilities) {
        Json::Value entry;
        entry["category"] = p.first;
        entry["probability"] = p.second;
        probs.append(entry);
    }
    root["probabilities"] = probs;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
