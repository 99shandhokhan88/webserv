#!/bin/bash

# HTTP Compliance Test Script for webserv
# Tests all HTTP methods, status codes, and edge cases

COLOR_GREEN='\033[0;32m'
COLOR_RED='\033[0;31m'
COLOR_YELLOW='\033[1;33m'
COLOR_BLUE='\033[0;34m'
COLOR_NC='\033[0m' # No Color

SERVER_URL="http://localhost:8080"
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

# Function to print test results
print_test() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    TEST_COUNT=$((TEST_COUNT + 1))
    
    echo -e "${COLOR_BLUE}[$TEST_COUNT]${COLOR_NC} Testing: $test_name"
    if [[ "$actual" == *"$expected"* ]]; then
        echo -e "  ${COLOR_GREEN}✓ PASS${COLOR_NC} - Expected: $expected, Got: $actual"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo -e "  ${COLOR_RED}✗ FAIL${COLOR_NC} - Expected: $expected, Got: $actual"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
    echo
}

# Function to get HTTP status from response
get_status() {
    echo "$1" | head -n1 | cut -d' ' -f2
}

# Function to check if server is running
check_server() {
    if ! curl -s --connect-timeout 2 "$SERVER_URL" > /dev/null 2>&1; then
        echo -e "${COLOR_RED}ERROR: Server not running on $SERVER_URL${COLOR_NC}"
        echo "Please start the server first: ./webserv configs/default.conf"
        exit 1
    fi
}

echo -e "${COLOR_YELLOW}🚀 HTTP Compliance Test Suite for webserv${COLOR_NC}"
echo "=================================================="

# Check if server is running
check_server

# Prepare test files
mkdir -p www/uploads www/test_dir
echo "Test file content" > www/test.txt
echo "Test upload content" > www/uploads/test_upload.txt

echo -e "${COLOR_YELLOW}📋 1. HTTP METHOD VALIDATION TESTS${COLOR_NC}"
echo "=================================================="

# 1.1 Implemented methods (should work)
response=$(curl -s -i "$SERVER_URL/test.txt" 2>/dev/null)
print_test "GET valid file" "200" "$(get_status "$response")"

response=$(curl -s -i -X POST "$SERVER_URL/uploads/" -H "Content-Type: text/plain" -d "test data" 2>/dev/null)
print_test "POST to uploads" "201" "$(get_status "$response")"

response=$(curl -s -i -X DELETE "$SERVER_URL/uploads/test_upload.txt" 2>/dev/null)
print_test "DELETE valid file" "200" "$(get_status "$response")"

# 1.2 Not implemented methods (should be 501)
response=$(curl -s -i -X PUT "$SERVER_URL/" 2>/dev/null)
print_test "PUT method (not implemented)" "501" "$(get_status "$response")"

response=$(curl -s -i -X PATCH "$SERVER_URL/" 2>/dev/null)
print_test "PATCH method (not implemented)" "501" "$(get_status "$response")"

response=$(curl -s -i -X TRACE "$SERVER_URL/" 2>/dev/null)
print_test "TRACE method (not implemented)" "501" "$(get_status "$response")"

response=$(curl -s -i -X CONNECT "$SERVER_URL/" 2>/dev/null)
print_test "CONNECT method (not implemented)" "501" "$(get_status "$response")"

# 1.3 Method not allowed for specific locations (should be 405)
response=$(curl -s -i -X OPTIONS "$SERVER_URL/" 2>/dev/null)
print_test "OPTIONS on / (not in allow_methods)" "405" "$(get_status "$response")"

response=$(curl -s -i -X POST "$SERVER_URL/css/" 2>/dev/null)
print_test "POST on /css/ (only GET allowed)" "405" "$(get_status "$response")"

response=$(curl -s -i -X DELETE "$SERVER_URL/images/" 2>/dev/null)
print_test "DELETE on /images/ (only GET allowed)" "405" "$(get_status "$response")"

echo -e "${COLOR_YELLOW}📋 2. HTTP STATUS CODE TESTS${COLOR_NC}"
echo "=================================================="

# 2.1 Success codes
response=$(curl -s -i "$SERVER_URL/test.txt" 2>/dev/null)
print_test "200 OK - Valid file" "200" "$(get_status "$response")"

# 2.2 Redirect codes
response=$(curl -s -i "$SERVER_URL/" 2>/dev/null)
print_test "301 Redirect - Root to index" "301" "$(get_status "$response")"

response=$(curl -s -i "$SERVER_URL/uploads" 2>/dev/null)
print_test "301 Redirect - Directory without slash" "301" "$(get_status "$response")"

# 2.3 Client error codes
response=$(curl -s -i -X POST "$SERVER_URL/uploads/" 2>/dev/null)
print_test "400 Bad Request - Empty POST body" "400" "$(get_status "$response")"

response=$(curl -s -i "$SERVER_URL/nonexistent.txt" 2>/dev/null)
print_test "404 Not Found - Missing file" "404" "$(get_status "$response")"

response=$(curl -s -i -X POST "$SERVER_URL/css/" 2>/dev/null)
print_test "405 Method Not Allowed - POST on CSS" "405" "$(get_status "$response")"

# 2.4 Server error codes
# Test 500 with invalid config or internal error would need special setup

echo -e "${COLOR_YELLOW}📋 3. CONTENT-TYPE AND MIME TESTS${COLOR_NC}"
echo "=================================================="

# Create test files with different extensions
echo "<html><body>Test HTML</body></html>" > www/test.html
echo "body { color: red; }" > www/test.css
echo "console.log('test');" > www/test.js

response=$(curl -s -i "$SERVER_URL/test.html" 2>/dev/null)
content_type=$(echo "$response" | grep -i "content-type" | cut -d' ' -f2-)
print_test "HTML Content-Type" "text/html" "$content_type"

response=$(curl -s -i "$SERVER_URL/test.css" 2>/dev/null)
content_type=$(echo "$response" | grep -i "content-type" | cut -d' ' -f2-)
print_test "CSS Content-Type" "text/css" "$content_type"

response=$(curl -s -i "$SERVER_URL/test.js" 2>/dev/null)
content_type=$(echo "$response" | grep -i "content-type" | cut -d' ' -f2-)
print_test "JS Content-Type" "application/javascript" "$content_type"

echo -e "${COLOR_YELLOW}📋 4. ERROR PAGE TESTS${COLOR_NC}"
echo "=================================================="

# Test custom error pages
response=$(curl -s "$SERVER_URL/nonexistent.txt" 2>/dev/null)
if [[ "$response" == *"404"* ]] && [[ "$response" == *"html"* ]]; then
    print_test "Custom 404 error page" "HTML error page" "HTML error page"
else
    print_test "Custom 404 error page" "HTML error page" "Plain text or missing"
fi

echo -e "${COLOR_YELLOW}📋 5. DIRECTORY LISTING TESTS${COLOR_NC}"
echo "=================================================="

# Test autoindex
response=$(curl -s -L "$SERVER_URL/uploads/" 2>/dev/null)
if [[ "$response" == *"Directory Listing"* ]]; then
    print_test "Autoindex directory listing" "Directory Listing" "Directory Listing"
else
    print_test "Autoindex directory listing" "Directory Listing" "No listing or error"
fi

echo -e "${COLOR_YELLOW}📋 6. UPLOAD TESTS${COLOR_NC}"
echo "=================================================="

# Test file upload with different content types
response=$(curl -s -i -X POST "$SERVER_URL/uploads/" \
    -H "Content-Type: text/plain" \
    -d "Test upload content" 2>/dev/null)
print_test "Text upload" "201" "$(get_status "$response")"

# Test multipart upload
response=$(curl -s -i -X POST "$SERVER_URL/uploads/" \
    -F "file=@www/test.txt" 2>/dev/null)
print_test "Multipart file upload" "201" "$(get_status "$response")"

# Test unsupported media type
response=$(curl -s -i -X POST "$SERVER_URL/uploads/" \
    -H "Content-Type: application/octet-stream" \
    -d "binary data" 2>/dev/null)
print_test "Binary upload handling" "201" "$(get_status "$response")"

echo -e "${COLOR_YELLOW}📋 7. CGI TESTS${COLOR_NC}"
echo "=================================================="

# Create simple Python CGI script
mkdir -p www/cgi-bin
cat > www/cgi-bin/test.py << 'EOF'
#!/usr/bin/env python3
print("Content-Type: text/html\r")
print("\r")
print("<html><body><h1>CGI Test Success</h1></body></html>")
EOF
chmod +x www/cgi-bin/test.py

response=$(curl -s -i "$SERVER_URL/cgi-bin/test.py" 2>/dev/null)
print_test "CGI script execution" "200" "$(get_status "$response")"

# Create timeout CGI script
cat > www/cgi-bin/timeout.py << 'EOF'
#!/usr/bin/env python3
import time
print("Content-Type: text/html\r")
print("\r")
time.sleep(35)  # Should trigger timeout
print("<html><body><h1>This should timeout</h1></body></html>")
EOF
chmod +x www/cgi-bin/timeout.py

echo "Testing CGI timeout (this will take ~30 seconds)..."
response=$(curl -s -i --max-time 35 "$SERVER_URL/cgi-bin/timeout.py" 2>/dev/null)
print_test "CGI timeout handling" "504" "$(get_status "$response")"

echo -e "${COLOR_YELLOW}📋 8. SECURITY TESTS${COLOR_NC}"
echo "=================================================="

# Test path traversal protection
response=$(curl -s -i "$SERVER_URL/../../../etc/passwd" 2>/dev/null)
print_test "Path traversal protection" "404" "$(get_status "$response")"

response=$(curl -s -i "$SERVER_URL/uploads/../../../etc/passwd" 2>/dev/null)
print_test "Path traversal in uploads" "404" "$(get_status "$response")"

echo -e "${COLOR_YELLOW}📋 9. HTTP HEADERS TESTS${COLOR_NC}"
echo "=================================================="

# Test CORS headers
response=$(curl -s -i -X OPTIONS "$SERVER_URL/" 2>/dev/null)
if [[ "$response" == *"Access-Control-Allow-Origin"* ]]; then
    print_test "CORS headers present" "Access-Control" "Access-Control"
else
    print_test "CORS headers present" "Access-Control" "Missing"
fi

# Test Allow header in 405 responses
response=$(curl -s -i -X POST "$SERVER_URL/css/" 2>/dev/null)
if [[ "$response" == *"Allow:"* ]]; then
    print_test "Allow header in 405" "Allow header" "Allow header"
else
    print_test "Allow header in 405" "Allow header" "Missing"
fi

echo -e "${COLOR_YELLOW}📊 TEST SUMMARY${COLOR_NC}"
echo "=================================================="
echo -e "Total Tests: $TEST_COUNT"
echo -e "${COLOR_GREEN}Passed: $PASS_COUNT${COLOR_NC}"
echo -e "${COLOR_RED}Failed: $FAIL_COUNT${COLOR_NC}"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${COLOR_GREEN}🎉 ALL TESTS PASSED! HTTP compliance verified.${COLOR_NC}"
    exit 0
else
    echo -e "${COLOR_RED}❌ Some tests failed. Please review the implementation.${COLOR_NC}"
    exit 1
fi 