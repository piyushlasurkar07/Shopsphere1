#ifndef SHOPSPHERE_PRODUCT_CONTROLLER_H
#define SHOPSPHERE_PRODUCT_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class ProductController : public drogon::HttpController<ProductController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ProductController::getProducts, "/api/products", drogon::Get);
    ADD_METHOD_TO(ProductController::getProductDetails, "/api/products/{id}", drogon::Get);
    ADD_METHOD_TO(ProductController::getSearchSuggestions, "/api/search/suggestions", drogon::Get);
    ADD_METHOD_TO(ProductController::getRecommendations, "/api/recommendations/{id}", drogon::Get);
    ADD_METHOD_TO(ProductController::getCategories, "/api/categories", drogon::Get);
    ADD_METHOD_TO(ProductController::predictPrice, "/api/products/predict-price", drogon::Post);
    ADD_METHOD_TO(ProductController::classifyCategory, "/api/products/classify-category", drogon::Post);
    METHOD_LIST_END

    void getProducts(const drogon::HttpRequestPtr &req, 
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getProductDetails(const drogon::HttpRequestPtr &req, 
                           std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                           int id);

    void getSearchSuggestions(const drogon::HttpRequestPtr &req, 
                              std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getRecommendations(const drogon::HttpRequestPtr &req, 
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                            int id);

    void getCategories(const drogon::HttpRequestPtr &req, 
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void predictPrice(const drogon::HttpRequestPtr &req, 
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void classifyCategory(const drogon::HttpRequestPtr &req, 
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_PRODUCT_CONTROLLER_H
