#!/usr/bin/env python3
import os
import sys
import cgi
import json
import platform
import traceback
from datetime import datetime
from urllib.parse import parse_qs

def send_error(message):
    print("Content-Type: text/html; charset=UTF-8\r\n")
    print("""<!DOCTYPE html>
<html>
<head>
    <title>CGI Error</title>
    <meta charset="UTF-8">
    <style>
        :root {
            --primary-color: #2196F3;
            --error-color: #f44336;
            --success-color: #4CAF50;
            --warning-color: #ff9800;
        }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            padding: 20px;
            margin: 0;
            background: #f0f2f5;
            color: #333;
        }
        .container {
            max-width: 1000px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .error-box {
            background: #ffebee;
            color: var(--error-color);
            padding: 15px;
            border-radius: 4px;
            border-left: 4px solid var(--error-color);
            margin: 10px 0;
        }
        .debug-box {
            background: #f5f5f5;
            padding: 15px;
            border-radius: 4px;
            border: 1px solid #ddd;
            margin: 10px 0;
        }
        pre {
            background: #2d2d2d;
            color: #fff;
            padding: 15px;
            border-radius: 4px;
            overflow-x: auto;
        }
        h1, h2 { color: var(--primary-color); }
    </style>
</head>
<body>
    <div class="container">
        <h1>CGI Error</h1>
        <div class="error-box">""")
    print(message)
    print("""</div>
        <h2>Debug Information</h2>
        <div class="debug-box">
            <pre>""")
    print("Python version:", sys.version)
    print("\nEnvironment Variables:")
    for key, value in sorted(os.environ.items()):
        print(f"{key}: {value}")
    print("\nTraceback:")
    print(traceback.format_exc())
    print("""</pre>
        </div>
    </div>
</body>
</html>""")
    sys.exit(0)

try:
    print("Content-Type: text/html; charset=UTF-8\r\n")
    print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Test Interface</title>
    <style>
        :root {
            --primary-color: #2196F3;
            --secondary-color: #673AB7;
            --success-color: #4CAF50;
            --error-color: #f44336;
            --warning-color: #ff9800;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            margin: 0;
            padding: 20px;
            background: #f0f2f5;
            color: #333;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        header {
            background: var(--primary-color);
            color: white;
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 20px;
            position: relative;
        }
        
        .home-button {
            position: absolute;
            top: 20px;
            right: 20px;
            background: white;
            color: var(--primary-color);
            padding: 8px 16px;
            border-radius: 4px;
            text-decoration: none;
            font-weight: bold;
            transition: opacity 0.2s;
        }
        
        .home-button:hover {
            opacity: 0.9;
        }
        
        .card {
            background: white;
            border-radius: 8px;
            padding: 20px;
            margin: 10px 0;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            border-left: 4px solid var(--primary-color);
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        
        .stat-box {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            border: 1px solid #dee2e6;
        }
        
        .stat-value {
            font-size: 24px;
            font-weight: bold;
            color: var(--primary-color);
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 10px 0;
        }
        
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #dee2e6;
        }
        
        th {
            background: #f8f9fa;
            font-weight: 600;
        }
        
        pre {
            background: #2d2d2d;
            color: #fff;
            padding: 15px;
            border-radius: 4px;
            overflow-x: auto;
        }
        
        .badge {
            display: inline-block;
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 12px;
            font-weight: 600;
        }
        
        .badge-success { background: var(--success-color); color: white; }
        .badge-error { background: var(--error-color); color: white; }
        .badge-warning { background: var(--warning-color); color: white; }
        
        form {
            margin: 15px 0;
        }
        
        input[type="text"], input[type="file"] {
            padding: 8px;
            margin: 5px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
            width: 100%;
        }
        
        input[type="submit"] {
            background: var(--primary-color);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 4px;
            cursor: pointer;
            margin-top: 10px;
        }
        
        input[type="submit"]:hover {
            opacity: 0.9;
        }
        
        footer {
            text-align: center;
            margin-top: 20px;
            padding: 20px;
            color: #6c757d;
            border-top: 1px solid #ddd;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>Python CGI Test Interface</h1>
            <p>A comprehensive testing interface for Python CGI functionality</p>
            <a href="/cgi-bin/index.html" class="home-button">Return to Home</a>
        </header>""")

    # System Information Section
    print("""
        <div class="card">
            <h2>💻 System Information</h2>
            <div class="info-grid">""")
    print(f"""
                <div class="stat-box">
                    <h3>Python Version</h3>
                    <div class="stat-value">{platform.python_version()}</div>
                </div>
                <div class="stat-box">
                    <h3>Platform</h3>
                    <div class="stat-value">{platform.system()}</div>
                </div>
                <div class="stat-box">
                    <h3>Time</h3>
                    <div class="stat-value">{datetime.now().strftime('%H:%M:%S')}</div>
                </div>
            </div>
        </div>""")

    # Request Information
    method = os.environ.get('REQUEST_METHOD', 'GET')
    print(f"""
        <div class="card">
            <h2>🌐 Request Information</h2>
            <table>
                <tr>
                    <th>Method</th>
                    <td><span class="badge badge-success">{method}</span></td>
                </tr>
                <tr>
                    <th>Server Protocol</th>
                    <td>{os.environ.get('SERVER_PROTOCOL', 'N/A')}</td>
                </tr>
                <tr>
                    <th>Server Name</th>
                    <td>{os.environ.get('SERVER_NAME', 'N/A')}</td>
                </tr>
                <tr>
                    <th>Server Port</th>
                    <td>{os.environ.get('SERVER_PORT', 'N/A')}</td>
                </tr>
            </table>
        </div>""")

    # Request Parameters
    print("""
        <div class="card">
            <h2>📝 Request Parameters</h2>""")

    if method == 'GET':
        query_string = os.environ.get('QUERY_STRING', '')
        if query_string:
            try:
                params = parse_qs(query_string)
                print("""
                <table>
                    <tr>
                        <th>Parameter</th>
                        <th>Value</th>
                    </tr>""")
                for key, values in params.items():
                    print(f"""
                    <tr>
                        <td>{key}</td>
                        <td>{values[0]}</td>
                    </tr>""")
                print("</table>")
            except Exception as e:
                print(f'<p class="badge badge-error">Error parsing query string: {str(e)}</p>')
        else:
            print('<p>No query parameters</p>')

    if method == 'POST':
        try:
            form = cgi.FieldStorage()
            content_type = os.environ.get('CONTENT_TYPE', '')
            
            print(f"""
                <p><strong>Content-Type:</strong> {content_type}</p>
                <table>
                    <tr>
                        <th>Field</th>
                        <th>Value</th>
                    </tr>""")
            
            if form.list:
                for field in form.list:
                    value = "File uploaded: " + field.filename if field.filename else form.getvalue(field.name)
                    print(f"""
                    <tr>
                        <td>{field.name}</td>
                        <td>{value}</td>
                    </tr>""")
            print("</table>")
        except Exception as e:
            print(f'<p class="badge badge-error">Error processing POST data: {str(e)}</p>')

    print("</div>")

    # Environment Variables
    print("""
        <div class="card">
            <h2>🔧 Environment Variables</h2>
            <pre>""")
    for key, value in sorted(os.environ.items()):
        print(f"{key}: {value}")
    print("""</pre>
        </div>""")

    # Test Forms Section
    print("""
        <div class="card">
            <h2>🧪 Test Forms</h2>
            <h3>GET Test</h3>
            <form action="/cgi-bin/test.py" method="get">
                <input type="text" name="test_param" placeholder="Enter test parameter">
                <input type="submit" value="Test GET">
            </form>

            <h3>POST Test</h3>
            <form action="/cgi-bin/test.py" method="post" enctype="multipart/form-data">
                <input type="text" name="text_field" placeholder="Enter text">
                <input type="file" name="file_field">
                <input type="submit" value="Test POST">
            </form>
        </div>""")

    print("""
        <footer>
            <p>Python CGI Test Interface - Running on Python {}</p>
        </footer>
    </div>
</body>
</html>""".format(platform.python_version()))

except Exception as e:
    send_error(f"Unexpected error: {str(e)}")
