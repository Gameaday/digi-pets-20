# API Documentation

## Overview

The DigiPets API is a RESTful HTTP API that manages digital pets. All endpoints return JSON data and use standard HTTP status codes.

## Base URL

```
http://localhost:8080
```

## Authentication

Currently, the API does not require authentication. This will be added in a future version.

## Error Handling

All errors return a JSON object with an `error` field:

```json
{
  "error": "Error message here"
}
```

### HTTP Status Codes

- `200 OK`: Success
- `201 Created`: Resource created successfully
- `400 Bad Request`: Invalid request data
- `404 Not Found`: Resource not found
- `500 Internal Server Error`: Server error

## Endpoints

### Health Check

Check if the server is running.

```http
GET /health
```

**Response**
```json
{
  "status": "ok"
}
```

---

### List All Pets

Retrieve all pets in the system.

```http
GET /api/pets
```

**Response**
```json
[
  {
    "id": "uuid-here",
    "name": "Agumon",
    "species": "agumon",
    "stage": "baby",
    "is_alive": true,
    "stats": {
      "health": 100,
      "hunger": 20,
      "happiness": 85,
      "strength": 12,
      "intelligence": 10,
      "age_hours": 2,
      "level": 1,
      "experience": 15
    },
    "birth_time": 1699372800,
    "last_update": 1699380000
  }
]
```

---

### Get Pet by ID

Retrieve a specific pet.

```http
GET /api/pets/{id}
```

**Parameters**
- `id` (path): Pet UUID

**Response**
```json
{
  "id": "uuid-here",
  "name": "Agumon",
  "species": "agumon",
  "stage": "baby",
  "is_alive": true,
  "stats": { ... }
}
```

**Error Response**
```json
{
  "error": "Pet not found"
}
```

---

### Create New Pet

Create a new pet.

```http
POST /api/pets
Content-Type: application/json
```

**Request Body**
```json
{
  "name": "Agumon",
  "species": "agumon"
}
```

**Species Options**
- `agumon`
- `gabumon`
- `patamon`
- `tailmon`

**Response** (201 Created)
```json
{
  "id": "newly-generated-uuid",
  "name": "Agumon",
  "species": "agumon",
  "stage": "egg",
  "is_alive": true,
  "stats": {
    "health": 100,
    "hunger": 0,
    "happiness": 100,
    "strength": 10,
    "intelligence": 10,
    "age_hours": 0,
    "level": 1,
    "experience": 0
  }
}
```

---

### Delete Pet

Delete a pet permanently.

```http
DELETE /api/pets/{id}
```

**Parameters**
- `id` (path): Pet UUID

**Response**
```json
{
  "success": true
}
```

---

### Feed Pet

Feed a pet to reduce hunger.

```http
POST /api/pets/{id}/feed
```

**Parameters**
- `id` (path): Pet UUID

**Effects**
- Hunger: -30
- Happiness: +5
- Health: +5
- Experience: +2

**Response**
```json
{
  "id": "uuid-here",
  "name": "Agumon",
  "stats": {
    "hunger": 0,
    "happiness": 105,
    "health": 105,
    "experience": 17
  }
}
```

---

### Train Pet

Train a pet to increase stats.

```http
POST /api/pets/{id}/train
```

**Parameters**
- `id` (path): Pet UUID

**Effects**
- Strength: +3
- Intelligence: +1
- Hunger: +15
- Happiness: -5
- Experience: +10

**Response**
```json
{
  "id": "uuid-here",
  "name": "Agumon",
  "stats": {
    "strength": 13,
    "intelligence": 11,
    "hunger": 15,
    "happiness": 100,
    "experience": 25
  }
}
```

---

### Play with Pet

Play with a pet to increase happiness.

```http
POST /api/pets/{id}/play
```

**Parameters**
- `id` (path): Pet UUID

**Effects**
- Happiness: +20
- Hunger: +10
- Experience: +5

**Response**
```json
{
  "id": "uuid-here",
  "name": "Agumon",
  "stats": {
    "happiness": 120,
    "hunger": 10,
    "experience": 20
  }
}
```

---

### Rest Pet

Let a pet rest to restore health.

```http
POST /api/pets/{id}/rest
```

**Parameters**
- `id` (path): Pet UUID

**Effects**
- Health: +20
- Happiness: +5
- Hunger: +5

**Response**
```json
{
  "id": "uuid-here",
  "name": "Agumon",
  "stats": {
    "health": 120,
    "happiness": 105,
    "hunger": 5
  }
}
```

## Data Models

### Pet Object

```typescript
{
  id: string,              // UUID
  name: string,            // Pet name
  species: string,         // agumon | gabumon | patamon | tailmon
  stage: string,           // egg | baby | child | adult | ultimate
  is_alive: boolean,       // Health > 0
  stats: {
    health: number,        // 0-100
    hunger: number,        // 0-100
    happiness: number,     // 0-100
    strength: number,      // 0+
    intelligence: number,  // 0+
    age_hours: number,     // Hours since birth
    level: number,         // 1+
    experience: number     // 0+
  },
  birth_time: number,      // Unix timestamp
  last_update: number      // Unix timestamp
}
```

## Rate Limiting

Currently not implemented. Future versions will include rate limiting to prevent abuse.

## CORS

The API includes CORS headers allowing requests from any origin:
- `Access-Control-Allow-Origin: *`
- `Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS`
- `Access-Control-Allow-Headers: Content-Type`

## Webhooks

Not currently supported. Planned for future versions.

## Changelog

### v1.0.0 (Current)
- Initial release
- Basic CRUD operations
- Pet actions (feed, train, play, rest)
- Automatic time-based updates
- JSON persistence
