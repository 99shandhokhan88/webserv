
---

# Webserv - HTTP Server in C++

## Overview
This project is a custom HTTP server implemented in C++98. It is designed to handle HTTP requests, serve static files, execute CGI scripts, and manage client connections using non-blocking I/O. The server is configurable via an NGINX-style configuration file and supports features like file uploads, error pages, and multiple server blocks.

---

## Project Structure

### Root Directory
- **`Makefile`**: Compiles the project with strict flags (`-Wall -Wextra -Werror`) and C++98 compliance.
- **`README.md`**: This file. Provides an overview of the project, its structure, and usage instructions.
- **`configs/`**: Contains configuration files for the server.
- **`include/`**: Main header file for the project.
- **`src/`**: Source code for the server, organized into subdirectories.
- **`www/`**: Directory for static files, uploads, and CGI scripts.
- **`tests/`**: Unit tests for various components of the server.

---

### `configs/`
- **`default.conf`**: Default configuration file for the server. Defines server blocks, ports, and routes.
- **`test.conf`**: Test configuration file for debugging and development.

Example `default.conf`:
```nginx
server {
    listen 8080;
    server_name localhost;
    client_max_body_size 1M;

    location / {
        root /www;
        index index.html;
    }

    location /uploads {
        root /www/uploads;
        allow_upload on;
    }

    location /cgi-bin {
        root /www/cgi-bin;
        cgi_extension .php;
        cgi_path /usr/bin/php-cgi;
    }
}
```

---

### `include/`
- **`webserv.hpp`**: Main header file that includes all necessary headers for the project. Ensures consistency across the codebase.

---

### `src/`
Contains all the source code, organized into subdirectories based on functionality.

#### `Config/`
Handles configuration file parsing and storage.
- **`ConfigParser.cpp/hpp`**: Parses the NGINX-style configuration file and extracts server and location blocks.
- **`ServerConfig.cpp/hpp`**: Stores server-specific settings (e.g., port, server_name, error pages).
- **`LocationConfig.cpp/hpp`**: Stores route-specific settings (e.g., allowed methods, root directory, CGI configuration).

#### `Core/`
Core server functionality.
- **`Server.cpp/hpp`**: Manages server sockets, client connections, and initialization.
- **`Client.cpp/hpp`**: Represents a client connection and handles I/O operations.
- **`EventLoop.cpp/hpp`**: Implements the event loop using `poll()` or equivalent for non-blocking I/O.

#### `HTTP/`
Handles HTTP protocol logic.
- **`Request.cpp/hpp`**: Represents an HTTP request and parses it (e.g., method, URI, headers, body).
- **`Response.cpp/hpp`**: Represents an HTTP response and generates it (e.g., status line, headers, body).
- **`RequestParser.cpp/hpp`**: State machine for parsing HTTP requests.
- **`ResponseGenerator.cpp/hpp`**: Generates HTTP responses based on the request and server configuration.

#### `CGI/`
Handles CGI script execution.
- **`CGIExecutor.cpp/hpp`**: Executes CGI scripts (e.g., PHP, Python) and manages input/output pipes.

#### `Utils/`
Utility functions and helpers.
- **`FileHandler.cpp/hpp`**: Reads/writes files and checks permissions.
- **`StringUtils.cpp/hpp`**: String manipulation functions (e.g., trimming, splitting, case conversion).
- **`MimeTypes.cpp/hpp`**: Maps file extensions to MIME types (e.g., `.html` → `text/html`).

#### `Errors/`
Error handling and logging.
- **`ErrorHandler.cpp/hpp`**: Handles and logs errors (e.g., file not found, permission denied).

---

### `www/`
Directory for static files, uploads, and CGI scripts.
- **`index.html`**: Default HTML file for testing.
- **`uploads/`**: Directory for uploaded files.
- **`cgi-bin/`**: Directory for CGI scripts (e.g., PHP, Python).

Example `index.html`:
```html
<!DOCTYPE html>
<html>
<head>
    <title>Webserv</title>
</head>
<body>
    <h1>Welcome to Webserv!</h1>
</body>
</html>
```

---

### `tests/`
Unit tests for critical components of the server.
- **`test_config.cpp`**: Tests configuration file parsing.
- **`test_request.cpp`**: Tests HTTP request parsing.
- **`test_response.cpp`**: Tests HTTP response generation.
- **`test_cgi.cpp`**: Tests CGI script execution.

---

## Features
1. **Non-blocking I/O**: Uses `poll()` or equivalent for efficient handling of multiple clients.
2. **Configuration File**: Supports NGINX-style configuration files with multiple server blocks and locations.
3. **HTTP Methods**: Handles `GET`, `POST`, and `DELETE` requests.
4. **Static File Serving**: Serves HTML, images, and other static files.
5. **File Uploads**: Supports file uploads via `POST` requests.
6. **CGI Execution**: Executes CGI scripts (e.g., PHP, Python) and returns the output.
7. **Error Pages**: Custom error pages for common HTTP errors (e.g., 404, 500).
8. **Client Body Size Limit**: Configurable limit for client request body size.

---

## Usage

### Build the Project
```bash
make
```

### Run the Server
```bash
./webserv configs/default.conf
```

### Test the Server
1. **Basic Request**:
   ```bash
   curl http://localhost:8080
   ```

2. **File Upload**:
   ```bash
   curl -X POST -F "file=@test.txt" http://localhost:8080/uploads
   ```

3. **CGI Script**:
   ```bash
   curl http://localhost:8080/cgi-bin/test.php
   ```

4. **Stress Test**:
   ```bash
   siege -c 100 -t 30S http://localhost:8080
   ```

---

## Testing
Run unit tests to verify functionality:
```bash
make test
```

---

## Dependencies
- C++98 compiler (e.g., `g++`)
- `poll()` or equivalent (e.g., `kqueue` on macOS, `epoll` on Linux)
- `curl` for testing
- `siege` for stress testing

---

## Contributing
1. Fork the repository.
2. Create a new branch for your feature or bugfix.
3. Submit a pull request with a detailed description of your changes.

---

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
```

---

