/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 13:27:17 by vzashev           #+#    #+#             */
/*   Updated: 2025/04/02 18:07:31 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */





#include "EventLoop.hpp"
#include <poll.h>
#include <cerrno>  // Add this line at the top of the file

// Costruttore
EventLoop::EventLoop(std::vector<Server>& servers) : servers(servers) {
    updatePollFds();
}

// Distruttore
EventLoop::~EventLoop() {}



void EventLoop::run() {
    updatePollFds(); // Initialize poll list

    while (true) {
        int ret = poll(&poll_fds[0], poll_fds.size(), -1);
        if (ret == -1) {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            throw std::runtime_error("Poll failed");
        }

        // Handle events
        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                for (size_t j = 0; j < servers.size(); ++j) {
                    if (poll_fds[i].fd == servers[j].getServerFd()) {
                        servers[j].acceptNewConnection();  // Now matches Server class
                    }
                }
            }
        }
    }
}



void EventLoop::updatePollFds() {
    poll_fds.clear();
    for (size_t i = 0; i < servers.size(); ++i) {
        pollfd pfd;
        pfd.fd = servers[i].getServerFd(); // Ensure this returns valid FD
        pfd.events = POLLIN; // Monitor read events
        pfd.revents = 0;
        poll_fds.push_back(pfd);
        std::cout << "Added server FD " << pfd.fd << " to poll list" << std::endl; // Debug log
    }
}



