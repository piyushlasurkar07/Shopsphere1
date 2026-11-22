#ifndef SHOPSPHERE_AUTH_MIDDLEWARE_H
#define SHOPSPHERE_AUTH_MIDDLEWARE_H

#include <drogon/HttpFilter.h>

namespace ShopSphere {
namespace Middleware {

class AdminAuthFilter : public drogon::HttpFilter<AdminAuthFilter> {
public:
    void doFilter(const drogon::HttpRequestPtr &req,
                  drogon::FilterCallback &&fcb,
                  drogon::FilterChainCallback &&fccb) override;
};

class CustomerAuthFilter : public drogon::HttpFilter<CustomerAuthFilter> {
public:
    void doFilter(const drogon::HttpRequestPtr &req,
                  drogon::FilterCallback &&fcb,
                  drogon::FilterChainCallback &&fccb) override;
};

} // namespace Middleware
} // namespace ShopSphere

#endif // SHOPSPHERE_AUTH_MIDDLEWARE_H
