/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 17:05:19 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/19 18:08:45 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <poll.h>
#include <sys/types.h>
#include <cstdio>
#include <sstream>

class Server;  // Forward declaration

class Client {
private:
    int fd;
    std::string pending_data;
    bool keep_alive;

public:
    Client() : fd(-1), keep_alive(true) {}
    Client(int client_fd) : fd(client_fd), keep_alive(true) {}
    ~Client() {}

    void handleRequest(Server& server);
    bool send_pending_data();
    bool should_keep_alive() const { return keep_alive; }
    
    // Add these new method declarations
    std::string readData();
    void sendData(const std::string& data);
    void prepare_response(const std::string& content);
};

#endif