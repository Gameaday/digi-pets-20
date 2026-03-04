# Quick Start Guide

Get started with DigiPets in 5 minutes!

## Option 1: Docker (Easiest)

### Prerequisites
- Docker
- Docker Compose

### Steps

1. **Clone the repository**
```bash
git clone https://github.com/Gameaday/digi-pets-20.git
cd digi-pets-20
```

2. **Start the server**
```bash
docker-compose up -d
```

3. **Test the API**
```bash
curl http://localhost:8080/health
```

4. **Create your first pet**
```bash
curl -X POST http://localhost:8080/api/pets \
  -H "Content-Type: application/json" \
  -d '{"name":"Agumon","species":"agumon"}'
```

That's it! Your DigiPets server is running on http://localhost:8080

## Option 2: Native Build

### Prerequisites
- C++20 compiler
- CMake 3.20+

### Steps

1. **Clone the repository**
```bash
git clone https://github.com/Gameaday/digi-pets-20.git
cd digi-pets-20
```

2. **Build**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

3. **Run the server**
```bash
./server/digipets_server 8080
```

4. **In another terminal, use the CLI client**
```bash
./client/digipets_cli http://localhost:8080
```

## Next Steps

- Read the [API Documentation](docs/API.md)
- Check out the [Deployment Guide](docs/DEPLOYMENT.md)
- Explore the game mechanics in the [README](README.md)

## Common Tasks

### Create a pet
```bash
curl -X POST http://localhost:8080/api/pets \
  -H "Content-Type: application/json" \
  -d '{"name":"MyPet","species":"gabumon"}'
```

### List all pets
```bash
curl http://localhost:8080/api/pets
```

### Feed a pet (replace {id} with actual pet ID)
```bash
curl -X POST http://localhost:8080/api/pets/{id}/feed
```

### Train a pet
```bash
curl -X POST http://localhost:8080/api/pets/{id}/train
```

## Troubleshooting

**Port already in use?**
```bash
# Change port in docker-compose.yml or run:
docker-compose down
```

**Can't connect to server?**
```bash
# Check if server is running:
docker-compose ps
# View logs:
docker-compose logs -f
```

**Need help?**
- Check the [README](README.md) for more details
- Open an issue on GitHub

## Have fun raising your DigiPets! 🐾
