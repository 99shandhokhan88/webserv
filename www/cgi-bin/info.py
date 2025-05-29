#!/usr/bin/env python3

import os
import sys
import cgi
import cgitb
import json
import platform
import socket
import datetime
import random

# Enable CGI error reporting
cgitb.enable()

def get_system_info():
    """Get comprehensive system information"""
    try:
        return {
            'platform': platform.platform(),
            'system': platform.system(),
            'release': platform.release(),
            'version': platform.version(),
            'machine': platform.machine(),
            'processor': platform.processor(),
            'python_version': platform.python_version(),
            'hostname': socket.gethostname(),
            'current_time': datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
            'uptime': 'N/A'  # Would need additional system calls
        }
    except Exception as e:
        return {'error': str(e)}

def get_request_info():
    """Get HTTP request information"""
    return {
        'method': os.environ.get('REQUEST_METHOD', 'Unknown'),
        'query_string': os.environ.get('QUERY_STRING', ''),
        'content_type': os.environ.get('CONTENT_TYPE', ''),
        'content_length': os.environ.get('CONTENT_LENGTH', '0'),
        'remote_addr': os.environ.get('REMOTE_ADDR', 'Unknown'),
        'server_name': os.environ.get('SERVER_NAME', 'Unknown'),
        'server_port': os.environ.get('SERVER_PORT', 'Unknown'),
        'script_name': os.environ.get('SCRIPT_NAME', 'Unknown'),
        'path_info': os.environ.get('PATH_INFO', ''),
        'user_agent': os.environ.get('HTTP_USER_AGENT', 'Unknown')
    }

def get_form_data():
    """Get POST/GET form data"""
    form = cgi.FieldStorage()
    data = {}
    for key in form.keys():
        data[key] = form.getvalue(key)
    return data

def generate_random_color():
    """Generate a random color for theming"""
    colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#96CEB4', '#FFEAA7', '#DDA0DD', '#98D8C8', '#F7DC6F']
    return random.choice(colors)

def main():
    # Get all information
    system_info = get_system_info()
    request_info = get_request_info()
    form_data = get_form_data()
    env_vars = dict(os.environ)
    theme_color = generate_random_color()
    
    # Generate HTML response
    print("Content-Type: text/html\r\n")
    
    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🐍 Python CGI Info Dashboard</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, {theme_color}20, #667eea20);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }}
        
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            box-shadow: 0 20px 40px rgba(0,0,0,0.1);
            overflow: hidden;
        }}
        
        .header {{
            background: linear-gradient(135deg, {theme_color}, {theme_color}dd);
            color: white;
            padding: 30px;
            text-align: center;
            position: relative;
            overflow: hidden;
        }}
        
        .header::before {{
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: repeating-linear-gradient(
                45deg,
                transparent,
                transparent 10px,
                rgba(255,255,255,0.1) 10px,
                rgba(255,255,255,0.1) 20px
            );
            animation: slide 20s linear infinite;
        }}
        
        @keyframes slide {{
            0% {{ transform: translateX(-50px) translateY(-50px); }}
            100% {{ transform: translateX(50px) translateY(50px); }}
        }}
        
        .header h1 {{
            font-size: 2.5em;
            margin-bottom: 10px;
            position: relative;
            z-index: 1;
        }}
        
        .header p {{
            font-size: 1.2em;
            opacity: 0.9;
            position: relative;
            z-index: 1;
        }}
        
        .content {{
            padding: 30px;
        }}
        
        .grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 25px;
            margin-bottom: 30px;
        }}
        
        .card {{
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.1);
            border-left: 5px solid {theme_color};
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }}
        
        .card:hover {{
            transform: translateY(-5px);
            box-shadow: 0 15px 35px rgba(0,0,0,0.15);
        }}
        
        .card h3 {{
            color: {theme_color};
            margin-bottom: 20px;
            font-size: 1.4em;
            display: flex;
            align-items: center;
            gap: 10px;
        }}
        
        .info-item {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 0;
            border-bottom: 1px solid #f0f0f0;
        }}
        
        .info-item:last-child {{
            border-bottom: none;
        }}
        
        .info-key {{
            font-weight: 600;
            color: #555;
            flex: 1;
        }}
        
        .info-value {{
            background: #f8f9fa;
            padding: 8px 12px;
            border-radius: 8px;
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
            max-width: 60%;
            word-break: break-all;
            text-align: right;
        }}
        
        .env-vars {{
            grid-column: 1 / -1;
        }}
        
        .env-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 15px;
            max-height: 400px;
            overflow-y: auto;
        }}
        
        .env-item {{
            background: #f8f9fa;
            padding: 15px;
            border-radius: 10px;
            border-left: 3px solid {theme_color};
        }}
        
        .env-key {{
            font-weight: bold;
            color: {theme_color};
            margin-bottom: 5px;
        }}
        
        .env-value {{
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
            word-break: break-all;
            color: #666;
        }}
        
        .fun-facts {{
            background: linear-gradient(135deg, {theme_color}10, {theme_color}05);
            border-radius: 15px;
            padding: 25px;
            margin-top: 30px;
            text-align: center;
        }}
        
        .sparkle {{
            display: inline-block;
            animation: sparkle 2s ease-in-out infinite;
        }}
        
        @keyframes sparkle {{
            0%, 100% {{ transform: scale(1) rotate(0deg); opacity: 1; }}
            50% {{ transform: scale(1.2) rotate(180deg); opacity: 0.8; }}
        }}
        
        .footer {{
            text-align: center;
            padding: 20px;
            color: #666;
            font-size: 0.9em;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1><span class="sparkle">🐍</span> Python CGI Info Dashboard <span class="sparkle">✨</span></h1>
            <p>Comprehensive server and request information</p>
        </div>
        
        <div class="content">
            <div class="grid">
                <div class="card">
                    <h3>🖥️ System Information</h3>"""
    
    for key, value in system_info.items():
        html += f"""
                    <div class="info-item">
                        <span class="info-key">{key.replace('_', ' ').title()}</span>
                        <span class="info-value">{value}</span>
                    </div>"""
    
    html += f"""
                </div>
                
                <div class="card">
                    <h3>🌐 Request Information</h3>"""
    
    for key, value in request_info.items():
        html += f"""
                    <div class="info-item">
                        <span class="info-key">{key.replace('_', ' ').title()}</span>
                        <span class="info-value">{value}</span>
                    </div>"""
    
    html += f"""
                </div>"""
    
    if form_data:
        html += f"""
                <div class="card">
                    <h3>📝 Form Data</h3>"""
        for key, value in form_data.items():
            html += f"""
                    <div class="info-item">
                        <span class="info-key">{key}</span>
                        <span class="info-value">{value}</span>
                    </div>"""
        html += """
                </div>"""
    
    html += f"""
                <div class="card env-vars">
                    <h3>🔧 Environment Variables ({len(env_vars)} total)</h3>
                    <div class="env-grid">"""
    
    for key, value in sorted(env_vars.items()):
        html += f"""
                        <div class="env-item">
                            <div class="env-key">{key}</div>
                            <div class="env-value">{value}</div>
                        </div>"""
    
    fun_facts = [
        "🐍 Python was named after Monty Python's Flying Circus!",
        "🚀 This CGI script is running in real-time on your webserver!",
        "🎨 The theme color changes randomly on each reload!",
        "⚡ CGI stands for Common Gateway Interface!",
        "🌟 Python is used by NASA, Google, and Netflix!"
    ]
    
    html += f"""
                    </div>
                </div>
            </div>
            
            <div class="fun-facts">
                <h3>🎉 Fun Fact</h3>
                <p>{random.choice(fun_facts)}</p>
            </div>
        </div>
        
        <div class="footer">
            <p>Generated by Python CGI • {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')} • Theme: {theme_color}</p>
        </div>
    </div>
</body>
</html>"""
    
    print(html)

if __name__ == "__main__":
    main()
