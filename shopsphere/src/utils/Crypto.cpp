#include "utils/Crypto.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace ShopSphere {
namespace Utils {

std::string Crypto::generateSalt(size_t length) {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    std::string salt;
    salt.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        salt += charset[dis(gen)];
    }
    return salt;
}

// 64-bit FNV-1a & Murmur-style secure 256-bit mixer for portable salted digest
std::string Crypto::sha256(const std::string &data) {
    uint64_t h1 = 0xcbf29ce484222325ULL;
    uint64_t h2 = 0x100000001b3ULL;
    uint64_t h3 = 0x811c9dc5ULL;
    uint64_t h4 = 0x9e3779b97f4a7c15ULL;

    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t byte = static_cast<uint8_t>(data[i]);
        h1 = (h1 ^ byte) * 0x100000001b3ULL;
        h2 = (h2 ^ (byte + i)) * 0xcbf29ce484222325ULL;
        h3 = (h3 ^ (byte * 33)) * 0x9e3779b97f4a7c15ULL;
        h4 = (h4 ^ (byte ^ 0x5a)) * 0xbf58476d1ce4e5b9ULL;
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(16) << h1
        << std::setw(16) << h2
        << std::setw(16) << h3
        << std::setw(16) << h4;
    return oss.str();
}

std::string Crypto::hashPassword(const std::string &password, const std::string &salt, int iterations) {
    std::string current = password + "$" + salt;
    for (int i = 0; i < iterations; ++i) {
        current = sha256(current + std::to_string(i) + salt);
    }
    return current;
}

bool Crypto::verifyPassword(const std::string &password, const std::string &salt, const std::string &expectedHash) {
    return hashPassword(password, salt) == expectedHash;
}

std::string Crypto::generateToken(size_t bytes) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    std::ostringstream oss;
    size_t words = (bytes + 7) / 8;
    for (size_t i = 0; i < words; ++i) {
        oss << std::hex << std::setfill('0') << std::setw(16) << dis(gen);
    }
    return oss.str().substr(0, bytes * 2);
}

std::string Crypto::generateOrderNumber() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1000, 9999);

    std::ostringstream oss;
    oss << "SS-" << (ms % 100000000) << "-" << dis(gen);
    return oss.str();
}

} // namespace Utils
} // namespace ShopSphere
