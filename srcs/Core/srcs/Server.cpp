/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/09 00:03:52 by vzashev          ###   ########.fr       */
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

void Server::handleClient(int client_fd) {
    Client& client = clients[client_fd];
    std::vector<char> buffer(1024);  // Use a dynamic buffer

    ssize_t bytes_read;
    while ((bytes_read = recv(client_fd, buffer.data(), buffer.size(), 0)) > 0) {
        client.appendRequestData(buffer.data(), bytes_read);

        try {
            if (client.isRequestComplete()) {
                client.parseRequest();
                processRequest(&client);
                if (!client.shouldKeepAlive()) {
                    removeClient(client_fd);
                    return;
                }
                client.reset();
            }
        } catch (const std::exception& e) {
            std::cerr << "Request error: " << e.what() << std::endl;
            sendErrorResponse(&client, 400, "Bad Request");
            removeClient(client_fd);
            return;
        }
    }

    // Connection closed by client
    if (bytes_read == 0) {
        removeClient(client_fd);
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
                sendErrorResponse(client, 403, "Forbidden");
            }
            return;
        }

        if (!FileHandler::fileExists(path)) {
            sendErrorResponse(client, 404, "Not Found");
            return;
        }

        sendFileResponse(client, path);
    } catch (const std::exception& e) {
        std::cerr << "Error handling GET request: " << e.what() << std::endl;
        sendErrorResponse(client, 500, "Internal Server Error");
    }
}

void Server::removeClient(int client_fd) {
    close(client_fd);
    clients.erase(client_fd);
    removePollFD(client_fd);
}


void Server::sendErrorResponse(Client* client, int statusCode, const std::string& message) {
    // Since this is static, we need access to a server object to get the error page
    // Either pass a server object or use the first available server
    std::string errorPage;
    if (!servers.empty()) {
        errorPage = servers[0]->getErrorPage(statusCode);
    } else {
        // Default error page if no server is available
        std::stringstream ss;
        ss << "<html><body><h1>" << statusCode << " Error</h1></body></html>";
        errorPage = ss.str();
    }
    
    std::string response = "HTTP/1.1 " + toString(statusCode) + " " + message + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + toString(errorPage.size()) + "\r\n";
    response += "\r\n" + errorPage;
    send(client->fd, response.c_str(), response.size(), 0);
}

void Server::handlePostRequest(Client* client) {
    try {
        std::string requestPath = client->request.getPath();
        requestPath = FileHandler::sanitizePath(requestPath);
        
        const LocationConfig& location = config.getLocationForPath(requestPath);
        std::string uploadDir = location.getUploadDir();
        
        if (uploadDir.empty()) {
            sendErrorResponse(client, 500, "Upload directory not configured");
            return;
        }

        // Check if the upload directory exists, create if not
        if (!FileHandler::isDirectory(uploadDir)) {
            std::cout << "Creating upload directory: " << uploadDir << std::endl;
            if (!FileHandler::createDirectory(uploadDir)) {
                std::cerr << "Failed to create upload directory: " << uploadDir << std::endl;
                sendErrorResponse(client, 500, "Could not create upload directory");
                return;
            }
        }
        
        // Extract content type from headers
        std::string contentType = client->request.getHeader("Content-Type");
        if (contentType.find("multipart/form-data") != std::string::npos) {
            std::string boundary = extractBoundary(contentType);
            parseMultipartBody(client->request.getBody(), boundary, uploadDir);
            sendResponse(client, 201, "Multipart file uploaded successfully");
            return;
        }
        
        // Default file upload handling (not multipart)
        std::string filename = "uploaded_file.jpg";  // Example, take filename from HTML form
        std::string fullPath = FileHandler::sanitizePath(uploadDir + "/" + filename);
        
        // Debug output
        std::cout << "Attempting to write to: " << fullPath << std::endl;
        
        // Write the file to upload directory
        if (FileHandler::writeFile(fullPath, client->request.getBody())) {
            std::cout << "Successfully wrote " << client->request.getBody().size() 
                      << " bytes to " << fullPath << std::endl;
            sendResponse(client, 201, "File uploaded successfully");
        } else {
            sendErrorResponse(client, 500, "Failed to write file");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Upload error: " << e.what() << std::endl;
        sendErrorResponse(client, 500, "Internal server error");
    }
}

void Server::parseMultipartBody(const std::string& body, const std::string& boundary, const std::string& uploadDir) {
    std::string delimiter = "--" + boundary;
    size_t pos = body.find(delimiter);
    if (pos == std::string::npos) return;

    size_t fileStart = body.find("\r\n\r\n", pos);  // Start of file content
    if (fileStart == std::string::npos) return;

    fileStart += 4;  // Skip \r\n\r\n
    size_t fileEnd = body.find(delimiter, fileStart);
    if (fileEnd == std::string::npos) return;

    std::string fileContent = body.substr(fileStart, fileEnd - fileStart - 2);  // -2 to remove \r\n

    std::string filename = "uploaded_file.jpg";  // Or extract from Content-Disposition
    std::string fullPath = uploadDir + "/" + filename;
    FileHandler::writeFile(fullPath, fileContent);
}

void Server::handleDeleteRequest(Client* client) {
    try {
        const LocationConfig& location = config.getLocationForPath(client->request.getPath());
        std::string path = config.getFullPath(client->request.getPath());
        
        // Debug output
        std::cout << "DELETE request processing:\n"
                << "  - Location path: " << location.getPath() << "\n"
                << "  - Request path: " << client->request.getPath() << "\n"
                << "  - Resolved path: " << path << std::endl;
        
        // Check if deletion is allowed
        if (!location.getAllowDelete()) {
            sendErrorResponse(client, 403, "Forbidden");
            return;
        }
        
        // Check if file exists
        if (!FileHandler::fileExists(path)) {
            sendErrorResponse(client, 404, "Not Found");
            return;
        }
        
        bool isDirectory = FileHandler::isDirectory(path);
        
        // Handle file deletion
        if (!isDirectory) {
            if (FileHandler::deleteFile(path)) {
                std::string successContent = "<html><body><h1>File Deleted</h1>";
                successContent += "<p>Successfully deleted: " + client->request.getPath() + "</p>";
                successContent += "<p><a href=\"/\">Return to home</a></p></body></html>";
                sendResponse(client, 200, successContent);
            } else {
                sendErrorResponse(client, 500, "Internal Server Error");
            }
            return;
        }
        
        // Handle directory deletion
        if (!path.empty() && path[path.size() - 1] != '/') {
            path += '/';
        }
        
        // Check if directory has trailing slash
        std::string requestPath = client->request.getPath();
        if (!requestPath.empty() && requestPath[requestPath.size() - 1] != '/') {
            // Redirect to add trailing slash
            std::string redirectUrl = client->request.getPath() + "/";
            std::string redirectContent = "<html><body>Redirecting to <a href=\"" + 
                                       redirectUrl + "\">" + redirectUrl + "</a></body></html>";
            std::string response = "HTTP/1.1 301 Moved Permanently\r\n";
            response += "Location: " + redirectUrl + "\r\n";
            response += "Content-Length: " + toString(redirectContent.size()) + "\r\n";
            response += "\r\n" + redirectContent;
            send(client->fd, response.c_str(), response.size(), 0);
            return;
        }
        
        // Try to delete the directory
        if (FileHandler::deleteDirectory(path)) {
            std::string successContent = "<html><body><h1>Directory Deleted</h1>";
            successContent += "<p>Successfully deleted directory: " + client->request.getPath() + "</p>";
            successContent += "<p><a href=\"/\">Return to home</a></p></body></html>";
            sendResponse(client, 204, successContent); // No Content
        } else {
            sendErrorResponse(client, 500, "Internal Server Error");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "DELETE request error: " << e.what() << std::endl;
        sendErrorResponse(client, 500, "Internal Server Error");
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
                Server::sendErrorResponse(client, 501, "Not Implemented"); 
            }
        } else {
            throw std::runtime_error("No server available to handle request");
        }
    } catch (const std::exception& e) {
        std::cerr << "Request error: " << e.what() << std::endl;
        Server::sendErrorResponse(client, 400, "Bad Request");
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

