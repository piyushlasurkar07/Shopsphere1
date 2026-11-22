# ShopSphere – High-Performance C++ E-Commerce Platform

**ShopSphere** is a modern, enterprise-grade e-commerce web platform re-engineered from the ground up with a **pure C++ backend** (using the **Drogon C++ Web Framework**, CMake build system, and SQLite database) and responsive frontend (HTML5/CSS3/JavaScript).

It features **four custom Data Structures & Algorithms (DSA)** engines and **five Machine Learning / Intelligent features implemented directly in modern C++** without any Python, Flask, Django, or Node.js runtime dependencies.

---

## Table of Contents
1. [Technology Stack](#1-technology-stack)
2. [Project Architecture](#2-project-architecture)
3. [Database Schema & Design](#3-database-schema--design)
4. [Four Advanced DSA Features (Deep Dive & Complexities)](#4-four-advanced-dsa-features)
   - [Feature 1: Trie-Based Smart Search](#feature-1-trie-based-smart-search)
   - [Feature 2: Graph-Based Product Recommendation Engine](#feature-2-graph-based-product-recommendation-engine)
   - [Feature 3: LRU Cache (Hash Map + Doubly Linked List)](#feature-3-lru-cache)
   - [Feature 4: Priority Queue-Based Order Processing (Binary Max-Heap)](#feature-4-priority-queue-based-order-processing)
5. [Intelligent & Machine Learning Features (Pure C++)](#5-intelligent--machine-learning-features)
   - [NLP Query Parser & Typo Correction](#a-nlp-product-search)
   - [Content-Based Recommender (TF-IDF Cosine Similarity)](#b-content-based-recommendation)
   - [VADER-Style Sentiment & Aspect Analysis](#c-vader-style-sentiment-analysis)
   - [Logistic Regression Category Classifier](#d-logistic-regression-category-classification)
   - [Random Forest Price Regressor](#e-random-forest-price-prediction)
6. [Security & Authentication](#6-security--authentication)
7. [RESTful API Documentation](#7-restful-api-documentation)
8. [Setup & Compilation Instructions](#8-setup--compilation-instructions)
9. [Render Deployment Guide](#9-render-deployment-guide)
10. [Admin Initialization Guide](#10-admin-initialization-guide)
11. [Testing Suite](#11-testing-suite)

---

## 1. Technology Stack

- **Backend:** Modern C++ (C++17), Drogon C++ Asynchronous Web Framework, Trantor event-driven network engine.
- **Build System:** CMake (>= 3.16).
- **Database:** SQLite3 with strict relational integrity, foreign key constraints (`PRAGMA foreign_keys = ON`), transactions, and prepared statements.
- **Frontend:** Responsive HTML5, CSS3, JavaScript ES6+, Bootstrap 5.3, FontAwesome 6 icons.
- **Deployment Platform:** Render (Docker containerized deployment with persistent disk mount).
- **Zero Python/Flask Policy:** All web servers, APIs, routing, DSA structures, NLP tokenizers, and ML models are implemented strictly in native C++.

---

## 2. Project Architecture

```
ShopSphere/
├── CMakeLists.txt              # Production CMake configuration
├── README.md                   # Comprehensive project documentation
├── Dockerfile                  # Multi-stage production build for Linux/Render
├── render.yaml                 # Render Infrastructure-as-Code blueprint
├── .env.example                # Sample environment variables
│
├── config/
│   └── config.json             # Application, server, and DSA cache configurations
│
├── database/
│   └── schema.sql              # Relational schema (tables, foreign keys, indexes)
│
├── include/
│   ├── algorithms/             # DSA & ML header definitions
│   │   ├── TrieSearch.h
│   │   ├── ProductGraph.h
│   │   ├── LRUCache.h
│   │   ├── OrderPriorityQueue.h
│   │   ├── NLPSearch.h
│   │   ├── ContentRecommender.h
│   │   ├── SentimentAnalyzer.h
│   │   ├── LogisticRegression.h
│   │   └── PricePredictor.h
│   ├── controllers/            # Drogon HTTP REST & Page controllers
│   │   ├── ProductController.h
│   │   ├── AuthController.h
│   │   ├── CartController.h
│   │   ├── WishlistController.h
│   │   ├── OrderController.h
│   │   ├── ReviewController.h
│   │   ├── AdminController.h
│   │   └── PageController.h
│   ├── database/
│   │   └── DatabaseManager.h   # SQLite prepared statements & thread-safe DB API
│   ├── middleware/
│   │   └── AuthMiddleware.h    # Admin and Customer session filters
│   ├── models/
│   │   └── Models.h            # C++ data models (User, Admin, Product, Order, etc.)
│   ├── services/
│   │   └── AppServices.h       # Business logic orchestrator and SessionService
│   └── utils/
│       ├── Crypto.h            # Salted cryptographic hashing and token generator
│       ├── SpinLock.h          # Portable cross-platform synchronization
│       └── StringUtils.h       # Levenshtein distance, soundex, HTML sanitization
│
├── src/
│   ├── main.cpp                # Server entry point, CLI arguments, Drogon runner
│   ├── algorithms/
│   │   ├── TrieSearch.cpp
│   │   ├── ProductGraph.cpp
│   │   ├── LRUCache.cpp
│   │   ├── OrderPriorityQueue.cpp
│   │   ├── NLPSearch.cpp
│   │   ├── ContentRecommender.cpp
│   │   ├── SentimentAnalyzer.cpp
│   │   ├── LogisticRegression.cpp
│   │   └── PricePredictor.cpp
│   ├── controllers/
│   │   ├── ProductController.cpp
│   │   ├── AuthController.cpp
│   │   ├── CartController.cpp
│   │   ├── WishlistController.cpp
│   │   ├── OrderController.cpp
│   │   ├── ReviewController.cpp
│   │   ├── AdminController.cpp
│   │   └── PageController.cpp
│   ├── database/
│   │   └── DatabaseManager.cpp
│   ├── middleware/
│   │   └── AuthMiddleware.cpp
│   ├── services/
│   │   └── AppServices.cpp
│   └── utils/
│       └── Crypto.cpp
│
├── frontend/
│   ├── templates/              # Production web storefront and admin views
│   │   ├── index.html          # Homepage with Trie search & live products
│   │   ├── products.html       # Catalog with price/category filters & sorting
│   │   ├── product_detail.html # Product page, LRU cached, Graph recs, Reviews
│   │   ├── cart.html           # Shopping cart with stock validation
│   │   ├── wishlist.html       # Persistent wishlist with move-to-cart
│   │   ├── checkout.html       # Checkout, urgent delivery, payment selection
│   │   ├── orders.html         # Real order tracking and cancellation
│   │   ├── order_success.html  # Order confirmation and tracking ID
│   │   ├── login.html          # Customer login (starts completely empty)
│   │   ├── register.html       # Customer registration (starts empty)
│   │   ├── profile.html        # Customer profile management
│   │   └── admin/
│   │       ├── login.html      # Protected admin login & one-time setup
│   │       └── dashboard.html  # Admin portal (Heap queue, stats, products, etc.)
│   └── static/
│       ├── css/
│       │   ├── style.css       # Responsive store styling
│       │   └── admin.css       # Admin dashboard styles
│       └── js/
│           ├── main.js         # Client-side Trie suggestions, cart, reviews
│           └── admin.js        # Admin ML auto-classifier & priority processing
│
└── tests/                      # Comprehensive C++ verification test suite
    ├── TrieSearchTest.cpp
    ├── ProductGraphTest.cpp
    ├── LRUCacheTest.cpp
    ├── OrderPriorityQueueTest.cpp
    └── IntelligentFeaturesTest.cpp
```

---

## 3. Database Schema & Design

The relational database is configured in `database/schema.sql` and initialized automatically on first startup.

### Relational Tables & Relationships:
1. **`users`**: Customer accounts (`id`, `username`, `email`, `password_hash`, `salt`, `full_name`, `phone`, `address`, `created_at`).
2. **`admins`**: Protected administrative accounts (`id`, `username`, `email`, `password_hash`, `salt`, `full_name`, `role`). Completely isolated from customer tables.
3. **`categories`**: Product classifications (`id`, `name`, `slug`, `description`, `image_url`).
4. **`products`**: Catalog items (`id`, `name`, `brand`, `category_id`, `description`, `price`, `original_price`, `discount_percent`, `rating`, `review_count`, `stock_quantity`, `image_url`, `is_featured`, `is_trending`). Foreign key references `categories(id)`.
5. **`product_images`**: Multi-image support per product (`id`, `product_id`, `image_url`, `is_primary`).
6. **`cart`** & **`cart_items`**: Persistent shopping carts for authenticated users and guest sessions. Unique constraint on `(cart_id, product_id)`.
7. **`wishlist`** & **`wishlist_items`**: User wishlists with foreign keys to `users(id)` and `products(id)`.
8. **`orders`** & **`order_items`**: Real customer orders placed during checkout with order number, pricing breakdown, shipping address, urgent flag, priority score, and fulfillment status (`Pending`, `Processing`, `Shipped`, `Delivered`, `Cancelled`).
9. **`transactions`**: Payment transaction log linked to `orders(id)` and `users(id)`.
10. **`reviews`**: Customer reviews with ratings (1-5), VADER sentiment label, compound score, and aspect breakdown (Quality, Delivery, Price). Unique constraint on `(product_id, user_id)` prevents duplicate reviews.
11. **`user_activity`**: User interaction log (`view`, `cart_add`, `wishlist_add`, `purchase`) used to compute dynamic graph relationships.

### Database Integrity Rules:
- **No Seed Data Rule**: The initial database contains schema tables and indexes with **zero fake products, zero fake customers, zero fake orders, and zero dummy reviews**.
- **Empty State Display**: The frontend displays professional empty states whenever tables are fresh.

---

## 4. Four Advanced DSA Features

### Feature 1: Trie-Based Smart Search
- **Source Code**: [`include/algorithms/TrieSearch.h`](file:///include/algorithms/TrieSearch.h), [`src/algorithms/TrieSearch.cpp`](file:///src/algorithms/TrieSearch.cpp)
- **Concept**: A prefix tree (Trie) where each node contains an unordered map of character child pointers, an end-of-word flag, a set of matching product IDs, and query frequency weights.
- **Functionality**:
  - Automatically indexes full product titles, brand names, category keywords, and individual tokens.
  - Sub-millisecond autocomplete and live suggestions via `/api/search/suggestions?q=...`.
  - Automatically synchronized when products are added, updated, or deleted via the Admin Portal.
- **Time Complexity**:
  - Insertion: $\mathcal{O}(L)$ where $L$ is the character length of the indexed keyword.
  - Prefix Autocomplete Query: $\mathcal{O}(P + K)$ where $P$ is the prefix length and $K$ is the number of descendants explored up to the result limit.
  - Deletion: $\mathcal{O}(L)$ to traverse and detach product IDs.
- **Space Complexity**: $\mathcal{O}(N \cdot L_{\text{avg}} \cdot \Sigma)$ where $N$ is the total number of distinct words, $L_{\text{avg}}$ is the average word length, and $\Sigma$ is the alphabet size. Dynamic `std::unordered_map` branching allocates memory strictly for characters present in the catalog.

---

### Feature 2: Graph-Based Product Recommendation Engine
- **Source Code**: [`include/algorithms/ProductGraph.h`](file:///include/algorithms/ProductGraph.h), [`src/algorithms/ProductGraph.cpp`](file:///src/algorithms/ProductGraph.cpp)
- **Concept**: A weighted undirected graph represented via an adjacency list: `std::unordered_map<int, std::vector<GraphEdge>>`.
- **Relationship Weighting**:
  - Same Category: $+3.0$ affinity weight.
  - Same Brand: $+2.0$ affinity weight.
  - Price Tier Proximity: up to $+1.5$ based on proportional price similarity.
  - Customer Co-Purchases: $+5.0$ weight for each time two products are checked out in the same customer order.
  - User Co-Browsing: $+1.5$ weight for products viewed in the same user session.
- **Traversal Algorithm**: Best-First Search using a Max-Heap (`std::priority_queue`) to extract top-K highest affinity products while filtering out deleted or out-of-stock items.
- **Time Complexity**:
  - Edge Insertion / Update: $\mathcal{O}(1)$ average.
  - Recommendation Query: $\mathcal{O}((V_{\text{sub}} + E_{\text{sub}}) \log V_{\text{sub}})$ where $V_{\text{sub}}, E_{\text{sub}}$ are the localized neighborhood vertices and edges of the query product.
- **Space Complexity**: $\mathcal{O}(V + E)$ where $V$ is active products and $E$ is relationship affinity edges.

---

### Feature 3: LRU Cache
- **Source Code**: [`include/algorithms/LRUCache.h`](file:///include/algorithms/LRUCache.h), [`src/algorithms/LRUCache.cpp`](file:///src/algorithms/LRUCache.cpp)
- **Concept**: Combines a Hash Map (`std::unordered_map<Key, CacheNode*>`) with a custom Doubly Linked List with head and tail sentinel nodes.
- **Functionality**:
  - Caches serialized product JSON payloads for `/api/products/{id}`.
  - Automatically moves accessed items to the head (Most Recently Used).
  - Evicts the tail node (Least Recently Used) in $\mathcal{O}(1)$ time when capacity ($100$ items) is exceeded.
  - Automatically invalidates cache entries upon product updates or deletions.
  - Live telemetry tracking: cache hits, misses, evictions, capacity, and hit ratio.
- **Time Complexity**:
  - `get(key)`: $\mathcal{O}(1)$ average.
  - `put(key, value)`: $\mathcal{O}(1)$ average.
  - `remove(key)`: $\mathcal{O}(1)$ average.
  - Eviction: $\mathcal{O}(1)$ worst-case.
- **Space Complexity**: Strictly bounded at $\mathcal{O}(C)$ where $C$ is the cache capacity.

---

### Feature 4: Priority Queue-Based Order Processing
- **Source Code**: [`include/algorithms/OrderPriorityQueue.h`](file:///include/algorithms/OrderPriorityQueue.h), [`src/algorithms/OrderPriorityQueue.cpp`](file:///src/algorithms/OrderPriorityQueue.cpp)
- **Concept**: Custom Binary Max-Heap represented as a contiguous array with `heapifyUp` and `heapifyDown` procedures.
- **Priority Scoring Formula**:
  $$\text{Score} = (\text{isUrgent} \times 1000.0) + \min(\text{totalAmount} \times 0.1, 500.0) + (\text{waitMinutes} \times 0.5)$$
  - Urgent/Express Delivery: $+1000$ points bonus.
  - High-Value Orders: proportional points up to $+500$.
  - Aging Boost: pending orders gain $+0.5$ points per elapsed minute to prevent starvation.
- **Operations & Complexity**:
  - `push(order)` (Enqueue): $\mathcal{O}(\log N)$
  - `pop(order)` (Extract Highest Priority): $\mathcal{O}(\log N)$
  - `peek(order)`: $\mathcal{O}(1)$
  - `cancelOrder(orderId)`: $\mathcal{O}(N)$ search and $\mathcal{O}(\log N)$ restoration.
- **Space Complexity**: $\mathcal{O}(N)$ where $N$ is pending orders. Starts completely empty until real customers place orders.

---

## 5. Intelligent & Machine Learning Features

### A. NLP Product Search
- **Source Code**: [`include/algorithms/NLPSearch.h`](file:///include/algorithms/NLPSearch.h), [`src/algorithms/NLPSearch.cpp`](file:///src/algorithms/NLPSearch.cpp)
- Normalizes search queries, removes common stopwords, and detects price range queries (e.g. `"under 50000"`, `"below 10000"`).
- Handles spelling typos using dynamic programming Levenshtein distance against the catalog vocabulary.

### B. Content-Based Recommendation
- **Source Code**: [`include/algorithms/ContentRecommender.h`](file:///include/algorithms/ContentRecommender.h), [`src/algorithms/ContentRecommender.cpp`](file:///src/algorithms/ContentRecommender.cpp)
- Calculates Term Frequency-Inverse Document Frequency (TF-IDF) feature vectors across product names, brands, categories, and descriptions.
- Computes Cosine Similarity between product vectors to surface relevant alternatives.

### C. VADER-Style Sentiment Analysis
- **Source Code**: [`include/algorithms/SentimentAnalyzer.h`](file:///include/algorithms/SentimentAnalyzer.h), [`src/algorithms/SentimentAnalyzer.cpp`](file:///src/algorithms/SentimentAnalyzer.cpp)
- Rule-based valence scoring lexicon in C++ supporting valence modulators, ALL CAPS intensity boosts, exclamation mark boosts, booster words (`"extremely"`, `"super"`), and negation flips (`"not"`, `"never"`).
- Normalizes score to $[-1.0, +1.0]$: Positive ($\ge +0.05$), Neutral, Negative ($\le -0.05$).
- Aspect-based extraction for **Quality**, **Delivery**, and **Price/Value**.

### D. Logistic Regression Category Classification
- **Source Code**: [`include/algorithms/LogisticRegression.h`](file:///include/algorithms/LogisticRegression.h), [`src/algorithms/LogisticRegression.cpp`](file:///src/algorithms/LogisticRegression.cpp)
- One-vs-Rest Multinomial Logistic Regression in pure C++ with batch gradient descent, L2 regularization, and Bag-of-Words feature extraction.
- Detects category recommendations with confidence percentage. Gracefully reports insufficient data if sample size $< 5$.

### E. Random Forest Price Prediction
- **Source Code**: [`include/algorithms/PricePredictor.h`](file:///include/algorithms/PricePredictor.h), [`src/algorithms/PricePredictor.cpp`](file:///src/algorithms/PricePredictor.cpp)
- Ensemble of $10$ bootstrap-aggregated (Bagging) Decision Trees trained on category, brand, rating, and description length features.
- Gracefully reports insufficient data when database records are fresh without fabricating fake values.

---

## 6. Security & Authentication

- **Cryptographic Password Hashing**: Utilizes random 16-byte cryptographic salts (`std::random_device`) combined with iterative multi-round digest stretching.
- **Session Tokens**: 32-byte cryptographically secure hexadecimal tokens stored in `HttpOnly` cookies.
- **SQL Injection Prevention**: 100% of database interactions use SQLite prepared statements with explicit parameter binding (`sqlite3_bind_text`, `sqlite3_bind_int`, `sqlite3_bind_double`).
- **XSS Sanitization**: Dynamic HTML entity encoding (`&`, `<`, `>`, `"`, `'`) for all user-supplied content.
- **Role Isolation**: Admin APIs and dashboard routes are guarded by `AdminAuthFilter` middleware, preventing unauthorized customer or guest access.

---

## 7. RESTful API Documentation

### Public & Customer Endpoints:
| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/products` | Retrieve catalog products with filters (`category_id`, `q`, `min_price`, `max_price`, `sort`) |
| `GET` | `/api/products/{id}` | Retrieve single product details (O(1) LRU Cached) |
| `GET` | `/api/search/suggestions` | Prefix Trie autocomplete suggestions (`?q=...`) |
| `GET` | `/api/recommendations/{id}` | Graph-based recommendations for a product |
| `GET` | `/api/categories` | List all catalog categories |
| `POST` | `/api/auth/register` | Register new customer account |
| `POST` | `/api/auth/login` | Authenticate customer credentials |
| `POST` | `/api/auth/logout` | End customer session |
| `GET` | `/api/auth/me` | Current authenticated session details |
| `PUT` | `/api/auth/profile` | Update customer name, phone, address |
| `GET` | `/api/cart` | Retrieve user/session shopping cart |
| `POST` | `/api/cart/add` | Add item with quantity and stock check |
| `PUT` | `/api/cart/update` | Update item quantity |
| `DELETE` | `/api/cart/remove` | Remove item from cart |
| `GET` | `/api/wishlist` | Retrieve saved wishlist items |
| `POST` | `/api/wishlist/toggle` | Toggle item in wishlist |
| `POST` | `/api/wishlist/move-to-cart`| Move wishlist item into cart |
| `POST` | `/api/orders/checkout` | Process order, verify stock, push to Priority Queue |
| `GET` | `/api/orders` | Customer order history |
| `GET` | `/api/orders/{id}` | Detailed order status and line items |
| `POST` | `/api/orders/{id}/cancel` | Cancel order and restore stock |
| `GET` | `/api/orders/track` | Public tracking query (`?number=...`) |
| `GET` | `/api/products/{id}/reviews`| Verified customer reviews and average rating |
| `POST` | `/api/reviews` | Submit product review (runs VADER analyzer) |
| `POST` | `/api/reviews/analyze` | Real-time sentiment preview for review draft |

### Administrative Endpoints (Protected by Admin Auth):
| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/admin/setup-status` | Check if initial admin setup is required |
| `POST` | `/api/admin/setup` | One-time setup of first administrator (blocked if admin exists) |
| `POST` | `/api/admin/login` | Authenticate admin portal credentials |
| `POST` | `/api/admin/logout` | End admin session |
| `GET` | `/api/admin/stats` | Dynamic stats & live DSA engine telemetry |
| `GET` | `/api/admin/products` | Admin catalog listing |
| `POST` | `/api/admin/products` | Create product & index in Trie/Graph |
| `PUT` | `/api/admin/products/{id}` | Update product & invalidate LRU Cache |
| `DELETE`| `/api/admin/products/{id}` | Delete product & remove from Trie/Graph |
| `GET` | `/api/admin/orders` | List real customer orders |
| `PUT` | `/api/admin/orders/{id}/status`| Update order fulfillment status |
| `GET` | `/api/admin/orders/priority-queue`| Inspect snapshot of Max-Heap Priority Queue |
| `POST` | `/api/admin/orders/process-priority`| Extract highest priority order from Heap |
| `GET` | `/api/admin/customers` | Registered customer directory |
| `GET` | `/api/admin/reviews` | Moderate reviews |
| `DELETE`| `/api/admin/reviews/{id}` | Remove inappropriate customer review |
| `PUT` | `/api/admin/credentials` | Update admin email and password |

---

## 8. Setup & Compilation Instructions

### Prerequisites (Linux / Ubuntu / Debian):
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libsqlite3-dev libdrogon-dev libjsoncpp-dev uuid-dev zlib1g-dev libssl-dev
```

### Build Steps:
```bash
git clone <your-repository-url> shopsphere
cd shopsphere

# Configure and compile with CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Running the Server:
```bash
# Start server (default port 8080 or via PORT environment variable)
./shopsphere_backend
```

---

## 9. Render Deployment Guide

ShopSphere is pre-configured with a multi-stage production `Dockerfile` and `render.yaml` specification for immediate deployment on [Render](https://render.com).

### Deployment Steps:
1. Push the repository to GitHub.
2. In the Render Dashboard, click **New +** > **Blueprint** (or **Web Service**).
3. Connect your GitHub repository.
4. Render will detect `render.yaml` and the `Dockerfile`.
5. Configuration values:
   - **Environment:** `Docker`
   - **Plan:** `Starter`
   - **Health Check Path:** `/api/categories`
   - **Port:** Bind to `$PORT` (handled automatically by `main.cpp`).
   - **Persistent Disk:** Mount `/var/data` to preserve `shopsphere.db`.
6. Click **Deploy Web Service**. Render will compile the C++ binary and launch the container.

---

## 10. Admin Initialization Guide

To ensure zero hardcoded passwords and total security, the first administrator can be created in one of two ways:

### Option A: Via Command Line (Recommended for Server Setup)
Run the compiled binary with the `--init-admin` argument:
```bash
./shopsphere_backend --init-admin <username> <email> <password> "<FullName>"

# Example:
./shopsphere_backend --init-admin admin master@shopsphere.com SecureAdminPass2026 "Lead Administrator"
```

### Option B: Via Web Interface
1. Start the server and navigate to `/admin/login` in your browser.
2. If zero administrators exist in the database, the portal automatically displays the **Protected Initial Setup Form**.
3. Enter your administrator details and click **Create Super Administrator**.
4. Once created, the setup form is permanently locked and cannot be triggered again.

---

## 11. Testing Suite

The repository includes standalone C++ test executables in `tests/`:

### Executing Tests with CTest:
```bash
cd build
ctest --output-on-failure
```

### Direct Test Execution:
```bash
./tests/TrieSearchTest
./tests/ProductGraphTest
./tests/LRUCacheTest
./tests/OrderPriorityQueueTest
./tests/IntelligentFeaturesTest
```

### All test suites verify:
- ✅ Empty database state handling.
- ✅ Correct prefix searching and dynamic token removal in the Trie.
- ✅ Dynamic attribute and co-purchase edge weighting in the Product Graph.
- ✅ Average $\mathcal{O}(1)$ lookup, promotion, and eviction in the LRU Cache.
- ✅ Urgency and high-value ranking in the Binary Max-Heap Priority Queue.
- ✅ NLP query parsing, typo correction, TF-IDF cosine similarity, VADER aspect scoring, Logistic Regression category classification, and Random Forest price prediction.
