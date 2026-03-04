#!/bin/bash

# Build and run the server in Docker

echo "Building DigiPets server Docker image..."
docker build -t digipets-server .

echo "Starting DigiPets server..."
docker-compose up -d

echo "Server started! Check status with: docker-compose ps"
echo "View logs with: docker-compose logs -f"
echo "Stop server with: docker-compose down"
