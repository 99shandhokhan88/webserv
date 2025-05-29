#!/bin/bash

# Comprehensive WebServ Test Suite
# Tests all edge cases, HTTP compliance, and project requirements

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test configuration
SERVER_HOST="localhost"
SERVER_PORT="8080"
BASE_URL="http://${SERVER_HOST}:${SERVER_PORT}"
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

# Function to print test results
print_test_result() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    local description="$4"
    
    TEST_COUNT=$((TEST_COUNT + 1))
    
    if [ "$expected" = "$actual" ]; then
        echo -e "${GREEN}✓ PASS${NC} - $test_name: $description (Expected: $expected, Got: $actual)"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo -e "${RED}✗ FAIL${NC} - $test_name: $description (Expected: $expected, Got: $actual)"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

# Function to check if server is running
check_server() {
    echo -e "${BLUE}Checking if server is running on ${SERVER_HOST}:${SERVER_PORT}...${NC}"
    if ! curl -s --connect-timeout 5 "$BASE_URL" > /dev/null; then
        echo -e "${RED}Server is not running! Please start the server first.${NC}"
        exit 1
    fi
    echo -e "${GREEN}Server is running!${NC}"
    echo
}

# Function to test HTTP status codes
test_status_code() {
    local url="$1"
    local method="$2"
    local expected_code="$3"
    local test_name="$4"
    local data="$5"
    
    if [ "$method" = "POST" ] && [ -n "$data" ]; then
        actual_code=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" -d "$data" "$url")
    else
        actual_code=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" "$url")
    fi
    
    print_test_result "$test_name" "$expected_code" "$actual_code" "HTTP $method to $url"
}

# Function to test response headers
test_header() {
    local url="$1"
    local method="$2"
    local header_name="$3"
    local expected_pattern="$4"
    local test_name="$5"
    
    actual_header=$(curl -s -I -X "$method" "$url" | grep -i "$header_name:" | head -1)
    
    if echo "$actual_header" | grep -q "$expected_pattern"; then
        print_test_result "$test_name" "MATCH" "MATCH" "Header $header_name contains $expected_pattern"
    else
        print_test_result "$test_name" "MATCH" "NO_MATCH" "Header $header_name should contain $expected_pattern, got: $actual_header"
    fi
}

# Function to test content
test_content() {
    local url="$1"
    local method="$2"
    local expected_pattern="$3"
    local test_name="$4"
    
    actual_content=$(curl -s -X "$method" "$url")
    
    if echo "$actual_content" | grep -q "$expected_pattern"; then
        print_test_result "$test_name" "MATCH" "MATCH" "Content contains expected pattern"
    else
        print_test_result "$test_name" "MATCH" "NO_MATCH" "Content should contain $expected_pattern"
    fi
}

echo -e "${BLUE}=== COMPREHENSIVE WEBSERV TEST SUITE ===${NC}"
echo -e "${BLUE}Testing HTTP compliance and project requirements${NC}"
echo

check_server

echo -e "${YELLOW}=== 1. BASIC HTTP METHODS TESTS ===${NC}"

# GET method tests
test_status_code "$BASE_URL/" "GET" "200" "GET_ROOT"
test_status_code "$BASE_URL/index.html" "GET" "200" "GET_INDEX"
test_status_code "$BASE_URL/test.html" "GET" "200" "GET_EXISTING_FILE"
test_status_code "$BASE_URL/nonexistent.html" "GET" "404" "GET_NONEXISTENT"

# POST method tests
test_status_code "$BASE_URL/" "POST" "200" "POST_ROOT" "test=data"
test_status_code "$BASE_URL/uploads/" "POST" "200" "POST_UPLOAD_DIR" "test=upload"

# DELETE method tests
echo "test file for deletion" > /tmp/test_delete.txt
cp /tmp/test_delete.txt www/uploads/test_delete.txt 2>/dev/null || true
test_status_code "$BASE_URL/uploads/test_delete.txt" "DELETE" "200" "DELETE_EXISTING_FILE"
test_status_code "$BASE_URL/uploads/nonexistent.txt" "DELETE" "404" "DELETE_NONEXISTENT"

echo

echo -e "${YELLOW}=== 2. HTTP HEADER COMPLIANCE TESTS ===${NC}"

# Test required HTTP headers
test_header "$BASE_URL/" "GET" "Content-Type" "text/html" "CONTENT_TYPE_HTML"
test_header "$BASE_URL/test.css" "GET" "Content-Type" "text/css" "CONTENT_TYPE_CSS"
test_header "$BASE_URL/" "GET" "Content-Length" "[0-9]" "CONTENT_LENGTH_PRESENT"
test_header "$BASE_URL/" "GET" "Server" "webserv" "SERVER_HEADER"

echo

echo -e "${YELLOW}=== 3. METHOD NOT ALLOWED TESTS ===${NC}"

# Test methods not allowed in specific locations
test_status_code "$BASE_URL/css/" "POST" "405" "POST_CSS_NOT_ALLOWED"
test_status_code "$BASE_URL/css/" "DELETE" "405" "DELETE_CSS_NOT_ALLOWED"
test_status_code "$BASE_URL/images/" "DELETE" "405" "DELETE_IMAGES_NOT_ALLOWED"

echo

echo -e "${YELLOW}=== 4. AUTOINDEX TESTS ===${NC}"

# Test autoindex functionality
test_content "$BASE_URL/uploads/" "GET" "Index of" "AUTOINDEX_TITLE"
test_content "$BASE_URL/uploads/" "GET" "Parent Directory" "AUTOINDEX_PARENT"

echo

echo -e "${YELLOW}=== 5. ERROR PAGE TESTS ===${NC}"

# Test custom error pages
test_status_code "$BASE_URL/nonexistent/path/deep" "GET" "404" "DEEP_404"
test_content "$BASE_URL/nonexistent/path/deep" "GET" "404" "CUSTOM_404_PAGE"

echo

echo -e "${YELLOW}=== 6. CGI TESTS ===${NC}"

# Test CGI execution
test_status_code "$BASE_URL/cgi-bin/" "GET" "200" "CGI_DIRECTORY"
if [ -f "www/cgi-bin/info.py" ]; then
    test_status_code "$BASE_URL/cgi-bin/info.py" "GET" "200" "CGI_PYTHON_SCRIPT"
fi

echo

echo -e "${YELLOW}=== 7. UPLOAD TESTS ===${NC}"

# Test file upload
UPLOAD_DATA="This is test upload data"
UPLOAD_RESPONSE=$(curl -s -X POST -d "$UPLOAD_DATA" "$BASE_URL/uploads/test_upload.txt")
test_status_code "$BASE_URL/uploads/test_upload.txt" "POST" "200" "FILE_UPLOAD" "$UPLOAD_DATA"

# Verify uploaded file exists
test_status_code "$BASE_URL/uploads/test_upload.txt" "GET" "200" "UPLOADED_FILE_EXISTS"

echo

echo -e "${YELLOW}=== 8. LARGE REQUEST TESTS ===${NC}"

# Test large request body
LARGE_DATA=$(python3 -c "print('x' * 10000)")
test_status_code "$BASE_URL/uploads/large_test.txt" "POST" "200" "LARGE_UPLOAD" "$LARGE_DATA"

# Test very large request (should be rejected)
VERY_LARGE_DATA=$(python3 -c "print('x' * 1000000)")
LARGE_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$VERY_LARGE_DATA" "$BASE_URL/uploads/very_large.txt")
if [ "$LARGE_STATUS" = "413" ] || [ "$LARGE_STATUS" = "400" ]; then
    print_test_result "VERY_LARGE_UPLOAD" "413_OR_400" "413_OR_400" "Very large upload rejected"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "VERY_LARGE_UPLOAD" "413_OR_400" "$LARGE_STATUS" "Very large upload should be rejected"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 9. MALFORMED REQUEST TESTS ===${NC}"

# Test malformed HTTP requests
echo -e "GET /\r\n\r\n" | nc -w 1 "$SERVER_HOST" "$SERVER_PORT" > /tmp/malformed_response.txt 2>/dev/null || true
if grep -q "400\|HTTP" /tmp/malformed_response.txt; then
    print_test_result "MALFORMED_REQUEST" "400_OR_RESPONSE" "400_OR_RESPONSE" "Malformed request handled"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "MALFORMED_REQUEST" "400_OR_RESPONSE" "NO_RESPONSE" "Malformed request should be handled"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

# Test invalid HTTP version
INVALID_VERSION_STATUS=$(echo -e "GET / HTTP/2.0\r\n\r\n" | nc -w 1 "$SERVER_HOST" "$SERVER_PORT" | head -1 | grep -o "[0-9][0-9][0-9]" || echo "000")
if [ "$INVALID_VERSION_STATUS" = "400" ] || [ "$INVALID_VERSION_STATUS" = "505" ]; then
    print_test_result "INVALID_HTTP_VERSION" "400_OR_505" "400_OR_505" "Invalid HTTP version handled"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "INVALID_HTTP_VERSION" "400_OR_505" "$INVALID_VERSION_STATUS" "Invalid HTTP version should be rejected"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 10. CONCURRENT REQUEST TESTS ===${NC}"

# Test multiple concurrent requests
echo "Testing concurrent requests..."
for i in {1..5}; do
    curl -s "$BASE_URL/" > /tmp/concurrent_$i.txt &
done
wait

# Check if all concurrent requests succeeded
CONCURRENT_SUCCESS=0
for i in {1..5}; do
    if [ -f "/tmp/concurrent_$i.txt" ] && [ -s "/tmp/concurrent_$i.txt" ]; then
        CONCURRENT_SUCCESS=$((CONCURRENT_SUCCESS + 1))
    fi
done

if [ "$CONCURRENT_SUCCESS" -eq 5 ]; then
    print_test_result "CONCURRENT_REQUESTS" "5" "5" "All concurrent requests succeeded"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "CONCURRENT_REQUESTS" "5" "$CONCURRENT_SUCCESS" "Some concurrent requests failed"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 11. EDGE CASE TESTS ===${NC}"

# Test empty request
test_status_code "$BASE_URL/" "GET" "200" "EMPTY_PATH"

# Test path with special characters
test_status_code "$BASE_URL/test%20file" "GET" "404" "URL_ENCODED_SPACE"

# Test very long URL
LONG_URL="$BASE_URL/$(python3 -c "print('a' * 1000)")"
LONG_URL_STATUS=$(curl -s -o /dev/null -w "%{http_code}" "$LONG_URL")
if [ "$LONG_URL_STATUS" = "414" ] || [ "$LONG_URL_STATUS" = "404" ]; then
    print_test_result "LONG_URL" "414_OR_404" "414_OR_404" "Long URL handled appropriately"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "LONG_URL" "414_OR_404" "$LONG_URL_STATUS" "Long URL should be handled"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

# Test directory traversal attempt
test_status_code "$BASE_URL/../../../etc/passwd" "GET" "404" "DIRECTORY_TRAVERSAL"

echo

echo -e "${YELLOW}=== 12. HTTP/1.1 COMPLIANCE TESTS ===${NC}"

# Test Host header requirement
NO_HOST_RESPONSE=$(echo -e "GET / HTTP/1.1\r\n\r\n" | nc -w 1 "$SERVER_HOST" "$SERVER_PORT" | head -1 | grep -o "[0-9][0-9][0-9]" || echo "000")
if [ "$NO_HOST_RESPONSE" = "400" ]; then
    print_test_result "NO_HOST_HEADER" "400" "400" "Missing Host header rejected"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "NO_HOST_HEADER" "400" "$NO_HOST_RESPONSE" "Missing Host header should be rejected"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

# Test Connection: close
test_header "$BASE_URL/" "GET" "Connection" "close\|keep-alive" "CONNECTION_HEADER"

echo

echo -e "${YELLOW}=== 13. STRESS TESTS ===${NC}"

# Test rapid requests
echo "Testing rapid requests..."
RAPID_SUCCESS=0
for i in {1..20}; do
    if curl -s --max-time 1 "$BASE_URL/" > /dev/null; then
        RAPID_SUCCESS=$((RAPID_SUCCESS + 1))
    fi
done

if [ "$RAPID_SUCCESS" -ge 18 ]; then
    print_test_result "RAPID_REQUESTS" "18+" "$RAPID_SUCCESS" "Most rapid requests succeeded"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "RAPID_REQUESTS" "18+" "$RAPID_SUCCESS" "Too many rapid requests failed"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 14. SECURITY TESTS ===${NC}"

# Test for common vulnerabilities
test_status_code "$BASE_URL/<script>alert('xss')</script>" "GET" "404" "XSS_ATTEMPT"
test_status_code "$BASE_URL/'; DROP TABLE users; --" "GET" "404" "SQL_INJECTION_ATTEMPT"

echo

echo -e "${YELLOW}=== 15. CLEANUP ===${NC}"

# Clean up test files
rm -f /tmp/test_delete.txt
rm -f /tmp/malformed_response.txt
rm -f /tmp/concurrent_*.txt
rm -f www/uploads/test_upload.txt 2>/dev/null || true
rm -f www/uploads/large_test.txt 2>/dev/null || true
rm -f www/uploads/very_large.txt 2>/dev/null || true

echo -e "${GREEN}Cleanup completed${NC}"

echo

echo -e "${BLUE}=== TEST SUMMARY ===${NC}"
echo -e "Total tests: ${BLUE}$TEST_COUNT${NC}"
echo -e "Passed: ${GREEN}$PASS_COUNT${NC}"
echo -e "Failed: ${RED}$FAIL_COUNT${NC}"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}🎉 ALL TESTS PASSED! Your webserver is HTTP compliant and meets project requirements.${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠️  Some tests failed. Please review the failures above.${NC}"
    exit 1
fi 