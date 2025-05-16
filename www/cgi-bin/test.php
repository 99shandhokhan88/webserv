#!/usr/bin/env php
<?php
header('Content-Type: text/html');

function getSystemInfo() {
    return [
        'PHP Version' => phpversion(),
        'System' => php_uname('s'),
        'Time' => date('H:i:s')
    ];
}

function getRequestInfo() {
    return [
        'Method' => $_SERVER['REQUEST_METHOD'],
        'Protocol' => $_SERVER['SERVER_PROTOCOL'],
        'Server Name' => $_SERVER['SERVER_NAME'],
        'Server Port' => $_SERVER['SERVER_PORT']
    ];
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP CGI Test Interface</title>
    <style>
        :root {
            --primary-color: #9C27B0;
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

        .php-info {
            background: #1a1a1a;
            color: #00ff00;
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
            <h1>🐘 PHP CGI Test Interface</h1>
            <p>A comprehensive testing interface for PHP CGI functionality</p>
            <a href="/cgi-bin/index.html" class="home-button">Return to Home</a>
        </header>

        <!-- System Information -->
        <div class="card">
            <h2>🖥️ System Information</h2>
            <div class="info-grid">
                <?php foreach (getSystemInfo() as $key => $value): ?>
                <div class="stat-box">
                    <h3><?= htmlspecialchars($key) ?></h3>
                    <div class="stat-value"><?= htmlspecialchars($value) ?></div>
                </div>
                <?php endforeach; ?>
            </div>
        </div>

        <!-- Request Information -->
        <div class="card">
            <h2>🌐 Request Information</h2>
            <table>
                <?php foreach (getRequestInfo() as $key => $value): ?>
                <tr>
                    <th><?= htmlspecialchars($key) ?></th>
                    <td><?= $key === 'Method' ? 
                        '<span class="badge badge-success">' . htmlspecialchars($value) . '</span>' : 
                        htmlspecialchars($value) ?></td>
                </tr>
                <?php endforeach; ?>
            </table>
        </div>

        <!-- Request Parameters -->
        <div class="card">
            <h2>📝 Request Parameters</h2>
            <?php if ($_SERVER['REQUEST_METHOD'] === 'GET'): ?>
                <?php if (!empty($_GET)): ?>
                    <table>
                        <tr>
                            <th>Parameter</th>
                            <th>Value</th>
                        </tr>
                        <?php foreach ($_GET as $key => $value): ?>
                        <tr>
                            <td><?= htmlspecialchars($key) ?></td>
                            <td><?= htmlspecialchars($value) ?></td>
                        </tr>
                        <?php endforeach; ?>
                    </table>
                <?php else: ?>
                    <p>No query parameters</p>
                <?php endif; ?>
            <?php elseif ($_SERVER['REQUEST_METHOD'] === 'POST'): ?>
                <?php if (!empty($_POST)): ?>
                    <table>
                        <tr>
                            <th>Field</th>
                            <th>Value</th>
                        </tr>
                        <?php foreach ($_POST as $key => $value): ?>
                        <tr>
                            <td><?= htmlspecialchars($key) ?></td>
                            <td><?= htmlspecialchars($value) ?></td>
                        </tr>
                        <?php endforeach; ?>
                    </table>
                <?php else: ?>
                    <p>No POST data received</p>
                <?php endif; ?>

                <?php if (!empty($_FILES)): ?>
                    <h3>Uploaded Files</h3>
                    <table>
                        <tr>
                            <th>File Field</th>
                            <th>Details</th>
                        </tr>
                        <?php foreach ($_FILES as $key => $file): ?>
                        <tr>
                            <td><?= htmlspecialchars($key) ?></td>
                            <td>
                                Name: <?= htmlspecialchars($file['name']) ?><br>
                                Type: <?= htmlspecialchars($file['type']) ?><br>
                                Size: <?= htmlspecialchars($file['size']) ?> bytes
                            </td>
                        </tr>
                        <?php endforeach; ?>
                    </table>
                <?php endif; ?>
            <?php endif; ?>
        </div>

        <!-- PHP Information -->
        <div class="card">
            <h2>🔧 PHP Information</h2>
            <div class="php-info">
                <pre><?php
                    $info = [
                        'PHP Version' => PHP_VERSION,
                        'Zend Version' => zend_version(),
                        'PHP SAPI' => php_sapi_name(),
                        'Loaded Extensions' => implode(', ', get_loaded_extensions())
                    ];
                    foreach ($info as $key => $value) {
                        echo htmlspecialchars("$key: $value\n");
                    }
                ?></pre>
            </div>
        </div>

        <!-- Test Forms -->
        <div class="card">
            <h2>🧪 Test Forms</h2>
            <h3>GET Test</h3>
            <form action="/cgi-bin/test.php" method="get">
                <input type="text" name="test_param" placeholder="Enter test parameter">
                <input type="submit" value="Test GET">
            </form>

            <h3>POST Test</h3>
            <form action="/cgi-bin/test.php" method="post" enctype="multipart/form-data">
                <input type="text" name="text_field" placeholder="Enter text">
                <input type="file" name="file_field">
                <input type="submit" value="Test POST">
            </form>
        </div>

        <footer>
            <p>PHP CGI Test Interface - Running on PHP <?= phpversion() ?></p>
        </footer>
    </div>
</body>
</html> 