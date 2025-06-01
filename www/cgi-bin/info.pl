#!/usr/bin/perl
use strict;
use warnings;
use utf8;
binmode(STDOUT, ":utf8");

print "Content-type: text/html; charset=utf-8\n\n";

# Get system information
my $hostname = `hostname` || "webserv-node";
chomp $hostname;
my $uptime = `uptime` || "System online";
chomp $uptime;
my $date = `date` || localtime();
chomp $date;
my $load = `cat /proc/loadavg 2>/dev/null` || "Load: N/A";
chomp $load;

# Generate random server stats
my $cpu_usage = int(rand(40)) + 20;
my $memory_usage = int(rand(30)) + 40;
my $network_traffic = int(rand(1000)) + 500;
my $active_connections = int(rand(50)) + 10;

# Game logic
my $secret = int(rand(100)) + 1;
my $guess = $ENV{QUERY_STRING} =~ /guess=(\d+)/ ? $1 : 0;
my $attempts = $ENV{QUERY_STRING} =~ /attempts=(\d+)/ ? $1 : 0;
my $message = "";
my $message_style = "";

if ($guess) {
    $attempts++;
    if ($guess == $secret) {
        $message = "🎉 SYSTEM HACKED! You cracked the code in $attempts attempts!";
        $message_style = "color: #00ff41; text-shadow: 0 0 15px rgba(0,255,65,0.8);";
        $secret = int(rand(100)) + 1;
        $attempts = 0;
    } elsif ($guess < $secret) {
        $message = "🔺 ACCESS DENIED - Security level too low!";
        $message_style = "color: #ff0080;";
    } else {
        $message = "🔻 FIREWALL ALERT - Security level too high!";
        $message_style = "color: #ff4500;";
    }
}

print <<'HTML';
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🚀 WebServ Control Panel - Perl Dashboard</title>
    <style>
        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #0a0a0a 0%, #1a1a2e 25%, #16213e 50%, #0f3460 75%, #533483 100%);
            color: #00ff41;
            min-height: 100vh;
            margin: 0;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            text-align: center;
            margin-bottom: 40px;
            padding: 30px;
            background: rgba(0,255,65,0.1);
            border: 2px solid #00ff41;
            border-radius: 20px;
            box-shadow: 0 0 30px rgba(0,255,65,0.3);
        }
        
        .header h1 {
            font-size: 3em;
            font-weight: bold;
            color: #00ff41;
            text-shadow: 0 0 20px rgba(0,255,65,0.5);
            margin: 0;
        }
        
        .subtitle {
            font-size: 1.2em;
            color: #00ffff;
            margin-top: 10px;
            text-shadow: 0 0 10px rgba(0,255,255,0.5);
        }
        
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 25px;
            margin-bottom: 30px;
        }
        
        .panel {
            background: rgba(0,0,0,0.8);
            border: 2px solid #00ff41;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }
        
        .panel h3 {
            color: #ff0080;
            font-size: 1.4em;
            margin-bottom: 15px;
            text-transform: uppercase;
            letter-spacing: 2px;
            text-shadow: 0 0 10px rgba(255,0,128,0.5);
        }
        
        .stat {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin: 10px 0;
            padding: 10px;
            background: rgba(0,255,65,0.1);
            border-radius: 8px;
            border-left: 4px solid #00ff41;
        }
        
        .stat-label {
            color: #00ffff;
            font-weight: bold;
        }
        
        .stat-value {
            color: #00ff41;
            font-weight: bold;
            text-shadow: 0 0 5px rgba(0,255,65,0.5);
        }
        
        .progress-bar {
            width: 100%;
            height: 20px;
            background: rgba(0,0,0,0.5);
            border-radius: 10px;
            overflow: hidden;
            margin: 10px 0;
            border: 1px solid #00ff41;
        }
        
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #00ff41, #ff0080, #00ffff);
            border-radius: 10px;
            transition: width 2s ease;
            box-shadow: 0 0 10px rgba(0,255,65,0.5);
        }
        
        .server-status {
            text-align: center;
            padding: 20px;
            background: rgba(0,255,65,0.1);
            border: 2px solid #00ffff;
            border-radius: 15px;
            margin-top: 20px;
        }
        
        .status-online {
            color: #00ff41;
            font-size: 1.5em;
            font-weight: bold;
            text-shadow: 0 0 15px rgba(0,255,65,0.8);
        }
        
        .terminal {
            background: #000;
            border: 2px solid #00ff41;
            border-radius: 10px;
            padding: 20px;
            font-family: 'Courier New', monospace;
            color: #00ff41;
            margin-top: 20px;
            box-shadow: 0 0 20px rgba(0,255,65,0.3);
        }
        
        .terminal-line {
            margin: 5px 0;
        }
        
        .game-section {
            background: rgba(255,0,128,0.1);
            border: 2px solid #ff0080;
            border-radius: 15px;
            padding: 25px;
            margin-top: 30px;
            text-align: center;
        }
        
        .game-input {
            background: rgba(0,0,0,0.8);
            border: 2px solid #00ff41;
            border-radius: 8px;
            padding: 12px;
            color: #00ff41;
            font-family: 'Courier New', monospace;
            font-size: 1.1em;
            margin: 10px;
            text-align: center;
        }
        
        .game-button {
            background: linear-gradient(45deg, #00ff41, #00ffff);
            border: none;
            border-radius: 8px;
            padding: 12px 25px;
            color: #000;
            font-family: 'Courier New', monospace;
            font-weight: bold;
            cursor: pointer;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .game-button:hover {
            transform: scale(1.05);
            box-shadow: 0 0 20px rgba(0,255,65,0.5);
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚀 WEBSERV CONTROL PANEL</h1>
            <div class="subtitle">🔥 Perl-Powered Server Dashboard 🔥</div>
        </div>
        
        <div class="dashboard">
            <div class="panel">
                <h3>��️ System Info</h3>
                <div class="stat">
                    <span class="stat-label">Hostname:</span>
                    <span class="stat-value">$hostname</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Date:</span>
                    <span class="stat-value">$date</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Load:</span>
                    <span class="stat-value">$load</span>
                </div>
            </div>
            
            <div class="panel">
                <h3>📊 Performance</h3>
                <div class="stat">
                    <span class="stat-label">CPU Usage:</span>
                    <span class="stat-value">$cpu_usage%</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: $cpu_usage%"></div>
                </div>
                
                <div class="stat">
                    <span class="stat-label">Memory:</span>
                    <span class="stat-value">$memory_usage%</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill" style="width: $memory_usage%"></div>
                </div>
            </div>
            
            <div class="panel">
                <h3>🌐 Network</h3>
                <div class="stat">
                    <span class="stat-label">Traffic:</span>
                    <span class="stat-value">$network_traffic MB/s</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Connections:</span>
                    <span class="stat-value">$active_connections</span>
                </div>
                <div class="stat">
                    <span class="stat-label">Port:</span>
                    <span class="stat-value">8080</span>
                </div>
            </div>
        </div>
        
        <div class="server-status">
            <div class="status-online">🟢 SERVER ONLINE - ALL SYSTEMS OPERATIONAL</div>
            <p>Uptime: $uptime</p>
        </div>
        
        <div class="terminal">
            <div class="terminal-line">[webserv\@$hostname ~]\$ systemctl status webserv</div>
            <div class="terminal-line">● webserv.service - High Performance Web Server</div>
            <div class="terminal-line">   Loaded: loaded (/etc/systemd/system/webserv.service; enabled)</div>
            <div class="terminal-line">   Active: <span style="color: #00ff41;">active (running)</span> since $date</div>
            <div class="terminal-line">   Memory: $memory_usage% CPU: $cpu_usage%</div>
            <div class="terminal-line">[webserv\@$hostname ~]\$ <span style="animation: blink 1s infinite;">█</span></div>
        </div>
        
        <div class="game-section">
            <h3 style="color: #ff0080; font-size: 1.8em; margin-bottom: 20px;">🔐 HACK THE MAINFRAME</h3>
            <p style="color: #00ffff; margin-bottom: 20px;">Crack the security code (1-100) to gain access!</p>
            
            <form method="get">
                <input type="number" name="guess" min="1" max="100" required 
                       class="game-input" placeholder="Enter security code...">
                <input type="hidden" name="attempts" value="$attempts">
                <br>
                <input type="submit" value="🚀 HACK SYSTEM" class="game-button">
            </form>
            
            <div style="margin-top: 20px;">
                <p style="$message_style"><strong>$message</strong></p>
                <p style="color: #00ffff;">Hack attempts: <span style="color: #00ff41;">$attempts</span></p>
                <p style="color: #666; font-size: 0.9em; margin-top: 10px;">Secret number for testing: $secret</p>
            </div>
        </div>
    </div>
</body>
</html>
HTML