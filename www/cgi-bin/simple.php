#!/usr/bin/php
<?php
header("Content-type: text/html");
echo "<!DOCTYPE html>
<html>
<head>
    <title>Simple PHP CGI Test</title>
</head>
<body>
    <h1>PHP CGI Test</h1>
    <p>PHP Version: " . phpversion() . "</p>
    <p>Current Time: " . date('Y-m-d H:i:s') . "</p>
</body>
</html>";
?> 