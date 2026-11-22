#ifndef SHOPSPHERE_ORDER_CONTROLLER_H
#define SHOPSPHERE_ORDER_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class OrderController : public drogon::HttpController<OrderController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(OrderController::checkout, "/api/orders/checkout", drogon::Post);
    ADD_METHOD_TO(OrderController::getUserOrders, "/api/orders", drogon::Get);
    ADD_METHOD_TO(OrderController::getOrderDetails, "/api/orders/{id}", drogon::Get);
    ADD_METHOD_TO(OrderController::cancelOrder, "/api/orders/{id}/cancel", drogon::Post);
    ADD_METHOD_TO(OrderController::trackOrder, "/api/orders/track", drogon::Get);
    METHOD_LIST_END

    void checkout(const drogon::HttpRequestPtr &req, 
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getUserOrders(const drogon::HttpRequestPtr &req, 
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getOrderDetails(const drogon::HttpRequestPtr &req, 
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                         int id);

    void cancelOrder(const drogon::HttpRequestPtr &req, 
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                     int id);

    void trackOrder(const drogon::HttpRequestPtr &req, 
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
    std::pair<int, std::string> getAuthContext(const drogon::HttpRequestPtr &req);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_ORDER_CONTROLLER_H
