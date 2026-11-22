#ifndef SHOPSPHERE_REVIEW_CONTROLLER_H
#define SHOPSPHERE_REVIEW_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class ReviewController : public drogon::HttpController<ReviewController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ReviewController::getProductReviews, "/api/products/{id}/reviews", drogon::Get);
    ADD_METHOD_TO(ReviewController::submitReview, "/api/reviews", drogon::Post);
    ADD_METHOD_TO(ReviewController::analyzeReviewDraft, "/api/reviews/analyze", drogon::Post);
    METHOD_LIST_END

    void getProductReviews(const drogon::HttpRequestPtr &req, 
                           std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                           int id);

    void submitReview(const drogon::HttpRequestPtr &req, 
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void analyzeReviewDraft(const drogon::HttpRequestPtr &req, 
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_REVIEW_CONTROLLER_H
