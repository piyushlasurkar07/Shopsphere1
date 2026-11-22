#ifndef SHOPSPHERE_WISHLIST_CONTROLLER_H
#define SHOPSPHERE_WISHLIST_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class WishlistController : public drogon::HttpController<WishlistController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(WishlistController::getWishlist, "/api/wishlist", drogon::Get);
    ADD_METHOD_TO(WishlistController::toggleWishlist, "/api/wishlist/toggle", drogon::Post);
    ADD_METHOD_TO(WishlistController::removeFromWishlist, "/api/wishlist/remove", drogon::Delete);
    ADD_METHOD_TO(WishlistController::moveToCart, "/api/wishlist/move-to-cart", drogon::Post);
    METHOD_LIST_END

    void getWishlist(const drogon::HttpRequestPtr &req, 
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void toggleWishlist(const drogon::HttpRequestPtr &req, 
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void removeFromWishlist(const drogon::HttpRequestPtr &req, 
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void moveToCart(const drogon::HttpRequestPtr &req, 
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
    int getAuthUserId(const drogon::HttpRequestPtr &req);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_WISHLIST_CONTROLLER_H
