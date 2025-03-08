/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/03/09 00:41:29 by vzashev          ###   ########.fr       */
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


#include <iostream>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <sstream>

template <typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

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

Server::Server(const ServerConfig& config) : config(config), server_fd(-1) {
    setupSocket();
}

Server::~Server() {
    if (server_fd != -1)
        close(server_fd);
}

void Server::setupSocket() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
    }
    std::cout << "Socket created with FD: " << server_fd << std::endl;

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        throw std::runtime_error("setsockopt() failed: " + std::string(strerror(errno)));
    }
    std::cout << "Socket options set" << std::endl;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config.getPort());

    std::cout << "Binding to port " << config.getPort() << std::endl;
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        throw std::runtime_error("bind() failed: " + std::string(strerror(errno)));
    }
    std::cout << "Bound to port " << config.getPort() << std::endl;

    if (listen(server_fd, SOMAXCONN) < 0) {
        close(server_fd);
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
    }
    std::cout << "Listening on port " << config.getPort() << std::endl;

    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    std::cout << "Server FD: " << server_fd << " (port " << config.getPort() << ")" << std::endl;
}

void Server::start() {
    pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    poll_fds.push_back(server_pollfd);

    std::cout << "Server ready on port " << config.getPort() << std::endl;
}

void Server::acceptNewConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
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

void Server::handleClient(int client_fd) {
    Client& client = clients[client_fd];
    char buffer[1024];
    
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_read <= 0) {
        removeClient(client_fd);
        return;
    }

    client.request_data.append(buffer, bytes_read);

    // Check for complete headers
    size_t header_end = client.request_data.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return; // Wait for more data
    }

    try {
        client.request.parse(client.request_data.c_str(), client.request_data.size());
        
        // Validate minimum request requirements
        if (client.request.getMethod().empty() || client.request.getPath().empty()) {
            throw std::runtime_error("Invalid request structure");
        }
    } catch (const std::exception& e) {
        std::cerr << "Request parsing error: " << e.what() << std::endl;
        sendErrorResponse(&client, 400);
        removeClient(client_fd);
        return;
    }

    // Handle keep-alive
    std::map<std::string, std::string> headers = client.request.getHeaders();
    bool keep_alive = headers.count("Connection") && 
                     (headers["Connection"] == "keep-alive" || 
                      headers["Connection"] == "Keep-Alive");
    client.set_keep_alive(keep_alive);

    processRequest(&client);

    // Prepare for next request if keep-alive
    if (client.should_keep_alive()) {
        client.request_data.clear();
        client.request = Request(); // Reset request object
    } else {
        removeClient(client_fd);
    }
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

void Server::handleGetRequest(Client* client) {
    // Basic GET handling example
    std::string path = config.getRoot() + client->request.getPath();
    
    if (FileHandler::isDirectory(path)) {
        handleDirectoryListing(client, path);
    } else if (FileHandler::fileExists(path)) {
        sendFileResponse(client, path);
    } else {
        sendErrorResponse(client, 404);
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
    // Normalize HTTP method to uppercase
    std::string method = client->request.getMethod();
    for (size_t i = 0; i < method.length(); ++i) {
        method[i] = toupper(method[i]);
    }

    try {
        std::cout << "Processing " << method << " request for: " 
                  << client->request.getPath() << std::endl;

        if (method == "GET") {
            handleGetRequest(client);
        } else if (method == "POST") {
            handlePostRequest(client);
        } else {
            std::cerr << "Unsupported method: " << method << std::endl;
            sendErrorResponse(client, 501);
        }
    } catch (const std::exception& e) {
        std::cerr << "Request processing error: " << e.what() << std::endl;
        sendErrorResponse(client, 500);
    }
}


void Server::run() {
    std::cout << "Server entering main loop..." << std::endl;
    while (true) {
        int ret = poll(&poll_fds[0], poll_fds.size(), -1);
        if (ret == -1) {
            std::cerr << "poll() failed: " << strerror(errno) << std::endl;
            continue;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == server_fd) {
                    acceptNewConnection();
                } else {
                    handleClient(poll_fds[i].fd);
                }
            }
        }
    }
}

void Server::removeClient(int client_fd) {
    std::cout << "Closing connection for FD " << client_fd << std::endl;
    for (size_t i = 0; i < poll_fds.size(); ++i) {
        if (poll_fds[i].fd == client_fd) {
            poll_fds.erase(poll_fds.begin() + i);
            break;
        }
    }
    close(client_fd);
    clients.erase(client_fd);
}

std::vector<pollfd>& Server::getPollFds() {
    return poll_fds;
}

void Server::setPollEvents(size_t index, short events) {
    if (index < poll_fds.size()) {
        poll_fds[index].events = events;
    }
}

int Server::getServerFd() const {
    return server_fd;
}

void Server::sendResponse(Client* client, int status, const std::string& content) {
    std::string response = "HTTP/1.1 " + toString(status) + " OK\r\n";
    response += "Content-Length: " + toString(content.size()) + "\r\n";
    
    if (client->should_keep_alive()) {
        response += "Connection: keep-alive\r\n";
        response += "Keep-Alive: timeout=5, max=100\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    response += "\r\n" + content;
    send(client->fd, response.c_str(), response.size(), 0);
}

void Server::sendErrorResponse(Client* client, int errorCode) {
    const std::map<int, std::string>& errorPages = config.getErrorPages();
    std::string content;
    
    try {
        if (errorPages.count(errorCode)) {
            content = FileHandler::readFile(config.getRoot() + errorPages.at(errorCode));
        } else {
            content = "<html><body><h1>" + toString(errorCode) + " Error</h1></body></html>";
        }
    } catch (const std::exception& e) {
        content = "<html><body><h1>Error Generating Error Page</h1></body></html>";
    }

    std::string response = "HTTP/1.1 " + toString(errorCode) + " Error\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + toString(content.size()) + "\r\n";
    
    if (client->should_keep_alive()) {
        response += "Connection: keep-alive\r\n";
    } else {
        response += "Connection: close\r\n";
    }
    
    response += "\r\n" + content;
    send(client->fd, response.c_str(), response.size(), 0);
}