#ifndef SHOPSPHERE_AUTH_CONTROLLER_H
#define SHOPSPHERE_AUTH_CONTROLLER_H

#include <drogon/HttpController.h>

namespace ShopSphere {
namespace Controllers {

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::registerUser, "/api/auth/register", drogon::Post);
    ADD_METHOD_TO(AuthController::loginUser, "/api/auth/login", drogon::Post);
    ADD_METHOD_TO(AuthController::logoutUser, "/api/auth/logout", drogon::Post);
    ADD_METHOD_TO(AuthController::getCurrentUser, "/api/auth/me", drogon::Get);
    ADD_METHOD_TO(AuthController::updateProfile, "/api/auth/profile", drogon::Put);
    METHOD_LIST_END

    void registerUser(const drogon::HttpRequestPtr &req, 
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void loginUser(const drogon::HttpRequestPtr &req, 
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void logoutUser(const drogon::HttpRequestPtr &req, 
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getCurrentUser(const drogon::HttpRequestPtr &req, 
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void updateProfile(const drogon::HttpRequestPtr &req, 
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

} // namespace Controllers
} // namespace ShopSphere

#endif // SHOPSPHERE_AUTH_CONTROLLER_H
