FROM ubuntu:22.04 as builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code
COPY . .

# Build the server
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_CLIENT=OFF -DBUILD_TESTS=OFF && \
    cmake --build build --target digipets_server -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary from builder
COPY --from=builder /app/build/server/digipets_server /app/

# Expose port
EXPOSE 8080

# Run server
CMD ["/app/digipets_server", "8080"]
