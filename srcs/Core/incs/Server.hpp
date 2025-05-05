/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 15:43:39 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/05 17:35:10 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "../../Config/incs/ServerConfig.hpp"
#include "../../HTTP/incs/Request.hpp"
#include "../../HTTP/incs/Response.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>


class Server {
private:
    ServerConfig config;
    int server_fd;
    std::string getErrorPage(int errorCode) const;
    struct sockaddr_in address;
    static std::vector<Server*> servers;
    static std::vector<struct pollfd> poll_fds;
    static std::map<int, Client> clients;

    // Private methods
    void handleGetRequest(Client* client);
    void handlePostRequest(Client* client);
    void handleDirectoryListing(Client* client, const std::string& path);
    void sendFileResponse(Client* client, const std::string& path);
    void addPollFD(int fd, short events);
    bool isServerFD(int fd) const;

public:
    Server(const ServerConfig& config);
    ~Server();

    // Core functionality
    bool isCgiRequest(const LocationConfig& location, const std::string& path) const;
    
    void setupSocket();
    void run();
    void acceptNewConnection();
    void handleClient(int client_fd);
    void removeClient(int client_fd);
    void handleDeleteRequest(Client* client);

    // HTTP handling
    void processRequest(Client* client);
    void sendResponse(Client* client, int status, const std::string& content);
    void sendErrorResponse(Client* client, int errorCode);

    // Static cleanup
    static void cleanup();

    // Getters
    int getServerFd() const;
    const ServerConfig& getConfig() const { return config; }

    const std::vector<struct pollfd>& getPollFds() const;
    void setPollEvents(size_t index, short events);
};

#endif