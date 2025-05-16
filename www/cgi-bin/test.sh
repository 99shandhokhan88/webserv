#!/bin/bash

# Send HTTP headers
echo "Content-type: text/html"
echo ""

# Start HTML output
cat << HTML_START
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Shell CGI Test Interface</title>
    <style>
        :root {
            --primary-color: #FF5722;
            --secondary-color: #795548;
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
            text-align: center;
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
        
        footer {
            text-align: center;
            margin-top: 20px;
            padding: 20px;
            color: #6c757d;
        }

        .terminal {
            background: #1e1e1e;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            padding: 15px;
            border-radius: 4px;
            margin: 10px 0;
        }

        form {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 8px;
            margin: 10px 0;
        }

        input[type="text"], input[type="file"] {
            width: 100%;
            padding: 8px;
            margin: 8px 0;
            border: 1px solid #ddd;
            border-radius: 4px;
        }

        input[type="submit"] {
            background: var(--primary-color);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 4px;
            cursor: pointer;
        }

        input[type="submit"]:hover {
            opacity: 0.9;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🐚 Shell CGI Test Interface</h1>
            <p>A comprehensive testing interface for Shell CGI functionality</p>
            <a href="/cgi-bin/index.html" class="home-button">Return to Home</a>
        </header>
HTML_START

# System Information Section
echo '<div class="card">
    <h2>🖥️ System Information</h2>
    <div class="info-grid">
        <div class="stat-box">
            <h3>Shell Version</h3>
            <div class="stat-value">'
echo $BASH_VERSION
echo '</div>
        </div>
        <div class="stat-box">
            <h3>System</h3>
            <div class="stat-value">'
uname -s
echo '</div>
        </div>
        <div class="stat-box">
            <h3>Time</h3>
            <div class="stat-value">'
date +"%H:%M:%S"
echo '</div>
        </div>
    </div>
</div>'

# Request Information
echo '<div class="card">
    <h2>🌐 Request Information</h2>
    <table>
        <tr>
            <th>Method</th>
            <td><span class="badge badge-success">'$REQUEST_METHOD'</span></td>
        </tr>
        <tr>
            <th>Server Protocol</th>
            <td>'$SERVER_PROTOCOL'</td>
        </tr>
        <tr>
            <th>Server Name</th>
            <td>'$SERVER_NAME'</td>
        </tr>
        <tr>
            <th>Server Port</th>
            <td>'$SERVER_PORT'</td>
        </tr>
    </table>
</div>'

# Request Parameters
echo '<div class="card">
    <h2>📝 Request Parameters</h2>'

if [ "$REQUEST_METHOD" = "GET" ]; then
    if [ -n "$QUERY_STRING" ]; then
        echo '<table>
            <tr>
                <th>Parameter</th>
                <th>Value</th>
            </tr>'
        echo "$QUERY_STRING" | tr '&' '\n' | while read -r param; do
            KEY=$(echo "$param" | cut -d= -f1)
            VALUE=$(echo "$param" | cut -d= -f2)
            echo "<tr><td>$KEY</td><td>$VALUE</td></tr>"
        done
        echo '</table>'
    else
        echo '<p>No query parameters</p>'
    fi
elif [ "$REQUEST_METHOD" = "POST" ]; then
    if [ -n "$CONTENT_TYPE" ]; then
        echo "<p><strong>Content-Type:</strong> $CONTENT_TYPE</p>"
        if [ $(echo "$CONTENT_TYPE" | grep -c "multipart/form-data") -eq 1 ]; then
            echo '<p>Multipart form data received</p>'
        else
            read -n "$CONTENT_LENGTH" POST_DATA
            echo '<table>
                <tr>
                    <th>Field</th>
                    <th>Value</th>
                </tr>'
            echo "$POST_DATA" | tr '&' '\n' | while read -r param; do
                KEY=$(echo "$param" | cut -d= -f1)
                VALUE=$(echo "$param" | cut -d= -f2)
                echo "<tr><td>$KEY</td><td>$VALUE</td></tr>"
            done
            echo '</table>'
        fi
    fi
fi
echo '</div>'

# Environment Variables
echo '<div class="card">
    <h2>🔧 Environment Variables</h2>
    <pre>'
env | sort
echo '</pre>
</div>'

# Shell Commands Demo
echo '<div class="card">
    <h2>💻 Shell Commands Demo</h2>
    <div class="terminal">
        <p>$ pwd</p>'
pwd
echo '
        <p>$ ls -la</p>'
ls -la
echo '
        <p>$ whoami</p>'
whoami
echo '
    </div>
</div>'

# Test Forms
echo '<div class="card">
    <h2>🧪 Test Forms</h2>
    <h3>GET Test</h3>
    <form action="/cgi-bin/test.sh" method="get">
        <input type="text" name="test_param" placeholder="Enter test parameter">
        <input type="submit" value="Test GET">
    </form>

    <h3>POST Test</h3>
    <form action="/cgi-bin/test.sh" method="post" enctype="multipart/form-data">
        <input type="text" name="text_field" placeholder="Enter text">
        <input type="file" name="file_field">
        <input type="submit" value="Test POST">
    </form>
</div>'

# Footer
echo '<footer>
    <p>Shell CGI Test Interface - Running on Bash '${BASH_VERSION}'</p>
</footer>
</div>
</body>
</html>' 