        <div class="game-section">
            <h3 style="color: #ff0080; font-size: 1.8em; margin-bottom: 20px;">🔐 HACK THE MAINFRAME</h3>
            <p style="color: #00ffff; margin-bottom: 20px;">Crack the security code (1-100) to gain access!</p>
            
            <form method="get">
                <input type="number" name="guess" min="1" max="100" required 
                       class="game-input" placeholder="Enter security code...">
HTML

print <<"HTML";
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