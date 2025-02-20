/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 18:24:36 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/19 22:21:49 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */





#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <vector>
#include <poll.h>
#include "Server.hpp"

class EventLoop {
private:
    std::vector<Server>& servers; // Riferimento a vettore di oggetti Server
    std::vector<pollfd> poll_fds;

    void updatePollFds();

public:
    EventLoop(std::vector<Server>& servers); // Solo costruttore con parametri
    ~EventLoop();

    void run();
};

#endif