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
    try {
        std::vector<std::string> files = FileHandler::listDirectory(path);
        std::stringstream html;
        
        html << "<!DOCTYPE html>\n<html>\n<head>\n"
             << "<title>Directory Listing</title>\n"
             << "<style>\n"
             << "body { font-family: Arial, sans-serif; margin: 20px; }\n"
             << "h1 { color: #333; }\n"
             << ".file-list { list-style: none; padding: 0; }\n"
             << ".file-list li { margin: 10px 0; padding: 10px; background: #f5f5f5; border-radius: 5px; }\n"
             << ".file-list a { color: #007bff; text-decoration: none; }\n"
             << ".file-list a:hover { text-decoration: underline; }\n"
             << "</style>\n"
             << "</head>\n<body>\n"
             << "<h1>Directory Listing for " << client->request.getPath() << "</h1>\n"
             << "<ul class='file-list'>\n";

        // Add parent directory link if not in root
        if (client->request.getPath() != "/") {
            html << "<li><a href=\"../\">Parent Directory</a></li>\n";
        }

        // Add files and directories
        for (size_t i = 0; i < files.size(); ++i) {
            std::string fullPath = path + "/" + files[i];
            std::string displayPath = client->request.getPath() + 
                                    (client->request.getPath().empty() ? "/" : 
                                     (client->request.getPath()[client->request.getPath().length() - 1] == '/' ? "" : "/")) + 
                                    files[i];
            
            if (FileHandler::isDirectory(fullPath)) {
                displayPath += "/";
                html << "<li><a href=\"" << displayPath << "\">[DIR] " << files[i] << "/</a></li>\n";
            } else {
                html << "<li><a href=\"" << displayPath << "\">" << files[i] << "</a></li>\n";
            }
        }

        html << "</ul>\n</body>\n</html>";

        std::string content = html.str();
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: " + toString(content.size()) + "\r\n";
        response += "\r\n";
        response += content;

        send(client->fd, response.c_str(), response.size(), 0);

    } catch (const std::exception& e) {
        std::cerr << "Error generating directory listing: " << e.what() << std::endl;
        sendErrorResponse(client, 500, "Internal Server Error", servers[0]->config);
    }
}

bool Server::isCgiRequest(const LocationConfig& location, const std::string& path) const {
    std::cout << "DEBUG: Checking if '" << path << "' is a CGI request" << std::endl;
    std::cout << "DEBUG: Location path: '" << location.getPath() << "'" << std::endl;
    
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string ext = path.substr(dot_pos);
        std::cout << "DEBUG: File extension: '" << ext << "'" << std::endl;
        
        const std::map<std::string, std::string>& cgiMap = location.getCgiInterpreters();
        std::cout << "DEBUG: CGI Interpreters map size: " << cgiMap.size() << std::endl;
        
        for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
            std::cout << "DEBUG: CGI Map entry: '" << it->first << "' -> '" << it->second << "'" << std::endl;
        }
        
        if (cgiMap.find(ext) != cgiMap.end()) {
            std::cout << "DEBUG: Extension '" << ext << "' found in CGI interpreters map" << std::endl;
            
            // Verifica la condizione path.find(location.getPath())
            bool pathMatches = path.find(location.getPath()) == 0;
            std::cout << "DEBUG: Path match condition: " << pathMatches 
                     << " (path.find('" << location.getPath() << "') == " 
                     << path.find(location.getPath()) << ")" << std::endl;
            
            // Verifica aggiuntiva: il path deve iniziare con il path della location CGI
            if (pathMatches) {
                std::cout << "DEBUG: CGI request confirmed for " << path << " with extension " << ext << std::endl;
                return true;
            }
        } else {
            std::cout << "DEBUG: Extension '" << ext << "' NOT found in CGI interpreters map" << std::endl;
        }
    } else {
        std::cout << "DEBUG: No file extension found in path" << std::endl;
    }
    return false;
}

void Server::handleGetRequest(Client* client) {
    try {
        const LocationConfig& location = config.getLocationForPath(client->request.getPath());
        std::string path = config.getFullPath(client->request.getPath());
        
        std::cout << "DEBUG: Handling GET request for " << client->request.getPath() << std::endl;
        std::cout << "DEBUG: Location path: " << location.getPath() << std::endl;
        std::cout << "DEBUG: Full file path: " << path << std::endl;

        // Handle root path redirect
        if (client->request.getPath() == "/") {
            std::string redirectUrl = "/index.html";
            std::string redirectContent = "<html><body>Redirecting to <a href=\"" + redirectUrl + "\">" + redirectUrl + "</a></body></html>";
            std::string response = "HTTP/1.1 301 Moved Permanently\r\n";
            response += "Location: " + redirectUrl + "\r\n";
            response += "Content-Length: " + toString(redirectContent.size()) + "\r\n";
            response += "\r\n" + redirectContent;
            send(client->fd, response.c_str(), response.size(), 0);
            return;
        }

        // Verifica speciale per script in /cgi-bin/
        if (client->request.getPath().find("/cgi-bin/") == 0) {
            std::cout << "DEBUG: Special handling for /cgi-bin/ path" << std::endl;
            
            // Verifica estensione
            size_t dot_pos = path.find_last_of('.');
            if (dot_pos != std::string::npos) {
                std::string ext = path.substr(dot_pos);
                std::cout << "DEBUG: File extension: " << ext << std::endl;
                
                // SOLUZIONE DIRETTA: Invece di controllare la mappa, eseguiamo direttamente 
                // gli script in base all'estensione
                std::string interpreter = "";
                if (ext == ".py") {
                    interpreter = "/usr/bin/python3";
                    std::cout << "DEBUG: Using hardcoded Python interpreter: " << interpreter << std::endl;
                } else if (ext == ".sh") {
                    interpreter = "/bin/bash";
                    std::cout << "DEBUG: Using hardcoded Bash interpreter: " << interpreter << std::endl;
                } else if (ext == ".cgi") {
                    interpreter = "/usr/bin/env";
                    std::cout << "DEBUG: Using hardcoded Env interpreter: " << interpreter << std::endl;
                } else if (ext == ".php") {
                    interpreter = "/usr/bin/php";
                    std::cout << "DEBUG: Using hardcoded PHP interpreter: " << interpreter << std::endl;
                } else if (ext == ".ts") {
                    interpreter = "/home/vzashev/.nvm/versions/node/v16.20.2/bin/ts-node";
                    std::cout << "DEBUG: Using hardcoded TypeScript interpreter: " << interpreter << std::endl;
                }
                
                if (!interpreter.empty()) {
                    try {
                        std::cout << "DEBUG: Executing CGI script: " << path << std::endl;
                        
                        // Verifica che il file esista ed abbia permessi di esecuzione
                        if (!FileHandler::fileExists(path)) {
                            std::cerr << "CGI Error: File not found: " << path << std::endl;
                            sendErrorResponse(client, 404, "CGI Script Not Found", servers[0]->config);
                            return;
                        }
                        
                        // Imposta permessi di esecuzione se necessario
                        if (!FileHandler::isExecutable(path)) {
                            std::cout << "DEBUG: Setting executable permissions for " << path << std::endl;
                            chmod(path.c_str(), 0755);
                        }
                        
                        // Aggiungiamo manualmente l'interprete alla location
                        LocationConfig modifiedLocation = location;
                        modifiedLocation.addCgiInterpreter(ext, interpreter);
                        
                        // Verifichiamo che sia stato aggiunto
                        std::cout << "DEBUG: Manually added interpreter: " << ext << " -> " << interpreter << std::endl;
                        const std::map<std::string, std::string>& cgiMap = modifiedLocation.getCgiInterpreters();
                        std::cout << "DEBUG: Modified location CGI map size: " << cgiMap.size() << std::endl;
                        
                        CGIExecutor cgi(client->request, modifiedLocation);
                        std::string output = cgi.execute();
                        if (output.empty()) {
                            throw std::runtime_error("CGI execution failed");
                        }
                        std::cout << "DEBUG: CGI execution successful, output size: " << output.size() << std::endl;
                        send(client->fd, output.c_str(), output.size(), 0);
                        return;
                    } catch (const std::exception& e) {
                        std::cerr << "CGI Error: " << e.what() << std::endl;
                        sendErrorResponse(client, 500, "CGI Execution Failed", servers[0]->config);
                        return;
                    }
                } else {
                    std::cout << "DEBUG: Extension " << ext << " not supported for CGI" << std::endl;
                }
            }
        }

        // Handle directory requests
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

            // Try to serve index file
            std::string indexPath = path + location.getIndex();
            if (FileHandler::fileExists(indexPath)) {
                sendFileResponse(client, indexPath);
                return;
            }

            // Handle directory listing
            if (location.getAutoIndex()) {
                handleDirectoryListing(client, path);
            } else {
                sendErrorResponse(client, 403, "Forbidden", servers[0]->config);
            }
            return;
        }

        // Handle file requests
        if (!FileHandler::fileExists(path)) {
            sendErrorResponse(client, 404, "Not Found", servers[0]->config);
            return;
        }

        // Get file extension and set appropriate content type
        std::string extension = path.substr(path.find_last_of(".") + 1);
        std::string contentType = "text/plain";
        
        if (extension == "html" || extension == "htm") contentType = "text/html";
        else if (extension == "css") contentType = "text/css";
        else if (extension == "js") contentType = "application/javascript";
        else if (extension == "jpg" || extension == "jpeg") contentType = "image/jpeg";
        else if (extension == "png") contentType = "image/png";
        else if (extension == "gif") contentType = "image/gif";
        else if (extension == "txt") contentType = "text/plain";
        else if (extension == "pdf") contentType = "application/pdf";

        // Read and send file with appropriate content type
        std::string fileContent = FileHandler::readFile(path);
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + contentType + "\r\n";
        response += "Content-Length: " + toString(fileContent.size()) + "\r\n";
        response += "\r\n";
        response += fileContent;
        
        send(client->fd, response.c_str(), response.size(), 0);

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
                 << "Requested path: " << client->request.getPath() << "\n"
                 << "Matched location: " << location.getPath() << "\n"
                 << "Allow Delete: " << location.getAllowDelete() << "\n"
                 << "Allowed methods: ";

        const std::vector<std::string>& methods = location.getAllowedMethods();
        for (size_t i=0; i<methods.size(); ++i) {
            std::cout << methods[i] << " ";
        }
        std::cout << "\n====================\n";

        // Check if DELETE is allowed for this location
        if (!location.getAllowDelete()) {
            sendErrorResponse(client, 403, "DELETE method not allowed for this location", config);
            return;
        }

        // Get the full path of the file to delete
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

        // Check if the file exists
        if (!FileHandler::exists(resolvedPath)) {
            sendErrorResponse(client, 404, "File not found", config);
            return;
        }

        // Try to delete the file
        bool success;
        if (FileHandler::isDirectory(resolvedPath)) {
            success = FileHandler::deleteDirectory(resolvedPath);
        } else {
            success = FileHandler::deleteFile(resolvedPath);
        }

        if (success) {
            // Send success response
            std::string response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Content-Length: 7\r\n";
            response += "\r\n";
            response += "Deleted";
            send(client->fd, response.c_str(), response.length(), 0);
        } else {
            sendErrorResponse(client, 500, "Failed to delete file", config);
        }
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
    try {
        std::cout << "Processing " << client->request.getMethod() << " request for: " << client->request.getPath() << std::endl;
        
        // Debug delle location disponibili
        std::cout << "DEBUG: Available locations:" << std::endl;
        if (!servers.empty()) {
            const std::vector<LocationConfig>& locations = servers[0]->config.getLocations();
            for (std::vector<LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
                std::cout << " - '" << it->getPath() << "'" << std::endl;
                
                // Per la location /cgi-bin/, mostra gli interpreti
                if (it->getPath() == "/cgi-bin") {
                    const std::map<std::string, std::string>& cgiMap = it->getCgiInterpreters();
                    std::cout << "   CGI interpreters: " << cgiMap.size() << std::endl;
                    for (std::map<std::string, std::string>::const_iterator cgi_it = cgiMap.begin(); cgi_it != cgiMap.end(); ++cgi_it) {
                        std::cout << "   - '" << cgi_it->first << "' -> '" << cgi_it->second << "'" << std::endl;
                    }
                }
            }
        }

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
