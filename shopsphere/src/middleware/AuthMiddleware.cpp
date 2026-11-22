#include "middleware/AuthMiddleware.h"
#include "services/AppServices.h"
#include <json/json.h>

namespace ShopSphere {
namespace Middleware {

void AdminAuthFilter::doFilter(const drogon::HttpRequestPtr &req,
                               drogon::FilterCallback &&fcb,
                               drogon::FilterChainCallback &&fccb) {
    auto token = req->getCookie("ss_admin_session");
    Services::SessionInfo session;
    if (token.empty() || !Services::SessionService::instance().getSession(token, session) || session.adminId <= 0 || session.role != "admin") {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Administrator authorization required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        fcb(resp);
        return;
    }

    fccb(); // Authorized, proceed to handler
}

void CustomerAuthFilter::doFilter(const drogon::HttpRequestPtr &req,
                                  drogon::FilterCallback &&fcb,
                                  drogon::FilterChainCallback &&fccb) {
    auto token = req->getCookie("ss_session");
    Services::SessionInfo session;
    if (token.empty() || !Services::SessionService::instance().getSession(token, session) || session.userId <= 0) {
        Json::Value err;
        err["success"] = false;
        err["message"] = "Customer login required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k401Unauthorized);
        fcb(resp);
        return;
    }

    fccb();
}

} // namespace Middleware
} // namespace ShopSphere
