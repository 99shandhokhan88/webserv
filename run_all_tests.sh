#!/bin/bash

# Master Test Runner for WebServ
# Runs all test suites and provides comprehensive results

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

SERVER_HOST="localhost"
SERVER_PORT="8080"
BASE_URL="http://${SERVER_HOST}:${SERVER_PORT}"

echo -e "${BOLD}${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${BLUE}║                    WEBSERV COMPLETE TEST SUITE              ║${NC}"
echo -e "${BOLD}${BLUE}║                                                              ║${NC}"
echo -e "${BOLD}${BLUE}║  Testing HTTP compliance, subject requirements, and         ║${NC}"
echo -e "${BOLD}${BLUE}║  extreme edge cases for 42 webserv project                  ║${NC}"
echo -e "${BOLD}${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"
echo

# Check if server is running
echo -e "${CYAN}Checking server status...${NC}"
if ! curl -s --connect-timeout 5 "$BASE_URL" > /dev/null; then
    echo -e "${RED}❌ Server is not running on ${SERVER_HOST}:${SERVER_PORT}!${NC}"
    echo -e "${YELLOW}Please start your webserver first:${NC}"
    echo -e "${BLUE}  ./webserv configs/default.conf${NC}"
    echo
    exit 1
fi
echo -e "${GREEN}✅ Server is running and responding!${NC}"
echo

# Make all test scripts executable
chmod +x comprehensive_test.sh 2>/dev/null || true
chmod +x extreme_edge_cases_test.sh 2>/dev/null || true
chmod +x subject_compliance_test.sh 2>/dev/null || true

# Initialize results tracking
TOTAL_SUITES=0
PASSED_SUITES=0
FAILED_SUITES=0

# Function to run a test suite
run_test_suite() {
    local script_name="$1"
    local suite_name="$2"
    local description="$3"
    
    TOTAL_SUITES=$((TOTAL_SUITES + 1))
    
    echo -e "${BOLD}${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${BOLD}${YELLOW}  $suite_name${NC}"
    echo -e "${BOLD}${YELLOW}  $description${NC}"
    echo -e "${BOLD}${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
    echo
    
    if [ -f "$script_name" ]; then
        if ./"$script_name"; then
            echo -e "${GREEN}✅ $suite_name: PASSED${NC}"
            PASSED_SUITES=$((PASSED_SUITES + 1))
            return 0
        else
            echo -e "${RED}❌ $suite_name: FAILED${NC}"
            FAILED_SUITES=$((FAILED_SUITES + 1))
            return 1
        fi
    else
        echo -e "${RED}❌ Test script $script_name not found!${NC}"
        FAILED_SUITES=$((FAILED_SUITES + 1))
        return 1
    fi
}

# Run all test suites
echo -e "${BOLD}${CYAN}Starting comprehensive test execution...${NC}"
echo

# 1. Subject Compliance Tests
run_test_suite "subject_compliance_test.sh" "SUBJECT COMPLIANCE" "Testing 42 webserv subject requirements"
echo

# 2. Comprehensive HTTP Tests
run_test_suite "comprehensive_test.sh" "COMPREHENSIVE HTTP" "Testing HTTP compliance and general functionality"
echo

# 3. Extreme Edge Cases
run_test_suite "extreme_edge_cases_test.sh" "EXTREME EDGE CASES" "Testing challenging HTTP parsing and edge cases"
echo

# Additional quick tests
echo -e "${BOLD}${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${BOLD}${YELLOW}  QUICK SANITY CHECKS${NC}"
echo -e "${BOLD}${YELLOW}  Basic functionality verification${NC}"
echo -e "${BOLD}${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo

# Quick sanity checks
SANITY_PASS=0
SANITY_TOTAL=0

# Test basic GET
SANITY_TOTAL=$((SANITY_TOTAL + 1))
if curl -s "$BASE_URL/" | grep -q "html\|HTML\|<"; then
    echo -e "${GREEN}✓${NC} Basic GET request works"
    SANITY_PASS=$((SANITY_PASS + 1))
else
    echo -e "${RED}✗${NC} Basic GET request failed"
fi

# Test 404 handling
SANITY_TOTAL=$((SANITY_TOTAL + 1))
if [ "$(curl -s -o /dev/null -w "%{http_code}" "$BASE_URL/nonexistent")" = "404" ]; then
    echo -e "${GREEN}✓${NC} 404 error handling works"
    SANITY_PASS=$((SANITY_PASS + 1))
else
    echo -e "${RED}✗${NC} 404 error handling failed"
fi

# Test POST method
SANITY_TOTAL=$((SANITY_TOTAL + 1))
POST_STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "test=data" "$BASE_URL/")
if [ "$POST_STATUS" = "200" ] || [ "$POST_STATUS" = "201" ]; then
    echo -e "${GREEN}✓${NC} POST method works"
    SANITY_PASS=$((SANITY_PASS + 1))
else
    echo -e "${RED}✗${NC} POST method failed (status: $POST_STATUS)"
fi

# Test autoindex
SANITY_TOTAL=$((SANITY_TOTAL + 1))
if curl -s "$BASE_URL/uploads/" | grep -q "Index\|Directory\|Parent"; then
    echo -e "${GREEN}✓${NC} Autoindex functionality works"
    SANITY_PASS=$((SANITY_PASS + 1))
else
    echo -e "${RED}✗${NC} Autoindex functionality failed"
fi

# Test static file serving
SANITY_TOTAL=$((SANITY_TOTAL + 1))
if [ "$(curl -s -o /dev/null -w "%{http_code}" "$BASE_URL/index.html")" = "200" ]; then
    echo -e "${GREEN}✓${NC} Static file serving works"
    SANITY_PASS=$((SANITY_PASS + 1))
else
    echo -e "${RED}✗${NC} Static file serving failed"
fi

echo

# Performance test
echo -e "${BOLD}${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo -e "${BOLD}${YELLOW}  PERFORMANCE TEST${NC}"
echo -e "${BOLD}${YELLOW}  Testing server performance under load${NC}"
echo -e "${BOLD}${YELLOW}═══════════════════════════════════════════════════════════════${NC}"
echo

echo "Running 50 concurrent requests..."
START_TIME=$(date +%s.%N)
for i in {1..50}; do
    curl -s "$BASE_URL/" > /dev/null &
done
wait
END_TIME=$(date +%s.%N)

DURATION=$(echo "$END_TIME - $START_TIME" | bc 2>/dev/null || echo "N/A")
if [ "$DURATION" != "N/A" ]; then
    echo -e "${GREEN}✓${NC} 50 concurrent requests completed in ${DURATION} seconds"
    
    # Calculate requests per second
    if command -v bc >/dev/null 2>&1; then
        RPS=$(echo "scale=2; 50 / $DURATION" | bc)
        echo -e "${BLUE}  Performance: ~${RPS} requests/second${NC}"
    fi
else
    echo -e "${YELLOW}⚠${NC} Performance test completed (timing unavailable)"
fi

echo

# Final summary
echo -e "${BOLD}${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${BOLD}${BLUE}║                        FINAL RESULTS                        ║${NC}"
echo -e "${BOLD}${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"
echo

echo -e "${BOLD}Test Suites Summary:${NC}"
echo -e "  Total suites: ${BLUE}$TOTAL_SUITES${NC}"
echo -e "  Passed: ${GREEN}$PASSED_SUITES${NC}"
echo -e "  Failed: ${RED}$FAILED_SUITES${NC}"
echo

echo -e "${BOLD}Sanity Checks Summary:${NC}"
echo -e "  Total checks: ${BLUE}$SANITY_TOTAL${NC}"
echo -e "  Passed: ${GREEN}$SANITY_PASS${NC}"
echo -e "  Failed: ${RED}$((SANITY_TOTAL - SANITY_PASS))${NC}"
echo

# Calculate overall score
SUITE_SCORE=$((PASSED_SUITES * 100 / TOTAL_SUITES))
SANITY_SCORE=$((SANITY_PASS * 100 / SANITY_TOTAL))
OVERALL_SCORE=$(((SUITE_SCORE + SANITY_SCORE) / 2))

echo -e "${BOLD}Overall Score: ${BLUE}${OVERALL_SCORE}%${NC}"
echo

# Final verdict
if [ $FAILED_SUITES -eq 0 ] && [ $SANITY_PASS -eq $SANITY_TOTAL ]; then
    echo -e "${BOLD}${GREEN}🎉 EXCELLENT! Your webserver passes all tests!${NC}"
    echo -e "${GREEN}   ✅ Full HTTP compliance${NC}"
    echo -e "${GREEN}   ✅ All subject requirements met${NC}"
    echo -e "${GREEN}   ✅ Handles all edge cases correctly${NC}"
    echo -e "${GREEN}   ✅ Ready for evaluation!${NC}"
    EXIT_CODE=0
elif [ $OVERALL_SCORE -ge 80 ]; then
    echo -e "${BOLD}${YELLOW}⚠️  GOOD! Your webserver is mostly compliant.${NC}"
    echo -e "${YELLOW}   Some minor issues need attention.${NC}"
    echo -e "${YELLOW}   Review the failed tests above.${NC}"
    EXIT_CODE=1
elif [ $OVERALL_SCORE -ge 60 ]; then
    echo -e "${BOLD}${YELLOW}⚠️  FAIR! Your webserver has basic functionality.${NC}"
    echo -e "${YELLOW}   Several important issues need fixing.${NC}"
    echo -e "${YELLOW}   Focus on the failed test suites.${NC}"
    EXIT_CODE=2
else
    echo -e "${BOLD}${RED}❌ POOR! Your webserver needs significant work.${NC}"
    echo -e "${RED}   Major functionality is missing or broken.${NC}"
    echo -e "${RED}   Review all failed tests carefully.${NC}"
    EXIT_CODE=3
fi

echo
echo -e "${BOLD}${BLUE}Test execution completed.${NC}"
echo -e "${BLUE}For detailed results, check the individual test outputs above.${NC}"
echo

exit $EXIT_CODE 