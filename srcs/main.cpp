/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:58:41 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/20 18:18:16 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include <iostream>
#include <stdexcept>
#include "ServerConfig.hpp"
#include "Server.hpp" // Include your Server class

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        // Load config file
        ServerConfig config(argv[1]);

        // Initialize and start the server
        Server server(config);
        server.start(); // Start the server
        server.run();   // Enter the event loop
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}