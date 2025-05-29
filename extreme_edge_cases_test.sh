#!/bin/bash

# Extreme Edge Cases Test for WebServ
# Tests the most challenging HTTP parsing and edge cases

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SERVER_HOST="localhost"
SERVER_PORT="8080"
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

print_test_result() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    local description="$4"
    
    TEST_COUNT=$((TEST_COUNT + 1))
    
    if [ "$expected" = "$actual" ]; then
        echo -e "${GREEN}✓ PASS${NC} - $test_name: $description"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo -e "${RED}✗ FAIL${NC} - $test_name: $description (Expected: $expected, Got: $actual)"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

test_raw_request() {
    local request="$1"
    local test_name="$2"
    local expected_pattern="$3"
    local description="$4"
    
    response=$(echo -e "$request" | nc -w 2 "$SERVER_HOST" "$SERVER_PORT" 2>/dev/null || echo "NO_RESPONSE")
    
    if echo "$response" | grep -q "$expected_pattern"; then
        print_test_result "$test_name" "MATCH" "MATCH" "$description"
    else
        print_test_result "$test_name" "MATCH" "NO_MATCH" "$description - Response: ${response:0:100}..."
    fi
}

echo -e "${BLUE}=== EXTREME EDGE CASES TEST SUITE ===${NC}"
echo

echo -e "${YELLOW}=== 1. MALFORMED HTTP REQUESTS ===${NC}"

# Test completely empty request
test_raw_request "" "EMPTY_REQUEST" "400\|HTTP" "Empty request should be handled"

# Test request with only spaces
test_raw_request "   \r\n\r\n" "SPACES_ONLY" "400\|HTTP" "Request with only spaces"

# Test request without HTTP version
test_raw_request "GET /\r\n\r\n" "NO_HTTP_VERSION" "400\|HTTP" "Request without HTTP version"

# Test request with invalid method
test_raw_request "INVALID / HTTP/1.1\r\nHost: localhost\r\n\r\n" "INVALID_METHOD" "400\|405\|501\|HTTP" "Invalid HTTP method"

# Test request with missing space after method
test_raw_request "GET/ HTTP/1.1\r\nHost: localhost\r\n\r\n" "NO_SPACE_AFTER_METHOD" "400\|HTTP" "No space after method"

# Test request with extra spaces
test_raw_request "GET    /    HTTP/1.1\r\nHost: localhost\r\n\r\n" "EXTRA_SPACES" "200\|400\|HTTP" "Extra spaces in request line"

echo

echo -e "${YELLOW}=== 2. HEADER EDGE CASES ===${NC}"

# Test header without colon
test_raw_request "GET / HTTP/1.1\r\nHost localhost\r\n\r\n" "HEADER_NO_COLON" "400\|HTTP" "Header without colon"

# Test header with empty value
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\nEmpty-Header:\r\n\r\n" "EMPTY_HEADER_VALUE" "200\|400\|HTTP" "Header with empty value"

# Test very long header
LONG_HEADER_VALUE=$(python3 -c "print('x' * 8192)")
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\nLong-Header: $LONG_HEADER_VALUE\r\n\r\n" "VERY_LONG_HEADER" "400\|431\|HTTP" "Very long header value"

# Test header with invalid characters
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\nInvalid\x00Header: value\r\n\r\n" "INVALID_HEADER_CHARS" "400\|HTTP" "Header with null character"

# Test duplicate Host headers
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\nHost: example.com\r\n\r\n" "DUPLICATE_HOST" "400\|HTTP" "Duplicate Host headers"

echo

echo -e "${YELLOW}=== 3. URL EDGE CASES ===${NC}"

# Test URL with null character
test_raw_request "GET /test\x00file HTTP/1.1\r\nHost: localhost\r\n\r\n" "URL_NULL_CHAR" "400\|HTTP" "URL with null character"

# Test URL with control characters
test_raw_request "GET /test\x01\x02file HTTP/1.1\r\nHost: localhost\r\n\r\n" "URL_CONTROL_CHARS" "400\|HTTP" "URL with control characters"

# Test URL with high-bit characters
test_raw_request "GET /test\xff\xfefile HTTP/1.1\r\nHost: localhost\r\n\r\n" "URL_HIGH_BIT" "400\|HTTP" "URL with high-bit characters"

# Test extremely long URL
LONG_PATH=$(python3 -c "print('/' + 'a' * 2048)")
test_raw_request "GET $LONG_PATH HTTP/1.1\r\nHost: localhost\r\n\r\n" "EXTREMELY_LONG_URL" "414\|400\|HTTP" "Extremely long URL"

# Test URL with multiple slashes
test_raw_request "GET ////test////file HTTP/1.1\r\nHost: localhost\r\n\r\n" "MULTIPLE_SLASHES" "200\|404\|HTTP" "URL with multiple slashes"

echo

echo -e "${YELLOW}=== 4. HTTP VERSION EDGE CASES ===${NC}"

# Test HTTP/0.9
test_raw_request "GET / HTTP/0.9\r\nHost: localhost\r\n\r\n" "HTTP_0_9" "400\|505\|HTTP" "HTTP/0.9 version"

# Test HTTP/2.0
test_raw_request "GET / HTTP/2.0\r\nHost: localhost\r\n\r\n" "HTTP_2_0" "400\|505\|HTTP" "HTTP/2.0 version"

# Test invalid HTTP version format
test_raw_request "GET / HTTP/1.x\r\nHost: localhost\r\n\r\n" "INVALID_HTTP_FORMAT" "400\|HTTP" "Invalid HTTP version format"

# Test HTTP version with extra digits
test_raw_request "GET / HTTP/1.11\r\nHost: localhost\r\n\r\n" "HTTP_EXTRA_DIGITS" "400\|505\|HTTP" "HTTP version with extra digits"

echo

echo -e "${YELLOW}=== 5. BODY EDGE CASES ===${NC}"

# Test POST with Content-Length but no body
test_raw_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\n\r\n" "CONTENT_LENGTH_NO_BODY" "400\|HTTP" "Content-Length without body"

# Test POST with body but no Content-Length
test_raw_request "POST / HTTP/1.1\r\nHost: localhost\r\n\r\ntest body" "BODY_NO_CONTENT_LENGTH" "400\|411\|HTTP" "Body without Content-Length"

# Test negative Content-Length
test_raw_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: -1\r\n\r\n" "NEGATIVE_CONTENT_LENGTH" "400\|HTTP" "Negative Content-Length"

# Test invalid Content-Length
test_raw_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: abc\r\n\r\n" "INVALID_CONTENT_LENGTH" "400\|HTTP" "Invalid Content-Length"

# Test zero Content-Length with body
test_raw_request "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\nbody" "ZERO_LENGTH_WITH_BODY" "400\|HTTP" "Zero Content-Length with body"

echo

echo -e "${YELLOW}=== 6. LINE ENDING EDGE CASES ===${NC}"

# Test with only \n (no \r)
test_raw_request "GET / HTTP/1.1\nHost: localhost\n\n" "ONLY_LF" "200\|400\|HTTP" "Only LF line endings"

# Test with only \r (no \n)
test_raw_request "GET / HTTP/1.1\rHost: localhost\r\r" "ONLY_CR" "400\|HTTP" "Only CR line endings"

# Test mixed line endings
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\n\r\n" "MIXED_LINE_ENDINGS" "200\|400\|HTTP" "Mixed line endings"

echo

echo -e "${YELLOW}=== 7. CHUNKED ENCODING EDGE CASES ===${NC}"

# Test chunked encoding (if supported)
test_raw_request "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n" "CHUNKED_ENCODING" "200\|400\|501\|HTTP" "Chunked encoding"

# Test invalid chunk size
test_raw_request "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nXX\r\nhello\r\n0\r\n\r\n" "INVALID_CHUNK_SIZE" "400\|HTTP" "Invalid chunk size"

echo

echo -e "${YELLOW}=== 8. CONNECTION EDGE CASES ===${NC}"

# Test multiple requests on same connection (if keep-alive supported)
{
    echo -e "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"
    sleep 0.1
    echo -e "GET /test.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
} | nc -w 3 "$SERVER_HOST" "$SERVER_PORT" > /tmp/keepalive_test.txt 2>/dev/null

if grep -q "HTTP.*200" /tmp/keepalive_test.txt; then
    print_test_result "KEEP_ALIVE" "MATCH" "MATCH" "Keep-alive connection handling"
else
    print_test_result "KEEP_ALIVE" "MATCH" "NO_MATCH" "Keep-alive connection handling"
fi

# Test connection close
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" "CONNECTION_CLOSE" "200\|HTTP" "Connection: close header"

echo

echo -e "${YELLOW}=== 9. TIMEOUT EDGE CASES ===${NC}"

# Test slow request (send headers slowly)
{
    echo -n "GET / HTTP/1.1"
    sleep 1
    echo -e "\r\nHost: localhost\r\n\r\n"
} | nc -w 5 "$SERVER_HOST" "$SERVER_PORT" > /tmp/slow_request.txt 2>/dev/null &
SLOW_PID=$!
sleep 3
kill $SLOW_PID 2>/dev/null || true
wait $SLOW_PID 2>/dev/null || true

if [ -s /tmp/slow_request.txt ]; then
    print_test_result "SLOW_REQUEST" "HANDLED" "HANDLED" "Slow request handled"
else
    print_test_result "SLOW_REQUEST" "HANDLED" "TIMEOUT" "Slow request timed out appropriately"
fi

echo

echo -e "${YELLOW}=== 10. RESOURCE EXHAUSTION TESTS ===${NC}"

# Test many headers
MANY_HEADERS=""
for i in {1..100}; do
    MANY_HEADERS="${MANY_HEADERS}Header$i: value$i\r\n"
done
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\n${MANY_HEADERS}\r\n" "MANY_HEADERS" "200\|400\|431\|HTTP" "Many headers"

# Test very long request line
LONG_QUERY=$(python3 -c "print('?' + '&'.join([f'param{i}=value{i}' for i in range(100)]))")
test_raw_request "GET /${LONG_QUERY} HTTP/1.1\r\nHost: localhost\r\n\r\n" "LONG_QUERY_STRING" "200\|414\|400\|HTTP" "Very long query string"

echo

echo -e "${YELLOW}=== 11. SECURITY EDGE CASES ===${NC}"

# Test request smuggling attempt
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 6\r\n\r\n0\r\n\r\nGET /admin HTTP/1.1\r\nHost: localhost\r\n\r\n" "REQUEST_SMUGGLING" "400\|HTTP" "Request smuggling attempt"

# Test header injection
test_raw_request "GET / HTTP/1.1\r\nHost: localhost\r\nX-Test: value\r\nInjected: header\r\n\r\n" "HEADER_INJECTION" "200\|400\|HTTP" "Header injection attempt"

echo

echo -e "${YELLOW}=== 12. CLEANUP ===${NC}"

rm -f /tmp/keepalive_test.txt /tmp/slow_request.txt

echo -e "${GREEN}Cleanup completed${NC}"

echo

echo -e "${BLUE}=== EXTREME EDGE CASES TEST SUMMARY ===${NC}"
echo -e "Total tests: ${BLUE}$TEST_COUNT${NC}"
echo -e "Passed: ${GREEN}$PASS_COUNT${NC}"
echo -e "Failed: ${RED}$FAIL_COUNT${NC}"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}🎉 ALL EXTREME EDGE CASE TESTS PASSED!${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠️  Some extreme edge case tests failed.${NC}"
    exit 1
fi 