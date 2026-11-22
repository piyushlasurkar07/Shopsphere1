#ifndef SHOPSPHERE_CRYPTO_H
#define SHOPSPHERE_CRYPTO_H

#include <string>

namespace ShopSphere {
namespace Utils {

class Crypto {
public:
    // Generate a secure random salt string
    static std::string generateSalt(size_t length = 16);

    // Compute standard SHA-256 hex digest
    static std::string sha256(const std::string &data);

    // Compute salted and stretched password hash
    static std::string hashPassword(const std::string &password, const std::string &salt, int iterations = 10000);

    // Verify a password against salt and stored hash
    static bool verifyPassword(const std::string &password, const std::string &salt, const std::string &expectedHash);

    // Generate secure random session token
    static std::string generateToken(size_t bytes = 32);

    // Generate a unique order tracking number
    static std::string generateOrderNumber();
};

} // namespace Utils
} // namespace ShopSphere

#endif // SHOPSPHERE_CRYPTO_H
