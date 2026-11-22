#ifndef SHOPSPHERE_MODELS_H
#define SHOPSPHERE_MODELS_H

#include <string>
#include <vector>

namespace ShopSphere {
namespace Models {

struct User {
    int id{0};
    std::string username;
    std::string email;
    std::string passwordHash;
    std::string salt;
    std::string fullName;
    std::string phone;
    std::string address;
    std::string createdAt;
};

struct Admin {
    int id{0};
    std::string username;
    std::string email;
    std::string passwordHash;
    std::string salt;
    std::string fullName;
    std::string role{"admin"};
    std::string createdAt;
};

struct Category {
    int id{0};
    std::string name;
    std::string slug;
    std::string description;
    std::string imageUrl;
    int productCount{0};
};

struct Product {
    int id{0};
    std::string name;
    std::string brand;
    int categoryId{0};
    std::string categoryName;
    std::string description;
    std::string specifications;
    double price{0.0};
    double originalPrice{0.0};
    int discountPercent{0};
    double rating{0.0};
    int reviewCount{0};
    int stockQuantity{0};
    std::string imageUrl;
    bool isFeatured{false};
    bool isTrending{false};
    bool isBestseller{false};
    std::string createdAt;
};

struct Review {
    int id{0};
    int productId{0};
    std::string productName;
    int userId{0};
    std::string userName;
    int rating{5};
    std::string reviewTitle;
    std::string reviewText;
    std::string sentiment{"Neutral"};
    double sentimentScore{0.0};
    double aspectQuality{0.0};
    double aspectDelivery{0.0};
    double aspectPrice{0.0};
    bool isModerated{false};
    bool verifiedPurchase{true};
    std::string createdAt;
};

struct CartItem {
    int id{0};
    int cartId{0};
    int productId{0};
    std::string productName;
    std::string productBrand;
    double productPrice{0.0};
    std::string productImage;
    int stockQuantity{0};
    int quantity{1};
    double itemTotal{0.0};
};

struct Cart {
    int id{0};
    int userId{0};
    std::string sessionId;
    std::vector<CartItem> items;
    double subtotal{0.0};
    double discount{0.0};
    double shippingFee{0.0};
    double total{0.0};
    int totalItems{0};
};

struct WishlistItem {
    int id{0};
    int userId{0};
    int productId{0};
    std::string productName;
    std::string productBrand;
    double productPrice{0.0};
    double originalPrice{0.0};
    int discountPercent{0};
    std::string productImage;
    int stockQuantity{0};
    std::string addedAt;
};

struct OrderItem {
    int id{0};
    int orderId{0};
    int productId{0};
    std::string productName;
    double price{0.0};
    int quantity{1};
    double total{0.0};
};

struct Order {
    int id{0};
    std::string orderNumber;
    int userId{0};
    std::string customerName;
    std::string customerEmail;
    std::string customerPhone;
    std::string shippingAddress;
    std::string paymentMethod;
    std::string paymentStatus{"Completed"};
    double subtotal{0.0};
    double discount{0.0};
    double shippingFee{0.0};
    double totalAmount{0.0};
    std::string orderStatus{"Pending"}; // Pending, Processing, Shipped, Delivered, Cancelled
    std::string trackingNumber;
    bool isUrgent{false};
    double priorityScore{0.0};
    std::string createdAt;
    std::vector<OrderItem> items;
};

struct AdminStats {
    int totalProducts{0};
    int totalCustomers{0};
    int totalOrders{0};
    double totalRevenue{0.0};
    int pendingOrders{0};
    int completedOrders{0};
    int totalReviews{0};
    double averageRating{0.0};
};

} // namespace Models
} // namespace ShopSphere

#endif // SHOPSPHERE_MODELS_H
