# DigiPets - C++20 Digital Pets Game

A server/client C++20 implementation of a digital pet game inspired by Digimon. Features a RESTful API server with **multi-user support** that can be containerized with Docker and a cross-platform client library.

## Features

- **C++20 Implementation**: Modern C++ with concepts, ranges, and modules support
- **Multi-User Support**: Each user has their own profile and pets with complete isolation
- **User Authentication**: Secure registration and login system
- **RESTful API**: Full CRUD operations for pet management
- **Docker Support**: Server runs in a containerized environment
- **Cross-Platform**: Client library works on Windows, Linux, macOS, and can be compiled for Android/Web
- **Thread-Safe**: All operations are thread-safe with proper synchronization
- **Persistent Storage**: Users and pets are saved to JSON files
- **Real-Time Updates**: Pets age and their stats change over time
- **Evolution System**: Pets evolve through multiple stages based on level and age
- **Ownership Protection**: Users can only access and manage their own pets

## Architecture

```
┌─────────────────┐
│   Client Apps   │  (CLI, Android, Web)
│   (C++20)       │
└────────┬────────┘
         │
         │ HTTP/REST
         │
┌────────▼────────┐
│   API Server    │  (Docker Container)
│   (C++20)       │
└────────┬────────┘
         │
         │ JSON Files
         │
┌────────▼────────┐
│   Persistence   │
└─────────────────┘
```

## Project Structure

```
digi-pets-20/
├── common/          # Shared game logic library
│   ├── include/     # Pet class, game mechanics
│   └── src/
├── server/          # HTTP server with REST API
│   ├── include/     # Pet manager
│   └── src/
├── client/          # Client library and CLI
│   ├── include/     # API wrapper
│   └── src/
├── tests/           # Unit and integration tests
├── docker/          # Docker configuration
├── Dockerfile       # Server container
└── docker-compose.yml
```

## Game Mechanics

### Pet Lifecycle

1. **Egg** → **Baby** (1 hour)
2. **Baby** → **Child** (Level 5)
3. **Child** → **Adult** (Level 15)
4. **Adult** → **Ultimate** (Level 30)

### Pet Stats

- **Health**: 0-100, decreases when hungry or unhappy
- **Hunger**: 0-100, increases over time
- **Happiness**: 0-100, decreases when neglected
- **Strength**: Increases through training
- **Intelligence**: Increases through training
- **Level**: Increases through experience
- **Experience**: Gained through actions (feed, train, play)

### Actions

- **Feed**: Reduces hunger, increases health and happiness
- **Train**: Increases strength and intelligence, costs happiness
- **Play**: Increases happiness, costs hunger
- **Rest**: Restores health

## Getting Started

### Prerequisites

- CMake 3.20 or higher
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Docker (for containerized deployment)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Gameaday/digi-pets-20.git
cd digi-pets-20

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Run tests
ctest --output-on-failure
```

### Running the Server

#### Option 1: Native Build

```bash
# From build directory
./server/digipets_server 8080
```

#### Option 2: Docker (Recommended)

```bash
# Build and start the server
docker-compose up -d

# View logs
docker-compose logs -f

# Stop the server
docker-compose down
```

### Using the CLI Client

```bash
# Connect to local server
./client/digipets_cli http://localhost:8080

# Connect to remote server
./client/digipets_cli http://your-server:8080
```

## API Reference

For complete multi-user API documentation, see [Multi-User API Documentation](docs/MULTIUSER_API.md).

### Quick Start

#### 1. Register a User
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"password123"}'
```

#### 2. Login
```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"password123"}'
```

#### 3. Create a Pet (requires authentication)
```bash
curl -X POST http://localhost:8080/api/pets \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <your-token>" \
  -d '{"name":"Agumon","species":"agumon"}'
```

### Base URL
```
http://localhost:8080
```

### Authentication

All pet-related endpoints require authentication. Include the token in the Authorization header:

```
Authorization: Bearer <token>
```

### Main Endpoints

#### Authentication
- `POST /api/auth/register` - Register a new user
- `POST /api/auth/login` - Login and get token

#### Health Check
```http
GET /health
```

#### Pet Management (all require authentication)
```http
GET /api/pets
GET /api/pets/{id}
POST /api/pets
DELETE /api/pets/{id}
POST /api/pets/{id}/feed
POST /api/pets/{id}/train
POST /api/pets/{id}/play
POST /api/pets/{id}/rest
```

### Response Format

All successful responses return JSON. Pet data now includes owner_id:

```json
{
  "id": "a1b2c3d4-e5f6-4a7b-8c9d-0e1f2a3b4c5d",
  "name": "Agumon",
  "species": "agumon",
  "stage": "baby",
  "is_alive": true,
  "stats": {
    "health": 100,
    "hunger": 25,
    "happiness": 80,
    "strength": 15,
    "intelligence": 12,
    "age_hours": 2,
    "level": 3,
    "experience": 45
  },
  "birth_time": 1699372800,
  "last_update": 1699380000
}
```

## Client Library Usage

### C++ Example

```cpp
#include "digipets_client.hpp"

using namespace digipets;

int main() {
    // Create client
    DigiPetsClient client("http://localhost:8080");
    
    // Check server connection
    if (!client.check_health()) {
        std::cerr << "Server not available\n";
        return 1;
    }
    
    // Create a new pet
    auto pet = client.create_pet("Agumon", PetSpecies::AGUMON);
    if (!pet) {
        std::cerr << "Failed to create pet\n";
        return 1;
    }
    
    std::string pet_id = pet->get_id();
    
    // Interact with the pet
    client.feed_pet(pet_id);
    client.train_pet(pet_id);
    client.play_with_pet(pet_id);
    
    // Get updated pet data
    auto updated_pet = client.get_pet(pet_id);
    if (updated_pet) {
        std::cout << "Pet level: " << updated_pet->get_stats().level << "\n";
    }
    
    return 0;
}
```

## Cross-Platform Deployment

### Android

The client library can be compiled for Android using the NDK:

```bash
# Set up Android NDK
export ANDROID_NDK=/path/to/ndk

# Configure for Android
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-21 \
  -DBUILD_SERVER=OFF

cmake --build .
```

### WebAssembly (Emscripten)

```bash
# Install Emscripten
# https://emscripten.org/docs/getting_started/downloads.html

# Configure for Web
emcmake cmake .. -DBUILD_SERVER=OFF

# Build
emmake make
```

### Windows

```bash
# Use Visual Studio or MinGW
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Development

### Running Tests

```bash
# From build directory
ctest --output-on-failure

# Or run specific test
./tests/digipets_tests
```

### Code Style

The project uses modern C++20 features:
- Concepts and constraints
- Ranges
- Coroutines (where applicable)
- Modules (planned)
- `std::format` (where available)

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## Security

### Current Implementation
- User authentication with password hashing
- Bearer token-based authorization
- Complete user isolation (users can only access their own pets)
- Ownership verification on all pet operations
- Input validation on all API endpoints
- Thread-safe operations with mutex protection
- No SQL injection vulnerabilities (uses JSON file storage)
- CORS headers for web client support
- Graceful shutdown handling

### ⚠️ Production Security Notes
**Important:** This implementation uses simplified authentication suitable for development/demo purposes.

For production deployment, you should:
- Replace simple password hashing with bcrypt or argon2
- Implement proper JWT tokens with expiration and refresh mechanism
- Add HTTPS/TLS encryption for all communications
- Implement rate limiting on authentication endpoints
- Add account lockout after failed login attempts
- Consider adding email verification for registration
- Use environment variables for sensitive configuration
- Implement proper session management

See [Multi-User API Documentation](docs/MULTIUSER_API.md) for details.

## Performance

- Asynchronous I/O where applicable
- Minimal memory footprint
- Efficient JSON serialization
- Thread pool for concurrent requests
- Background updates every 5 minutes

## Roadmap

- [x] Multi-user authentication and authorization ✅
- [ ] Enhanced security (JWT tokens, bcrypt, rate limiting)
- [ ] WebSocket support for real-time updates
- [ ] Multiplayer battles between pets
- [ ] Database backend (PostgreSQL/SQLite)
- [ ] Mobile apps (Android/iOS)
- [ ] Web frontend (React/Vue)
- [ ] Pet trading system
- [ ] Achievements and leaderboards

## License

MIT License - See LICENSE file for details

## Acknowledgments

- Inspired by the original Digimon virtual pets
- Built with modern C++20 features
- Uses [nlohmann/json](https://github.com/nlohmann/json) for JSON handling
- Uses [cpp-httplib](https://github.com/yhirose/cpp-httplib) for HTTP server/client
- Uses [Google Test](https://github.com/google/googletest) for testing

## Support

For issues, questions, or contributions, please open an issue on GitHub.