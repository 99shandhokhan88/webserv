#!/bin/bash

# Simple test script for max body size validation using files
# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SERVER_HOST="localhost"
SERVER_PORT="8080"
BASE_URL="http://$SERVER_HOST:$SERVER_PORT"

echo -e "${YELLOW}=== SIMPLE MAX BODY SIZE TEST ===${NC}"
echo "Testing client_max_body_size configuration (1MB = 1048576 bytes limit)"
echo

# Create test files
echo "Creating test files..."

# Test 1: Small file (1KB)
dd if=/dev/zero of=test_1kb.txt bs=1024 count=1 2>/dev/null
echo -e "${YELLOW}Test 1: Small request (1KB) - Should PASS${NC}"
SMALL_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary @test_1kb.txt "$BASE_URL/uploads/small_test.txt")
if [ "$SMALL_STATUS" = "200" ]; then
    echo -e "${GREEN}✓ PASS: Small request accepted (Status: $SMALL_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Small request rejected (Status: $SMALL_STATUS)${NC}"
fi
echo

# Test 2: Medium file (500KB)
dd if=/dev/zero of=test_500kb.txt bs=1024 count=500 2>/dev/null
echo -e "${YELLOW}Test 2: Medium request (500KB) - Should PASS${NC}"
MEDIUM_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary @test_500kb.txt "$BASE_URL/uploads/medium_test.txt")
if [ "$MEDIUM_STATUS" = "200" ]; then
    echo -e "${GREEN}✓ PASS: Medium request accepted (Status: $MEDIUM_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Medium request rejected (Status: $MEDIUM_STATUS)${NC}"
fi
echo

# Test 3: Large file at limit (1MB)
dd if=/dev/zero of=test_1mb.txt bs=1024 count=1024 2>/dev/null
echo -e "${YELLOW}Test 3: Large request at limit (1MB) - Should PASS${NC}"
LIMIT_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary @test_1mb.txt "$BASE_URL/uploads/limit_test.txt")
if [ "$LIMIT_STATUS" = "200" ]; then
    echo -e "${GREEN}✓ PASS: Request at limit accepted (Status: $LIMIT_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Request at limit rejected (Status: $LIMIT_STATUS)${NC}"
fi
echo

# Test 4: File over limit (2MB)
dd if=/dev/zero of=test_2mb.txt bs=1024 count=2048 2>/dev/null
echo -e "${YELLOW}Test 4: Request over limit (2MB) - Should FAIL with 413${NC}"
OVER_LIMIT_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary @test_2mb.txt "$BASE_URL/uploads/overlimit_test.txt")
if [ "$OVER_LIMIT_STATUS" = "413" ]; then
    echo -e "${GREEN}✓ PASS: Over-limit request rejected with 413 (Status: $OVER_LIMIT_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Over-limit request not properly rejected (Status: $OVER_LIMIT_STATUS, Expected: 413)${NC}"
fi
echo

# Test 5: Very small request to verify basic functionality
echo "test data" > test_tiny.txt
echo -e "${YELLOW}Test 5: Tiny request (9 bytes) - Should PASS${NC}"
TINY_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary @test_tiny.txt "$BASE_URL/uploads/tiny_test.txt")
if [ "$TINY_STATUS" = "200" ]; then
    echo -e "${GREEN}✓ PASS: Tiny request accepted (Status: $TINY_STATUS)${NC}"
else
    echo -e "${RED}✗ FAIL: Tiny request rejected (Status: $TINY_STATUS)${NC}"
fi
echo

# Cleanup test files
echo "Cleaning up test files..."
rm -f test_1kb.txt test_500kb.txt test_1mb.txt test_2mb.txt test_tiny.txt

echo -e "${YELLOW}=== TEST SUMMARY ===${NC}"
echo "Max body size validation test completed."
echo "Configuration: client_max_body_size = 1048576 bytes (1MB)"
echo "Expected behavior:"
echo "- Requests ≤ 1MB should return 200 (accepted)"
echo "- Requests > 1MB should return 413 (Request Entity Too Large)" 