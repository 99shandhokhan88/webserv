#!/usr/bin/php
<?php
header('Content-Type: text/html');

// Funzioni di utilità
function getSystemInfo() {
    return [
        'PHP Version' => phpversion(),
        'Sistema Operativo' => php_uname('s'),
        'Server' => $_SERVER['SERVER_SOFTWARE'] ?? 'Webserv',
        'Data e Ora' => date('d/m/Y H:i:s'),
        'Utente' => get_current_user(),
        'Directory' => getcwd()
    ];
}

function getRequestInfo() {
    return [
        'Metodo' => $_SERVER['REQUEST_METHOD'] ?? 'GET',
        'Protocollo' => $_SERVER['SERVER_PROTOCOL'] ?? 'HTTP/1.1',
        'Nome Server' => $_SERVER['SERVER_NAME'] ?? 'localhost',
        'Porta Server' => $_SERVER['SERVER_PORT'] ?? '8080',
        'IP Client' => $_SERVER['REMOTE_ADDR'] ?? '127.0.0.1',
        'Query String' => $_SERVER['QUERY_STRING'] ?? ''
    ];
}

function getServerVariables() {
    $filtered = [];
    foreach ($_SERVER as $key => $value) {
        // Filtra solo le variabili CGI pertinenti
        if (strpos($key, 'HTTP_') === 0 || in_array($key, ['REMOTE_ADDR', 'REQUEST_METHOD', 'QUERY_STRING'])) {
            $filtered[$key] = $value;
        }
    }
    return $filtered;
}

// Genera un colore casuale per l'interfaccia
$colors = ['#2196F3', '#9C27B0', '#4CAF50', '#FF5722', '#3F51B5'];
$themeColor = $colors[array_rand($colors)];
?>
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP CGI Info | Webserv</title>
    <style>
        :root {
            --primary-color: <?php echo $themeColor; ?>;
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
        
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
        
        @keyframes slideInLeft {
            from { transform: translateX(-50px); opacity: 0; }
            to { transform: translateX(0); opacity: 1; }
        }
        
        @keyframes bounce {
            0%, 20%, 50%, 80%, 100% { transform: translateY(0); }
            40% { transform: translateY(-10px); }
            60% { transform: translateY(-5px); }
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: var(--background-color);
            color: var(--text-color);
            margin: 0;
            padding: 0;
            transition: var(--transition);
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
            box-shadow: var(--box-shadow);
            position: relative;
            overflow: hidden;
            text-align: center;
            animation: fadeIn 0.8s ease-out;
        }
        
        header::before {
            content: '';
            position: absolute;
            top: -50px;
            left: -50px;
            right: -50px;
            bottom: -50px;
            background: radial-gradient(circle at center, rgba(255,255,255,0.2) 0%, rgba(255,255,255,0) 70%);
            z-index: 0;
            animation: pulse 3s infinite;
        }
        
        header .content {
            position: relative;
            z-index: 1;
        }
        
        h1 {
            margin: 0;
            font-size: 3em;
            font-weight: 700;
            letter-spacing: -1px;
        }
        
        h1 span.emoji {
            display: inline-block;
            animation: bounce 2s infinite;
            transform-origin: bottom center;
        }
        
        header p {
            margin: 10px 0 0;
            font-size: 1.2em;
            opacity: 0.9;
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
            animation: slideInLeft 0.5s ease-out;
        }
        
        .card h2::after {
            content: '';
            position: absolute;
            bottom: -8px;
            left: 0;
            width: 50px;
            height: 3px;
            background: var(--primary-color);
            transition: var(--transition);
        }
        
        .card:hover h2::after {
            width: 100px;
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
            color: #888;
            font-size: 0.9em;
        }
        
        /* Tasti con hover effect */
        .buttons {
            display: flex;
            justify-content: center;
            gap: 15px;
            margin-top: 20px;
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
            cursor: pointer;
        }
        
        .btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 6px 15px rgba(0,0,0,0.15);
            background: var(--secondary-color);
        }

        /* Animazione per la barra di stato */
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
            left: 0;
            height: 100%;
            width: 30%;
            background: white;
            border-radius: 10px;
            animation: statusMove 2.5s infinite;
        }
        
        @keyframes statusMove {
            0% { left: -30%; }
            100% { left: 100%; }
        }
    </style>
</head>
<body>
    <header>
        <div class="content">
            <h1><span class="emoji">🐘</span> PHP CGI Info</h1>
            <p>Informazioni dettagliate sul server e sull'ambiente CGI</p>
            <div class="status-bar"></div>
        </div>
    </header>
    
    <div class="container">
        <div class="main">
            <div class="card">
                <h2>💻 Informazioni di Sistema</h2>
                <div class="info-grid">
                    <?php foreach (getSystemInfo() as $key => $value): ?>
                    <div class="info-label"><?= htmlspecialchars($key) ?>:</div>
                    <div class="info-value"><?= htmlspecialchars($value) ?></div>
                    <?php endforeach; ?>
                </div>
            </div>
            
            <div class="card">
                <h2>🌐 Dettagli Richiesta</h2>
                <div class="info-grid">
                    <?php foreach (getRequestInfo() as $key => $value): ?>
                    <div class="info-label"><?= htmlspecialchars($key) ?>:</div>
                    <div class="info-value">
                        <?php if ($key === 'Metodo'): ?>
                            <span class="badge"><?= htmlspecialchars($value) ?></span>
                        <?php else: ?>
                            <?= htmlspecialchars($value) ?>
                        <?php endif; ?>
                    </div>
                    <?php endforeach; ?>
                </div>
            </div>
            
            <div class="card">
                <h2>🔍 Variabili CGI</h2>
                <div class="info-grid">
                    <?php foreach (getServerVariables() as $key => $value): ?>
                    <div class="info-label"><?= htmlspecialchars($key) ?>:</div>
                    <div class="info-value"><?= htmlspecialchars($value) ?></div>
                    <?php endforeach; ?>
                </div>
            </div>
        </div>
        
        <div class="buttons">
            <a href="/cgi-bin/test.py" class="btn">Python</a>
            <a href="/cgi-bin/test.sh" class="btn">Shell</a>
            <a href="/cgi-bin/test.cgi" class="btn">C++</a>
            <a href="/cgi-bin/info.ts" class="btn">TypeScript</a>
            <a href="/index.html" class="btn">Home</a>
        </div>
    </div>
    
    <footer>
        <p>Webserv CGI Implementation &copy; <?= date('Y') ?></p>
    </footer>
</body>
</html> 