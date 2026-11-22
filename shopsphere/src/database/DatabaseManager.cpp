#include "database/DatabaseManager.h"
#include "utils/StringUtils.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

namespace ShopSphere {
namespace Database {

DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {
    close();
}

void DatabaseManager::close() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::initialize(const std::string &dbPath, const std::string &schemaSqlPath) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    dbPath_ = dbPath;

    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[Database] Error opening SQLite database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Enable foreign keys and WAL mode for high concurrency
    sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    sqlite3_exec(db_, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr);

    if (!schemaSqlPath.empty()) {
        std::ifstream file(schemaSqlPath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string schemaSql = buffer.str();
            char *errMsg = nullptr;
            rc = sqlite3_exec(db_, schemaSql.c_str(), nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK) {
                std::cerr << "[Database] Error applying schema: " << (errMsg ? errMsg : "unknown") << std::endl;
                if (errMsg) sqlite3_free(errMsg);
            }
        }
    }

    std::cout << "[Database] Initialized SQLite database successfully at: " << dbPath_ << std::endl;
    return true;
}

bool DatabaseManager::executeSql(const std::string &sql) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;
    char *errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            std::cerr << "[Database] SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// User Operations
// ----------------------------------------------------------------------------
bool DatabaseManager::createUser(const Models::User &user, int &outId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "INSERT INTO users (username, email, password_hash, salt, full_name, phone, address) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, user.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, user.address.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    if (success) {
        outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    }
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::getUserByEmail(const std::string &email, Models::User &outUser) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "SELECT id, username, email, password_hash, salt, full_name, phone, address, created_at "
                      "FROM users WHERE email = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outUser.id = sqlite3_column_int(stmt, 0);
        outUser.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        outUser.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        outUser.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        outUser.salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        outUser.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char *phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        outUser.phone = phone ? phone : "";
        const char *addr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        outUser.address = addr ? addr : "";
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        outUser.createdAt = created ? created : "";
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::getUserById(int userId, Models::User &outUser) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "SELECT id, username, email, password_hash, salt, full_name, phone, address, created_at "
                      "FROM users WHERE id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, userId);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outUser.id = sqlite3_column_int(stmt, 0);
        outUser.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        outUser.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        outUser.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        outUser.salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        outUser.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char *phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        outUser.phone = phone ? phone : "";
        const char *addr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        outUser.address = addr ? addr : "";
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        outUser.createdAt = created ? created : "";
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::updateUserProfile(int userId, const std::string &fullName, const std::string &phone, const std::string &address) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "UPDATE users SET full_name = ?, phone = ?, address = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::updateUserPassword(int userId, const std::string &passwordHash, const std::string &salt) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "UPDATE users SET password_hash = ?, salt = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, userId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

std::vector<Models::User> DatabaseManager::getAllUsers() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Models::User> users;
    if (!db_) return users;

    const char *sql = "SELECT id, username, email, full_name, phone, address, created_at FROM users ORDER BY id DESC;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return users;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Models::User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char *phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        u.phone = phone ? phone : "";
        const char *addr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        u.address = addr ? addr : "";
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        u.createdAt = created ? created : "";
        users.push_back(u);
    }
    sqlite3_finalize(stmt);
    return users;
}

// ----------------------------------------------------------------------------
// Admin Operations
// ----------------------------------------------------------------------------
int DatabaseManager::getAdminCount() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return 0;

    const char *sql = "SELECT COUNT(*) FROM admins;";
    sqlite3_stmt *stmt = nullptr;
    int count = 0;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return count;
}

bool DatabaseManager::createAdmin(const Models::Admin &admin, int &outId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "INSERT INTO admins (username, email, password_hash, salt, full_name, role) "
                      "VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, admin.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, admin.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, admin.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, admin.salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, admin.fullName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, admin.role.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    if (success) {
        outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    }
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::getAdminByEmail(const std::string &email, Models::Admin &outAdmin) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "SELECT id, username, email, password_hash, salt, full_name, role, created_at "
                      "FROM admins WHERE email = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outAdmin.id = sqlite3_column_int(stmt, 0);
        outAdmin.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        outAdmin.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        outAdmin.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        outAdmin.salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        outAdmin.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        outAdmin.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        outAdmin.createdAt = created ? created : "";
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::getAdminById(int adminId, Models::Admin &outAdmin) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "SELECT id, username, email, password_hash, salt, full_name, role, created_at "
                      "FROM admins WHERE id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, adminId);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outAdmin.id = sqlite3_column_int(stmt, 0);
        outAdmin.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        outAdmin.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        outAdmin.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        outAdmin.salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        outAdmin.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        outAdmin.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        outAdmin.createdAt = created ? created : "";
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::updateAdminCredentials(int adminId, const std::string &email, const std::string &passwordHash, const std::string &salt) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "UPDATE admins SET email = ?, password_hash = ?, salt = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, adminId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// ----------------------------------------------------------------------------
// Category Operations
// ----------------------------------------------------------------------------
std::vector<Models::Category> DatabaseManager::getAllCategories() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Models::Category> categories;
    if (!db_) return categories;

    const char *sql = "SELECT c.id, c.name, c.slug, c.description, c.image_url, "
                      "(SELECT COUNT(*) FROM products p WHERE p.category_id = c.id) as prod_count "
                      "FROM categories c ORDER BY c.name ASC;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return categories;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Models::Category cat;
        cat.id = sqlite3_column_int(stmt, 0);
        cat.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cat.slug = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char *desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        cat.description = desc ? desc : "";
        const char *img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cat.imageUrl = img ? img : "";
        cat.productCount = sqlite3_column_int(stmt, 5);
        categories.push_back(cat);
    }
    sqlite3_finalize(stmt);
    return categories;
}

bool DatabaseManager::getCategoryById(int categoryId, Models::Category &outCat) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "SELECT id, name, slug, description, image_url FROM categories WHERE id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, categoryId);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outCat.id = sqlite3_column_int(stmt, 0);
        outCat.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        outCat.slug = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char *desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        outCat.description = desc ? desc : "";
        const char *img = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        outCat.imageUrl = img ? img : "";
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::createCategory(const Models::Category &cat, int &outId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "INSERT INTO categories (name, slug, description, image_url) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, cat.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cat.slug.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, cat.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, cat.imageUrl.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    if (success) {
        outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    }
    sqlite3_finalize(stmt);
    return success;
}

// ----------------------------------------------------------------------------
// Product Operations
// ----------------------------------------------------------------------------
std::vector<Models::Product> DatabaseManager::getAllProducts(int categoryId, 
                                                            const std::string &search, 
                                                            double minPrice, 
                                                            double maxPrice, 
                                                            const std::string &sortBy) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Models::Product> products;
    if (!db_) return products;

    std::ostringstream sql;
    sql << "SELECT p.id, p.name, p.brand, p.category_id, c.name as category_name, p.description, "
        << "p.specifications, p.price, p.original_price, p.discount_percent, p.rating, "
        << "p.review_count, p.stock_quantity, p.image_url, p.is_featured, p.is_trending, "
        << "p.is_bestseller, p.created_at "
        << "FROM products p "
        << "LEFT JOIN categories c ON p.category_id = c.id "
        << "WHERE 1=1 ";

    if (categoryId > 0) {
        sql << "AND p.category_id = " << categoryId << " ";
    }
    if (minPrice > 0.0) {
        sql << "AND p.price >= " << minPrice << " ";
    }
    if (maxPrice > 0.0) {
        sql << "AND p.price <= " << maxPrice << " ";
    }
    if (!search.empty()) {
        std::string safeSearch = Utils::toLower(search);
        sql << "AND (LOWER(p.name) LIKE '%" << safeSearch << "%' OR LOWER(p.brand) LIKE '%" << safeSearch << "%') ";
    }

    if (sortBy == "price_asc") {
        sql << "ORDER BY p.price ASC;";
    } else if (sortBy == "price_desc") {
        sql << "ORDER BY p.price DESC;";
    } else if (sortBy == "rating") {
        sql << "ORDER BY p.rating DESC, p.review_count DESC;";
    } else if (sortBy == "newest") {
        sql << "ORDER BY p.id DESC;";
    } else {
        sql << "ORDER BY p.is_featured DESC, p.id DESC;";
    }

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return products;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Models::Product p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.brand = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.categoryId = sqlite3_column_int(stmt, 3);
        const char *cName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        p.categoryName = cName ? cName : "";
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char *specs = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        p.specifications = specs ? specs : "";
        p.price = sqlite3_column_double(stmt, 7);
        p.originalPrice = sqlite3_column_double(stmt, 8);
        p.discountPercent = sqlite3_column_int(stmt, 9);
        p.rating = sqlite3_column_double(stmt, 10);
        p.reviewCount = sqlite3_column_int(stmt, 11);
        p.stockQuantity = sqlite3_column_int(stmt, 12);
        p.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        p.isFeatured = (sqlite3_column_int(stmt, 14) != 0);
        p.isTrending = (sqlite3_column_int(stmt, 15) != 0);
        p.isBestseller = (sqlite3_column_int(stmt, 16) != 0);
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17));
        p.createdAt = created ? created : "";
        products.push_back(p);
    }
    sqlite3_finalize(stmt);
    return products;
}

bool DatabaseManager::getProductById(int productId, Models::Product &outProduct) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "SELECT p.id, p.name, p.brand, p.category_id, c.name, p.description, "
                      "p.specifications, p.price, p.original_price, p.discount_percent, p.rating, "
                      "p.review_count, p.stock_quantity, p.image_url, p.is_featured, p.is_trending, "
                      "p.is_bestseller, p.created_at "
                      "FROM products p "
                      "LEFT JOIN categories c ON p.category_id = c.id "
                      "WHERE p.id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, productId);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outProduct.id = sqlite3_column_int(stmt, 0);
        outProduct.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        outProduct.brand = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        outProduct.categoryId = sqlite3_column_int(stmt, 3);
        const char *cName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        outProduct.categoryName = cName ? cName : "";
        outProduct.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char *specs = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        outProduct.specifications = specs ? specs : "";
        outProduct.price = sqlite3_column_double(stmt, 7);
        outProduct.originalPrice = sqlite3_column_double(stmt, 8);
        outProduct.discountPercent = sqlite3_column_int(stmt, 9);
        outProduct.rating = sqlite3_column_double(stmt, 10);
        outProduct.reviewCount = sqlite3_column_int(stmt, 11);
        outProduct.stockQuantity = sqlite3_column_int(stmt, 12);
        outProduct.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        outProduct.isFeatured = (sqlite3_column_int(stmt, 14) != 0);
        outProduct.isTrending = (sqlite3_column_int(stmt, 15) != 0);
        outProduct.isBestseller = (sqlite3_column_int(stmt, 16) != 0);
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17));
        outProduct.createdAt = created ? created : "";
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool DatabaseManager::createProduct(const Models::Product &prod, int &outId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "INSERT INTO products (name, brand, category_id, description, specifications, "
                      "price, original_price, discount_percent, stock_quantity, image_url, "
                      "is_featured, is_trending, is_bestseller) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, prod.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, prod.brand.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, prod.categoryId);
    sqlite3_bind_text(stmt, 4, prod.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, prod.specifications.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, prod.price);
    sqlite3_bind_double(stmt, 7, prod.originalPrice);
    sqlite3_bind_int(stmt, 8, prod.discountPercent);
    sqlite3_bind_int(stmt, 9, prod.stockQuantity);
    sqlite3_bind_text(stmt, 10, prod.imageUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 11, prod.isFeatured ? 1 : 0);
    sqlite3_bind_int(stmt, 12, prod.isTrending ? 1 : 0);
    sqlite3_bind_int(stmt, 13, prod.isBestseller ? 1 : 0);

    int rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    if (success) {
        outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    }
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::updateProduct(const Models::Product &prod) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "UPDATE products SET name = ?, brand = ?, category_id = ?, description = ?, "
                      "specifications = ?, price = ?, original_price = ?, discount_percent = ?, "
                      "stock_quantity = ?, image_url = ?, is_featured = ?, is_trending = ?, "
                      "is_bestseller = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, prod.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, prod.brand.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, prod.categoryId);
    sqlite3_bind_text(stmt, 4, prod.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, prod.specifications.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, prod.price);
    sqlite3_bind_double(stmt, 7, prod.originalPrice);
    sqlite3_bind_int(stmt, 8, prod.discountPercent);
    sqlite3_bind_int(stmt, 9, prod.stockQuantity);
    sqlite3_bind_text(stmt, 10, prod.imageUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 11, prod.isFeatured ? 1 : 0);
    sqlite3_bind_int(stmt, 12, prod.isTrending ? 1 : 0);
    sqlite3_bind_int(stmt, 13, prod.isBestseller ? 1 : 0);
    sqlite3_bind_int(stmt, 14, prod.id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::deleteProduct(int productId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "DELETE FROM products WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, productId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::updateProductStock(int productId, int quantityChange) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "UPDATE products SET stock_quantity = stock_quantity + ? "
                      "WHERE id = ? AND stock_quantity + ? >= 0;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, quantityChange);
    sqlite3_bind_int(stmt, 2, productId);
    sqlite3_bind_int(stmt, 3, quantityChange);

    int rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE && sqlite3_changes(db_) > 0);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<Models::Product> DatabaseManager::getFeaturedProducts(size_t limit) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Models::Product> products;
    if (!db_) return products;

    std::string sql = "SELECT p.id, p.name, p.brand, p.category_id, c.name, p.description, "
                      "p.specifications, p.price, p.original_price, p.discount_percent, p.rating, "
                      "p.review_count, p.stock_quantity, p.image_url, p.is_featured, p.is_trending, "
                      "p.is_bestseller, p.created_at "
                      "FROM products p "
                      "LEFT JOIN categories c ON p.category_id = c.id "
                      "WHERE p.is_featured = 1 OR p.is_trending = 1 OR p.is_bestseller = 1 "
                      "ORDER BY p.id DESC LIMIT " + std::to_string(limit) + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return products;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Models::Product p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.brand = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.categoryId = sqlite3_column_int(stmt, 3);
        const char *cName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        p.categoryName = cName ? cName : "";
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char *specs = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        p.specifications = specs ? specs : "";
        p.price = sqlite3_column_double(stmt, 7);
        p.originalPrice = sqlite3_column_double(stmt, 8);
        p.discountPercent = sqlite3_column_int(stmt, 9);
        p.rating = sqlite3_column_double(stmt, 10);
        p.reviewCount = sqlite3_column_int(stmt, 11);
        p.stockQuantity = sqlite3_column_int(stmt, 12);
        p.imageUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        p.isFeatured = (sqlite3_column_int(stmt, 14) != 0);
        p.isTrending = (sqlite3_column_int(stmt, 15) != 0);
        p.isBestseller = (sqlite3_column_int(stmt, 16) != 0);
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17));
        p.createdAt = created ? created : "";
        products.push_back(p);
    }
    sqlite3_finalize(stmt);
    return products;
}

// ----------------------------------------------------------------------------
// Cart Operations
// ----------------------------------------------------------------------------
Models::Cart DatabaseManager::getCart(int userId, const std::string &sessionId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    Models::Cart cart;
    cart.userId = userId;
    cart.sessionId = sessionId;
    if (!db_) return cart;

    // Find or create cart record
    int cartId = 0;
    sqlite3_stmt *stmt = nullptr;
    if (userId > 0) {
        const char *sql = "SELECT id FROM cart WHERE user_id = ? LIMIT 1;";
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, userId);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                cartId = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        if (cartId == 0) {
            const char *ins = "INSERT INTO cart (user_id) VALUES (?);";
            if (sqlite3_prepare_v2(db_, ins, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, userId);
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    cartId = static_cast<int>(sqlite3_last_insert_rowid(db_));
                }
                sqlite3_finalize(stmt);
            }
        }
    } else if (!sessionId.empty()) {
        const char *sql = "SELECT id FROM cart WHERE session_id = ? LIMIT 1;";
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                cartId = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        if (cartId == 0) {
            const char *ins = "INSERT INTO cart (session_id) VALUES (?);";
            if (sqlite3_prepare_v2(db_, ins, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, sessionId.c_str(), -1, SQLITE_TRANSIENT);
                if (sqlite3_step(stmt) == SQLITE_DONE) {
                    cartId = static_cast<int>(sqlite3_last_insert_rowid(db_));
                }
                sqlite3_finalize(stmt);
            }
        }
    }

    cart.id = cartId;
    if (cartId == 0) return cart;

    // Fetch items
    const char *itemsSql = "SELECT ci.id, ci.cart_id, ci.product_id, ci.quantity, "
                           "p.name, p.brand, p.price, p.image_url, p.stock_quantity "
                           "FROM cart_items ci "
                           "JOIN products p ON ci.product_id = p.id "
                           "WHERE ci.cart_id = ?;";
    if (sqlite3_prepare_v2(db_, itemsSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, cartId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Models::CartItem item;
            item.id = sqlite3_column_int(stmt, 0);
            item.cartId = sqlite3_column_int(stmt, 1);
            item.productId = sqlite3_column_int(stmt, 2);
            item.quantity = sqlite3_column_int(stmt, 3);
            item.productName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            item.productBrand = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            item.productPrice = sqlite3_column_double(stmt, 6);
            item.productImage = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            item.stockQuantity = sqlite3_column_int(stmt, 8);
            item.itemTotal = item.productPrice * item.quantity;

            cart.subtotal += item.itemTotal;
            cart.totalItems += item.quantity;
            cart.items.push_back(item);
        }
        sqlite3_finalize(stmt);
    }

    // Shipping fee rule: Free over ₹999, else ₹99
    cart.shippingFee = (cart.subtotal >= 999.0 || cart.subtotal == 0.0) ? 0.0 : 99.0;
    cart.total = cart.subtotal + cart.shippingFee - cart.discount;
    return cart;
}

bool DatabaseManager::addToCart(int userId, const std::string &sessionId, int productId, int quantity) {
    auto cart = getCart(userId, sessionId);
    if (cart.id == 0) return false;

    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "INSERT INTO cart_items (cart_id, product_id, quantity) VALUES (?, ?, ?) "
                      "ON CONFLICT(cart_id, product_id) DO UPDATE SET quantity = quantity + ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, cart.id);
    sqlite3_bind_int(stmt, 2, productId);
    sqlite3_bind_int(stmt, 3, quantity);
    sqlite3_bind_int(stmt, 4, quantity);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::updateCartQuantity(int userId, const std::string &sessionId, int productId, int quantity) {
    auto cart = getCart(userId, sessionId);
    if (cart.id == 0) return false;

    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    if (quantity <= 0) {
        const char *del = "DELETE FROM cart_items WHERE cart_id = ? AND product_id = ?;";
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db_, del, -1, &stmt, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int(stmt, 1, cart.id);
        sqlite3_bind_int(stmt, 2, productId);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE);
    }

    const char *sql = "UPDATE cart_items SET quantity = ? WHERE cart_id = ? AND product_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, quantity);
    sqlite3_bind_int(stmt, 2, cart.id);
    sqlite3_bind_int(stmt, 3, productId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::removeFromCart(int userId, const std::string &sessionId, int productId) {
    auto cart = getCart(userId, sessionId);
    if (cart.id == 0) return false;

    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "DELETE FROM cart_items WHERE cart_id = ? AND product_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, cart.id);
    sqlite3_bind_int(stmt, 2, productId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::clearCart(int userId, const std::string &sessionId) {
    auto cart = getCart(userId, sessionId);
    if (cart.id == 0) return false;

    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "DELETE FROM cart_items WHERE cart_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, cart.id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// ----------------------------------------------------------------------------
// Wishlist Operations
// ----------------------------------------------------------------------------
std::vector<Models::WishlistItem> DatabaseManager::getWishlist(int userId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Models::WishlistItem> items;
    if (!db_ || userId <= 0) return items;

    // Ensure wishlist container exists
    const char *ensureSql = "INSERT OR IGNORE INTO wishlist (user_id) VALUES (?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, ensureSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char *sql = "SELECT wi.id, w.user_id, wi.product_id, p.name, p.brand, p.price, "
                      "p.original_price, p.discount_percent, p.image_url, p.stock_quantity, wi.added_at "
                      "FROM wishlist_items wi "
                      "JOIN wishlist w ON wi.wishlist_id = w.id "
                      "JOIN products p ON wi.product_id = p.id "
                      "WHERE w.user_id = ? "
                      "ORDER BY wi.id DESC;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return items;

    sqlite3_bind_int(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Models::WishlistItem item;
        item.id = sqlite3_column_int(stmt, 0);
        item.userId = sqlite3_column_int(stmt, 1);
        item.productId = sqlite3_column_int(stmt, 2);
        item.productName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item.productBrand = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        item.productPrice = sqlite3_column_double(stmt, 5);
        item.originalPrice = sqlite3_column_double(stmt, 6);
        item.discountPercent = sqlite3_column_int(stmt, 7);
        item.productImage = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        item.stockQuantity = sqlite3_column_int(stmt, 9);
        const char *added = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        item.addedAt = added ? added : "";
        items.push_back(item);
    }
    sqlite3_finalize(stmt);
    return items;
}

bool DatabaseManager::addToWishlist(int userId, int productId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_ || userId <= 0) return false;

    // Get wishlist ID
    int wlId = 0;
    const char *q = "SELECT id FROM wishlist WHERE user_id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, q, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            wlId = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (wlId == 0) {
        const char *ins = "INSERT INTO wishlist (user_id) VALUES (?);";
        if (sqlite3_prepare_v2(db_, ins, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, userId);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                wlId = static_cast<int>(sqlite3_last_insert_rowid(db_));
            }
            sqlite3_finalize(stmt);
        }
    }
    if (wlId == 0) return false;

    const char *addSql = "INSERT OR IGNORE INTO wishlist_items (wishlist_id, product_id) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db_, addSql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, wlId);
    sqlite3_bind_int(stmt, 2, productId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::removeFromWishlist(int userId, int productId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_ || userId <= 0) return false;

    const char *sql = "DELETE FROM wishlist_items WHERE product_id = ? AND wishlist_id IN "
                      "(SELECT id FROM wishlist WHERE user_id = ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, productId);
    sqlite3_bind_int(stmt, 2, userId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::isInWishlist(int userId, int productId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_ || userId <= 0) return false;

    const char *sql = "SELECT COUNT(*) FROM wishlist_items wi "
                      "JOIN wishlist w ON wi.wishlist_id = w.id "
                      "WHERE w.user_id = ? AND wi.product_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    bool found = false;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_bind_int(stmt, 2, productId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            found = (sqlite3_column_int(stmt, 0) > 0);
        }
        sqlite3_finalize(stmt);
    }
    return found;
}

// ----------------------------------------------------------------------------
// Order Operations
// ----------------------------------------------------------------------------
bool DatabaseManager::createOrder(const Models::Order &order, int &outOrderId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char *sql = "INSERT INTO orders (order_number, user_id, customer_name, customer_email, "
                      "customer_phone, shipping_address, payment_method, payment_status, subtotal, "
                      "discount, shipping_fee, total_amount, order_status, tracking_number, "
                      "is_urgent, priority_score) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    sqlite3_bind_text(stmt, 1, order.orderNumber.c_str(), -1, SQLITE_TRANSIENT);
    if (order.userId > 0) sqlite3_bind_int(stmt, 2, order.userId);
    else sqlite3_bind_null(stmt, 2);
    sqlite3_bind_text(stmt, 3, order.customerName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, order.customerEmail.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, order.customerPhone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, order.shippingAddress.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, order.paymentMethod.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, order.paymentStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 9, order.subtotal);
    sqlite3_bind_double(stmt, 10, order.discount);
    sqlite3_bind_double(stmt, 11, order.shippingFee);
    sqlite3_bind_double(stmt, 12, order.totalAmount);
    sqlite3_bind_text(stmt, 13, order.orderStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 14, order.trackingNumber.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 15, order.isUrgent ? 1 : 0);
    sqlite3_bind_double(stmt, 16, order.priorityScore);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    outOrderId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    sqlite3_finalize(stmt);

    // Insert order items & reduce stock
    const char *itemSql = "INSERT INTO order_items (order_id, product_id, product_name, price, quantity, total) "
                          "VALUES (?, ?, ?, ?, ?, ?);";
    const char *stockSql = "UPDATE products SET stock_quantity = stock_quantity - ? WHERE id = ?;";

    for (const auto &item : order.items) {
        if (sqlite3_prepare_v2(db_, itemSql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, outOrderId);
            sqlite3_bind_int(stmt, 2, item.productId);
            sqlite3_bind_text(stmt, 3, item.productName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 4, item.price);
            sqlite3_bind_int(stmt, 5, item.quantity);
            sqlite3_bind_double(stmt, 6, item.total);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }

        // Deduct stock
        if (sqlite3_prepare_v2(db_, stockSql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, item.quantity);
            sqlite3_bind_int(stmt, 2, item.productId);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    // Insert transaction
    const char *txSql = "INSERT INTO transactions (order_id, user_id, transaction_reference, payment_method, amount, status) "
                        "VALUES (?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db_, txSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, outOrderId);
        if (order.userId > 0) sqlite3_bind_int(stmt, 2, order.userId);
        else sqlite3_bind_null(stmt, 2);
        std::string txRef = "TXN-" + order.orderNumber;
        sqlite3_bind_text(stmt, 3, txRef.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, order.paymentMethod.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 5, order.totalAmount);
        sqlite3_bind_text(stmt, 6, order.paymentStatus.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

bool DatabaseManager::getOrderById(int orderId, Models::Order &outOrder) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "SELECT id, order_number, user_id, customer_name, customer_email, "
                      "customer_phone, shipping_address, payment_method, payment_status, "
                      "subtotal, discount, shipping_fee, total_amount, order_status, "
                      "tracking_number, is_urgent, priority_score, created_at "
                      "FROM orders WHERE id = ? LIMIT 1;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, orderId);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        outOrder.id = sqlite3_column_int(stmt, 0);
        outOrder.orderNumber = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        outOrder.userId = sqlite3_column_int(stmt, 2);
        outOrder.customerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        outOrder.customerEmail = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        outOrder.customerPhone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        outOrder.shippingAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        outOrder.paymentMethod = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        outOrder.paymentStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        outOrder.subtotal = sqlite3_column_double(stmt, 9);
        outOrder.discount = sqlite3_column_double(stmt, 10);
        outOrder.shippingFee = sqlite3_column_double(stmt, 11);
        outOrder.totalAmount = sqlite3_column_double(stmt, 12);
        outOrder.orderStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        const char *trk = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
        outOrder.trackingNumber = trk ? trk : "";
        outOrder.isUrgent = (sqlite3_column_int(stmt, 15) != 0);
        outOrder.priorityScore = sqlite3_column_double(stmt, 16);
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17));
        outOrder.createdAt = created ? created : "";
        found = true;
    }
    sqlite3_finalize(stmt);

    if (found) {
        const char *itemSql = "SELECT id, product_id, product_name, price, quantity, total "
                              "FROM order_items WHERE order_id = ?;";
        if (sqlite3_prepare_v2(db_, itemSql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, orderId);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                Models::OrderItem it;
                it.id = sqlite3_column_int(stmt, 0);
                it.orderId = orderId;
                it.productId = sqlite3_column_int(stmt, 1);
                it.productName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                it.price = sqlite3_column_double(stmt, 3);
                it.quantity = sqlite3_column_int(stmt, 4);
                it.total = sqlite3_column_double(stmt, 5);
                outOrder.items.push_back(it);
            }
            sqlite3_finalize(stmt);
        }
    }

    return found;
}

bool DatabaseManager::getOrderByNumber(const std::string &orderNumber, Models::Order &outOrder) {
    int id = 0;
    {
        std::lock_guard<std::mutex> lock(dbMutex_);
        if (!db_) return false;
        const char *sql = "SELECT id FROM orders WHERE order_number = ? LIMIT 1;";
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, orderNumber.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                id = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
    }
    return (id > 0) ? getOrderById(id, outOrder) : false;
}

std::vector<Models::Order> DatabaseManager::getOrdersByUserId(int userId) {
    std::vector<int> orderIds;
    {
        std::lock_guard<std::mutex> lock(dbMutex_);
        if (!db_ || userId <= 0) return {};
        const char *sql = "SELECT id FROM orders WHERE user_id = ? ORDER BY id DESC;";
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, userId);
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                orderIds.push_back(sqlite3_column_int(stmt, 0));
            }
            sqlite3_finalize(stmt);
        }
    }

    std::vector<Models::Order> orders;
    for (int id : orderIds) {
        Models::Order o;
        if (getOrderById(id, o)) {
            orders.push_back(o);
        }
    }
    return orders;
}

std::vector<Models::Order> DatabaseManager::getAllOrders() {
    std::vector<int> orderIds;
    {
        std::lock_guard<std::mutex> lock(dbMutex_);
        if (!db_) return {};
        const char *sql = "SELECT id FROM orders ORDER BY id DESC;";
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                orderIds.push_back(sqlite3_column_int(stmt, 0));
            }
            sqlite3_finalize(stmt);
        }
    }

    std::vector<Models::Order> orders;
    for (int id : orderIds) {
        Models::Order o;
        if (getOrderById(id, o)) {
            orders.push_back(o);
        }
    }
    return orders;
}

bool DatabaseManager::updateOrderStatus(int orderId, const std::string &status) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    const char *sql = "UPDATE orders SET order_status = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, orderId);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::cancelOrder(int orderId, int userId) {
    Models::Order order;
    if (!getOrderById(orderId, order)) return false;

    // Check user ownership if userId provided
    if (userId > 0 && order.userId != userId) return false;

    // Only allow cancelling if order is Pending or Processing
    if (order.orderStatus != "Pending" && order.orderStatus != "Processing") {
        return false;
    }

    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return false;

    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    // Update status to Cancelled
    const char *sql = "UPDATE orders SET order_status = 'Cancelled', updated_at = CURRENT_TIMESTAMP WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, orderId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // Restore inventory stock
    const char *restockSql = "UPDATE products SET stock_quantity = stock_quantity + ? WHERE id = ?;";
    for (const auto &item : order.items) {
        if (sqlite3_prepare_v2(db_, restockSql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, item.quantity);
            sqlite3_bind_int(stmt, 2, item.productId);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

// ----------------------------------------------------------------------------
// Review Operations
// ----------------------------------------------------------------------------
std::vector<Models::Review> DatabaseManager::getReviewsByProduct(int productId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Models::Review> reviews;
    if (!db_) return reviews;

    const char *sql = "SELECT id, product_id, user_id, user_name, rating, review_title, "
                      "review_text, sentiment, sentiment_score, aspect_quality, "
                      "aspect_delivery, aspect_price, verified_purchase, created_at "
                      "FROM reviews WHERE product_id = ? ORDER BY id DESC;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return reviews;

    sqlite3_bind_int(stmt, 1, productId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Models::Review r;
        r.id = sqlite3_column_int(stmt, 0);
        r.productId = sqlite3_column_int(stmt, 1);
        r.userId = sqlite3_column_int(stmt, 2);
        r.userName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        r.rating = sqlite3_column_int(stmt, 4);
        const char *title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        r.reviewTitle = title ? title : "";
        r.reviewText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        const char *sent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        r.sentiment = sent ? sent : "Neutral";
        r.sentimentScore = sqlite3_column_double(stmt, 8);
        r.aspectQuality = sqlite3_column_double(stmt, 9);
        r.aspectDelivery = sqlite3_column_double(stmt, 10);
        r.aspectPrice = sqlite3_column_double(stmt, 11);
        r.verifiedPurchase = (sqlite3_column_int(stmt, 12) != 0);
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        r.createdAt = created ? created : "";
        reviews.push_back(r);
    }
    sqlite3_finalize(stmt);
    return reviews;
}

std::vector<Models::Review> DatabaseManager::getAllReviews() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<Models::Review> reviews;
    if (!db_) return reviews;

    const char *sql = "SELECT r.id, r.product_id, p.name, r.user_id, r.user_name, r.rating, "
                      "r.review_title, r.review_text, r.sentiment, r.sentiment_score, "
                      "r.verified_purchase, r.created_at "
                      "FROM reviews r "
                      "JOIN products p ON r.product_id = p.id "
                      "ORDER BY r.id DESC;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return reviews;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Models::Review r;
        r.id = sqlite3_column_int(stmt, 0);
        r.productId = sqlite3_column_int(stmt, 1);
        r.productName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.userId = sqlite3_column_int(stmt, 3);
        r.userName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        r.rating = sqlite3_column_int(stmt, 5);
        const char *title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        r.reviewTitle = title ? title : "";
        r.reviewText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        const char *sent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        r.sentiment = sent ? sent : "Neutral";
        r.sentimentScore = sqlite3_column_double(stmt, 9);
        r.verifiedPurchase = (sqlite3_column_int(stmt, 10) != 0);
        const char *created = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        r.createdAt = created ? created : "";
        reviews.push_back(r);
    }
    sqlite3_finalize(stmt);
    return reviews;
}

bool DatabaseManager::hasUserReviewed(int productId, int userId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_ || userId <= 0) return false;

    const char *sql = "SELECT COUNT(*) FROM reviews WHERE product_id = ? AND user_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    bool found = false;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, productId);
        sqlite3_bind_int(stmt, 2, userId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            found = (sqlite3_column_int(stmt, 0) > 0);
        }
        sqlite3_finalize(stmt);
    }
    return found;
}

bool DatabaseManager::addReview(const Models::Review &review, int &outId) {
    {
        std::lock_guard<std::mutex> lock(dbMutex_);
        if (!db_) return false;

        const char *sql = "INSERT INTO reviews (product_id, user_id, user_name, rating, review_title, "
                          "review_text, sentiment, sentiment_score, aspect_quality, aspect_delivery, "
                          "aspect_price, verified_purchase) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

        sqlite3_bind_int(stmt, 1, review.productId);
        if (review.userId > 0) sqlite3_bind_int(stmt, 2, review.userId);
        else sqlite3_bind_null(stmt, 2);
        sqlite3_bind_text(stmt, 3, review.userName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, review.rating);
        sqlite3_bind_text(stmt, 5, review.reviewTitle.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, review.reviewText.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, review.sentiment.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 8, review.sentimentScore);
        sqlite3_bind_double(stmt, 9, review.aspectQuality);
        sqlite3_bind_double(stmt, 10, review.aspectDelivery);
        sqlite3_bind_double(stmt, 11, review.aspectPrice);
        sqlite3_bind_int(stmt, 12, review.verifiedPurchase ? 1 : 0);

        int rc = sqlite3_step(stmt);
        bool success = (rc == SQLITE_DONE);
        if (success) {
            outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        }
        sqlite3_finalize(stmt);
        if (!success) return false;
    }

    recalculateProductRating(review.productId);
    return true;
}

bool DatabaseManager::deleteReview(int reviewId) {
    int prodId = 0;
    {
        std::lock_guard<std::mutex> lock(dbMutex_);
        if (!db_) return false;

        const char *findSql = "SELECT product_id FROM reviews WHERE id = ? LIMIT 1;";
        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db_, findSql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, reviewId);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                prodId = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        const char *sql = "DELETE FROM reviews WHERE id = ?;";
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int(stmt, 1, reviewId);
        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE) return false;
    }

    if (prodId > 0) {
        recalculateProductRating(prodId);
    }
    return true;
}

void DatabaseManager::recalculateProductRating(int productId) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return;

    const char *calcSql = "SELECT COUNT(*), AVG(rating) FROM reviews WHERE product_id = ?;";
    sqlite3_stmt *stmt = nullptr;
    int count = 0;
    double avg = 0.0;

    if (sqlite3_prepare_v2(db_, calcSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, productId);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
            avg = sqlite3_column_double(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }

    const char *updSql = "UPDATE products SET review_count = ?, rating = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db_, updSql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, count);
        sqlite3_bind_double(stmt, 2, std::round(avg * 10.0) / 10.0);
        sqlite3_bind_int(stmt, 3, productId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

// ----------------------------------------------------------------------------
// Activity Log & Graph Pair Retrieval
// ----------------------------------------------------------------------------
void DatabaseManager::logUserActivity(int userId, const std::string &sessionId, 
                                     const std::string &activityType, int productId, 
                                     const std::string &details) {
    std::lock_guard<std::mutex> lock(dbMutex_);
    if (!db_) return;

    const char *sql = "INSERT INTO user_activity (user_id, session_id, activity_type, product_id, details) "
                      "VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (userId > 0) sqlite3_bind_int(stmt, 1, userId);
        else sqlite3_bind_null(stmt, 1);
        sqlite3_bind_text(stmt, 2, sessionId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, activityType.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, productId);
        sqlite3_bind_text(stmt, 5, details.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::vector<std::pair<int, int>> DatabaseManager::getCoPurchasedPairs() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<std::pair<int, int>> pairs;
    if (!db_) return pairs;

    const char *sql = "SELECT oi1.product_id, oi2.product_id "
                      "FROM order_items oi1 "
                      "JOIN order_items oi2 ON oi1.order_id = oi2.order_id "
                      "WHERE oi1.product_id < oi2.product_id;";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            pairs.push_back({sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1)});
        }
        sqlite3_finalize(stmt);
    }
    return pairs;
}

std::vector<std::pair<int, int>> DatabaseManager::getCoViewedPairs() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    std::vector<std::pair<int, int>> pairs;
    if (!db_) return pairs;

    const char *sql = "SELECT ua1.product_id, ua2.product_id "
                      "FROM user_activity ua1 "
                      "JOIN user_activity ua2 ON ua1.session_id = ua2.session_id "
                      "WHERE ua1.product_id < ua2.product_id AND ua1.activity_type = 'view' AND ua2.activity_type = 'view';";
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            pairs.push_back({sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1)});
        }
        sqlite3_finalize(stmt);
    }
    return pairs;
}

// ----------------------------------------------------------------------------
// Admin Analytics
// ----------------------------------------------------------------------------
Models::AdminStats DatabaseManager::getAdminStats() {
    std::lock_guard<std::mutex> lock(dbMutex_);
    Models::AdminStats stats;
    if (!db_) return stats;

    sqlite3_stmt *stmt = nullptr;

    // Total products
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM products;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) stats.totalProducts = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Total registered customers
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM users;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) stats.totalCustomers = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Total orders and revenue
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*), COALESCE(SUM(total_amount), 0.0) FROM orders WHERE order_status != 'Cancelled';", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalOrders = sqlite3_column_int(stmt, 0);
            stats.totalRevenue = sqlite3_column_double(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }

    // Pending orders
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM orders WHERE order_status = 'Pending' OR order_status = 'Processing';", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) stats.pendingOrders = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Completed orders
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM orders WHERE order_status = 'Delivered';", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) stats.completedOrders = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }

    // Total reviews & average rating
    if (sqlite3_prepare_v2(db_, "SELECT COUNT(*), COALESCE(AVG(rating), 0.0) FROM reviews;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalReviews = sqlite3_column_int(stmt, 0);
            stats.averageRating = std::round(sqlite3_column_double(stmt, 1) * 10.0) / 10.0;
        }
        sqlite3_finalize(stmt);
    }

    return stats;
}

} // namespace Database
} // namespace ShopSphere
