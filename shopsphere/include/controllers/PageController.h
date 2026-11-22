#ifndef SHOPSPHERE_PAGE_CONTROLLER_H
#define SHOPSPHERE_PAGE_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class PageController : public drogon::HttpController<PageController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PageController::homePage, "/", drogon::Get);
    ADD_METHOD_TO(PageController::productsPage, "/products", drogon::Get);
    ADD_METHOD_TO(PageController::productDetailPage, "/product/{id}", drogon::Get);
    ADD_METHOD_TO(PageController::cartPage, "/cart", drogon::Get);
    ADD_METHOD_TO(PageController::wishlistPage, "/wishlist", drogon::Get);
    ADD_METHOD_TO(PageController::checkoutPage, "/checkout", drogon::Get);
    ADD_METHOD_TO(PageController::ordersPage, "/orders", drogon::Get);
    ADD_METHOD_TO(PageController::orderSuccessPage, "/order-success", drogon::Get);
    ADD_METHOD_TO(PageController::loginPage, "/login", drogon::Get);
    ADD_METHOD_TO(PageController::registerPage, "/register", drogon::Get);
    ADD_METHOD_TO(PageController::profilePage, "/profile", drogon::Get);
    ADD_METHOD_TO(PageController::adminLoginPage, "/admin/login", drogon::Get);
    ADD_METHOD_TO(PageController::adminDashboardPage, "/admin/dashboard", drogon::Get);
    METHOD_LIST_END

    void homePage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void productsPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void productDetailPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void cartPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void wishlistPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void checkoutPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void ordersPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void orderSuccessPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void loginPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void registerPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void profilePage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void adminLoginPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void adminDashboardPage(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
    std::string loadTemplate(const std::string &relativeTemplatePath);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_PAGE_CONTROLLER_H
