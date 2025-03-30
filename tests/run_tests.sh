#!/bin/bash
# Integration tests for gRPC application

set -e  # Exit on any error

echo "Starting integration tests..."

# Test 1: API Health Check
echo "Test 1: Checking API health..."
if curl -s -f http://localhost:8002/docs > /dev/null; then
  echo "✅ API is up and docs are accessible"
else
  echo "❌ API health check failed"
  exit 1
fi

# Test 2: Create a new item
echo "Test 2: Creating a new item..."
CREATE_RESPONSE=$(curl -s -X POST http://localhost:8002/items/ \
  -H "Content-Type: application/json" \
  -d '{"id": 1, "name": "Test Item"}')

if echo "$CREATE_RESPONSE" | grep -q "Test Item"; then
  echo "✅ Item created successfully"
else
  echo "❌ Item creation failed"
  echo "Response: $CREATE_RESPONSE"
  exit 1
fi

# Test 3: Retrieve the created item
echo "Test 3: Retrieving item..."
GET_RESPONSE=$(curl -s -X GET http://localhost:8002/items/1)

if echo "$GET_RESPONSE" | grep -q "Test Item"; then
  echo "✅ Item retrieved successfully"
else
  echo "❌ Item retrieval failed"
  echo "Response: $GET_RESPONSE"
  exit 1
fi

# Test 4: Update the item
echo "Test 4: Updating item..."
UPDATE_RESPONSE=$(curl -s -X PUT http://localhost:8002/items/1 \
  -H "Content-Type: application/json" \
  -d '{"id": 1, "name": "Updated Item"}')

if echo "$UPDATE_RESPONSE" | grep -q "Updated Item"; then
  echo "✅ Item updated successfully"
else
  echo "❌ Item update failed"
  echo "Response: $UPDATE_RESPONSE"
  exit 1
fi

# Test 5: Verify the update
echo "Test 5: Verifying update..."
VERIFY_RESPONSE=$(curl -s -X GET http://localhost:8002/items/1)

if echo "$VERIFY_RESPONSE" | grep -q "Updated Item"; then
  echo "✅ Update verified successfully"
else
  echo "❌ Update verification failed"
  echo "Response: $VERIFY_RESPONSE"
  exit 1
fi

# Test 6: Test error handling - Get non-existent item
echo "Test 6: Testing error handling..."
ERROR_RESPONSE=$(curl -s -X GET http://localhost:8002/items/999 -w "%{http_code}" -o /dev/null)

if [ "$ERROR_RESPONSE" = "404" ]; then
  echo "✅ Error handling works correctly"
else
  echo "❌ Error handling failed"
  echo "Expected 404, got: $ERROR_RESPONSE"
  exit 1
fi

echo "🎉 All tests passed successfully!"
exit 0