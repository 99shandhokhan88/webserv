#!/bin/bash

# Extended Autoindex Test Script for webserv
# Focuses specifically on autoindex functionality and edge cases

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

echo -e "${COLOR_YELLOW}🔍 Extended Autoindex Test Suite for webserv${COLOR_NC}"
echo "=================================================="

# Check if server is running
check_server

# Prepare test directories and files
mkdir -p www/test_autoindex
mkdir -p www/test_autoindex/subdir1
mkdir -p www/test_autoindex/subdir2
mkdir -p www/test_autoindex/empty_dir
echo "Test file 1" > www/test_autoindex/file1.txt
echo "Test file 2" > www/test_autoindex/file2.html
echo "Test file 3" > www/test_autoindex/subdir1/nested_file.txt
echo "Script file" > www/test_autoindex/script.py

echo -e "${COLOR_YELLOW}📋 1. BASIC AUTOINDEX TESTS${COLOR_NC}"
echo "=================================================="

# Test 1: Basic directory listing with autoindex enabled
response=$(curl -s -i "$SERVER_URL/uploads/" 2>/dev/null)
status=$(get_status "$response")
print_test "Basic autoindex on /uploads/" "200" "$status"

# Check if response contains directory listing HTML
if [[ "$response" == *"Directory Listing"* ]]; then
    print_test "Directory listing HTML generated" "Directory Listing" "Directory Listing"
else
    print_test "Directory listing HTML generated" "Directory Listing" "Missing or malformed"
fi

# Test 2: Check for proper HTML structure
if [[ "$response" == *"<html>"* ]] && [[ "$response" == *"</html>"* ]]; then
    print_test "Valid HTML structure" "HTML tags" "HTML tags"
else
    print_test "Valid HTML structure" "HTML tags" "Missing or malformed"
fi

# Test 3: Check for CSS styling
if [[ "$response" == *"<style>"* ]] || [[ "$response" == *"stylesheet"* ]]; then
    print_test "CSS styling present" "CSS" "CSS"
else
    print_test "CSS styling present" "CSS" "Missing"
fi

echo -e "${COLOR_YELLOW}📋 2. DIRECTORY NAVIGATION TESTS${COLOR_NC}"
echo "=================================================="

# Test 4: Parent directory link (should be present when not in root)
response=$(curl -s "$SERVER_URL/uploads/" 2>/dev/null)
if [[ "$response" == *"Parent Directory"* ]] || [[ "$response" == *"../"* ]]; then
    print_test "Parent directory link present" "Parent Directory" "Parent Directory"
else
    print_test "Parent directory link present" "Parent Directory" "Missing"
fi

# Test 5: Directory vs file distinction
response=$(curl -s "$SERVER_URL/uploads/" 2>/dev/null)
if [[ "$response" == *"[DIR]"* ]]; then
    print_test "Directory markers present" "[DIR]" "[DIR]"
else
    print_test "Directory markers present" "[DIR]" "Missing"
fi

echo -e "${COLOR_YELLOW}📋 3. AUTOINDEX EDGE CASES${COLOR_NC}"
echo "=================================================="

# Test 6: Directory without trailing slash (should redirect)
response=$(curl -s -i "$SERVER_URL/uploads" 2>/dev/null)
status=$(get_status "$response")
print_test "Directory without slash redirect" "301" "$status"

# Test 7: Empty directory listing
response=$(curl -s -i "$SERVER_URL/test_autoindex/empty_dir/" 2>/dev/null)
status=$(get_status "$response")
print_test "Empty directory autoindex" "200" "$status"

# Test 8: Directory with mixed content
response=$(curl -s "$SERVER_URL/test_autoindex/" 2>/dev/null)
if [[ "$response" == *"file1.txt"* ]] && [[ "$response" == *"subdir1"* ]]; then
    print_test "Mixed content listing" "Files and directories" "Files and directories"
else
    print_test "Mixed content listing" "Files and directories" "Missing content"
fi

echo -e "${COLOR_YELLOW}📋 4. AUTOINDEX SECURITY TESTS${COLOR_NC}"
echo "=================================================="

# Test 9: No access to parent directories beyond root
response=$(curl -s -i "$SERVER_URL/../" 2>/dev/null)
status=$(get_status "$response")
if [[ "$status" == "404" ]] || [[ "$status" == "403" ]]; then
    print_test "Parent directory traversal blocked" "404 or 403" "$status"
else
    print_test "Parent directory traversal blocked" "404 or 403" "$status (SECURITY ISSUE)"
fi

# Test 10: Hidden files should not be listed
touch www/uploads/.hidden_file
response=$(curl -s "$SERVER_URL/uploads/" 2>/dev/null)
if [[ "$response" != *".hidden_file"* ]]; then
    print_test "Hidden files not listed" "Hidden files excluded" "Hidden files excluded"
else
    print_test "Hidden files not listed" "Hidden files excluded" "Hidden files visible (potential issue)"
fi

echo -e "${COLOR_YELLOW}📋 5. AUTOINDEX CONFIGURATION TESTS${COLOR_NC}"
echo "=================================================="

# Test 11: CGI directory autoindex
response=$(curl -s -i "$SERVER_URL/cgi-bin/" 2>/dev/null)
status=$(get_status "$response")
print_test "CGI directory autoindex" "200" "$status"

# Test 12: CSS directory (check if autoindex is disabled)
response=$(curl -s -i "$SERVER_URL/css/" 2>/dev/null)
status=$(get_status "$response")
# CSS directory might have autoindex disabled, check config
if [[ "$status" == "200" ]]; then
    print_test "CSS directory access" "200 (autoindex enabled)" "$status"
elif [[ "$status" == "403" ]]; then
    print_test "CSS directory access" "403 (autoindex disabled)" "$status"
else
    print_test "CSS directory access" "200 or 403" "$status"
fi

echo -e "${COLOR_YELLOW}📋 6. AUTOINDEX CONTENT VALIDATION${COLOR_NC}"
echo "=================================================="

# Test 13: Check for proper file links
response=$(curl -s "$SERVER_URL/uploads/" 2>/dev/null)
if [[ "$response" == *"href="* ]]; then
    print_test "File links present" "href attributes" "href attributes"
else
    print_test "File links present" "href attributes" "Missing"
fi

# Test 14: Check for proper Content-Type header
response=$(curl -s -i "$SERVER_URL/uploads/" 2>/dev/null)
content_type=$(echo "$response" | grep -i "content-type" | cut -d' ' -f2-)
print_test "Autoindex Content-Type" "text/html" "$content_type"

# Test 15: Check for proper Content-Length header
if [[ "$response" == *"Content-Length:"* ]]; then
    print_test "Content-Length header present" "Content-Length" "Content-Length"
else
    print_test "Content-Length header present" "Content-Length" "Missing"
fi

echo -e "${COLOR_YELLOW}📋 7. AUTOINDEX PERFORMANCE TESTS${COLOR_NC}"
echo "=================================================="

# Test 16: Large directory handling (create many files)
mkdir -p www/large_dir
for i in {1..50}; do
    echo "File $i content" > "www/large_dir/file_$i.txt"
done

start_time=$(date +%s%N)
response=$(curl -s "$SERVER_URL/large_dir/" 2>/dev/null)
end_time=$(date +%s%N)
duration=$(( (end_time - start_time) / 1000000 )) # Convert to milliseconds

if [[ $duration -lt 1000 ]]; then # Less than 1 second
    print_test "Large directory performance" "< 1000ms" "${duration}ms"
else
    print_test "Large directory performance" "< 1000ms" "${duration}ms (slow)"
fi

# Test 17: Check if all files are listed
file_count=$(echo "$response" | grep -o "file_[0-9]*.txt" | wc -l)
print_test "Large directory file count" "50" "$file_count"

echo -e "${COLOR_YELLOW}📋 8. AUTOINDEX ERROR HANDLING${COLOR_NC}"
echo "=================================================="

# Test 18: Non-existent directory
response=$(curl -s -i "$SERVER_URL/nonexistent_dir/" 2>/dev/null)
status=$(get_status "$response")
print_test "Non-existent directory" "404" "$status"

# Test 19: Permission denied directory (if possible to create)
mkdir -p www/no_permission_dir
chmod 000 www/no_permission_dir 2>/dev/null
response=$(curl -s -i "$SERVER_URL/no_permission_dir/" 2>/dev/null)
status=$(get_status "$response")
chmod 755 www/no_permission_dir 2>/dev/null # Restore permissions
if [[ "$status" == "403" ]] || [[ "$status" == "500" ]]; then
    print_test "Permission denied directory" "403 or 500" "$status"
else
    print_test "Permission denied directory" "403 or 500" "$status"
fi

# Cleanup
rm -rf www/test_autoindex www/large_dir www/no_permission_dir
rm -f www/uploads/.hidden_file

echo -e "${COLOR_YELLOW}📊 EXTENDED AUTOINDEX TEST SUMMARY${COLOR_NC}"
echo "=================================================="
echo -e "Total Tests: $TEST_COUNT"
echo -e "${COLOR_GREEN}Passed: $PASS_COUNT${COLOR_NC}"
echo -e "${COLOR_RED}Failed: $FAIL_COUNT${COLOR_NC}"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${COLOR_GREEN}🎉 ALL AUTOINDEX TESTS PASSED!${COLOR_NC}"
    exit 0
else
    echo -e "${COLOR_RED}❌ Some autoindex tests failed. Please review the implementation.${COLOR_NC}"
    exit 1
fi 