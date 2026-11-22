#ifndef SHOPSPHERE_ADMIN_CONTROLLER_H
#define SHOPSPHERE_ADMIN_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class AdminController : public drogon::HttpController<AdminController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AdminController::setupFirstAdmin, "/api/admin/setup", drogon::Post);
    ADD_METHOD_TO(AdminController::loginAdmin, "/api/admin/login", drogon::Post);
    ADD_METHOD_TO(AdminController::logoutAdmin, "/api/admin/logout", drogon::Post);
    ADD_METHOD_TO(AdminController::checkSetupStatus, "/api/admin/setup-status", drogon::Get);
    ADD_METHOD_TO(AdminController::getStats, "/api/admin/stats", drogon::Get);
    ADD_METHOD_TO(AdminController::getProducts, "/api/admin/products", drogon::Get);
    ADD_METHOD_TO(AdminController::createProduct, "/api/admin/products", drogon::Post);
    ADD_METHOD_TO(AdminController::updateProduct, "/api/admin/products/{id}", drogon::Put);
    ADD_METHOD_TO(AdminController::deleteProduct, "/api/admin/products/{id}", drogon::Delete);
    ADD_METHOD_TO(AdminController::getOrders, "/api/admin/orders", drogon::Get);
    ADD_METHOD_TO(AdminController::updateOrderStatus, "/api/admin/orders/{id}/status", drogon::Put);
    ADD_METHOD_TO(AdminController::getPriorityQueueSnapshot, "/api/admin/orders/priority-queue", drogon::Get);
    ADD_METHOD_TO(AdminController::processNextPriorityOrder, "/api/admin/orders/process-priority", drogon::Post);
    ADD_METHOD_TO(AdminController::getCustomers, "/api/admin/customers", drogon::Get);
    ADD_METHOD_TO(AdminController::getReviews, "/api/admin/reviews", drogon::Get);
    ADD_METHOD_TO(AdminController::deleteReview, "/api/admin/reviews/{id}", drogon::Delete);
    ADD_METHOD_TO(AdminController::updateCredentials, "/api/admin/credentials", drogon::Put);
    METHOD_LIST_END

    void setupFirstAdmin(const drogon::HttpRequestPtr &req, 
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void checkSetupStatus(const drogon::HttpRequestPtr &req, 
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void loginAdmin(const drogon::HttpRequestPtr &req, 
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void logoutAdmin(const drogon::HttpRequestPtr &req, 
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getStats(const drogon::HttpRequestPtr &req, 
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getProducts(const drogon::HttpRequestPtr &req, 
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void createProduct(const drogon::HttpRequestPtr &req, 
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void updateProduct(const drogon::HttpRequestPtr &req, 
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                       int id);

    void deleteProduct(const drogon::HttpRequestPtr &req, 
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                       int id);

    void getOrders(const drogon::HttpRequestPtr &req, 
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void updateOrderStatus(const drogon::HttpRequestPtr &req, 
                           std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                           int id);

    void getPriorityQueueSnapshot(const drogon::HttpRequestPtr &req, 
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void processNextPriorityOrder(const drogon::HttpRequestPtr &req, 
                                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getCustomers(const drogon::HttpRequestPtr &req, 
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getReviews(const drogon::HttpRequestPtr &req, 
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void deleteReview(const drogon::HttpRequestPtr &req, 
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                      int id);

    void updateCredentials(const drogon::HttpRequestPtr &req, 
                           std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
    bool verifyAdminAuth(const drogon::HttpRequestPtr &req, int &outAdminId);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_ADMIN_CONTROLLER_H
