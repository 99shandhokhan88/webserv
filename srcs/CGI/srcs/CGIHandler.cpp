/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 16:56:13 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/05 16:56:19 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// CGIHandler.cpp
#include "CGIHandler.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <fcntl.h>

std::string CGIHandler::executeCGI(const std::string &scriptPath, const std::string &method,
                                   const std::map<std::string, std::string> &env,
                                   const std::string &body) {
    int inputPipe[2], outputPipe[2];
    pipe(inputPipe);
    pipe(outputPipe);

    pid_t pid = fork();
    if (pid == 0) {
        // CHILD
        dup2(inputPipe[0], STDIN_FILENO);
        dup2(outputPipe[1], STDOUT_FILENO);

        close(inputPipe[1]);
        close(outputPipe[0]);

        // Set environment variables
        for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it)
            setenv(it->first.c_str(), it->second.c_str(), 1);

        setenv("REQUEST_METHOD", method.c_str(), 1);

        execl(scriptPath.c_str(), scriptPath.c_str(), NULL);
        exit(1); // Fail-safe
    }

    // PARENT
    close(inputPipe[0]);
    close(outputPipe[1]);

    write(inputPipe[1], body.c_str(), body.length());
    close(inputPipe[1]);

    char buffer[4096];
    std::ostringstream oss;
    int bytesRead;
    while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0) {
        oss.write(buffer, bytesRead);
    }

    close(outputPipe[0]);
    waitpid(pid, NULL, 0);

    return oss.str();
}
