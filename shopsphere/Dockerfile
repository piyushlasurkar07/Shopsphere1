# ============================================================================
# ShopSphere Multi-Stage Production Dockerfile for Linux / Render Deployment
# Pure C++ Backend with Drogon Web Framework & SQLite
# ============================================================================

# ----------------------------------------------------------------------------
# Stage 1: Build Environment
# ----------------------------------------------------------------------------
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install modern C++ compiler, CMake, SQLite, and Drogon dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    pkg-config \
    libsqlite3-dev \
    libssl-dev \
    libjsoncpp-dev \
    uuid-dev \
    zlib1g-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Build and install Drogon C++ Web Framework from source
WORKDIR /drogon_build
RUN git clone --depth 1 --recurse-submodules https://github.com/drogonframework/drogon.git \
    && cd drogon \
    && mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF .. \
    && cmake --build . --parallel 1 \
    && make install \
    && ldconfig

# Build ShopSphere C++ Application
WORKDIR /app
COPY . .

RUN mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && cmake --build . --parallel 1 \
    && ctest --output-on-failure

# ----------------------------------------------------------------------------
# Stage 2: Minimal Production Runtime
# ----------------------------------------------------------------------------
FROM ubuntu:22.04 AS runner

ENV DEBIAN_FRONTEND=noninteractive
ENV PORT=8080
ENV DATABASE_PATH=/app/database/shopsphere.db
ENV SCHEMA_PATH=/app/database/schema.sql

RUN apt-get update && apt-get install -y --no-install-recommends \
    libsqlite3-0 \
    libssl3 \
    libjsoncpp25 \
    uuid-runtime \
    zlib1g \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy Drogon dynamic libraries and ShopSphere compiled executable
COPY --from=builder /usr/local/lib/libdrogon* /usr/local/lib/
COPY --from=builder /usr/local/lib/libtrantor* /usr/local/lib/
RUN ldconfig

COPY --from=builder /app/build/shopsphere_backend /app/shopsphere_backend
COPY --from=builder /app/build/TrieSearchTest /app/tests/TrieSearchTest
COPY --from=builder /app/build/ProductGraphTest /app/tests/ProductGraphTest
COPY --from=builder /app/build/LRUCacheTest /app/tests/LRUCacheTest
COPY --from=builder /app/build/OrderPriorityQueueTest /app/tests/OrderPriorityQueueTest
COPY --from=builder /app/build/IntelligentFeaturesTest /app/tests/IntelligentFeaturesTest

# Copy static frontend assets, templates, and database schema
COPY frontend /app/frontend
COPY database /app/database
COPY config /app/config

# Create storage directory for database
RUN mkdir -p /app/database /app/uploads

EXPOSE 8080

CMD ["/app/shopsphere_backend"]
