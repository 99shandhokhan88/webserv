<?php
header('Content-Type: text/html; charset=UTF-8');

// Check for special query parameters for easter eggs
$konami = isset($_GET['konami']) ? true : false;
$secret = isset($_GET['secret']) ? $_GET['secret'] : '';
$matrix = isset($_GET['matrix']) ? true : false;
$rainbow = isset($_GET['rainbow']) ? true : false;

// Fun facts array
$facts = [
    "PHP originally stood for 'Personal Home Page'!",
    "PHP powers over 78% of all websites!",
    "The PHP elephant mascot is named 'elePHPant'!",
    "PHP was created by Rasmus Lerdorf in 1994!",
    "Facebook was originally built with PHP!",
    "WordPress, which powers 40% of the web, is written in PHP!",
    "PHP 8 introduced JIT compilation for better performance!"
];

$randomFact = $facts[array_rand($facts)];

// Get current time and date
$currentTime = date('Y-m-d H:i:s');
$serverTime = date('H:i:s');

// Check if it's a special time (easter egg)
$hour = (int)date('H');
$isNightMode = ($hour >= 22 || $hour <= 6);
$isLunchTime = ($hour >= 12 && $hour <= 13);

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title><?php echo $konami ? '🎮 KONAMI CODE ACTIVATED!' : '🐘 PHP Easter Egg Hunt'; ?></title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: <?php echo $matrix ? 'linear-gradient(135deg, #000 0%, #001100 100%)' : 
                              ($rainbow ? 'linear-gradient(45deg, #ff0000, #ff7f00, #ffff00, #00ff00, #0000ff, #4b0082, #9400d3)' :
                              ($isNightMode ? 'linear-gradient(135deg, #2c3e50 0%, #34495e 100%)' : 
                               'linear-gradient(135deg, #667eea 0%, #764ba2 100%)')); ?>;
            min-height: 100vh;
            color: <?php echo $matrix ? '#00ff00' : 'white'; ?>;
            padding: 20px;
            <?php if ($rainbow): ?>
            background-size: 400% 400%;
            animation: rainbow 4s ease infinite;
            <?php endif; ?>
        }
        
        <?php if ($rainbow): ?>
        @keyframes rainbow {
            0% { background-position: 0% 50%; }
            50% { background-position: 100% 50%; }
            100% { background-position: 0% 50%; }
        }
        <?php endif; ?>
        
        .container {
            max-width: 1000px;
            margin: 0 auto;
            background: rgba(255, 255, 255, <?php echo $matrix ? '0.05' : '0.1'; ?>);
            border-radius: 20px;
            padding: 30px;
            backdrop-filter: blur(10px);
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.3);
            <?php if ($matrix): ?>
            border: 2px solid #00ff00;
            box-shadow: 0 0 30px #00ff0050;
            <?php endif; ?>
        }
        
        .header {
            text-align: center;
            margin-bottom: 30px;
            <?php if ($konami): ?>
            animation: konami-glow 1s ease-in-out infinite alternate;
            <?php endif; ?>
        }
        
        <?php if ($konami): ?>
        @keyframes konami-glow {
            from { text-shadow: 0 0 20px #ff6b6b; }
            to { text-shadow: 0 0 40px #ff6b6b, 0 0 60px #ff6b6b; }
        }
        <?php endif; ?>
        
        .title {
            font-size: 3em;
            margin-bottom: 15px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
            <?php if ($matrix): ?>
            color: #00ff00;
            font-family: 'Courier New', monospace;
            <?php endif; ?>
        }
        
        .subtitle {
            font-size: 1.3em;
            opacity: 0.9;
            margin-bottom: 20px;
        }
        
        .easter-egg-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 25px;
            margin-bottom: 30px;
        }
        
        .easter-egg-card {
            background: rgba(255, 255, 255, <?php echo $matrix ? '0.05' : '0.15'; ?>);
            border-radius: 15px;
            padding: 25px;
            text-align: center;
            transition: all 0.3s ease;
            cursor: pointer;
            border: 2px solid transparent;
            <?php if ($matrix): ?>
            border-color: #00ff00;
            <?php endif; ?>
        }
        
        .easter-egg-card:hover {
            transform: translateY(-10px) scale(1.05);
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.3);
            <?php if ($matrix): ?>
            box-shadow: 0 15px 35px #00ff0030;
            <?php endif; ?>
        }
        
        .easter-egg-icon {
            font-size: 4em;
            margin-bottom: 15px;
            display: block;
            <?php if ($konami): ?>
            animation: bounce 2s infinite;
            <?php endif; ?>
        }
        
        @keyframes bounce {
            0%, 20%, 50%, 80%, 100% { transform: translateY(0); }
            40% { transform: translateY(-30px); }
            60% { transform: translateY(-15px); }
        }
        
        .easter-egg-title {
            font-size: 1.5em;
            font-weight: bold;
            margin-bottom: 10px;
            color: <?php echo $matrix ? '#00ff00' : '#fff'; ?>;
        }
        
        .easter-egg-desc {
            opacity: 0.8;
            line-height: 1.6;
        }
        
        .secret-section {
            background: linear-gradient(135deg, #ff6b6b, #ee5a24);
            border-radius: 15px;
            padding: 30px;
            margin: 30px 0;
            text-align: center;
            <?php if (!$konami && $secret !== 'php'): ?>
            display: none;
            <?php endif; ?>
        }
        
        .time-section {
            background: <?php echo $isLunchTime ? 'linear-gradient(135deg, #fdcb6e, #e17055)' : 
                                ($isNightMode ? 'linear-gradient(135deg, #2d3436, #636e72)' : 
                                'linear-gradient(135deg, #74b9ff, #0984e3)'); ?>;
            border-radius: 15px;
            padding: 25px;
            margin: 20px 0;
            text-align: center;
        }
        
        .fun-fact {
            background: linear-gradient(135deg, #a29bfe, #6c5ce7);
            border-radius: 15px;
            padding: 25px;
            margin: 20px 0;
            text-align: center;
            position: relative;
            overflow: hidden;
        }
        
        .fun-fact::before {
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
            animation: slide 10s linear infinite;
        }
        
        @keyframes slide {
            0% { transform: translateX(-50px) translateY(-50px); }
            100% { transform: translateX(50px) translateY(50px); }
        }
        
        .controls {
            display: flex;
            justify-content: center;
            gap: 15px;
            margin: 30px 0;
            flex-wrap: wrap;
        }
        
        .btn {
            padding: 12px 24px;
            border: none;
            border-radius: 25px;
            font-size: 1em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            background: linear-gradient(135deg, #fd79a8, #e84393);
            color: white;
            text-decoration: none;
            display: inline-block;
        }
        
        .btn:hover {
            transform: translateY(-3px);
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.3);
        }
        
        .btn.matrix {
            background: linear-gradient(135deg, #00ff00, #008800);
            color: #000;
        }
        
        .btn.rainbow {
            background: linear-gradient(45deg, #ff0000, #ff7f00, #ffff00, #00ff00, #0000ff, #4b0082, #9400d3);
            background-size: 400% 400%;
            animation: rainbow 2s ease infinite;
        }
        
        .hidden-message {
            position: fixed;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            background: rgba(0, 0, 0, 0.9);
            color: #00ff00;
            padding: 30px;
            border-radius: 15px;
            font-family: 'Courier New', monospace;
            font-size: 1.2em;
            text-align: center;
            z-index: 1000;
            display: none;
            border: 2px solid #00ff00;
            box-shadow: 0 0 30px #00ff0050;
        }
        
        .php-info {
            background: rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            padding: 20px;
            margin: 20px 0;
        }
        
        .php-info h3 {
            color: #74b9ff;
            margin-bottom: 15px;
        }
        
        .php-info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
        }
        
        .php-info-item {
            background: rgba(255, 255, 255, 0.05);
            padding: 15px;
            border-radius: 10px;
            border-left: 4px solid #74b9ff;
        }
        
        .php-info-key {
            font-weight: bold;
            color: #74b9ff;
            margin-bottom: 5px;
        }
        
        .php-info-value {
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
            word-break: break-all;
        }
        
        .footer {
            text-align: center;
            margin-top: 40px;
            padding-top: 20px;
            border-top: 1px solid rgba(255, 255, 255, 0.2);
            opacity: 0.7;
        }
        
        <?php if ($matrix): ?>
        .matrix-rain {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            pointer-events: none;
            z-index: -1;
        }
        
        .matrix-column {
            position: absolute;
            top: -100px;
            font-family: 'Courier New', monospace;
            font-size: 14px;
            color: #00ff00;
            animation: matrix-fall linear infinite;
            opacity: 0.3;
        }
        
        @keyframes matrix-fall {
            to { transform: translateY(100vh); }
        }
        <?php endif; ?>
    </style>
</head>
<body>
    <?php if ($matrix): ?>
    <div class="matrix-rain" id="matrixRain"></div>
    <?php endif; ?>
    
    <div class="container">
        <div class="header">
            <h1 class="title">
                <?php if ($konami): ?>
                    🎮 KONAMI CODE ACTIVATED! 🎮
                <?php elseif ($matrix): ?>
                    🔴 MATRIX MODE ENGAGED 🔴
                <?php elseif ($rainbow): ?>
                    🌈 RAINBOW POWER! 🌈
                <?php else: ?>
                    🐘 PHP Easter Egg Hunt
                <?php endif; ?>
            </h1>
            <p class="subtitle">
                <?php if ($konami): ?>
                    You found the secret! All easter eggs are now revealed!
                <?php elseif ($matrix): ?>
                    Welcome to the Matrix, Neo... The PHP has you.
                <?php elseif ($rainbow): ?>
                    Taste the rainbow of PHP possibilities!
                <?php else: ?>
                    Discover hidden features and secret modes!
                <?php endif; ?>
            </p>
        </div>
        
        <div class="time-section">
            <h3>
                <?php if ($isLunchTime): ?>
                    🍽️ Lunch Time Special!
                <?php elseif ($isNightMode): ?>
                    🌙 Night Owl Mode
                <?php else: ?>
                    ⏰ Current Server Time
                <?php endif; ?>
            </h3>
            <p style="font-size: 1.5em; margin: 10px 0;">
                <?php echo $currentTime; ?>
            </p>
            <p>
                <?php if ($isLunchTime): ?>
                    Perfect time for a coding break! 🥪
                <?php elseif ($isNightMode): ?>
                    Burning the midnight oil? Don't forget to rest! 😴
                <?php else: ?>
                    Great time to be coding! ☀️
                <?php endif; ?>
            </p>
        </div>
        
        <div class="easter-egg-grid">
            <div class="easter-egg-card" onclick="activateKonami()">
                <span class="easter-egg-icon">🎮</span>
                <h3 class="easter-egg-title">Konami Code</h3>
                <p class="easter-egg-desc">
                    <?php echo $konami ? 'ACTIVATED! You are a true gamer!' : 'Try the classic cheat code: ↑↑↓↓←→←→BA'; ?>
                </p>
            </div>
            
            <div class="easter-egg-card" onclick="window.location.href='?matrix=1'">
                <span class="easter-egg-icon">🔴</span>
                <h3 class="easter-egg-title">Matrix Mode</h3>
                <p class="easter-egg-desc">
                    <?php echo $matrix ? 'You took the red pill!' : 'Take the red pill and see how deep the rabbit hole goes...'; ?>
                </p>
            </div>
            
            <div class="easter-egg-card" onclick="window.location.href='?rainbow=1'">
                <span class="easter-egg-icon">🌈</span>
                <h3 class="easter-egg-title">Rainbow Power</h3>
                <p class="easter-egg-desc">
                    <?php echo $rainbow ? 'You are now fabulous!' : 'Activate rainbow mode for a colorful experience!'; ?>
                </p>
            </div>
            
            <div class="easter-egg-card" onclick="showSecret()">
                <span class="easter-egg-icon">🔐</span>
                <h3 class="easter-egg-title">Secret Message</h3>
                <p class="easter-egg-desc">Click to reveal a hidden message from the PHP developers!</p>
            </div>
            
            <div class="easter-egg-card" onclick="showPhpInfo()">
                <span class="easter-egg-icon">🐘</span>
                <h3 class="easter-egg-title">PHP Info</h3>
                <p class="easter-egg-desc">Discover detailed information about this PHP installation!</p>
            </div>
            
            <div class="easter-egg-card" onclick="randomFun()">
                <span class="easter-egg-icon">🎲</span>
                <h3 class="easter-egg-title">Random Fun</h3>
                <p class="easter-egg-desc">Click for a random surprise! You never know what you'll get!</p>
            </div>
        </div>
        
        <?php if ($konami || $secret === 'php'): ?>
        <div class="secret-section">
            <h2>🎉 SECRET UNLOCKED! 🎉</h2>
            <p style="font-size: 1.3em; margin: 20px 0;">
                Congratulations! You've discovered one of the hidden features!
            </p>
            <p>
                <?php if ($konami): ?>
                    The Konami Code (↑↑↓↓←→←→BA) is a famous cheat code that originated in Konami video games!
                <?php else: ?>
                    You found the secret PHP message! This proves you're a true developer!
                <?php endif; ?>
            </p>
        </div>
        <?php endif; ?>
        
        <div class="fun-fact">
            <h3 style="position: relative; z-index: 1;">🧠 PHP Fun Fact</h3>
            <p style="font-size: 1.2em; margin: 15px 0; position: relative; z-index: 1;">
                <?php echo $randomFact; ?>
            </p>
        </div>
        
        <div class="php-info" id="phpInfoSection" style="display: none;">
            <h3>🐘 PHP Environment Information</h3>
            <div class="php-info-grid">
                <div class="php-info-item">
                    <div class="php-info-key">PHP Version</div>
                    <div class="php-info-value"><?php echo phpversion(); ?></div>
                </div>
                <div class="php-info-item">
                    <div class="php-info-key">Server Software</div>
                    <div class="php-info-value"><?php echo $_SERVER['SERVER_SOFTWARE'] ?? 'Unknown'; ?></div>
                </div>
                <div class="php-info-item">
                    <div class="php-info-key">Request Method</div>
                    <div class="php-info-value"><?php echo $_SERVER['REQUEST_METHOD'] ?? 'Unknown'; ?></div>
                </div>
                <div class="php-info-item">
                    <div class="php-info-key">Request URI</div>
                    <div class="php-info-value"><?php echo $_SERVER['REQUEST_URI'] ?? 'Unknown'; ?></div>
                </div>
                <div class="php-info-item">
                    <div class="php-info-key">User Agent</div>
                    <div class="php-info-value"><?php echo substr($_SERVER['HTTP_USER_AGENT'] ?? 'Unknown', 0, 50) . '...'; ?></div>
                </div>
                <div class="php-info-item">
                    <div class="php-info-key">Remote Address</div>
                    <div class="php-info-value"><?php echo $_SERVER['REMOTE_ADDR'] ?? 'Unknown'; ?></div>
                </div>
            </div>
        </div>
        
        <div class="controls">
            <a href="?" class="btn">🔄 Reset</a>
            <a href="?konami=1" class="btn">🎮 Konami</a>
            <a href="?matrix=1" class="btn matrix">🔴 Matrix</a>
            <a href="?rainbow=1" class="btn rainbow">🌈 Rainbow</a>
            <a href="?secret=php" class="btn">🔐 Secret</a>
        </div>
        
        <div class="footer">
            <p>🐘 PHP Easter Egg Hunt • Made with ❤️ and lots of ☕</p>
            <p>Current time: <?php echo $serverTime; ?> | PHP <?php echo phpversion(); ?></p>
        </div>
    </div>
    
    <div class="hidden-message" id="hiddenMessage">
        <h2>🔐 SECRET MESSAGE DECODED 🔐</h2>
        <p style="margin: 20px 0;">
            "Hello, fellow developer! 👋<br>
            You've discovered this hidden message!<br>
            Keep exploring, keep coding, keep being awesome! 🚀<br>
            - The PHP Team"
        </p>
        <button onclick="closeSecret()" class="btn">Close</button>
    </div>
    
    <script>
        // Konami Code implementation
        let konamiCode = [];
        const konamiSequence = [
            'ArrowUp', 'ArrowUp', 'ArrowDown', 'ArrowDown',
            'ArrowLeft', 'ArrowRight', 'ArrowLeft', 'ArrowRight',
            'KeyB', 'KeyA'
        ];
        
        document.addEventListener('keydown', function(e) {
            konamiCode.push(e.code);
            
            if (konamiCode.length > konamiSequence.length) {
                konamiCode.shift();
            }
            
            if (JSON.stringify(konamiCode) === JSON.stringify(konamiSequence)) {
                window.location.href = '?konami=1';
            }
        });
        
        function activateKonami() {
            alert('Try typing the Konami Code: ↑↑↓↓←→←→BA');
        }
        
        function showSecret() {
            document.getElementById('hiddenMessage').style.display = 'block';
        }
        
        function closeSecret() {
            document.getElementById('hiddenMessage').style.display = 'none';
        }
        
        function showPhpInfo() {
            const section = document.getElementById('phpInfoSection');
            section.style.display = section.style.display === 'none' ? 'block' : 'none';
        }
        
        function randomFun() {
            const surprises = [
                () => alert('🎉 Surprise! You found a random alert!'),
                () => document.body.style.transform = 'rotate(1deg)',
                () => document.body.style.filter = 'hue-rotate(180deg)',
                () => {
                    const colors = ['#ff6b6b', '#4ecdc4', '#45b7d1', '#96ceb4', '#ffeaa7'];
                    document.body.style.background = colors[Math.floor(Math.random() * colors.length)];
                },
                () => alert('🦄 You found a unicorn! (Not really, but you can pretend!)'),
                () => window.location.href = '?rainbow=1'
            ];
            
            const randomSurprise = surprises[Math.floor(Math.random() * surprises.length)];
            randomSurprise();
        }
        
        <?php if ($matrix): ?>
        // Matrix rain effect
        function createMatrixRain() {
            const matrixContainer = document.getElementById('matrixRain');
            const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()_+-=[]{}|;:,.<>?';
            
            for (let i = 0; i < 30; i++) {
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
                
                matrixContainer.appendChild(column);
            }
        }
        
        createMatrixRain();
        <?php endif; ?>
        
        // Add some sparkle effects for konami mode
        <?php if ($konami): ?>
        function createSparkles() {
            for (let i = 0; i < 20; i++) {
                setTimeout(() => {
                    const sparkle = document.createElement('div');
                    sparkle.innerHTML = '✨';
                    sparkle.style.position = 'fixed';
                    sparkle.style.left = Math.random() * 100 + '%';
                    sparkle.style.top = Math.random() * 100 + '%';
                    sparkle.style.fontSize = '20px';
                    sparkle.style.pointerEvents = 'none';
                    sparkle.style.zIndex = '999';
                    sparkle.style.animation = 'sparkle 2s ease-out forwards';
                    document.body.appendChild(sparkle);
                    
                    setTimeout(() => sparkle.remove(), 2000);
                }, i * 100);
            }
        }
        
        createSparkles();
        setInterval(createSparkles, 5000);
        <?php endif; ?>
    </script>
</body>
</html>
