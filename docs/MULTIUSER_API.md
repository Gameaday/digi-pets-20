# Multi-User API Documentation

## Overview

The DigiPets API supports multiple users with full authentication and session management. Each user has their own profile and pets that are completely isolated from other users.

## Base URL

```
http://localhost:8080
```

## Authentication

All pet-related endpoints require authentication using a **Bearer session token** in the `Authorization` header.

Session tokens:
- Are 64-character cryptographically random hex strings
- Expire after **24 hours**
- Are invalidated immediately on logout
- Can be refreshed by logging in again

### Input Validation

| Field    | Constraints                                          |
|----------|------------------------------------------------------|
| username | 3–32 characters, letters/digits/underscores only     |
| password | 8–128 characters (any characters)                   |
| pet name | 1–64 characters                                     |

---

### Register a New User

```http
POST /api/auth/register
Content-Type: application/json

{
  "username": "alice",
  "password": "hunter1234"
}
```

**Response** (201 Created)
```json
{
  "user_id": "uuid-here",
  "username": "alice"
}
```

**Errors**
- `400 Bad Request` — username/password fails validation
- `409 Conflict` — username already taken

---

### Login

```http
POST /api/auth/login
Content-Type: application/json

{
  "username": "alice",
  "password": "hunter1234"
}
```

**Response** (200 OK)
```json
{
  "user_id": "uuid-here",
  "username": "alice",
  "token": "a3f9...64-hex-chars...2b1c",
  "expires_in_seconds": 86400
}
```

**Error** (401 Unauthorized)
```json
{ "error": "Invalid credentials" }
```

---

### Logout

Invalidates the current session token immediately.

```http
POST /api/auth/logout
Authorization: Bearer <token>
```

**Response** (200 OK)
```json
{ "message": "Logged out" }
```

---


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

- Passwords are hashed with **SHA-256** and a **random per-user salt** (32 hex chars)
- Session tokens are **64-character cryptographically random hex strings** — not derived from or equal to user_id
- Tokens expire after **24 hours** and are immediately invalidated on logout
- Each user can only access their own pets
- All pet operations verify ownership before proceeding

**⚠️ Remaining Production Recommendations:**
- Upgrade password hashing to bcrypt or argon2id for resistance against brute-force
- Add rate limiting on `/api/auth/login` and `/api/auth/register`
- Use HTTPS/TLS for all communications
- Add refresh token mechanism for long-lived sessions
- Implement account lockout after repeated failed login attempts
- Add email verification for new registrations

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
