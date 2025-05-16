#!/usr/bin/env python3
import os
import sys
import cgi
import json
import platform
import socket
import datetime
import psutil
import random
from urllib.parse import parse_qs

def get_system_info():
    """Raccoglie informazioni sul sistema"""
    try:
        # Informazioni di base
        system_info = {
            "Python Version": platform.python_version(),
            "System": platform.system(),
            "Node": platform.node(),
            "Release": platform.release(),
            "Version": platform.version(),
            "Machine": platform.machine(),
            "Processor": platform.processor()
        }
        
        # Informazioni avanzate (se psutil è disponibile)
        try:
            # Memoria
            mem = psutil.virtual_memory()
            system_info["Memory Total"] = f"{mem.total / (1024 * 1024 * 1024):.2f} GB"
            system_info["Memory Available"] = f"{mem.available / (1024 * 1024 * 1024):.2f} GB"
            system_info["Memory Usage"] = f"{mem.percent}%"
            
            # CPU
            system_info["CPU Cores"] = psutil.cpu_count(logical=False)
            system_info["CPU Threads"] = psutil.cpu_count(logical=True)
            system_info["CPU Usage"] = f"{psutil.cpu_percent()}%"
            
            # Disco
            disk = psutil.disk_usage('/')
            system_info["Disk Total"] = f"{disk.total / (1024 * 1024 * 1024):.2f} GB"
            system_info["Disk Used"] = f"{disk.used / (1024 * 1024 * 1024):.2f} GB"
            system_info["Disk Free"] = f"{disk.free / (1024 * 1024 * 1024):.2f} GB"
            
            # Rete
            addresses = psutil.net_if_addrs()
            for interface, addrs in addresses.items():
                for addr in addrs:
                    if addr.family == socket.AF_INET:
                        system_info[f"Network ({interface})"] = addr.address
        except (ImportError, AttributeError):
            # psutil non disponibile, continua senza informazioni avanzate
            pass
            
        return system_info
    except Exception as e:
        return {"Error": str(e)}

def get_request_info():
    """Raccoglie informazioni sulla richiesta HTTP"""
    request_info = {
        "Method": os.environ.get("REQUEST_METHOD", "N/A"),
        "Protocol": os.environ.get("SERVER_PROTOCOL", "N/A"),
        "Server Name": os.environ.get("SERVER_NAME", "N/A"),
        "Server Port": os.environ.get("SERVER_PORT", "N/A"),
        "Remote Address": os.environ.get("REMOTE_ADDR", "N/A"),
        "Query String": os.environ.get("QUERY_STRING", "N/A")
    }
    return request_info

def get_environment_variables():
    """Raccoglie variabili d'ambiente rilevanti"""
    env_vars = {}
    for key, value in os.environ.items():
        if key.startswith("HTTP_") or key in [
            "REMOTE_ADDR", "REQUEST_METHOD", "QUERY_STRING",
            "CONTENT_TYPE", "CONTENT_LENGTH"
        ]:
            env_vars[key] = value
    return env_vars

def get_color_scheme():
    """Genera uno schema di colori per la pagina"""
    themes = [
        {"primary": "#3F51B5", "secondary": "#303F9F", "accent": "#FF4081", "base": "#BBDEFB"},  # Indigo
        {"primary": "#2196F3", "secondary": "#1976D2", "accent": "#FF5722", "base": "#E3F2FD"},  # Blue
        {"primary": "#009688", "secondary": "#00796B", "accent": "#FFC107", "base": "#E0F2F1"},  # Teal
        {"primary": "#673AB7", "secondary": "#512DA8", "accent": "#4CAF50", "base": "#D1C4E9"},  # Deep Purple
        {"primary": "#F44336", "secondary": "#D32F2F", "accent": "#03A9F4", "base": "#FFEBEE"}   # Red
    ]
    return random.choice(themes)

try:
    # Ottieni informazioni
    system_info = get_system_info()
    request_info = get_request_info()
    env_vars = get_environment_variables()
    color_scheme = get_color_scheme()
    
    # Inizio output HTML
    print("Content-Type: text/html; charset=UTF-8\r\n")
    print(f"""<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Info | Webserv</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
    <style>
        :root {{
            --primary-color: {color_scheme["primary"]};
            --secondary-color: {color_scheme["secondary"]};
            --accent-color: {color_scheme["accent"]};
            --base-color: {color_scheme["base"]};
            --text-color: #333;
            --text-light: #666;
            --text-dark: #000;
            --white: #fff;
            --border-radius: 12px;
            --card-shadow: 0 8px 20px rgba(0, 0, 0, 0.1);
            --transition: all 0.3s ease;
        }}
        
        @keyframes fadeInUp {{
            from {{ opacity: 0; transform: translateY(20px); }}
            to {{ opacity: 1; transform: translateY(0); }}
        }}
        
        @keyframes slideInLeft {{
            from {{ opacity: 0; transform: translateX(-50px); }}
            to {{ opacity: 1; transform: translateX(0); }}
        }}
        
        @keyframes pulse {{
            0% {{ transform: scale(1); }}
            50% {{ transform: scale(1.05); }}
            100% {{ transform: scale(1); }}
        }}
        
        @keyframes floatingIcons {{
            0% {{ transform: translateY(0px); }}
            50% {{ transform: translateY(-15px); }}
            100% {{ transform: translateY(0px); }}
        }}
        
        @keyframes rotate {{
            from {{ transform: rotate(0deg); }}
            to {{ transform: rotate(360deg); }}
        }}
        
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        
        body {{
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            background-color: var(--base-color);
            color: var(--text-color);
            padding: 0;
            margin: 0;
            overflow-x: hidden;
        }}
        
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }}
        
        header {{
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            color: var(--white);
            padding: 40px 0;
            text-align: center;
            border-radius: 0 0 var(--border-radius) var(--border-radius);
            box-shadow: var(--card-shadow);
            margin-bottom: 40px;
            position: relative;
            overflow: hidden;
            animation: fadeInUp 0.6s ease-out;
        }}
        
        header::before {{
            content: '';
            position: absolute;
            top: -20px;
            left: -20px;
            right: -20px;
            bottom: -20px;
            background: radial-gradient(circle at center, rgba(255,255,255,0.1) 0%, rgba(255,255,255,0) 70%);
            z-index: 0;
            animation: pulse 4s infinite;
        }}
        
        .header-content {{
            position: relative;
            z-index: 1;
        }}
        
        .header-title {{
            font-size: 3.5rem;
            margin-bottom: 10px;
            font-weight: 700;
            letter-spacing: -1px;
            display: flex;
            align-items: center;
            justify-content: center;
        }}
        
        .python-logo {{
            font-size: 3.5rem;
            margin-right: 15px;
            display: inline-block;
            animation: floatingIcons 3s ease-in-out infinite;
        }}
        
        .tagline {{
            font-size: 1.3rem;
            opacity: 0.9;
            max-width: 600px;
            margin: 0 auto;
        }}
        
        .floating-icons {{
            position: absolute;
            width: 100%;
            height: 100%;
            top: 0;
            left: 0;
            z-index: 0;
        }}
        
        .floating-icon {{
            position: absolute;
            color: rgba(255,255,255,0.3);
            z-index: 0;
        }}
        
        .floating-icon:nth-child(1) {{ top: 15%; left: 10%; font-size: 1.5rem; animation: floatingIcons 5s ease-in-out infinite; }}
        .floating-icon:nth-child(2) {{ top: 25%; right: 15%; font-size: 2rem; animation: floatingIcons 6s ease-in-out infinite 0.5s; }}
        .floating-icon:nth-child(3) {{ bottom: 20%; left: 20%; font-size: 1.8rem; animation: floatingIcons 7s ease-in-out infinite 1s; }}
        .floating-icon:nth-child(4) {{ bottom: 30%; right: 25%; font-size: 1.4rem; animation: floatingIcons 4s ease-in-out infinite 1.5s; }}
        
        .cards {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 30px;
            margin-bottom: 40px;
        }}
        
        .card {{
            background: var(--white);
            border-radius: var(--border-radius);
            box-shadow: var(--card-shadow);
            padding: 30px;
            transition: var(--transition);
            position: relative;
            overflow: hidden;
            opacity: 0;
            animation: fadeInUp 0.5s ease-out forwards;
        }}
        
        .card:nth-child(1) {{ animation-delay: 0.1s; }}
        .card:nth-child(2) {{ animation-delay: 0.3s; }}
        .card:nth-child(3) {{ animation-delay: 0.5s; }}
        
        .card:hover {{
            transform: translateY(-10px);
            box-shadow: 0 15px 30px rgba(0, 0, 0, 0.15);
        }}
        
        .card::before {{
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            height: 100%;
            width: 5px;
            background: var(--primary-color);
            border-radius: var(--border-radius) 0 0 var(--border-radius);
        }}
        
        .card-header {{
            display: flex;
            align-items: center;
            margin-bottom: 25px;
            animation: slideInLeft 0.5s ease-out;
        }}
        
        .card-icon {{
            width: 50px;
            height: 50px;
            background: var(--accent-color);
            color: var(--white);
            display: flex;
            align-items: center;
            justify-content: center;
            border-radius: 50%;
            margin-right: 15px;
            font-size: 1.5rem;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
        }}
        
        .card-title {{
            font-size: 1.5rem;
            color: var(--primary-color);
            font-weight: 600;
        }}
        
        .info-grid {{
            display: grid;
            grid-template-columns: auto 1fr;
            gap: 15px;
            align-items: center;
        }}
        
        .info-label {{
            color: var(--text-light);
            font-weight: 500;
            position: relative;
            padding-left: 20px;
        }}
        
        .info-label::before {{
            content: '●';
            position: absolute;
            left: 0;
            color: var(--accent-color);
            font-size: 0.8rem;
            top: 5px;
        }}
        
        .info-value {{
            color: var(--text-dark);
            font-weight: 600;
        }}
        
        .badge {{
            background: var(--primary-color);
            color: var(--white);
            padding: 5px 12px;
            border-radius: 30px;
            font-size: 0.75rem;
            font-weight: 600;
            display: inline-block;
        }}
        
        .loading-bar {{
            height: 4px;
            width: 100%;
            background: rgba(255,255,255,0.2);
            margin-top: 20px;
            border-radius: 10px;
            position: relative;
            overflow: hidden;
        }}
        
        .loading-bar::after {{
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            height: 100%;
            width: 20%;
            background: rgba(255,255,255,0.8);
            border-radius: 10px;
            animation: loading 2s infinite;
        }}
        
        @keyframes loading {{
            0% {{ left: -20%; }}
            100% {{ left: 100%; }}
        }}
        
        .buttons {{
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            gap: 15px;
            margin-top: 40px;
            animation: fadeInUp 0.8s ease-out;
        }}
        
        .btn {{
            background: var(--primary-color);
            color: var(--white);
            padding: 12px 24px;
            border: none;
            border-radius: 30px;
            font-size: 0.9rem;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 1px;
            cursor: pointer;
            text-decoration: none;
            transition: var(--transition);
            display: inline-flex;
            align-items: center;
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }}
        
        .btn:hover {{
            transform: translateY(-5px);
            box-shadow: 0 6px 15px rgba(0,0,0,0.2);
            background: var(--secondary-color);
        }}
        
        .btn i {{
            margin-right: 8px;
        }}
        
        footer {{
            text-align: center;
            padding: 30px 0;
            color: var(--text-light);
            font-size: 0.9rem;
            margin-top: 30px;
            border-top: 1px solid rgba(0,0,0,0.1);
        }}
        
        .copyright {{
            margin-top: 10px;
        }}
        
        .progress-container {{
            width: 100%;
            margin: 10px 0;
            background: #f1f1f1;
            border-radius: 10px;
            overflow: hidden;
        }}
        
        .progress-bar {{
            height: 10px;
            border-radius: 10px;
            background: linear-gradient(to right, var(--primary-color), var(--accent-color));
        }}
    </style>
</head>
<body>
    <header>
        <div class="floating-icons">
            <i class="floating-icon fas fa-code"></i>
            <i class="floating-icon fas fa-server"></i>
            <i class="floating-icon fas fa-database"></i>
            <i class="floating-icon fas fa-laptop-code"></i>
        </div>
        <div class="header-content">
            <h1 class="header-title">
                <span class="python-logo">🐍</span> Python CGI Info
            </h1>
            <p class="tagline">Informazioni dettagliate sul server e sull'ambiente di esecuzione Python CGI.</p>
            <div class="loading-bar"></div>
        </div>
    </header>
    
    <div class="container">
        <div class="cards">
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">
                        <i class="fas fa-server"></i>
                    </div>
                    <h2 class="card-title">Informazioni Sistema</h2>
                </div>
                <div class="info-grid">
                    {system_info_html}
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">
                        <i class="fas fa-exchange-alt"></i>
                    </div>
                    <h2 class="card-title">Dettagli Richiesta</h2>
                </div>
                <div class="info-grid">
                    {request_info_html}
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">
                        <i class="fas fa-cogs"></i>
                    </div>
                    <h2 class="card-title">Variabili Ambiente</h2>
                </div>
                <div class="info-grid">
                    {env_vars_html}
                </div>
            </div>
        </div>
        
        <div class="buttons">
            <a href="/cgi-bin/info.php" class="btn"><i class="fab fa-php"></i> PHP</a>
            <a href="/cgi-bin/info.sh" class="btn"><i class="fas fa-terminal"></i> Shell</a>
            <a href="/cgi-bin/info.cgi" class="btn"><i class="fas fa-file-code"></i> C++</a>
            <a href="/cgi-bin/info.ts" class="btn"><i class="fab fa-js"></i> TypeScript</a>
            <a href="/index.html" class="btn"><i class="fas fa-home"></i> Home</a>
        </div>
    </div>
    
    <footer>
        <p>Webserv CGI Implementation con Python</p>
        <p class="copyright">&copy; {datetime.datetime.now().year} - Tutti i diritti riservati</p>
    </footer>
</body>
</html>""")

except Exception as e:
    # Gestione degli errori
    print("Content-Type: text/html; charset=UTF-8\r\n")
    print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Python CGI Error</title>
    <style>
        body {{ 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            padding: 20px;
            margin: 0;
            background: #f0f2f5;
            color: #333;
        }}
        .container {{
            max-width: 800px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }}
        .error-box {{
            background: #ffebee;
            color: #f44336;
            padding: 15px;
            border-radius: 4px;
            border-left: 4px solid #f44336;
            margin: 10px 0;
        }}
        pre {{
            background: #f5f5f5;
            padding: 15px;
            border-radius: 4px;
            overflow-x: auto;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Python CGI Error</h1>
        <div class="error-box">
            <p>Si è verificato un errore durante l'esecuzione dello script:</p>
            <p>{str(e)}</p>
        </div>
        <h2>Traceback</h2>
        <pre>{traceback.format_exc()}</pre>
    </div>
</body>
</html>""")
    sys.exit(1)

# Formato le informazioni per l'output HTML
def format_html_output(data_dict):
    html = ""
    for key, value in data_dict.items():
        if key == "Method":
            html += f"""
                <div class="info-label">{key}:</div>
                <div class="info-value"><span class="badge">{value}</span></div>
            """
        elif key in ["CPU Usage", "Memory Usage"]:
            percentage = int(value.strip('%'))
            html += f"""
                <div class="info-label">{key}:</div>
                <div class="info-value">
                    <div class="progress-container">
                        <div class="progress-bar" style="width: {percentage}%;"></div>
                    </div>
                    {value}
                </div>
            """
        else:
            html += f"""
                <div class="info-label">{key}:</div>
                <div class="info-value">{value}</div>
            """
    return html

# Genero l'HTML per ciascuna sezione
system_info_html = format_html_output(system_info)
request_info_html = format_html_output(request_info)
env_vars_html = format_html_output(env_vars) 