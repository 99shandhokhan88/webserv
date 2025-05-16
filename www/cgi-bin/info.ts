#!/home/vzashev/.nvm/versions/node/v16.20.2/bin/ts-node

// Interfacce per la tipizzazione
interface SystemInfo {
    nodeVersion: string;
    platform: string;
    architecture: string;
    memoryUsage: string;
    uptime: string;
    time: string;
}

interface RequestInfo {
    method: string;
    protocol: string;
    serverName: string;
    serverPort: string;
    remoteAddr: string;
    queryString: string;
}

// Funzioni per raccogliere informazioni
function getSystemInfo(): SystemInfo {
    const memoryUsageMB = Math.round(process.memoryUsage().rss / 1024 / 1024);
    const uptimeHours = Math.floor(process.uptime() / 3600);
    const uptimeMinutes = Math.floor((process.uptime() % 3600) / 60);
    const uptimeSeconds = Math.floor(process.uptime() % 60);
    
    return {
        nodeVersion: process.version,
        platform: process.platform,
        architecture: process.arch,
        memoryUsage: `${memoryUsageMB} MB`,
        uptime: `${uptimeHours}h ${uptimeMinutes}m ${uptimeSeconds}s`,
        time: new Date().toLocaleString()
    };
}

function getRequestInfo(): RequestInfo {
    return {
        method: process.env.REQUEST_METHOD || 'GET',
        protocol: process.env.SERVER_PROTOCOL || 'HTTP/1.1',
        serverName: process.env.SERVER_NAME || 'localhost',
        serverPort: process.env.SERVER_PORT || '8080',
        remoteAddr: process.env.REMOTE_ADDR || '127.0.0.1',
        queryString: process.env.QUERY_STRING || ''
    };
}

function getEnvironmentVariables(): Record<string, string> {
    const filtered: Record<string, string> = {};
    for (const [key, value] of Object.entries(process.env)) {
        if (key.startsWith('HTTP_') || ['REMOTE_ADDR', 'REQUEST_METHOD', 'QUERY_STRING'].includes(key)) {
            if (value) {
                filtered[key] = value;
            }
        }
    }
    return filtered;
}

// Genera un colore casuale per il tema
const colors = ['#3178c6', '#61dafb', '#764abc', '#ff584f', '#41b883'];
const themeColor = colors[Math.floor(Math.random() * colors.length)];

// Inizia l'output
console.log('Content-type: text/html\r\n');

const systemInfo = getSystemInfo();
const requestInfo = getRequestInfo();
const envVariables = getEnvironmentVariables();

console.log(`<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TypeScript CGI Info | Webserv</title>
    <style>
        :root {
            --primary-color: ${themeColor};
            --secondary-color: #333;
            --background-color: #f9f9f9;
            --card-background: #ffffff;
            --text-color: #333;
            --border-radius: 10px;
            --box-shadow: 0 4px 20px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        @keyframes float {
            0% { transform: translateY(0px); }
            50% { transform: translateY(-10px); }
            100% { transform: translateY(0px); }
        }
        
        @keyframes slideInRight {
            from { transform: translateX(50px); opacity: 0; }
            to { transform: translateX(0); opacity: 1; }
        }
        
        @keyframes rotate {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }
        
        @keyframes typing {
            from { width: 0 }
            to { width: 100% }
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: var(--background-color);
            color: var(--text-color);
            margin: 0;
            padding: 0;
            transition: var(--transition);
            overflow-x: hidden;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        header {
            background: var(--primary-color);
            color: white;
            padding: 30px 0;
            text-align: center;
            position: relative;
            overflow: hidden;
            animation: fadeIn 0.8s ease-out;
        }
        
        header::before, header::after {
            content: '';
            position: absolute;
            width: 300px;
            height: 300px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 50%;
        }
        
        header::before {
            top: -100px;
            left: -100px;
            animation: float 6s infinite;
        }
        
        header::after {
            bottom: -100px;
            right: -100px;
            animation: float 8s infinite;
        }
        
        .content {
            position: relative;
            z-index: 1;
        }
        
        h1 {
            margin: 0;
            font-size: 3em;
            font-weight: 700;
            letter-spacing: -1px;
        }
        
        .ts-icon {
            display: inline-block;
            animation: rotate 10s linear infinite;
            font-size: 1.2em;
        }
        
        header p {
            margin: 10px 0 0;
            font-size: 1.2em;
            opacity: 0.9;
            overflow: hidden;
            white-space: nowrap;
            animation: typing 3.5s steps(40, end);
        }
        
        .main {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 20px;
            padding: 30px 0;
        }
        
        .card {
            background: var(--card-background);
            border-radius: var(--border-radius);
            box-shadow: var(--box-shadow);
            padding: 25px;
            transition: var(--transition);
            animation: fadeIn 0.5s ease-out forwards;
            opacity: 0;
            position: relative;
            overflow: hidden;
        }
        
        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 5px;
            height: 100%;
            background: var(--primary-color);
            opacity: 0.6;
        }
        
        .card:nth-child(1) { animation-delay: 0.1s; }
        .card:nth-child(2) { animation-delay: 0.2s; }
        .card:nth-child(3) { animation-delay: 0.3s; }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 30px rgba(0,0,0,0.15);
        }
        
        .card h2 {
            color: var(--primary-color);
            margin-top: 0;
            display: flex;
            align-items: center;
            position: relative;
            animation: slideInRight 0.5s ease-out;
        }
        
        .card h2 .icon {
            margin-right: 10px;
            font-size: 1.2em;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: auto 1fr;
            gap: 15px 20px;
            margin-top: 20px;
        }
        
        .info-label {
            font-weight: 600;
            color: var(--secondary-color);
            position: relative;
            padding-left: 15px;
        }
        
        .info-label::before {
            content: '●';
            position: absolute;
            left: 0;
            color: var(--primary-color);
            font-size: 0.7em;
            top: 5px;
        }
        
        .info-value {
            color: var(--primary-color);
        }
        
        .badge {
            display: inline-block;
            padding: 5px 10px;
            border-radius: 20px;
            background: var(--primary-color);
            color: white;
            font-size: 0.8em;
            font-weight: 600;
        }
        
        footer {
            text-align: center;
            padding: 20px;
            margin-top: 30px;
            color: #888;
            font-size: 0.9em;
            border-top: 1px solid #eee;
        }
        
        /* Pulsanti interattivi */
        .buttons {
            display: flex;
            justify-content: center;
            gap: 15px;
            margin-top: 30px;
            flex-wrap: wrap;
        }
        
        .btn {
            padding: 10px 20px;
            background: var(--primary-color);
            color: white;
            border: none;
            border-radius: 30px;
            font-weight: 600;
            text-decoration: none;
            text-transform: uppercase;
            letter-spacing: 1px;
            font-size: 0.8em;
            transition: var(--transition);
            box-shadow: 0 4px 10px rgba(0,0,0,0.1);
            position: relative;
            overflow: hidden;
            z-index: 1;
        }
        
        .btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: rgba(255,255,255,0.2);
            transition: all 0.3s;
            z-index: -1;
        }
        
        .btn:hover::before {
            left: 0;
        }
        
        .btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 6px 15px rgba(0,0,0,0.15);
        }
        
        /* Barra di stato animata */
        .status-bar {
            height: 5px;
            width: 100%;
            background: rgba(255,255,255,0.2);
            margin-top: 20px;
            border-radius: 10px;
            overflow: hidden;
            position: relative;
        }
        
        .status-bar::after {
            content: '';
            position: absolute;
            top: 0;
            left: -20%;
            height: 100%;
            width: 20%;
            background: rgba(255,255,255,0.8);
            animation: statusProgress 2s infinite;
        }
        
        @keyframes statusProgress {
            0% { left: -20%; }
            100% { left: 120%; }
        }
        
        /* Puntini di caricamento animati */
        .dots {
            display: inline-block;
        }
        
        .dots::after {
            content: '...';
            animation: dots 1.5s steps(4, end) infinite;
            display: inline-block;
            width: 20px;
            text-align: left;
        }
        
        @keyframes dots {
            0%, 20% { content: '.'; }
            40% { content: '..'; }
            60% { content: '...'; }
            80% { content: ''; }
        }
    </style>
</head>
<body>
    <header>
        <div class="content">
            <h1><span class="ts-icon">📘</span> TypeScript CGI Info</h1>
            <p>Analisi dell'ambiente TypeScript CGI <span class="dots"></span></p>
            <div class="status-bar"></div>
        </div>
    </header>
    
    <div class="container">
        <div class="main">
            <div class="card">
                <h2><span class="icon">⚙️</span> Informazioni di Sistema</h2>
                <div class="info-grid">
                    <div class="info-label">Node Version:</div>
                    <div class="info-value">${systemInfo.nodeVersion}</div>
                    
                    <div class="info-label">Piattaforma:</div>
                    <div class="info-value">${systemInfo.platform}</div>
                    
                    <div class="info-label">Architettura:</div>
                    <div class="info-value">${systemInfo.architecture}</div>
                    
                    <div class="info-label">Memoria Utilizzata:</div>
                    <div class="info-value">${systemInfo.memoryUsage}</div>
                    
                    <div class="info-label">Uptime:</div>
                    <div class="info-value">${systemInfo.uptime}</div>
                    
                    <div class="info-label">Data e Ora:</div>
                    <div class="info-value">${systemInfo.time}</div>
                </div>
            </div>
            
            <div class="card">
                <h2><span class="icon">🔄</span> Dettagli Richiesta</h2>
                <div class="info-grid">
                    <div class="info-label">Metodo:</div>
                    <div class="info-value">
                        <span class="badge">${requestInfo.method}</span>
                    </div>
                    
                    <div class="info-label">Protocollo:</div>
                    <div class="info-value">${requestInfo.protocol}</div>
                    
                    <div class="info-label">Nome Server:</div>
                    <div class="info-value">${requestInfo.serverName}</div>
                    
                    <div class="info-label">Porta Server:</div>
                    <div class="info-value">${requestInfo.serverPort}</div>
                    
                    <div class="info-label">Indirizzo Remoto:</div>
                    <div class="info-value">${requestInfo.remoteAddr}</div>
                    
                    <div class="info-label">Query String:</div>
                    <div class="info-value">${requestInfo.queryString || '(vuota)'}</div>
                </div>
            </div>
            
            <div class="card">
                <h2><span class="icon">🔍</span> Variabili Ambiente</h2>
                <div class="info-grid">
                    ${Object.entries(envVariables).map(([key, value]) => `
                        <div class="info-label">${key}:</div>
                        <div class="info-value">${value}</div>
                    `).join('')}
                </div>
            </div>
        </div>
        
        <div class="buttons">
            <a href="/cgi-bin/test.py" class="btn">Python</a>
            <a href="/cgi-bin/test.sh" class="btn">Shell</a>
            <a href="/cgi-bin/test.cgi" class="btn">C++</a>
            <a href="/cgi-bin/info.php" class="btn">PHP</a>
            <a href="/index.html" class="btn">Home</a>
        </div>
    </div>
    
    <footer>
        <p>Webserv TypeScript CGI Implementation &copy; ${new Date().getFullYear()}</p>
    </footer>
</body>
</html>`); 