/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 15:43:39 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/20 18:45:59 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

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

#include "Client.hpp"
#include "../../Config/incs/ServerConfig.hpp"

class Server {
private:
    ServerConfig config;
    int server_fd;
    int port;
    std::vector<pollfd> poll_fds;
    std::map<int, Client> clients;

public:
    Server(const ServerConfig& config);
    Server(int port_number) : port(port_number) {} // Fixed: added curly braces instead of semicolon
    ~Server();

    bool init();  // Move the implementation to the .cpp file

    void start();
    void removeClient(int client_fd);
    void acceptNewConnection();
    void handleClient(int client_fd);
    void setupSocket();
    void run();
     const std::string& getRoot() const;
     const std::string& getIndex() const;
     const std::string& getErrorPage(int code) const;

    int getServerFd() const;
    const ServerConfig& getConfig() const;

    std::vector<pollfd>& getPollFds();
    void setPollEvents(size_t index, short events);
};

#endif