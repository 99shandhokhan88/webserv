#!/usr/bin/env python3
import time
print("Content-Type: text/html\r")
print("\r")
time.sleep(35)  # Should trigger timeout
print("<html><body><h1>This should timeout</h1></body></html>")
