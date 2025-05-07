
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/19 18:05:17 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Client.hpp"
#include "../incs/Server.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <errno.h>


void Client::parseRequest() {
    request.parse(request_data.c_str(), request_data.size());
}

// Fix initialization order to match declaration
Client::Client(int client_fd) : 
    pending_data(),
    keep_alive(false),
    fd(client_fd),
    request(),
    request_data() {}



    void Client::appendRequestData(const char* data, size_t length) {
        request_data.append(data, length);
    }

    
    bool Client::shouldKeepAlive() const {
        return keep_alive;
    }
    
    void Client::setKeepAlive(bool value) {
        keep_alive = value;
    }

void Client::handleRequest(Server& server) {
    std::string data = readData();

    if (data.empty()) {
        server.removeClient(fd);
        return;
    }

    try {
        // Parse the request from accumulated data
        request.parse(data.c_str(), data.size());

        // Prepare response object
        Response response;

        // Call server logic to handle the request
        server.handleRequest(request, response);

        // Convert the response to a raw HTTP string
        pending_data = response.generate();

    } catch (const std::exception& e) {
        std::cerr << "Request handling error: " << e.what() << "\n";

        // Fallback error response
        Response response;
        response.setStatus(500);
        response.setBody("500 Internal Server Error");
        pending_data = response.generate();
    }

    // Mark this FD as ready to write
    const std::vector<struct pollfd>& pollfds = server.getPollFds();
    for (size_t i = 0; i < pollfds.size(); ++i) {
        if (pollfds[i].fd == fd) {
            server.setPollEvents(i, POLLIN | POLLOUT);
            break;
        }
    }
}


// Modify readData() to ensure complete requests
std::string Client::readData() {
    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        request_data.append(buffer, bytes_read);
        
        // Check for complete headers
        if (request_data.find("\r\n\r\n") != std::string::npos)
        {
                size_t headers_end = request_data.find("\r\n\r\n");
    std::string headers_part = request_data.substr(0, headers_end);

    size_t content_length = 0;
    size_t pos = headers_part.find("Content-Length:");
    if (pos != std::string::npos) {
        std::istringstream iss(headers_part.substr(pos + 15));
        iss >> content_length;
    }

    size_t total_length = headers_end + 4 + content_length;
    if (request_data.size() >= total_length) {
        break;  // Full request including body is received
    }
        }
    }
    
    if (bytes_read == -1 && errno != EAGAIN) {
        throw std::runtime_error("Read error");
    }
    
    return request_data;
}

bool Client::send_pending_data() {
    if (pending_data.empty())
        return true;

    ssize_t bytes_sent = write(fd, pending_data.c_str(), pending_data.size());
    if (bytes_sent == -1) {
        std::cerr << "Write failed: " << strerror(errno) << std::endl;
        return false;
    }

    if (bytes_sent > 0) {
        pending_data = pending_data.substr(bytes_sent);
    }

    return pending_data.empty();
}

void Client::prepare_response(const std::string& content) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    
    // Use stringstream instead of std::to_string for C++98 compatibility
    std::stringstream ss;
    ss << content.length();
    
    response += "Content-Length: " + ss.str() + "\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "\r\n";
    response += content;

    pending_data = response;
}