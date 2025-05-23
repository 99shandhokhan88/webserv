#!/usr/bin/env python3

import time
import sys

print("Content-Type: text/html\r")
print("\r")
print("<!DOCTYPE html>")
print("<html><head><title>Timeout Test</title></head><body>")
print("<h1>Testing CGI Timeout...</h1>")
print("<p>This script will sleep for 35 seconds to trigger a timeout.</p>")

# Flush output to ensure headers are sent
sys.stdout.flush()

# Sleep for 35 seconds - longer than the 30 second timeout
print("<p>Starting 35 second sleep...</p>")
sys.stdout.flush()

time.sleep(35)

# This should never be reached if timeout works correctly
print("<p>This message should never appear if timeout works correctly!</p>")
print("</body></html>") 