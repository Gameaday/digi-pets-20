FROM ubuntu:24.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ca-certificates \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code
COPY . .

# Build the server (skip client & tests for the Docker image)
RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_CLIENT=OFF \
        -DBUILD_TESTS=OFF \
    && cmake --build build --target digipets_server -j$(nproc)

# ---- Runtime stage --------------------------------------------------------
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Run as a non-root user for security
RUN useradd --system --no-create-home --shell /usr/sbin/nologin digipets

WORKDIR /app

# Create data directory and fix ownership
RUN mkdir -p /app/data && chown digipets /app/data

COPY --from=builder /app/build/server/digipets_server /app/digipets_server

USER digipets

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD curl -fs http://localhost:8080/health || exit 1

ENTRYPOINT ["/app/digipets_server"]
CMD ["8080"]
