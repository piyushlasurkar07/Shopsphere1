cmake_minimum_required(VERSION 3.16)
project(ShopSphere VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Core Algorithms & Services sources
set(SHOPSPHERE_CORE_SOURCES
    src/utils/Crypto.cpp
    src/algorithms/TrieSearch.cpp
    src/algorithms/ProductGraph.cpp
    src/algorithms/LRUCache.cpp
    src/algorithms/OrderPriorityQueue.cpp
    src/algorithms/NLPSearch.cpp
    src/algorithms/ContentRecommender.cpp
    src/algorithms/SentimentAnalyzer.cpp
    src/algorithms/LogisticRegression.cpp
    src/algorithms/PricePredictor.cpp
    src/database/DatabaseManager.cpp
    src/services/AppServices.cpp
)

# Controller sources
set(SHOPSPHERE_CONTROLLER_SOURCES
    src/controllers/ProductController.cpp
    src/controllers/AuthController.cpp
    src/controllers/CartController.cpp
    src/controllers/WishlistController.cpp
    src/controllers/OrderController.cpp
    src/controllers/ReviewController.cpp
    src/controllers/AdminController.cpp
    src/controllers/PageController.cpp
    src/middleware/AuthMiddleware.cpp
)

# Dependencies
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Drogon CONFIG REQUIRED)

# Main Executable: shopsphere_backend
add_executable(shopsphere_backend
    src/main.cpp
    ${SHOPSPHERE_CORE_SOURCES}
    ${SHOPSPHERE_CONTROLLER_SOURCES}
)

target_link_libraries(shopsphere_backend PRIVATE
    Drogon::Drogon
    SQLite::SQLite3
    Threads::Threads
)

# ----------------------------------------------------------------------------
# DSA & Machine Learning Unit Tests
# ----------------------------------------------------------------------------
enable_testing()

# 1. Trie Search Test
add_executable(TrieSearchTest
    tests/TrieSearchTest.cpp
    src/algorithms/TrieSearch.cpp
)
target_link_libraries(TrieSearchTest PRIVATE Threads::Threads)
add_test(NAME TrieSearchTest COMMAND TrieSearchTest)

# 2. Product Graph Test
add_executable(ProductGraphTest
    tests/ProductGraphTest.cpp
    src/algorithms/ProductGraph.cpp
)
target_link_libraries(ProductGraphTest PRIVATE Threads::Threads)
add_test(NAME ProductGraphTest COMMAND ProductGraphTest)

# 3. LRU Cache Test
add_executable(LRUCacheTest
    tests/LRUCacheTest.cpp
)
target_link_libraries(LRUCacheTest PRIVATE Threads::Threads)
add_test(NAME LRUCacheTest COMMAND LRUCacheTest)

# 4. Order Priority Queue Test
add_executable(OrderPriorityQueueTest
    tests/OrderPriorityQueueTest.cpp
    src/algorithms/OrderPriorityQueue.cpp
)
target_link_libraries(OrderPriorityQueueTest PRIVATE Threads::Threads)
add_test(NAME OrderPriorityQueueTest COMMAND OrderPriorityQueueTest)

# 5. Intelligent ML Features Test
add_executable(IntelligentFeaturesTest
    tests/IntelligentFeaturesTest.cpp
    src/algorithms/NLPSearch.cpp
    src/algorithms/ContentRecommender.cpp
    src/algorithms/SentimentAnalyzer.cpp
    src/algorithms/LogisticRegression.cpp
    src/algorithms/PricePredictor.cpp
)
target_link_libraries(IntelligentFeaturesTest PRIVATE Threads::Threads)
add_test(NAME IntelligentFeaturesTest COMMAND IntelligentFeaturesTest)
