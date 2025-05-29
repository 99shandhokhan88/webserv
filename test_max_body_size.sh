#!/bin/bash

# Test script for max body size validation
# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SERVER_HOST="localhost"
SERVER_PORT="8080"
BASE_URL="http://$SERVER_HOST:$SERVER_PORT"

echo -e "${YELLOW}=== MAX BODY SIZE VALIDATION TEST ===${NC}"
echo "Testing client_max_body_size configuration (1MB limit)"
echo

# Test 1: Small request (should pass)
echo -e "${YELLOW}Test 1: Small request (1KB) - Should PASS${NC}"
SMALL_DATA=$(python3 -c "print('x' * 1024)")  # 1KB
SMALL_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$SMALL_DATA" "$BASE_URL/uploads/small_test.txt")
if [ "$SMALL_STATUS" = "200" ]; then
    echo -e "${GREEN}✓ PASS: Small request accepted (Status: $SMALL_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Small request rejected (Status: $SMALL_STATUS)${NC}"
fi
echo

# Test 2: Medium request (should pass)
echo -e "${YELLOW}Test 2: Medium request (500KB) - Should PASS${NC}"
MEDIUM_DATA=$(python3 -c "print('x' * 512000)")  # 500KB
MEDIUM_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$MEDIUM_DATA" "$BASE_URL/uploads/medium_test.txt")
if [ "$MEDIUM_STATUS" = "200" ]; then
    echo -e "${GREEN}✓ PASS: Medium request accepted (Status: $MEDIUM_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Medium request rejected (Status: $MEDIUM_STATUS)${NC}"
fi
echo

# Test 3: Large request at limit (should pass)
echo -e "${YELLOW}Test 3: Large request at limit (1MB) - Should PASS${NC}"
LIMIT_DATA=$(python3 -c "print('x' * 1048576)")  # Exactly 1MB
LIMIT_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$LIMIT_DATA" "$BASE_URL/uploads/limit_test.txt")
if [ "$LIMIT_STATUS" = "200" ]; then
    echo -e "${GREEN}✓ PASS: Request at limit accepted (Status: $LIMIT_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Request at limit rejected (Status: $LIMIT_STATUS)${NC}"
fi
echo

# Test 4: Request over limit (should fail with 413)
echo -e "${YELLOW}Test 4: Request over limit (2MB) - Should FAIL with 413${NC}"
OVER_LIMIT_DATA=$(python3 -c "print('x' * 2097152)")  # 2MB
OVER_LIMIT_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$OVER_LIMIT_DATA" "$BASE_URL/uploads/overlimit_test.txt")
if [ "$OVER_LIMIT_STATUS" = "413" ]; then
    echo -e "${GREEN}✓ PASS: Over-limit request rejected with 413 (Status: $OVER_LIMIT_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Over-limit request not properly rejected (Status: $OVER_LIMIT_STATUS, Expected: 413)${NC}"
fi
echo

# Test 5: Very large request (should fail with 413)
echo -e "${YELLOW}Test 5: Very large request (10MB) - Should FAIL with 413${NC}"
VERY_LARGE_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$(python3 -c "print('x' * 10485760)")" "$BASE_URL/uploads/verylarge_test.txt")
if [ "$VERY_LARGE_STATUS" = "413" ]; then
    echo -e "${GREEN}✓ PASS: Very large request rejected with 413 (Status: $VERY_LARGE_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Very large request not properly rejected (Status: $VERY_LARGE_STATUS, Expected: 413)${NC}"
fi
echo

# Test 6: Test with Content-Length header explicitly
echo -e "${YELLOW}Test 6: Test with explicit Content-Length header (2MB) - Should FAIL with 413${NC}"
EXPLICIT_STATUS=$(echo -e "POST /uploads/explicit_test.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 2097152\r\n\r\n$(python3 -c "print('x' * 2097152)")" | nc -w 5 "$SERVER_HOST" "$SERVER_PORT" | head -1 | grep -o "[0-9][0-9][0-9]" || echo "000")
if [ "$EXPLICIT_STATUS" = "413" ]; then
    echo -e "${GREEN}✓ PASS: Request with explicit Content-Length rejected with 413 (Status: $EXPLICIT_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Request with explicit Content-Length not properly rejected (Status: $EXPLICIT_STATUS, Expected: 413)${NC}"
fi
echo

echo -e "${YELLOW}=== TEST SUMMARY ===${NC}"
echo "Max body size validation test completed."
echo "Expected behavior:"
echo "- Requests ≤ 1MB should return 200 (accepted)"
echo "- Requests > 1MB should return 413 (Request Entity Too Large)"
echo
echo "If tests are failing, check:"
echo "1. Server configuration has 'client_max_body_size 1M'"
echo "2. Server code validates body size against config"
echo "3. Server returns proper 413 status code for oversized requests" 