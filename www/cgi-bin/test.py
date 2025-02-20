#!/usr/bin/env python3

import os
import cgi

print("Content-Type: text/html\n\n")

print("<html><body>")
print("<h1>CGI Test Page</h1>")

# Print environment variables
print("<h2>Environment Variables</h2>")
print("<pre>")
for key, value in os.environ.items():
    print(f"{key}: {value}")
print("</pre>")

# Print form data
form = cgi.FieldStorage()
if form:
    print("<h2>Form Data</h2>")
    print("<pre>")
    for key in form.keys():
        print(f"{key}: {form[key].value}")
    print("</pre>")

print("</body></html>")