/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/15 22:08:18 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Server.hpp"
#include "../incs/Client.hpp"
#include "../../Config/incs/ServerConfig.hpp"
#include "../../HTTP/incs/Request.hpp"
#include "../../HTTP/incs/Response.hpp"
#include "../../Utils/incs/FileHandler.hpp"
#include "../../Utils/incs/MimeTypes.hpp"
#include "../../CGI/incs/CGIExecutor.hpp"
#include "../../Utils/incs/StringUtils.hpp" 

// Static member initialization
std::vector<Server*> Server::servers;
std::vector<struct pollfd> Server::poll_fds;
std::map<int, Client> Server::clients;


template <typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Helper predicate for find_if
struct PollFDFinder {
    int target_fd;
    PollFDFinder(int fd) : target_fd(fd) {}
    bool operator()(const struct pollfd& pfd) {
        return pfd.fd == target_fd;
    }
};

Server::Server(const ServerConfig& config) :
config(config),
server_fd(-1) {
    memset(&address, 0, sizeof(address));
    setupSocket();
}

Server::~Server() {
    if (server_fd != -1) {
        close(server_fd);
        std::cout << "Server on port " << ntohs(address.sin_port) << " closed." << std::endl;
    }
}

void Server::setupSocket() {
    // Socket creation and setup
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));

    // Socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        throw std::runtime_error("setsockopt() failed: " + std::string(strerror(errno)));

    // Address configuration
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config.getPort());

    // Bind and listen
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
        throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));

    if (listen(server_fd, SOMAXCONN) < 0)
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));

    // Non-blocking mode
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() failed: " + std::string(strerror(errno)));

    // Add to server list
    servers.push_back(this);
    addPollFD(server_fd, POLLIN);

    std::cout << "Server started on port " << config.getPort() << " (FD: " << server_fd << ")" << std::endl;
}

// Replace the handleClient method with this improved version
void Server::handleClient(int client_fd) {
    Client& client = clients[client_fd];
    std::vector<char> buffer(4096);  // Increase buffer size for larger chunks
    
    ssize_t bytes_read;
    while ((bytes_read = recv(client_fd, buffer.data(), buffer.size(), 0)) > 0) {
        client.appendRequestData(buffer.data(), bytes_read);
        
        try {
            // Only try to parse and process if the request is complete
            if (client.isRequestComplete()) {
                client.parseRequest();
                processRequest(&client);
                if (!client.shouldKeepAlive()) {
                    removeClient(client_fd);
                    return;
                }
                client.reset();
            } else {
                // Not complete yet, continue reading
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Request error: " << e.what() << std::endl;
            sendErrorResponse(&client, 400, "Bad Request", servers[0]->config);
            removeClient(client_fd);
            return;
        }
    }

    // Handle connection close or error
    if (bytes_read == 0) {
        removeClient(client_fd);
    } else if (bytes_read < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cerr << "recv() error: " << strerror(errno) << std::endl;
            removeClient(client_fd);
        }
    }
}

std::string Server::getErrorPage(int errorCode) const {
    const std::map<int, std::string>& errorPages = config.getErrorPages();
    std::map<int, std::string>::const_iterator it = errorPages.find(errorCode);

    if (it != errorPages.end()) {
        try {
            return FileHandler::readFile(config.getRoot() + it->second);
        } catch (const std::exception& e) {
            std::cerr << "Error reading error page: " << e.what() << std::endl;
        }
    }

    // Default error page
    std::stringstream ss;
    ss << "<html><body><h1>" << errorCode << " Error</h1></body></html>";
    return ss.str();
}


size_t getContentLength(const std::string& headers) {
    size_t pos = headers.find("Content-Length:");
    if (pos == std::string::npos) {
        return 0;
    }
    
    pos = headers.find(':', pos) + 1;
    while (pos < headers.size() && isspace(headers[pos])) {
        pos++;
    }
    
    size_t end = pos;
    while (end < headers.size() && isdigit(headers[end])) {
        end++;
    }
    
    if (pos == end) {
        return 0;
    }
    
    return static_cast<size_t>(atoi(headers.substr(pos, end - pos).c_str()));
}

void Server::sendFileResponse(Client* client, const std::string& path) {
    std::string content = FileHandler::readFile(path);
    std::string mimeType = MimeTypes::getType(path);

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + mimeType + "\r\n";
    response += "Content-Length: " + toString(content.size()) + "\r\n";
    response += "\r\n" + content;

    send(client->fd, response.c_str(), response.size(), 0);
}

void Server::handleDirectoryListing(Client* client, const std::string& path) {
    std::vector<std::string> files = FileHandler::listDirectory(path);
    std::string content = "<html><body><h1>Directory Listing</h1><ul>";

    // C++98 compatible for-loop
    for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
        content += "<li><a href=\"" + *file + "\">" + *file + "</a>";
        content += " <form style=\"display:inline\" action=\"" + *file + "\" method=\"POST\">";
        content += "<input type=\"hidden\" name=\"_method\" value=\"DELETE\">";
        content += "<button type=\"submit\">Delete</button></form></li>";
    }

    content += "</ul></body></html>";
    sendResponse(client, 200, content);
}

bool Server::isCgiRequest(const LocationConfig& location, const std::string& path) const {
    // Silence the unused parameter warning
    (void)location;
    
    if (path.find("/cgi-bin/") == 0) {
        return true;
    }

    size_t dot_pos = path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string ext = path.substr(dot_pos);
        return (ext == ".py");
    }

    return false;
}

void Server::handleGetRequest(Client* client) {
    try {
        const LocationConfig& location = config.getLocationForPath(client->request.getPath());
        std::string path = config.getFullPath(client->request.getPath());

        if (isCgiRequest(location, client->request.getPath())) {
            CGIExecutor cgi(client->request, location);
            std::string output = cgi.execute();
            sendResponse(client, 200, output);
            return;
        }

        if (FileHandler::isDirectory(path)) {
            if (!path.empty() && path[path.size() - 1] != '/') {
                std::string redirectUrl = client->request.getPath() + "/";
                std::string redirectContent = "<html><body>Redirecting to <a href=\"" + redirectUrl + "\">" + redirectUrl + "</a></body></html>";
                std::string response = "HTTP/1.1 301 Moved Permanently\r\n";
                response += "Location: " + redirectUrl + "\r\n";
                response += "Content-Length: " + toString(redirectContent.size()) + "\r\n";
                response += "\r\n" + redirectContent;
                send(client->fd, response.c_str(), response.size(), 0);
                return;
            }

            std::string indexPath = path + location.getIndex();
            if (FileHandler::fileExists(indexPath)) {
                sendFileResponse(client, indexPath);
                return;
            }

            if (location.getAutoIndex()) {
                handleDirectoryListing(client, path);
            } else {
                sendErrorResponse(client, 403, "Forbidden", servers[0]->config);
            }
            return;
        }

        if (!FileHandler::fileExists(path)) {
            sendErrorResponse(client, 404, "Not Found", servers[0]->config);
            return;
        }

        sendFileResponse(client, path);
    } catch (const std::exception& e) {
        std::cerr << "Error handling GET request: " << e.what() << std::endl;
        sendErrorResponse(client, 500, "Internal Server Error", servers[0]->config);
    }
}

void Server::removeClient(int client_fd) {
    close(client_fd);
    clients.erase(client_fd);
    removePollFD(client_fd);
}


void Server::sendErrorResponse(Client* client, int statusCode, const std::string& message, const ServerConfig& config) {
    std::string errorPage;
    const std::map<int, std::string>& errorPages = config.getErrorPages();
    std::map<int, std::string>::const_iterator it = errorPages.find(statusCode);

    // Try to get configured error page
    if (it != errorPages.end()) {
        try {
            errorPage = FileHandler::readFile(config.getRoot() + it->second);
        } catch (const std::exception& e) {
            std::cerr << "Error reading error page: " << e.what() << std::endl;
        }
    }

    // Fallback to default error page
    if (errorPage.empty()) {
        std::stringstream ss;
        ss << "<html><head><title>" << statusCode << " " << message << "</title></head>"
           << "<body><h1>" << statusCode << " " << message << "</h1>"
           << "<p>Error details: " << message << "</p>"
           << "<p>Please try again later or contact the administrator.</p></body></html>";
        errorPage = ss.str();
    }

    std::string response = "HTTP/1.1 " + toString(statusCode) + " " + message + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + toString(errorPage.size()) + "\r\n";
    
    if (client->shouldKeepAlive()) {
        response += "Connection: keep-alive\r\n";
        response += "Keep-Alive: timeout=5, max=100\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    response += "\r\n" + errorPage;
    
    send(client->fd, response.c_str(), response.size(), 0);
}


// Modified handlePostRequest to better handle Content-Length
void Server::handlePostRequest(Client* client) {
    try {
        std::string requestPath = client->request.getPath();
        requestPath = FileHandler::sanitizePath(requestPath);
        std::cout << "DEBUG: Request path = " << requestPath << std::endl;

        const LocationConfig& location = config.getLocationForPath(requestPath);
        std::cout << "DEBUG: Available locations: ";
        // Print available locations for debugging
        const std::vector<LocationConfig>& locations = config.getLocations();
        for (std::vector<LocationConfig>::const_iterator it = locations.begin(); 
             it != locations.end(); ++it) {
            std::cout << " - '" << it->getPath() << "'";
        }
        std::cout << std::endl;
        
        std::cout << "DEBUG: Location path = " << location.getPath() << std::endl;

        std::string uploadDir = location.getUploadDir();
        std::cout << "DEBUG: Get upload_dir = " << location.getUploadDir() << std::endl;
        std::cout << "DEBUG: Upload directory = " << uploadDir << std::endl;
        
        if (uploadDir.empty()) {
            sendErrorResponse(client, 500, "Upload directory not configured", servers[0]->config);
            return;
        }

        // Check if the upload directory exists, create if not
        if (!FileHandler::isDirectory(uploadDir)) {
            std::cout << "Creating upload directory: " << uploadDir << std::endl;
            if (!FileHandler::createDirectory(uploadDir)) {
                std::cerr << "Failed to create upload directory: " << uploadDir << std::endl;
                sendErrorResponse(client, 500, "Could not create upload directory", servers[0]->config);
                return;
            }
        }
        
        // Extract content type and length from headers
        std::string contentType = client->request.getHeader("Content-Type");
        std::string contentLengthStr = client->request.getHeader("Content-Length");
        
        // Check if we have a Content-Length header and validate it
        if (!contentLengthStr.empty()) {
            int contentLength = atoi(contentLengthStr.c_str());
            if (contentLength <= 0) {
                std::cerr << "WARNING: Invalid or zero Content-Length: " << contentLengthStr << std::endl;
                if (contentType.find("multipart/form-data") != std::string::npos) {
                    // For multipart form data, we need content
                    sendErrorResponse(client, 400, "Empty multipart form data", servers[0]->config);
                    return;
                }
            }
        }
        
        // Check the request body
        const std::string& requestBody = client->request.getBody();
        if (requestBody.empty()) {
            std::cerr << "WARNING: Empty request body received" << std::endl;
            
            // For multipart, this is an error
            if (contentType.find("multipart/form-data") != std::string::npos) {
                std::string errorContent = "<html><body><h1>Empty Form Data</h1>";
                errorContent += "<p>No file data was received. Please select a file and try again.</p>";
                errorContent += "<p><a href=\"/\">Return to home</a></p></body></html>";
                sendResponse(client, 400, errorContent); // Bad Request
                return;
            }
        }
        
        // Handle multipart/form-data (file uploads)
        if (contentType.find("multipart/form-data") != std::string::npos) {
            if (requestBody.empty()) {
                std::string errorContent = "<html><body><h1>Empty Form Data</h1>";
                errorContent += "<p>No file data was received. Please select a file and try again.</p>";
                errorContent += "<p><a href=\"/\">Return to home</a></p></body></html>";
                sendResponse(client, 400, errorContent); // Bad Request
                return;
            }
            
            std::string boundary = extractBoundary(contentType);
            parseMultipartBody(client->request.getBody(), boundary, uploadDir);
            sendResponse(client, 201, "Multipart file uploaded successfully");
            return;
        }
        
        // Handle application/x-www-form-urlencoded
        if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
            std::map<std::string, std::string> formData;
            parseFormUrlEncoded(client->request.getBody(), formData);
            
            // Log the form data
            std::cout << "Received form data:" << std::endl;
            for (std::map<std::string, std::string>::const_iterator it = formData.begin(); 
                 it != formData.end(); ++it) {
                std::cout << "  " << it->first << " = " << it->second << std::endl;
            }
            
            // Esempio: salva i dati del form in un file
            std::string formDataContent;
            for (std::map<std::string, std::string>::const_iterator it = formData.begin(); 
                 it != formData.end(); ++it) {
                formDataContent += it->first + ": " + it->second + "\n";
            }
            
            std::string timestamp = toString(time(NULL));
            std::string filename = "form_" + timestamp + ".txt";
            std::string fullPath = getAbsolutePath(uploadDir + "/" + filename);
            
            if (FileHandler::writeFile(fullPath, formDataContent)) {
                std::string successContent = "<html><body><h1>Form Data Received</h1>";
                successContent += "<p>Your form has been successfully submitted.</p>";
                successContent += "<p><a href=\"/\">Return to home</a></p></body></html>";
                sendResponse(client, 201, successContent);
            } else {
                sendErrorResponse(client, 500, "Failed to process form data", servers[0]->config);
            }
            return;
        }
        
        // Handle text/plain or other content types
        if (contentType.find("text/plain") != std::string::npos || contentType.empty()) {
            std::string filename = "text_" + toString(time(NULL)) + ".txt";
            std::string fullPath = getAbsolutePath(uploadDir + "/" + filename);
            
            if (FileHandler::writeFile(fullPath, client->request.getBody())) {
                std::string successContent = "<html><body><h1>Text Received</h1>";
                successContent += "<p>Your text has been successfully saved.</p>";
                successContent += "<p><a href=\"/\">Return to home</a></p></body></html>";
                sendResponse(client, 201, successContent);
            } else {
                sendErrorResponse(client, 500, "Failed to save text data", servers[0]->config);
            }
            return;
        }
        
        // Unknown content type
        std::cerr << "Unsupported content type: " << contentType << std::endl;
        sendErrorResponse(client, 415, "Unsupported Media Type", servers[0]->config);
        
    } catch (const std::exception& e) {
        std::cerr << "Upload error: " << e.what() << std::endl;
        sendErrorResponse(client, 500, "Internal server error", servers[0]->config);
    }
}

void Server::parseFormUrlEncoded(const std::string& body, std::map<std::string, std::string>& formData) {
    std::string remainingData = body;
    size_t pos = 0;
    
    // Dividi il corpo mediante '&'
    while ((pos = remainingData.find('&')) != std::string::npos) {
        std::string pair = remainingData.substr(0, pos);
        remainingData = remainingData.substr(pos + 1);
        
        // Dividi la coppia mediante '='
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = StringUtils::urlDecode(pair.substr(0, equalPos));
            std::string value = StringUtils::urlDecode(pair.substr(equalPos + 1));
            formData[key] = value;
        }
    }
    
    // Gestisci l'ultima coppia (o l'unica se non ci sono '&')
    if (!remainingData.empty()) {
        size_t equalPos = remainingData.find('=');
        if (equalPos != std::string::npos) {
            std::string key = StringUtils::urlDecode(remainingData.substr(0, equalPos));
            std::string value = StringUtils::urlDecode(remainingData.substr(equalPos + 1));
            formData[key] = value;
        }
    }
}


// Update the parseMultipartBody method to be more robust:
void Server::parseMultipartBody(const std::string& body, const std::string& boundary, const std::string& uploadDir) {
    // Check for empty body first
    if (body.empty()) {
        std::cerr << "ERROR: Received empty body for multipart/form-data request\n";
        throw std::runtime_error("Empty multipart data received");
    }
    
    // The full boundary pattern includes -- at the start
    std::string startBoundary = "--" + boundary;
    std::string endBoundary = startBoundary + "--";
    
    std::cout << "DEBUG: Multipart processing started\n";
    std::cout << "DEBUG: Body size: " << body.size() << " bytes\n";
    std::cout << "DEBUG: Boundary: " << boundary << "\n";
    std::cout << "DEBUG: First 50 chars of body: " << body.substr(0, std::min(body.size(), static_cast<size_t>(50))) << "\n";
    
    // Find all boundary positions
    std::vector<size_t> boundaryPositions;
    size_t pos = 0;
    while ((pos = body.find(startBoundary, pos)) != std::string::npos) {
        boundaryPositions.push_back(pos);
        pos += startBoundary.length();
    }
    
    if (boundaryPositions.empty()) {
        std::cerr << "ERROR: No boundaries found in multipart data\n";
        throw std::runtime_error("Malformed multipart data");
    }
    
    std::cout << "DEBUG: Found " << boundaryPositions.size() << " boundaries\n";
    
    // Process each part
    for (size_t i = 0; i < boundaryPositions.size(); i++) {
        // Skip the boundary itself
        size_t partStart = boundaryPositions[i] + startBoundary.length();
        
        // Check if this is the final boundary
        if (body.compare(boundaryPositions[i], endBoundary.length(), endBoundary) == 0) {
            std::cout << "DEBUG: End boundary found\n";
            break; // This is the end boundary
        }
        
        // Skip the CRLF after the boundary
        if (partStart < body.size() && body.compare(partStart, 2, "\r\n") == 0) {
            partStart += 2;
        } else {
            std::cerr << "WARNING: Expected CRLF after boundary not found\n";
        }
        
        // Find the end of this part (next boundary or end of data)
        size_t partEnd = (i + 1 < boundaryPositions.size()) 
                         ? boundaryPositions[i + 1]
                         : body.length();
                         
        // Split headers and content
        size_t headersEnd = body.find("\r\n\r\n", partStart);
        if (headersEnd == std::string::npos || headersEnd >= partEnd) {
            std::cerr << "ERROR: Malformed multipart format (headers end not found)\n";
            continue;
        }
        
        // Extract headers section
        std::string headers = body.substr(partStart, headersEnd - partStart);
        
        // Find content-disposition header
        size_t dispPos = headers.find("Content-Disposition:");
        if (dispPos == std::string::npos) {
            std::cerr << "ERROR: Content-Disposition header not found\n";
            continue;
        }
        
        // Extract filename
        size_t filenamePos = headers.find("filename=\"", dispPos);
        if (filenamePos == std::string::npos) {
            std::cout << "INFO: No filename found, might be a form field\n";
            continue; // Skip non-file parts
        }
        
        filenamePos += 10; // Skip "filename=\""
        size_t filenameEnd = headers.find("\"", filenamePos);
        if (filenameEnd == std::string::npos) {
            std::cerr << "ERROR: Malformed filename in Content-Disposition\n";
            continue;
        }
        
        std::string filename = headers.substr(filenamePos, filenameEnd - filenamePos);
        if (filename.empty()) {
            std::cout << "INFO: Empty filename, skipping\n";
            continue;
        }
        
        // Skip headers and the blank line
        size_t contentStart = headersEnd + 4; // +4 for \r\n\r\n
        
        // Content ends at the next boundary minus \r\n
        size_t contentEnd = partEnd;
        if (contentEnd > 2 && body.compare(partEnd - 2, 2, "\r\n") == 0) {
            contentEnd -= 2; // Remove trailing CRLF before boundary
        }
        
        // Extract binary content
        std::string content = body.substr(contentStart, contentEnd - contentStart);
        
        // Create full path
        std::string fullPath = uploadDir + "/" + filename;
        fullPath = FileHandler::sanitizePath(fullPath);
        
        std::cout << "DEBUG: Saving file: " << filename << "\n";
        std::cout << "DEBUG: Full path: " << fullPath << "\n";
        std::cout << "DEBUG: Content size: " << content.size() << " bytes\n";
        
        // Write file
        if (!writeBinaryFile(fullPath, content)) {
            std::cerr << "ERROR: Failed to write file: " << fullPath << "\n";
            throw std::runtime_error("Failed to write uploaded file");
        }
        
        std::cout << "SUCCESS: File '" << filename << "' saved successfully\n";
    }
}

void Server::handleDeleteRequest(Client* client) {
    try {
        const LocationConfig& location = config.getLocationForPath(client->request.getPath());
        std::cout << "=== DEBUG DELETE ===" << "\n"
                 << "Percorso richiesto: " << client->request.getPath() << "\n"
                 << "Location matchata: " << location.getPath() << "\n"
                 << "Allow Delete: " << location.getAllowDelete() << "\n"
                 << "Metodi permessi: ";

        const std::vector<std::string>& methods = location.getAllowedMethods();
        for (size_t i=0; i<methods.size(); ++i) {
            std::cout << methods[i] << " ";
        }
        std::cout << "\n====================\n";

        // Corrected line: use client->request.getPath() instead of requestPath
        std::string resolvedPath = config.getFullPath(client->request.getPath());

        // Debug information
        std::cout << "DELETE Request:\n"
                  << "  - Client: " << client->fd << "\n"
                  << "  - Path: " << client->request.getPath() << "\n"
                  << "  - Resolved: " << resolvedPath << std::endl;

        // Security: Prevent path traversal
        std::string serverRoot = getAbsolutePath(config.getRoot());
        if (!FileHandler::isPathWithinRoot(resolvedPath, serverRoot)) {
            std::cerr << "Path traversal attempt blocked: " << resolvedPath << std::endl;
            sendErrorResponse(client, 403, "Forbidden: Invalid path", config);
            return;
        }

        // ... rest of the code remains the same ...
    } catch (const std::exception& e) {
        std::cerr << "DELETE Error: " << e.what() << std::endl;
        sendErrorResponse(client, 500, "Internal Server Error", config);
    }
}

void Server::handleRequest(const Request& request, Response& response) {
    // Implementation of handleRequest that uses the passed parameters
    if (request.getMethod() == "GET") {
        response.setStatus(200);
        response.setBody("Hello from GET handler!");
    } else if (request.getMethod() == "POST") {
        response.setStatus(200);
        response.setBody("Hello from POST handler!");
    } else {
        response.setStatus(405);
        response.setBody("Method Not Allowed");
    }
}

void Server::processRequest(Client* client) {
    if (client->request.getMethod() == "OPTIONS") {
        servers[0]->handleOptionsRequest(client);
        return;
    }
    try {
        const Request& req = client->request;
        
        // Validate request components
        if (req.getMethod().empty() || req.getPath().empty()) {
            throw std::runtime_error("Invalid request structure");
        }

        std::string method = req.getMethod();
        std::cout << "Processing " << method << " request for: " 
                << req.getPath() << std::endl;

        // Find a server instance to handle the request
        // In this case we'll use the first server in the list
        if (!servers.empty()) {
            if (method == "GET") {
                servers[0]->handleGetRequest(client);
            } else if (method == "POST") {
                servers[0]->handlePostRequest(client);
            } else if (method == "DELETE") {
                servers[0]->handleDeleteRequest(client);
            } else {
                Server::sendErrorResponse(client, 501, "Not Implemented", servers[0]->config); 
            }
        } else {
            throw std::runtime_error("No server available to handle request");
        }
    } catch (const std::exception& e) {
        std::cerr << "Request error: " << e.what() << std::endl;
        Server::sendErrorResponse(client, 400, "Bad Request", servers[0]->config);
    }
}

void Server::acceptNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
        return;
    }

    // Make client_fd non-blocking
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "fcntl() failed on client_fd: " << strerror(errno) << std::endl;
        close(client_fd);
        return;
    }

    addPollFD(client_fd, POLLIN);
    clients[client_fd] = Client(client_fd);
    std::cout << "New connection accepted (FD: " << client_fd << ")" << std::endl;
}

void Server::run() {
    std::cout << "Starting server manager..." << std::endl;
    while (true) {
        int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_count == -1)
            throw std::runtime_error("poll() failed: " + std::string(strerror(errno)));

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                // Find the appropriate server for this FD
                bool is_server_fd = false;
                for (std::vector<Server*>::iterator srv = servers.begin(); srv != servers.end(); ++srv) {
                    if ((*srv)->getServerFd() == poll_fds[i].fd) {
                        (*srv)->acceptNewConnection();
                        is_server_fd = true;
                        break;
                    }
                }
                
                // If not a server FD, it's a client FD
                if (!is_server_fd) {
                    // Find the server responsible for this client - for simplicity, we'll handle it here
                    handleClient(poll_fds[i].fd);
                }
            }
        }
    }
}

// Helper method to remove poll FD - explicitly defined as it was missing
void Server::removePollFD(int fd) {
    std::vector<struct pollfd>::iterator it = 
        std::find_if(poll_fds.begin(), poll_fds.end(), PollFDFinder(fd));
    
    if (it != poll_fds.end()) {
        poll_fds.erase(it);
    }
}

int Server::getServerFd() const {
    return server_fd;
}

void Server::sendResponse(Client* client, int status, const std::string& content) {
    std::string statusText;
    switch (status) {
        case 200: statusText = "OK"; break;
        case 201: statusText = "Created"; break;
        case 204: statusText = "No Content"; break;
        case 301: statusText = "Moved Permanently"; break;
        case 400: statusText = "Bad Request"; break;
        case 403: statusText = "Forbidden"; break;
        case 404: statusText = "Not Found"; break;
        case 500: statusText = "Internal Server Error"; break;
        case 501: statusText = "Not Implemented"; break;
        default: statusText = "Unknown";
    }
    
    std::string response = "HTTP/1.1 " + toString(status) + " " + statusText + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + toString(content.size()) + "\r\n";
    
    if (client->shouldKeepAlive()) {
        response += "Connection: keep-alive\r\n";
        response += "Keep-Alive: timeout=5, max=100\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    response += "\r\n" + content;
    send(client->fd, response.c_str(), response.size(), 0);
}

void Server::cleanup() {
    for (std::vector<Server*>::iterator it = servers.begin(); it != servers.end(); ++it) {
        delete *it;
    }
    servers.clear();
    poll_fds.clear();
    clients.clear();
}

void Server::handleOptionsRequest(Client* client) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Content-Length\r\n";
    response += "Content-Length: 0\r\n\r\n";
    
    send(client->fd, response.c_str(), response.size(), 0);
}



// Helper methods
void Server::addPollFD(int fd, short events) {
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    poll_fds.push_back(pfd);
}

bool Server::isServerFD(int fd) const {
    for (std::vector<Server*>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        if ((*it)->server_fd == fd) return true;
    }
    return false;
}

const std::vector<struct pollfd>& Server::getPollFds() const {
    return poll_fds;
}

void Server::setPollEvents(size_t index, short events) {
    if (index < poll_fds.size()) {
        poll_fds[index].events = events;
    }
}
