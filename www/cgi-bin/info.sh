#!/bin/bash

echo "Content-Type: text/html"
echo ""

# Get system information
HOSTNAME=$(hostname)
UPTIME=$(uptime)
MEMORY=$(free -h 2>/dev/null || echo "Memory info unavailable")
DISK=$(df -h / 2>/dev/null || echo "Disk info unavailable")
PROCESSES=$(ps aux | wc -l)
LOAD=$(cat /proc/loadavg 2>/dev/null || echo "Load info unavailable")
KERNEL=$(uname -r)
ARCH=$(uname -m)

cat << 'EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MATRIX TERMINAL - ACCESS GRANTED</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Courier+Prime:wght@400;700&display=swap');
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            background: #000;
            color: #00ff00;
            font-family: 'Courier Prime', 'Courier New', monospace;
            overflow-x: hidden;
            position: relative;
        }
        
        /* Matrix rain effect */
        .matrix-bg {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: -1;
            opacity: 0.1;
        }
        
        .matrix-column {
            position: absolute;
            top: -100px;
            font-size: 14px;
            line-height: 14px;
            animation: matrix-fall linear infinite;
        }
        
        @keyframes matrix-fall {
            to {
                transform: translateY(100vh);
            }
        }
        
        .terminal {
            max-width: 1000px;
            margin: 0 auto;
            padding: 20px;
            background: rgba(0, 0, 0, 0.9);
            border: 2px solid #00ff00;
            border-radius: 10px;
            margin-top: 20px;
            box-shadow: 0 0 30px #00ff0050;
        }
        
        .terminal-header {
            border-bottom: 1px solid #00ff00;
            padding-bottom: 15px;
            margin-bottom: 20px;
            text-align: center;
        }
        
        .terminal-title {
            font-size: 24px;
            font-weight: bold;
            text-shadow: 0 0 10px #00ff00;
            animation: glow 2s ease-in-out infinite alternate;
        }
        
        @keyframes glow {
            from { text-shadow: 0 0 10px #00ff00; }
            to { text-shadow: 0 0 20px #00ff00, 0 0 30px #00ff00; }
        }
        
        .ascii-art {
            font-size: 10px;
            line-height: 10px;
            margin: 15px 0;
            text-align: center;
            color: #00aa00;
        }
        
        .prompt {
            color: #00ff00;
            margin: 10px 0;
        }
        
        .prompt::before {
            content: "root@matrix:~$ ";
            color: #00aa00;
        }
        
        .command {
            color: #ffffff;
            background: rgba(0, 255, 0, 0.1);
            padding: 2px 5px;
            border-radius: 3px;
        }
        
        .output {
            margin: 5px 0 15px 20px;
            color: #00dd00;
            white-space: pre-wrap;
        }
        
        .section {
            margin: 20px 0;
            border-left: 3px solid #00ff00;
            padding-left: 15px;
        }
        
        .section-title {
            color: #00ffff;
            font-weight: bold;
            margin-bottom: 10px;
            text-transform: uppercase;
        }
        
        .info-line {
            margin: 5px 0;
            display: flex;
            justify-content: space-between;
        }
        
        .info-key {
            color: #00aa00;
        }
        
        .info-value {
            color: #ffffff;
            text-align: right;
            max-width: 60%;
            word-break: break-all;
        }
        
        .typing {
            animation: typing 3s steps(40, end), blink-caret 0.75s step-end infinite;
            white-space: nowrap;
            overflow: hidden;
            border-right: 3px solid #00ff00;
        }
        
        @keyframes typing {
            from { width: 0; }
            to { width: 100%; }
        }
        
        @keyframes blink-caret {
            from, to { border-color: transparent; }
            50% { border-color: #00ff00; }
        }
        
        .warning {
            color: #ff0000;
            animation: blink 1s infinite;
        }
        
        @keyframes blink {
            0%, 50% { opacity: 1; }
            51%, 100% { opacity: 0; }
        }
        
        .access-granted {
            color: #00ff00;
            font-weight: bold;
            text-align: center;
            margin: 20px 0;
            font-size: 18px;
        }
        
        .footer {
            text-align: center;
            margin-top: 30px;
            padding-top: 15px;
            border-top: 1px solid #00ff00;
            color: #00aa00;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="matrix-bg" id="matrix"></div>
    
    <div class="terminal">
        <div class="terminal-header">
            <div class="terminal-title">MATRIX TERMINAL v2.1.0</div>
            <div class="ascii-art">
    ███╗   ███╗ █████╗ ████████╗██████╗ ██╗██╗  ██╗
    ████╗ ████║██╔══██╗╚══██╔══╝██╔══██╗██║╚██╗██╔╝
    ██╔████╔██║███████║   ██║   ██████╔╝██║ ╚███╔╝ 
    ██║╚██╔╝██║██╔══██║   ██║   ██╔══██╗██║ ██╔██╗ 
    ██║ ╚═╝ ██║██║  ██║   ██║   ██║  ██║██║██╔╝ ██╗
    ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝
            </div>
            <div class="access-granted typing">ACCESS GRANTED - WELCOME TO THE MATRIX</div>
        </div>
        
        <div class="prompt"><span class="command">whoami</span></div>
        <div class="output">root - SYSTEM ADMINISTRATOR</div>
        
        <div class="prompt"><span class="command">date</span></div>
        <div class="output">$(date)</div>
        
        <div class="section">
            <div class="section-title">SYSTEM STATUS</div>
            <div class="prompt"><span class="command">hostname</span></div>
            <div class="output">$HOSTNAME</div>
            
            <div class="prompt"><span class="command">uname -a</span></div>
            <div class="output">$(uname -a)</div>
            
            <div class="prompt"><span class="command">uptime</span></div>
            <div class="output">$UPTIME</div>
        </div>
        
        <div class="section">
            <div class="section-title">MEMORY & STORAGE</div>
            <div class="prompt"><span class="command">free -h</span></div>
            <div class="output">$MEMORY</div>
            
            <div class="prompt"><span class="command">df -h /</span></div>
            <div class="output">$DISK</div>
        </div>
        
        <div class="section">
            <div class="section-title">PROCESS INFORMATION</div>
            <div class="prompt"><span class="command">ps aux | wc -l</span></div>
            <div class="output">Total processes: $PROCESSES</div>
            
            <div class="prompt"><span class="command">cat /proc/loadavg</span></div>
            <div class="output">$LOAD</div>
        </div>
        
        <div class="section">
            <div class="section-title">ENVIRONMENT VARIABLES</div>
            <div class="prompt"><span class="command">env | grep -E "(REQUEST|SERVER|HTTP)"</span></div>
            <div class="output">
EOF

# Output environment variables
env | grep -E "(REQUEST|SERVER|HTTP|SCRIPT|QUERY|CONTENT)" | while read line; do
    echo "$line"
done

cat << 'EOF'
            </div>
        </div>
        
        <div class="section">
            <div class="section-title">NETWORK STATUS</div>
            <div class="prompt"><span class="command">netstat -tuln | head -10</span></div>
            <div class="output">
EOF

# Network information (if available)
netstat -tuln 2>/dev/null | head -10 || echo "Network information unavailable"

cat << 'EOF'
            </div>
        </div>
        
        <div class="section">
            <div class="section-title">SECURITY ALERT</div>
            <div class="warning">⚠️  UNAUTHORIZED ACCESS DETECTED ⚠️</div>
            <div class="output">
Connection from: $REMOTE_ADDR
User Agent: $HTTP_USER_AGENT
Time: $(date)
Status: MONITORING...
            </div>
        </div>
        
        <div class="prompt"><span class="command">echo "Welcome to the Matrix, Neo..."</span></div>
        <div class="output">Welcome to the Matrix, Neo...</div>
        
        <div class="footer">
            MATRIX TERMINAL - SECURE CONNECTION ESTABLISHED<br>
            Session ID: $(date +%s) | Encryption: AES-256 | Status: ACTIVE
        </div>
    </div>
    
    <script>
        // Matrix rain effect
        function createMatrixRain() {
            const matrix = document.getElementById('matrix');
            const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()_+-=[]{}|;:,.<>?';
            
            for (let i = 0; i < 50; i++) {
                const column = document.createElement('div');
                column.className = 'matrix-column';
                column.style.left = Math.random() * 100 + '%';
                column.style.animationDuration = (Math.random() * 3 + 2) + 's';
                column.style.animationDelay = Math.random() * 2 + 's';
                
                let text = '';
                for (let j = 0; j < 20; j++) {
                    text += chars[Math.floor(Math.random() * chars.length)] + '<br>';
                }
                column.innerHTML = text;
                
                matrix.appendChild(column);
            }
        }
        
        // Terminal typing effect
        function typeWriter(element, text, speed = 50) {
            let i = 0;
            element.innerHTML = '';
            function type() {
                if (i < text.length) {
                    element.innerHTML += text.charAt(i);
                    i++;
                    setTimeout(type, speed);
                }
            }
            type();
        }
        
        // Initialize effects
        document.addEventListener('DOMContentLoaded', function() {
            createMatrixRain();
            
            // Add random glitch effect
            setInterval(() => {
                const elements = document.querySelectorAll('.output');
                const randomElement = elements[Math.floor(Math.random() * elements.length)];
                if (randomElement) {
                    randomElement.style.textShadow = '2px 0 #ff0000';
                    setTimeout(() => {
                        randomElement.style.textShadow = 'none';
                    }, 100);
                }
            }, 3000);
        });
    </script>
</body>
</html>
EOF
