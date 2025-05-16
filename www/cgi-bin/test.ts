#!/usr/bin/env ts-node

interface SystemInfo {
    time: string;
    userAgent: string;
    platform: string;
}

interface RequestInfo {
    method: string;
    protocol: string;
    serverName: string;
    serverPort: string;
}

function getSystemInfo(): SystemInfo {
    const now = new Date();
    return {
        time: now.toLocaleTimeString(),
        userAgent: process.env.HTTP_USER_AGENT || 'Unknown',
        platform: 'Web'
    };
}

function getRequestInfo(): RequestInfo {
    return {
        method: process.env.REQUEST_METHOD || 'GET',
        protocol: process.env.SERVER_PROTOCOL || 'HTTP/1.1',
        serverName: process.env.SERVER_NAME || 'localhost',
        serverPort: process.env.SERVER_PORT || '8080'
    };
}

function htmlspecialchars(text: string): string {
    return text
        .replace(/&/g, '&amp;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&apos;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;');
}

function parseQueryString(qs: string): { [key: string]: string } {
    const params: { [key: string]: string } = {};
    if (!qs) return params;

    qs.split('&').forEach(pair => {
        const [key, value] = pair.split('=');
        if (key) params[decodeURIComponent(key)] = decodeURIComponent(value || '');
    });

    return params;
}

// Start output
console.log('Content-type: text/html\r\n');

const systemInfo = getSystemInfo();
const requestInfo = getRequestInfo();
const queryParams = parseQueryString(process.env.QUERY_STRING || '');

console.log(`<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TypeScript CGI Test Interface</title>
    <style>
        :root {
            --primary-color: #3178c6;
            --secondary-color: #235a97;
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

        .ts-info {
            background: #1a1a1a;
            color: #3178c6;
            font-family: 'Courier New', monospace;
            padding: 15px;
            border-radius: 4px;
            margin: 10px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>📘 TypeScript CGI Test Interface</h1>
            <p>A comprehensive testing interface for TypeScript CGI functionality</p>
            <a href="/cgi-bin/index.html" class="home-button">Return to Home</a>
        </header>

        <!-- System Information -->
        <div class="card">
            <h2>🖥️ System Information</h2>
            <div class="info-grid">
                <div class="stat-box">
                    <h3>Time</h3>
                    <div class="stat-value">${htmlspecialchars(systemInfo.time)}</div>
                </div>
                <div class="stat-box">
                    <h3>Platform</h3>
                    <div class="stat-value">${htmlspecialchars(systemInfo.platform)}</div>
                </div>
                <div class="stat-box">
                    <h3>User Agent</h3>
                    <div class="stat-value">${htmlspecialchars(systemInfo.userAgent)}</div>
                </div>
            </div>
        </div>

        <!-- Request Information -->
        <div class="card">
            <h2>🌐 Request Information</h2>
            <table>
                <tr>
                    <th>Method</th>
                    <td><span class="badge badge-success">${htmlspecialchars(requestInfo.method)}</span></td>
                </tr>
                <tr>
                    <th>Protocol</th>
                    <td>${htmlspecialchars(requestInfo.protocol)}</td>
                </tr>
                <tr>
                    <th>Server Name</th>
                    <td>${htmlspecialchars(requestInfo.serverName)}</td>
                </tr>
                <tr>
                    <th>Server Port</th>
                    <td>${htmlspecialchars(requestInfo.serverPort)}</td>
                </tr>
            </table>
        </div>

        <!-- Request Parameters -->
        <div class="card">
            <h2>📝 Request Parameters</h2>
            ${Object.keys(queryParams).length > 0 ? `
            <table>
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                </tr>
                ${Object.entries(queryParams).map(([key, value]) => `
                <tr>
                    <td>${htmlspecialchars(key)}</td>
                    <td>${htmlspecialchars(String(value))}</td>
                </tr>
                `).join('')}
            </table>
            ` : '<p>No query parameters</p>'}
        </div>

        <!-- Environment Variables -->
        <div class="card">
            <h2>🔧 Environment Variables</h2>
            <div class="ts-info">
                <pre>${Object.entries(process.env)
                    .sort(([a], [b]) => a.localeCompare(b))
                    .map(([key, value]) => `${htmlspecialchars(key)}: ${htmlspecialchars(value || '')}`)
                    .join('\n')}</pre>
            </div>
        </div>

        <!-- Test Forms -->
        <div class="card">
            <h2>🧪 Test Forms</h2>
            <h3>GET Test</h3>
            <form action="/cgi-bin/test.ts" method="get">
                <input type="text" name="test_param" placeholder="Enter test parameter">
                <input type="submit" value="Test GET">
            </form>

            <h3>POST Test</h3>
            <form action="/cgi-bin/test.ts" method="post" enctype="multipart/form-data">
                <input type="text" name="text_field" placeholder="Enter text">
                <input type="file" name="file_field">
                <input type="submit" value="Test POST">
            </form>
        </div>

        <footer>
            <p>TypeScript CGI Test Interface - Running on TypeScript</p>
        </footer>
    </div>
</body>
</html>`); 