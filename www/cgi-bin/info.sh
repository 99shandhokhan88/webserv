#!/bin/bash

# Genera un colore primario casuale per il tema
colors=("#E57373" "#F06292" "#BA68C8" "#9575CD" "#7986CB" "#64B5F6" "#4FC3F7" "#4DD0E1" "#4DB6AC" "#81C784" "#AED581" "#FF8A65" "#A1887F" "#90A4AE")
primary_color=${colors[$RANDOM % ${#colors[@]}]}

# Genera colori per il tema basati sul colore primario
secondary_color=$(echo "$primary_color" | sed 's/#\(..\)\(..\)\(..\)/#\1\2\3/')
accent_color=${colors[$RANDOM % ${#colors[@]}]}
base_color="#FAFAFA"

# Raccolta informazioni di sistema
get_system_info() {
  echo "<div class=\"info-item\"><span class=\"info-label\">Bash Version:</span> <span class=\"info-value\">$BASH_VERSION</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Kernel:</span> <span class=\"info-value\">$(uname -r)</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Sistema:</span> <span class=\"info-value\">$(uname -s)</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Hostname:</span> <span class=\"info-value\">$(hostname)</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">CPU:</span> <span class=\"info-value\">$(grep -m 1 "model name" /proc/cpuinfo | cut -d: -f2 | sed 's/^[ \t]*//')</span></div>"
  
  # Memoria
  mem_total=$(grep MemTotal /proc/meminfo | awk '{print $2}')
  mem_free=$(grep MemFree /proc/meminfo | awk '{print $2}')
  mem_available=$(grep MemAvailable /proc/meminfo | awk '{print $2}')
  
  if [[ -n "$mem_total" && -n "$mem_available" ]]; then
    mem_used=$((mem_total - mem_available))
    mem_percent=$((mem_used * 100 / mem_total))
    
    echo "<div class=\"info-item\"><span class=\"info-label\">Memoria Totale:</span> <span class=\"info-value\">$(echo "scale=2; $mem_total/1024/1024" | bc) GB</span></div>"
    echo "<div class=\"info-item\"><span class=\"info-label\">Memoria Usata:</span>"
    echo "<div class=\"progress-container\">"
    echo "<div class=\"progress-bar\" style=\"width: ${mem_percent}%\"></div>"
    echo "</div>"
    echo "<span class=\"info-value\">${mem_percent}%</span></div>"
  fi
  
  # Disco
  df_output=$(df -h / | tail -n 1)
  disk_size=$(echo "$df_output" | awk '{print $2}')
  disk_used=$(echo "$df_output" | awk '{print $3}')
  disk_avail=$(echo "$df_output" | awk '{print $4}')
  disk_percent=$(echo "$df_output" | awk '{print $5}' | sed 's/%//')
  
  echo "<div class=\"info-item\"><span class=\"info-label\">Spazio Disco:</span> <span class=\"info-value\">$disk_size</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Disco Usato:</span>"
  echo "<div class=\"progress-container\">"
  echo "<div class=\"progress-bar\" style=\"width: ${disk_percent}%\"></div>"
  echo "</div>"
  echo "<span class=\"info-value\">${disk_percent}%</span></div>"
  
  # Uptime
  uptime_output=$(uptime)
  uptime_days=$(echo "$uptime_output" | awk -F'( |,|:)+' '{if ($4=="day" || $4=="days") print $3; else print "0"}')
  
  echo "<div class=\"info-item\"><span class=\"info-label\">Uptime:</span> <span class=\"info-value\">$uptime_days giorni $(uptime -p | sed 's/up //')</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Data:</span> <span class=\"info-value\">$(date '+%d/%m/%Y %H:%M:%S')</span></div>"
}

# Raccolta informazioni sulla richiesta
get_request_info() {
  echo "<div class=\"info-item\"><span class=\"info-label\">Method:</span> <span class=\"badge\">${REQUEST_METHOD:-N/A}</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Protocol:</span> <span class=\"info-value\">${SERVER_PROTOCOL:-N/A}</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Server Name:</span> <span class=\"info-value\">${SERVER_NAME:-N/A}</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Server Port:</span> <span class=\"info-value\">${SERVER_PORT:-N/A}</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Remote Address:</span> <span class=\"info-value\">${REMOTE_ADDR:-N/A}</span></div>"
  echo "<div class=\"info-item\"><span class=\"info-label\">Query String:</span> <span class=\"info-value\">${QUERY_STRING:-N/A}</span></div>"
}

# Raccolta variabili d'ambiente pertinenti
get_environment_variables() {
  env | sort | grep -E "^(HTTP_|CONTENT_|REMOTE_|REQUEST_|QUERY_|SCRIPT_)" | while read -r line; do
    key="${line%%=*}"
    value="${line#*=}"
    echo "<div class=\"info-item\"><span class=\"info-label\">$key:</span> <span class=\"info-value\">$value</span></div>"
  done
}

# Inizio output HTML
echo "Content-type: text/html"
echo ""

cat << HTML
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Bash CGI Info | Webserv</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
    <style>
        :root {
            --primary-color: ${primary_color};
            --secondary-color: ${secondary_color};
            --accent-color: ${accent_color};
            --base-color: ${base_color};
            --text-color: #333;
            --text-light: #777;
            --white: #fff;
            --border-radius: 12px;
            --box-shadow: 0 8px 30px rgba(0, 0, 0, 0.1);
            --transition: all 0.3s ease;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        @keyframes slideInLeft {
            from { opacity: 0; transform: translateX(-50px); }
            to { opacity: 1; transform: translateX(0); }
        }
        
        @keyframes terminalType {
            from { width: 0; }
            to { width: 100%; }
        }
        
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
        
        @keyframes rotate {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }
        
        @keyframes blink {
            0%, 100% { opacity: 1; }
            50% { opacity: 0; }
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--base-color);
            color: var(--text-color);
            line-height: 1.6;
            padding: 0;
            margin: 0;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        header {
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            color: var(--white);
            padding: 40px 0;
            text-align: center;
            position: relative;
            overflow: hidden;
            margin-bottom: 40px;
            border-radius: 0 0 var(--border-radius) var(--border-radius);
            box-shadow: var(--box-shadow);
            animation: fadeIn 0.6s ease-out;
        }
        
        .header-content {
            position: relative;
            z-index: 1;
        }
        
        .terminal-header {
            display: flex;
            align-items: center;
            justify-content: center;
            margin-bottom: 20px;
        }
        
        .terminal-icon {
            font-size: 3rem;
            margin-right: 15px;
            animation: pulse 2s infinite;
        }
        
        .title {
            font-size: 2.5rem;
            font-weight: 700;
        }
        
        .terminal-window {
            background: rgba(0, 0, 0, 0.7);
            border-radius: 8px;
            padding: 12px;
            margin: 20px auto;
            max-width: 600px;
            font-family: 'Courier New', Courier, monospace;
            position: relative;
        }
        
        .terminal-buttons {
            position: absolute;
            top: 10px;
            left: 10px;
            display: flex;
        }
        
        .terminal-button {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        
        .terminal-button.red { background-color: #ff5f56; }
        .terminal-button.yellow { background-color: #ffbd2e; }
        .terminal-button.green { background-color: #27c93f; }
        
        .terminal-text {
            color: #ddd;
            padding-top: 15px;
            overflow: hidden;
            white-space: nowrap;
            animation: terminalType 3s steps(60, end);
        }
        
        .terminal-text::after {
            content: '|';
            animation: blink 1s infinite;
        }
        
        .pattern {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            opacity: 0.1;
            background-image: repeating-linear-gradient(
                45deg, 
                rgba(255, 255, 255, 0.1), 
                rgba(255, 255, 255, 0.1) 10px, 
                transparent 10px, 
                transparent 20px
            );
        }
        
        .cards {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 30px;
            margin-bottom: 40px;
        }
        
        .card {
            background: var(--white);
            border-radius: var(--border-radius);
            box-shadow: var(--box-shadow);
            padding: 30px;
            transition: var(--transition);
            position: relative;
            overflow: hidden;
            opacity: 0;
            animation: fadeIn 0.5s ease-out forwards;
        }
        
        .card:nth-child(1) { animation-delay: 0.1s; }
        .card:nth-child(2) { animation-delay: 0.3s; }
        .card:nth-child(3) { animation-delay: 0.5s; }
        
        .card:hover {
            transform: translateY(-10px);
            box-shadow: 0 15px 30px rgba(0, 0, 0, 0.15);
        }
        
        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            height: 100%;
            width: 5px;
            background: var(--primary-color);
            border-radius: var(--border-radius) 0 0 var(--border-radius);
        }
        
        .card-header {
            display: flex;
            align-items: center;
            margin-bottom: 25px;
            animation: slideInLeft 0.5s ease-out;
        }
        
        .card-icon {
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
        }
        
        .card-title {
            font-size: 1.5rem;
            color: var(--primary-color);
            font-weight: 600;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: 1fr;
            gap: 15px;
        }
        
        .info-item {
            display: grid;
            grid-template-columns: 150px 1fr;
            gap: 15px;
            align-items: center;
        }
        
        .info-label {
            color: var(--text-light);
            font-weight: 500;
            position: relative;
            padding-left: 20px;
        }
        
        .info-label::before {
            content: '●';
            position: absolute;
            left: 0;
            color: var(--accent-color);
            font-size: 0.8rem;
            top: 5px;
        }
        
        .info-value {
            color: var(--text-color);
            font-weight: 600;
        }
        
        .badge {
            background: var(--primary-color);
            color: var(--white);
            padding: 5px 12px;
            border-radius: 30px;
            font-size: 0.75rem;
            font-weight: 600;
            display: inline-block;
        }
        
        .progress-container {
            width: 100%;
            height: 10px;
            background: #f1f1f1;
            border-radius: 5px;
            overflow: hidden;
            margin: 5px 0;
        }
        
        .progress-bar {
            height: 100%;
            background: linear-gradient(to right, var(--primary-color), var(--accent-color));
            border-radius: 5px;
        }
        
        .buttons {
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            gap: 15px;
            margin-top: 40px;
            animation: fadeIn 0.8s ease-out;
        }
        
        .btn {
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
        }
        
        .btn:hover {
            transform: translateY(-5px);
            box-shadow: 0 6px 15px rgba(0,0,0,0.2);
            background: var(--secondary-color);
        }
        
        .btn i {
            margin-right: 8px;
        }
        
        footer {
            text-align: center;
            padding: 30px 0;
            color: var(--text-light);
            font-size: 0.9rem;
            margin-top: 30px;
            border-top: 1px solid rgba(0,0,0,0.1);
        }
        
        .copyright {
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <header>
        <div class="pattern"></div>
        <div class="header-content">
            <div class="terminal-header">
                <div class="terminal-icon">🖥️</div>
                <h1 class="title">Bash CGI Info</h1>
            </div>
            
            <div class="terminal-window">
                <div class="terminal-buttons">
                    <div class="terminal-button red"></div>
                    <div class="terminal-button yellow"></div>
                    <div class="terminal-button green"></div>
                </div>
                <div class="terminal-text">
                    $ ./webserv --display-info | Esecuzione script Bash CGI in corso...
                </div>
            </div>
        </div>
    </header>

    <div class="container">
        <div class="cards">
            <div class="card">
                <div class="card-header">
                    <div class="card-icon">
                        <i class="fas fa-microchip"></i>
                    </div>
                    <h2 class="card-title">Informazioni Sistema</h2>
                </div>
                <div class="info-grid">
                    $(get_system_info)
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
                    $(get_request_info)
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
                    $(get_environment_variables)
                </div>
            </div>
        </div>
        
        <div class="buttons">
            <a href="/cgi-bin/info.py" class="btn"><i class="fab fa-python"></i> Python</a>
            <a href="/cgi-bin/info.php" class="btn"><i class="fab fa-php"></i> PHP</a>
            <a href="/cgi-bin/info.cgi" class="btn"><i class="fas fa-file-code"></i> C++</a>
            <a href="/cgi-bin/info.ts" class="btn"><i class="fab fa-js"></i> TypeScript</a>
            <a href="/index.html" class="btn"><i class="fas fa-home"></i> Home</a>
        </div>
    </div>
    
    <footer>
        <p>Webserv CGI Implementation con Bash</p>
        <p class="copyright">&copy; $(date +%Y) - Tutti i diritti riservati</p>
    </footer>
</body>
</html>
HTML
</rewritten_file>