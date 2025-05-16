#!/home/vzashev/.nvm/versions/node/v16.20.2/bin/ts-node

console.log("Content-type: text/html\r\n");
console.log(`<!DOCTYPE html>
<html>
<head>
    <title>Simple TypeScript CGI Test</title>
</head>
<body>
    <h1>TypeScript CGI Test</h1>
    <p>Node Version: ${process.version}</p>
    <p>Current Time: ${new Date().toLocaleString()}</p>
</body>
</html>`); 