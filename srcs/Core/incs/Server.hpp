/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/15 19:23:23 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <map>
# include <set>
# include <string>
# include <cstring>
# include <sstream>
# include <fstream>
# include <algorithm>
# include <stdexcept>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <unistd.h>
# include <fcntl.h>
# include <poll.h>
# include <errno.h>
# include "../../Config/incs/ServerConfig.hpp"
# include "../../Config/incs/LocationConfig.hpp"

class Client;
class Request;
class Response;

// Function to extract boundary from Content-Type header

class Server {
private:
    // Static members for server management
    static std::vector<Server*>      servers;
    static std::vector<struct pollfd> poll_fds;
    static std::map<int, Client>      clients;

    // Server configuration
    ServerConfig     config;
    int              server_fd;
    struct sockaddr_in address;

    // Private methods for server operations
    void setupSocket();
    
    // Request handlers
    void handleGetRequest(Client* client);
    void handlePostRequest(Client* client);
    void handleDeleteRequest(Client* client);
    void handleOptionsRequest(Client* client);
    
    // File and directory operations
    void sendFileResponse(Client* client, const std::string& path);
    void handleDirectoryListing(Client* client, const std::string& path);
    bool isCgiRequest(const LocationConfig& location, const std::string& path) const;
    void parseMultipartBody(const std::string& body, const std::string& boundary, const std::string& uploadDir);
    // Helpers for poll functionality
    static void addPollFD(int fd, short events);
    static void removePollFD(int fd);
    bool isServerFD(int fd) const;
    
    // Helper for error pages
    std::string getErrorPage(int errorCode) const;

public:
    // Constructor and destructor
    Server(const ServerConfig& config);
    ~Server();
    
    // Getters
    int getServerFd() const;
    const std::vector<struct pollfd>& getPollFds() const;
    
    // Main server operation methods
    static void run();
    static void cleanup();
    
    // Client and request management
    void acceptNewConnection();
    static void handleClient(int client_fd);
    void sendResponse(Client* client, int status, const std::string& content);
    
    // Utility methods
    void setPollEvents(size_t index, short events);
    void handleRequest(const Request& request, Response& response);
    
    // Add these static methods as part of the class declaration
    static void processRequest(Client* client);
    static void removeClient(int client_fd);
    static void sendErrorResponse(Client* client, int statusCode, const std::string& message, const ServerConfig& config);
    static void sendMethodNotAllowedResponse(Client* client, const std::vector<std::string>& allowedMethods);
    static void sendOptionsResponse(Client* client, const std::vector<std::string>& allowedMethods);
    void parseFormUrlEncoded(const std::string& body, std::map<std::string, std::string>& formData);


};


size_t getContentLength(const std::string& headers);

#endif