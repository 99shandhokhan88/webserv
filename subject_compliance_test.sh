#!/bin/bash

# Subject Compliance Test for WebServ
# Tests specific requirements from the 42 webserv subject

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SERVER_HOST="localhost"
SERVER_PORT="8080"
BASE_URL="http://${SERVER_HOST}:${SERVER_PORT}"
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

test_header_present() {
    local url="$1"
    local method="$2"
    local header_name="$3"
    local test_name="$4"
    
    header_present=$(curl -s -I -X "$method" "$url" | grep -i "$header_name:" | wc -l)
    
    if [ "$header_present" -gt 0 ]; then
        print_test_result "$test_name" "PRESENT" "PRESENT" "Header $header_name is present"
    else
        print_test_result "$test_name" "PRESENT" "MISSING" "Header $header_name should be present"
    fi
}

echo -e "${BLUE}=== SUBJECT COMPLIANCE TEST SUITE ===${NC}"
echo -e "${BLUE}Testing 42 webserv subject requirements${NC}"
echo

# Check if server is running
if ! curl -s --connect-timeout 5 "$BASE_URL" > /dev/null; then
    echo -e "${RED}Server is not running! Please start the server first.${NC}"
    exit 1
fi

echo -e "${YELLOW}=== 1. MANDATORY HTTP METHODS ===${NC}"

# Subject requirement: GET, POST, DELETE methods
test_status_code "$BASE_URL/" "GET" "200" "GET_METHOD_SUPPORTED"
test_status_code "$BASE_URL/" "POST" "200" "POST_METHOD_SUPPORTED" "test=data"
test_status_code "$BASE_URL/uploads/test_delete.txt" "DELETE" "404" "DELETE_METHOD_SUPPORTED"

# Test unsupported methods return 405
test_status_code "$BASE_URL/" "PUT" "405" "PUT_METHOD_NOT_ALLOWED"
test_status_code "$BASE_URL/" "PATCH" "405" "PATCH_METHOD_NOT_ALLOWED"

echo

echo -e "${YELLOW}=== 2. HTTP STATUS CODES COMPLIANCE ===${NC}"

# Subject requirement: Proper HTTP status codes
test_status_code "$BASE_URL/" "GET" "200" "STATUS_200_OK"
test_status_code "$BASE_URL/nonexistent.html" "GET" "404" "STATUS_404_NOT_FOUND"
test_status_code "$BASE_URL/css/" "POST" "405" "STATUS_405_METHOD_NOT_ALLOWED"

# Test 400 Bad Request
BAD_REQUEST_STATUS=$(echo -e "INVALID REQUEST\r\n\r\n" | nc -w 1 "$SERVER_HOST" "$SERVER_PORT" | head -1 | grep -o "[0-9][0-9][0-9]" || echo "000")
if [ "$BAD_REQUEST_STATUS" = "400" ]; then
    print_test_result "STATUS_400_BAD_REQUEST" "400" "400" "Bad request returns 400"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "STATUS_400_BAD_REQUEST" "400" "$BAD_REQUEST_STATUS" "Bad request should return 400"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 3. HTTP HEADERS COMPLIANCE ===${NC}"

# Subject requirement: Proper HTTP headers
test_header_present "$BASE_URL/" "GET" "Content-Type" "CONTENT_TYPE_HEADER"
test_header_present "$BASE_URL/" "GET" "Content-Length" "CONTENT_LENGTH_HEADER"
test_header_present "$BASE_URL/" "GET" "Server" "SERVER_HEADER"

# Test Date header (optional but good practice)
test_header_present "$BASE_URL/" "GET" "Date" "DATE_HEADER"

echo

echo -e "${YELLOW}=== 4. CONFIGURATION FILE REQUIREMENTS ===${NC}"

# Subject requirement: Configuration file parsing
if [ -f "configs/default.conf" ]; then
    print_test_result "CONFIG_FILE_EXISTS" "EXISTS" "EXISTS" "Configuration file exists"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "CONFIG_FILE_EXISTS" "EXISTS" "MISSING" "Configuration file should exist"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

# Test multiple server blocks (if configured)
if grep -q "server {" configs/default.conf; then
    print_test_result "SERVER_BLOCKS" "PRESENT" "PRESENT" "Server blocks in config"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "SERVER_BLOCKS" "PRESENT" "MISSING" "Server blocks should be present"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 5. ERROR PAGES ===${NC}"

# Subject requirement: Custom error pages
test_status_code "$BASE_URL/nonexistent" "GET" "404" "CUSTOM_404_PAGE"

# Check if custom error page is served
ERROR_CONTENT=$(curl -s "$BASE_URL/nonexistent")
if echo "$ERROR_CONTENT" | grep -q "404\|Not Found\|Error"; then
    print_test_result "CUSTOM_ERROR_CONTENT" "CUSTOM" "CUSTOM" "Custom error page content"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "CUSTOM_ERROR_CONTENT" "CUSTOM" "DEFAULT" "Should serve custom error page"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 6. FILE UPLOAD ===${NC}"

# Subject requirement: File upload capability
UPLOAD_DATA="Test upload content for subject compliance"
test_status_code "$BASE_URL/uploads/subject_test.txt" "POST" "200" "FILE_UPLOAD_CAPABILITY" "$UPLOAD_DATA"

# Verify uploaded file can be retrieved
test_status_code "$BASE_URL/uploads/subject_test.txt" "GET" "200" "UPLOADED_FILE_RETRIEVAL"

echo

echo -e "${YELLOW}=== 7. DIRECTORY LISTING (AUTOINDEX) ===${NC}"

# Subject requirement: Directory listing
AUTOINDEX_CONTENT=$(curl -s "$BASE_URL/uploads/")
if echo "$AUTOINDEX_CONTENT" | grep -q "Index of\|Directory\|Parent"; then
    print_test_result "AUTOINDEX_FUNCTIONALITY" "WORKING" "WORKING" "Autoindex generates directory listing"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "AUTOINDEX_FUNCTIONALITY" "WORKING" "NOT_WORKING" "Autoindex should generate directory listing"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 8. CGI EXECUTION ===${NC}"

# Subject requirement: CGI execution
test_status_code "$BASE_URL/cgi-bin/" "GET" "200" "CGI_DIRECTORY_ACCESS"

# Test CGI script execution if available
if [ -f "www/cgi-bin/info.py" ]; then
    CGI_RESPONSE=$(curl -s "$BASE_URL/cgi-bin/info.py")
    if [ -n "$CGI_RESPONSE" ]; then
        print_test_result "CGI_SCRIPT_EXECUTION" "EXECUTED" "EXECUTED" "CGI script executes"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        print_test_result "CGI_SCRIPT_EXECUTION" "EXECUTED" "FAILED" "CGI script should execute"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
    TEST_COUNT=$((TEST_COUNT + 1))
fi

echo

echo -e "${YELLOW}=== 9. STATIC FILE SERVING ===${NC}"

# Subject requirement: Serve static files
test_status_code "$BASE_URL/index.html" "GET" "200" "STATIC_HTML_SERVING"
test_status_code "$BASE_URL/test.css" "GET" "200" "STATIC_CSS_SERVING"

# Test correct MIME types
CSS_CONTENT_TYPE=$(curl -s -I "$BASE_URL/test.css" | grep -i "content-type:" | head -1)
if echo "$CSS_CONTENT_TYPE" | grep -q "text/css"; then
    print_test_result "CSS_MIME_TYPE" "CORRECT" "CORRECT" "CSS files have correct MIME type"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "CSS_MIME_TYPE" "CORRECT" "INCORRECT" "CSS files should have text/css MIME type"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 10. HTTP/1.1 COMPLIANCE ===${NC}"

# Subject requirement: HTTP/1.1 compliance
HTTP_VERSION=$(curl -s -I "$BASE_URL/" | head -1)
if echo "$HTTP_VERSION" | grep -q "HTTP/1.1"; then
    print_test_result "HTTP_VERSION" "1.1" "1.1" "Server responds with HTTP/1.1"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "HTTP_VERSION" "1.1" "OTHER" "Server should respond with HTTP/1.1"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

# Test Host header requirement for HTTP/1.1
NO_HOST_STATUS=$(echo -e "GET / HTTP/1.1\r\n\r\n" | nc -w 1 "$SERVER_HOST" "$SERVER_PORT" | head -1 | grep -o "[0-9][0-9][0-9]" || echo "000")
if [ "$NO_HOST_STATUS" = "400" ]; then
    print_test_result "HOST_HEADER_REQUIRED" "400" "400" "Missing Host header rejected"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "HOST_HEADER_REQUIRED" "400" "$NO_HOST_STATUS" "HTTP/1.1 requires Host header"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 11. LOCATION BLOCKS ===${NC}"

# Subject requirement: Location blocks in configuration
if grep -q "location" configs/default.conf; then
    print_test_result "LOCATION_BLOCKS" "PRESENT" "PRESENT" "Location blocks configured"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "LOCATION_BLOCKS" "PRESENT" "MISSING" "Location blocks should be configured"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

# Test different behavior for different locations
test_status_code "$BASE_URL/css/test.css" "GET" "200" "CSS_LOCATION_WORKING"
test_status_code "$BASE_URL/uploads/" "GET" "200" "UPLOADS_LOCATION_WORKING"

echo

echo -e "${YELLOW}=== 12. METHOD RESTRICTIONS ===${NC}"

# Subject requirement: Method restrictions per location
test_status_code "$BASE_URL/css/" "POST" "405" "CSS_POST_RESTRICTED"
test_status_code "$BASE_URL/css/" "DELETE" "405" "CSS_DELETE_RESTRICTED"

echo

echo -e "${YELLOW}=== 13. DEFAULT FILES ===${NC}"

# Subject requirement: Default files (index)
test_status_code "$BASE_URL/" "GET" "200" "ROOT_DEFAULT_FILE"

# Check if index.html is served by default
ROOT_CONTENT=$(curl -s "$BASE_URL/")
INDEX_CONTENT=$(curl -s "$BASE_URL/index.html")
if [ "$ROOT_CONTENT" = "$INDEX_CONTENT" ]; then
    print_test_result "DEFAULT_INDEX" "SAME" "SAME" "Root serves index.html by default"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "DEFAULT_INDEX" "SAME" "DIFFERENT" "Root should serve index.html by default"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 14. NON-BLOCKING I/O ===${NC}"

# Subject requirement: Non-blocking I/O (test with concurrent requests)
echo "Testing concurrent requests for non-blocking I/O..."
CONCURRENT_PIDS=()
for i in {1..10}; do
    curl -s "$BASE_URL/" > /tmp/concurrent_subject_$i.txt &
    CONCURRENT_PIDS+=($!)
done

# Wait for all requests to complete
for pid in "${CONCURRENT_PIDS[@]}"; do
    wait $pid
done

# Check if all requests succeeded
CONCURRENT_SUCCESS=0
for i in {1..10}; do
    if [ -f "/tmp/concurrent_subject_$i.txt" ] && [ -s "/tmp/concurrent_subject_$i.txt" ]; then
        CONCURRENT_SUCCESS=$((CONCURRENT_SUCCESS + 1))
    fi
done

if [ "$CONCURRENT_SUCCESS" -eq 10 ]; then
    print_test_result "NON_BLOCKING_IO" "10" "10" "All concurrent requests handled"
    PASS_COUNT=$((PASS_COUNT + 1))
else
    print_test_result "NON_BLOCKING_IO" "10" "$CONCURRENT_SUCCESS" "Non-blocking I/O may have issues"
    FAIL_COUNT=$((FAIL_COUNT + 1))
fi
TEST_COUNT=$((TEST_COUNT + 1))

echo

echo -e "${YELLOW}=== 15. CLEANUP ===${NC}"

# Clean up test files
rm -f /tmp/concurrent_subject_*.txt
rm -f www/uploads/subject_test.txt 2>/dev/null || true

echo -e "${GREEN}Cleanup completed${NC}"

echo

echo -e "${BLUE}=== SUBJECT COMPLIANCE TEST SUMMARY ===${NC}"
echo -e "Total tests: ${BLUE}$TEST_COUNT${NC}"
echo -e "Passed: ${GREEN}$PASS_COUNT${NC}"
echo -e "Failed: ${RED}$FAIL_COUNT${NC}"

COMPLIANCE_PERCENTAGE=$((PASS_COUNT * 100 / TEST_COUNT))
echo -e "Compliance: ${BLUE}${COMPLIANCE_PERCENTAGE}%${NC}"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}🎉 FULL SUBJECT COMPLIANCE! Your webserver meets all requirements.${NC}"
    exit 0
elif [ $COMPLIANCE_PERCENTAGE -ge 80 ]; then
    echo -e "${YELLOW}⚠️  Good compliance but some requirements need attention.${NC}"
    exit 1
else
    echo -e "${RED}❌ Poor compliance. Major requirements are missing.${NC}"
    exit 1
fi 