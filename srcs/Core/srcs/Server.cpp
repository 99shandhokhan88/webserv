/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/04/03 19:41:17 by vzashev          ###   ########.fr       */
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


/*
void Server::handleStaticRequest(Client* client) {
    // Move the existing file serving logic here from handleClient
    // (from line 145-207 in your Server.cpp)
    char buffer[1024];
    ssize_t bytes_read = recv(client->fd, buffer, sizeof(buffer), 0);
    if (bytes_read <= 0) {
        removeClient(client->fd);
        return;
    }

    // ... rest of the file serving logic
}
*/
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
    char buffer[1024];
    
    ssize_t bytes_read;
    while ((bytes_read = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        client.appendRequestData(buffer, bytes_read);
        
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
            sendErrorResponse(&client, 400);  // Bad Request
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
    // Simple directory listing implementation
    std::vector<std::string> files = FileHandler::listDirectory(path);
    std::string content = "<html><body><h1>Directory Listing</h1><ul>";
    
    for (size_t i = 0; i < files.size(); ++i) {
        content += "<li>" + files[i] + "</li>";
    }
    
    content += "</ul></body></html>";
    sendResponse(client, 200, content);
}
bool Server::isCgiRequest(const LocationConfig& location, const std::string& path) const {
    size_t dot_pos = path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string ext = path.substr(dot_pos);
        return location.getCgiExtensions().count(ext) > 0;
    }
    return false;
}
void Server::handleGetRequest(Client* client) {
    try {
        const LocationConfig& location = config.getLocationForPath(client->request.getPath());
        std::string path = config.getFullPath(client->request.getPath());

        // Debug output
        std::cout << "GET request processing:\n"
                  << "  - Location path: " << location.getPath() << "\n"
                  << "  - Request path: " << client->request.getPath() << "\n"
                  << "  - Resolved path: " << path << std::endl;

        if (FileHandler::isDirectory(path))
        {
           if (!path.empty() && path[path.size() - 1] != '/')
           {
                path += "/";
            }
            path += location.getIndex();
            
            if (!FileHandler::fileExists(path)) {
                throw std::runtime_error("Index file not found in directory");
            }
        }

        if (!FileHandler::fileExists(path)) {
            throw std::runtime_error("File not found: " + path);
        }

        sendFileResponse(client, path);

    } catch (const std::exception& e) {
        std::cerr << "GET request error: " << e.what() << std::endl;
        sendErrorResponse(client, 404);  // Send 404 instead of 500 for missing files
    }
}




void Server::handlePostRequest(Client* client) {
    std::string uploadPath = config.getUploadDir() + client->request.getPath();
    
    // Create directory structure if needed
    std::size_t last_slash = uploadPath.find_last_of('/');
    if (last_slash != std::string::npos) {
        FileHandler::createDirectory(uploadPath.substr(0, last_slash));
    }

    if (FileHandler::writeFile(uploadPath, client->request.getBody())) {
        sendResponse(client, 201, "Resource created");
    } else {
        sendErrorResponse(client, 500);
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

        if (method == "GET") {
            handleGetRequest(client);
        } else if (method == "POST") {
            handlePostRequest(client);
        } else {
            sendErrorResponse(client, 501);
        }
    } catch (const std::exception& e) {
        std::cerr << "Request error: " << e.what() << std::endl;
        sendErrorResponse(client, 400);
    }
}

void Server::acceptNewConnection() {  // Remove parameter
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Use class member server_fd instead of parameter
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

    // Rest of the implementation remains the same...
    if (client_fd == -1) {
        std::cerr << "accept() failed: " << strerror(errno) << std::endl;
        return;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    poll_fds.push_back(client_pollfd);

    clients[client_fd] = Client(client_fd);

    std::cout << "New connection from "
              << inet_ntoa(client_addr.sin_addr)
              << ":" << ntohs(client_addr.sin_port)
              << " (FD: " << client_fd << ")"
              << std::endl;

}


void Server::run() {
    std::cout << "Starting server manager..." << std::endl;
    while (true) {
        int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_count == -1)
            throw std::runtime_error("poll() failed: " + std::string(strerror(errno)));

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (isServerFD(poll_fds[i].fd)) {
                    acceptNewConnection();
                } else {
                    handleClient(poll_fds[i].fd);
                }
            }
        }
    }
}

void Server::removeClient(int client_fd) {
    std::vector<struct pollfd>::iterator it = 
        std::find_if(poll_fds.begin(), poll_fds.end(), PollFDFinder(client_fd));
    
    if (it != poll_fds.end()) {
        poll_fds.erase(it);
        close(client_fd);
        clients.erase(client_fd);
        std::cout << "Closed connection (FD: " << client_fd << ")" << std::endl;
    }
}





int Server::getServerFd() const {
    return server_fd;
}

void Server::sendResponse(Client* client, int status, const std::string& content) {
    std::string response = "HTTP/1.1 " + toString(status) + " OK\r\n";
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

void Server::sendErrorResponse(Client* client, int errorCode) {
    std::string error_path = config.getRoot() + "/errors/" + StringUtils::toString(errorCode) + ".html";
    
    try {
        if (FileHandler::fileExists(error_path)) {
            sendFileResponse(client, error_path);
        } else {
            std::string content = "<h1>" + StringUtils::toString(errorCode) + " Error</h1>";
            sendResponse(client, errorCode, content);
        }
    } catch (const std::exception& e) {
        std::string fallback = "HTTP/1.1 " + StringUtils::toString(errorCode) + " Error\r\n"
                             "Content-Type: text/plain\r\n"
                             "Content-Length: 0\r\n\r\n";
        send(client->fd, fallback.c_str(), fallback.size(), 0);
    }
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