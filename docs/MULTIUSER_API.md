# Multi-User API Documentation

## Overview

The DigiPets API now supports multiple users with authentication. Each user has their own profile and pets that are isolated from other users.

## Base URL

```
http://localhost:8080
```

## Authentication

All pet-related endpoints require authentication using a Bearer token in the Authorization header.

### Register a New User

Create a new user account.

```http
POST /api/auth/register
Content-Type: application/json

{
  "username": "alice",
  "password": "password123"
}
```

**Response** (201 Created)
```json
{
  "user_id": "uuid-here",
  "username": "alice",
  "message": "User registered successfully"
}
```

**Error** (409 Conflict)
```json
{
  "error": "Username already exists"
}
```

---

### Login

Authenticate and receive an access token.

```http
POST /api/auth/login
Content-Type: application/json

{
  "username": "alice",
  "password": "password123"
}
```

**Response** (200 OK)
```json
{
  "user_id": "uuid-here",
  "username": "alice",
  "token": "uuid-here",
  "message": "Login successful"
}
```

**Error** (401 Unauthorized)
```json
{
  "error": "Invalid credentials"
}
```

---

## Pet Endpoints

All pet endpoints require the `Authorization` header with a Bearer token.

### List User's Pets

Get all pets belonging to the authenticated user.

```http
GET /api/pets
Authorization: Bearer <token>
```

**Response**
```json
[
  {
    "id": "pet-uuid",
    "name": "Agumon",
    "owner_id": "user-uuid",
    "species": "agumon",
    "stage": "baby",
    "is_alive": true,
    "stats": { ... }
  }
]
```

---

### Create New Pet

Create a new pet for the authenticated user.

```http
POST /api/pets
Authorization: Bearer <token>
Content-Type: application/json

{
  "name": "MyPet",
  "species": "agumon"
}
```

**Response** (201 Created)
```json
{
  "id": "pet-uuid",
  "name": "MyPet",
  "owner_id": "user-uuid",
  "species": "agumon",
  "stage": "egg",
  "stats": { ... }
}
```

---

### Get Pet Details

Get details of a specific pet. User must own the pet.

```http
GET /api/pets/{id}
Authorization: Bearer <token>
```

**Response**
```json
{
  "id": "pet-uuid",
  "name": "MyPet",
  "owner_id": "user-uuid",
  "species": "agumon",
  "stats": { ... }
}
```

**Error** (404 Not Found)
```json
{
  "error": "Pet not found or access denied"
}
```

---

### Delete Pet

Delete a pet. User must own the pet.

```http
DELETE /api/pets/{id}
Authorization: Bearer <token>
```

**Response**
```json
{
  "success": true
}
```

---

### Pet Actions

All actions require authentication and ownership.

#### Feed Pet
```http
POST /api/pets/{id}/feed
Authorization: Bearer <token>
```

#### Train Pet
```http
POST /api/pets/{id}/train
Authorization: Bearer <token>
```

#### Play with Pet
```http
POST /api/pets/{id}/play
Authorization: Bearer <token>
```

#### Rest Pet
```http
POST /api/pets/{id}/rest
Authorization: Bearer <token>
```

**Response** (All actions)
```json
{
  "id": "pet-uuid",
  "name": "MyPet",
  "owner_id": "user-uuid",
  "stats": {
    "health": 100,
    "hunger": 10,
    "happiness": 95,
    ...
  }
}
```

---

## Error Responses

### 401 Unauthorized
Missing or invalid authentication token.

```json
{
  "error": "Unauthorized"
}
```

### 404 Not Found
Pet not found or user doesn't have permission to access it.

```json
{
  "error": "Pet not found or access denied"
}
```

### 400 Bad Request
Invalid request data.

```json
{
  "error": "Invalid request: <details>"
}
```

---

## Example Usage

### Complete Workflow

```bash
# 1. Register a user
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"password123"}'

# 2. Login
TOKEN=$(curl -s -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"password123"}' | jq -r '.token')

# 3. Create a pet
PET_ID=$(curl -s -X POST http://localhost:8080/api/pets \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"name":"Agumon","species":"agumon"}' | jq -r '.id')

# 4. Feed the pet
curl -X POST http://localhost:8080/api/pets/$PET_ID/feed \
  -H "Authorization: Bearer $TOKEN"

# 5. List my pets
curl http://localhost:8080/api/pets \
  -H "Authorization: Bearer $TOKEN"
```

---

## Security Notes

- Passwords are hashed before storage (using simple hash for demo; use bcrypt/argon2 in production)
- Tokens are currently user_id (simple for demo; use JWT or session tokens in production)
- Each user can only access their own pets
- All pet operations verify ownership before proceeding

---

## Multi-User Features

### User Isolation
- Each user has their own set of pets
- Users cannot see or interact with other users' pets
- Pet listing is filtered by owner_id

### Ownership Verification
All pet operations verify:
1. User is authenticated
2. Pet exists
3. User owns the pet

### Data Persistence
- User data stored in `users.json`
- Pet data stored in `pets.json`
- Both files include owner relationships

---

## Migration from Single-User

Old pets without owner_id are backward compatible:
- They will have empty owner_id
- Can be accessed without authentication (if owner_id check is bypassed)
- New pets always have an owner_id

To migrate existing pets:
1. Backup `pets.json`
2. Create users
3. Update pet records to include owner_id
4. Save updated data
