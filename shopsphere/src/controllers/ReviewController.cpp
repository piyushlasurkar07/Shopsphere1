#include "controllers/ReviewController.h"
#include "services/AppServices.h"
#include <json/json.h>
#include <cmath>
namespace ShopSphere {
namespace Controllers {

void ReviewController::getProductReviews(const drogon::HttpRequestPtr &req, 
                                         std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                                         int id) {
    auto reviews = Database::DatabaseManager::instance().getReviewsByProduct(id);

    Json::Value root;
    root["success"] = true;
    root["count"] = static_cast<int>(reviews.size());

    double avg = 0.0;
    int totalRating = 0;
    Json::Value arr(Json::arrayValue);

    for (const auto &r : reviews) {
        Json::Value it;
        it["id"] = r.id;
        it["user_name"] = r.userName;
        it["rating"] = r.rating;
        it["title"] = r.reviewTitle;
        it["text"] = r.reviewText;
        it["sentiment"] = r.sentiment;
        it["sentiment_score"] = r.sentimentScore;
        it["aspect_quality"] = r.aspectQuality;
        it["aspect_delivery"] = r.aspectDelivery;
        it["aspect_price"] = r.aspectPrice;
        it["verified_purchase"] = r.verifiedPurchase;
        it["created_at"] = r.createdAt;
        arr.append(it);

        totalRating += r.rating;
    }

    if (!reviews.empty()) {
        avg = static_cast<double>(totalRating) / reviews.size();
    }

    root["average_rating"] = std::round(avg * 10.0) / 10.0;
    root["reviews"] = arr;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

void ReviewController::submitReview(const drogon::HttpRequestPtr &req, 
                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    int userId = 0;
    std::string userName = "Verified Customer";
    auto token = req->getCookie("ss_session");
    Services::SessionInfo session;
    if (!token.empty() && Services::SessionService::instance().getSession(token, session)) {
        userId = session.userId;
        userName = session.fullName;
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
    int rating = (*json).get("rating", 5).asInt();
    std::string title = (*json).get("title", "").asString();
    std::string text = (*json).get("text", "").asString();
    if (userName == "Verified Customer" && (*json).isMember("user_name")) {
        userName = (*json).get("user_name", "Customer").asString();
    }

    std::string errorMsg;
    bool success = Services::AppServices::instance().submitReview(productId, userId, userName, rating, title, text, errorMsg);

    Json::Value root;
    root["success"] = success;
    if (success) {
        root["message"] = "Review submitted successfully!";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        callback(resp);
    } else {
        root["message"] = errorMsg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    }
}

void ReviewController::analyzeReviewDraft(const drogon::HttpRequestPtr &req, 
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

    std::string text = (*json).get("text", "").asString();
    auto result = Services::AppServices::instance().getSentimentAnalyzer().analyze(text);

    Json::Value root;
    root["success"] = true;
    root["sentiment"] = result.sentimentLabel;
    root["score"] = result.compoundScore;
    root["aspect_quality"] = result.qualityAspect;
    root["aspect_delivery"] = result.deliveryAspect;
    root["aspect_price"] = result.priceAspect;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
    callback(resp);
}

} // namespace Controllers
} // namespace ShopSphere
