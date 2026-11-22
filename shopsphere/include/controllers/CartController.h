#ifndef SHOPSPHERE_CART_CONTROLLER_H
#define SHOPSPHERE_CART_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class CartController : public drogon::HttpController<CartController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CartController::getCart, "/api/cart", drogon::Get);
    ADD_METHOD_TO(CartController::addToCart, "/api/cart/add", drogon::Post);
    ADD_METHOD_TO(CartController::updateQuantity, "/api/cart/update", drogon::Put);
    ADD_METHOD_TO(CartController::removeItem, "/api/cart/remove", drogon::Delete);
    ADD_METHOD_TO(CartController::clearCart, "/api/cart/clear", drogon::Post);
    METHOD_LIST_END

    void getCart(const drogon::HttpRequestPtr &req, 
                 std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void addToCart(const drogon::HttpRequestPtr &req, 
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void updateQuantity(const drogon::HttpRequestPtr &req, 
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void removeItem(const drogon::HttpRequestPtr &req, 
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void clearCart(const drogon::HttpRequestPtr &req, 
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
    std::pair<int, std::string> getAuthContext(const drogon::HttpRequestPtr &req);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_CART_CONTROLLER_H
