#!/usr/bin/perl
use strict;
use warnings;
#!/usr/bin/perl
use strict;
use warnings;
use utf8;  # Abilita supporto UTF-8 nel codice Perl
binmode(STDOUT, ":utf8");  # Forza output UTF-8

print "Content-type: text/html; charset=utf-8\n\n";  # Header corretto


# Stile e logica del gioco
print <<'HTML';
<!DOCTYPE html>
<html>
<head>
    <title>Indovina il Numero! - Perl Game</title>
    <style>
        body { font-family: monospace; background: #000; color: #0f0; text-align: center; }
        h1 { color: #ff0; }
        .game-box { border: 2px solid #0f0; padding: 20px; width: 300px; margin: 0 auto; }
    </style>
</head>
<body>
    <div class="game-box">
        <h1>🔢 Indovina il Numero!</h1>
HTML

my $secret = int(rand(100)) + 1;
my $guess = $ENV{QUERY_STRING} =~ /guess=(\d+)/ ? $1 : 0;
my $attempts = $ENV{QUERY_STRING} =~ /attempts=(\d+)/ ? $1 : 0;
my $message = "";

if ($guess) {
    $attempts++;
    if ($guess == $secret) {
        $message = "<p>🎉 Hai vinto in $attempts tentativi!</p>";
        $secret = int(rand(100)) + 1;
        $attempts = 0;
    } elsif ($guess < $secret) {
        $message = "<p>🔺 Troppo basso!</p>";
    } else {
        $message = "<p>🔻 Troppo alto!</p>";
    }
}

print <<"HTML";
        <form method="get">
            <input type="number" name="guess" min="1" max="100" required>
            <input type="hidden" name="attempts" value="$attempts">
            <input type="submit" value="Indovina!">
        </form>
        $message
        <p>Tentativi: $attempts</p>
    </div>
</body>
</html>
HTML