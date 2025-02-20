/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:38:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/19 17:35:27 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include "Request.hpp"          // From HTTP
#include "ServerConfig.hpp"     // From Config

#include <string>
#include <map>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cerrno>

class CGIExecutor {
    public:
        CGIExecutor();
        ~CGIExecutor();

        // Execute a CGI script
        std::string execute(
            const std::string& script_path,
            const std::map<std::string, std::string>& env_vars,
            const std::string& request_body
        );

        void writeToPipe(int pipe_in[2], const std::string& request_body);

    private:
        // Set up environment variables for the CGI script
        void setupEnvironment(const std::map<std::string, std::string>& env_vars);

        // Execute the CGI script and capture its output
        std::string executeScript(const std::string& script_path, const std::string& request_body);

        // Helper function to read from a file descriptor
        std::string readFromFd(int fd);
};

#endif // CGIEXECUTOR_HPP

