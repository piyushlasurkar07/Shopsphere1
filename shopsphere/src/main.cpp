#include <drogon/drogon.h>
#include "services/AppServices.h"
#include <iostream>
#include <cstdlib>
#include <string>

int main(int argc, char *argv[]) {
    std::cout << "========================================================\n"
              << "   ShopSphere - Modern C++ E-Commerce Platform\n"
              << "   Powered by Drogon & Advanced DSA Engines\n"
              << "========================================================" << std::endl;

    // 1. Resolve environment configuration
    std::string dbPath = "database/shopsphere.db";
    const char *envDb = std::getenv("DATABASE_PATH");
    if (envDb && std::string(envDb).length() > 0) {
        dbPath = envDb;
    }

    std::string schemaPath = "database/schema.sql";
    const char *envSchema = std::getenv("SCHEMA_PATH");
    if (envSchema && std::string(envSchema).length() > 0) {
        schemaPath = envSchema;
    }

    // 2. Initialize Database & Advanced DSA Engines (Trie, Graph, LRU Cache, Priority Queue)
    if (!ShopSphere::Services::AppServices::instance().initialize(dbPath, schemaPath)) {
        std::cerr << "[FATAL] Failed to initialize Database & DSA Engine." << std::endl;
        return 1;
    }

    // 3. Handle CLI --init-admin command
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--init-admin" && i + 4 < argc) {
            std::string username = argv[i + 1];
            std::string email = argv[i + 2];
            std::string password = argv[i + 3];
            std::string fullName = argv[i + 4];
            std::string err;
            if (ShopSphere::Services::AppServices::instance().setupFirstAdmin(username, email, password, fullName, err)) {
                std::cout << "[SUCCESS] CLI: Initial administrator created for " << email << std::endl;
            } else {
                std::cerr << "[INFO] CLI: Admin setup result: " << err << std::endl;
            }
            return 0;
        }
    }

    // 4. Resolve listening port (Render compatibility via PORT environment variable)
    uint16_t port = 8080;
    const char *envPort = std::getenv("PORT");
    if (envPort && std::string(envPort).length() > 0) {
        try {
            port = static_cast<uint16_t>(std::stoi(envPort));
        } catch (...) {
            port = 8080;
        }
    }

    // 5. Configure Drogon Application
    drogon::app()
        .setLogLevel(trantor::Logger::kInfo)
        .addListener("0.0.0.0", port)
        .setThreadNum(4)
        .setDocumentRoot("./frontend")
        .setUploadPath("./uploads")
        .setStaticFilesCacheTime(3600);

    std::cout << "[SERVER] ShopSphere C++ server starting on port: " << port << std::endl;
    std::cout << "[SERVER] Ready for customer requests and admin operations.\n"
              << "========================================================" << std::endl;

    drogon::app().run();
    return 0;
}
