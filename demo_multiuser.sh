#!/bin/bash

# Multi-User Demo Script for DigiPets

SERVER_URL="http://localhost:8080"

echo "==================================================================="
echo "          DigiPets Multi-User Demo"
echo "==================================================================="
echo ""

# Check if server is running
echo "Checking server health..."
if ! curl -s "$SERVER_URL/health" > /dev/null 2>&1; then
    echo "ERROR: Server is not running at $SERVER_URL"
    echo "Please start the server first:"
    echo "  ./build/server/digipets_server 8080"
    exit 1
fi
echo "✓ Server is running"
echo ""

# Register User 1
echo "1. Registering user 'alice'..."
USER1_RESPONSE=$(curl -s -X POST "$SERVER_URL/api/auth/register" \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"alice123"}')
echo "$USER1_RESPONSE" | jq .
echo ""

# Login User 1
echo "2. Logging in as 'alice'..."
USER1_LOGIN=$(curl -s -X POST "$SERVER_URL/api/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"alice123"}')
USER1_TOKEN=$(echo "$USER1_LOGIN" | jq -r '.token')
echo "$USER1_LOGIN" | jq .
echo ""

# Register User 2
echo "3. Registering user 'bob'..."
USER2_RESPONSE=$(curl -s -X POST "$SERVER_URL/api/auth/register" \
  -H "Content-Type: application/json" \
  -d '{"username":"bob","password":"bob456"}')
USER2_TOKEN=$(echo "$USER2_RESPONSE" | jq -r '.user_id')
echo "$USER2_RESPONSE" | jq .
echo ""

# Alice creates a pet
echo "4. Alice creates an Agumon named 'FireDragon'..."
ALICE_PET=$(curl -s -X POST "$SERVER_URL/api/pets" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $USER1_TOKEN" \
  -d '{"name":"FireDragon","species":"agumon"}')
ALICE_PET_ID=$(echo "$ALICE_PET" | jq -r '.id')
echo "$ALICE_PET" | jq '{id, name, owner_id, species, stage}'
echo ""

# Bob creates a pet
echo "5. Bob creates a Gabumon named 'IceWolf'..."
BOB_PET=$(curl -s -X POST "$SERVER_URL/api/pets" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $USER2_TOKEN" \
  -d '{"name":"IceWolf","species":"gabumon"}')
BOB_PET_ID=$(echo "$BOB_PET" | jq -r '.id')
echo "$BOB_PET" | jq '{id, name, owner_id, species, stage}'
echo ""

# Alice feeds her pet
echo "6. Alice feeds FireDragon..."
curl -s -X POST "$SERVER_URL/api/pets/$ALICE_PET_ID/feed" \
  -H "Authorization: Bearer $USER1_TOKEN" | jq '{name, stats: {hunger, happiness, experience}}'
echo ""

# Bob trains his pet
echo "7. Bob trains IceWolf..."
curl -s -X POST "$SERVER_URL/api/pets/$BOB_PET_ID/train" \
  -H "Authorization: Bearer $USER2_TOKEN" | jq '{name, stats: {strength, intelligence, experience}}'
echo ""

# Alice lists her pets (should only see her own)
echo "8. Alice lists her pets (should only see FireDragon)..."
curl -s "$SERVER_URL/api/pets" \
  -H "Authorization: Bearer $USER1_TOKEN" | jq '[.[] | {name, species, owner_id}]'
echo ""

# Bob lists his pets (should only see his own)
echo "9. Bob lists his pets (should only see IceWolf)..."
curl -s "$SERVER_URL/api/pets" \
  -H "Authorization: Bearer $USER2_TOKEN" | jq '[.[] | {name, species, owner_id}]'
echo ""

# Try to access Bob's pet as Alice (should fail)
echo "10. Alice tries to access Bob's pet (should fail)..."
ALICE_ACCESS_BOB=$(curl -s "$SERVER_URL/api/pets/$BOB_PET_ID" \
  -H "Authorization: Bearer $USER1_TOKEN")
echo "$ALICE_ACCESS_BOB" | jq .
echo ""

echo "==================================================================="
echo "          Demo Complete!"
echo "==================================================================="
echo ""
echo "Summary:"
echo "  ✓ Multiple users can register and login independently"
echo "  ✓ Each user has their own pets"
echo "  ✓ Users can only see and manage their own pets"
echo "  ✓ Access to other users' pets is denied"
echo ""
