/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/20 19:41:01 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Server.hpp"
#include "../incs/Client.hpp"
#include "../../Config/incs/ServerConfig.hpp"
#include "../../HTTP/incs/Request.hpp"
#include "../../HTTP/incs/Response.hpp"
#include "../../Utils/incs/FileHandler.hpp"

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
    char buffer[1024];
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_read <= 0) {
        removeClient(client_fd);
        return;
    }

    // Parse the HTTP request
    std::string request(buffer, bytes_read);
    std::istringstream iss(request);
    std::string method, path, protocol;
    iss >> method >> path >> protocol;

    // Normalize the request path: remove the leading '/' if present
    std::string normalized_path = path;
    if (!normalized_path.empty() && normalized_path[0] == '/') {
        normalized_path = normalized_path.substr(1);
    }

    // Get the root directory from the config and ensure it ends with '/'
    std::string root = config.getRoot();
    if (root.empty() || root[root.size() - 1] != '/') {
        root += "/";
    }

    // Construct the full file path
    std::string full_path = root + normalized_path;
    if (!path.empty() && path[path.size() - 1] == '/') {
        full_path += config.getIndex();
    }

    std::cout << "Serving file: " << full_path << std::endl;

    std::ifstream file(full_path.c_str());
if (file.good()) {
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    std::cout << "Content size: " << content.size() << std::endl;
    if (content.size() < 100)
        std::cout << "Content: " << content << std::endl;
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Length: " + toString(content.size()) + "\r\n"
                           "Content-Type: text/html\r\n"
                           "Connection: close\r\n\r\n" + content;
    send(client_fd, response.c_str(), response.size(), 0);
}
 else {
        // Serve 404 error page
        std::map<int, std::string>::const_iterator it = config.getErrorPages().find(404);
        std::string error_page;
        if (it != config.getErrorPages().end()) {
            error_page = root + it->second;
        } else {
            error_page = root + "404.html"; // Fallback default
        }
        std::ifstream error_file(error_page.c_str());
        std::string error_content((std::istreambuf_iterator<char>(error_file)),
                                  std::istreambuf_iterator<char>());
        std::string response = "HTTP/1.1 404 Not Found\r\n"
                               "Content-Length: " + toString(error_content.size()) + "\r\n"
                               "Content-Type: text/html\r\n"
                               "Connection: close\r\n\r\n" + error_content;
        send(client_fd, response.c_str(), response.size(), 0);
    }

    removeClient(client_fd);
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

// These definitions are provided here for convenience.
// Ideally, they should be placed in ServerConfig.cpp.
const std::string& ServerConfig::getRoot() const {
    return root;
}

const std::string& ServerConfig::getIndex() const {
    return index;
}
